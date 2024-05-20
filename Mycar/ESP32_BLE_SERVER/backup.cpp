#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <DHT11.h>

#define TARGET_MAC_ADDRESS "4a:86:30:fb:83:2d"
#define SERVICE_UUID        "5daea2b4-1912-4839-8c9d-7657f074fa84"
#define CHARACTERISTIC_UUID "df847cb0-69aa-43e1-8aa9-1fad496c60b5"//send
#define CHARACTERISTIC_UUID_2 "f61ad6ec-247b-44e9-98e1-f2a61b9836ea"//receive


/////////////////////////////////////////////Server pointers
BLEScan* pBLEScan;
BLEServer *server;
BLEService *service;
BLECharacteristic *caracteristic;
BLECharacteristic *caracteristicRecive;

/////////////////////////////////////////////////////Global Variables

DHT11 temp(14);
char value[3]={0};

unsigned char deviceAdr[6]={0};
bool deviceConnected = false;
char contorLock=0;
int lastBtn=0;
int btn;
int globalRSSI=-100;

int lastRssi=0; 
int garbageRssi=0;
long int lastAdvertisedUpdate=0;
long int lastEngineAction=0;
bool inRange=true;
bool overRide=false;

bool keylessEnabled=true;
bool EngineState=false;
bool locked=true;
bool lightsOn=false;

/////////////////////////////////////////////////////////////////////////////////////////// Functions

void initializeDevAdr(){
  deviceAdr[0]=0x4a;
  deviceAdr[1]=0x86;
  deviceAdr[2]=0x30;
  deviceAdr[3]=0xfb;
  deviceAdr[4]=0x83;
  deviceAdr[5]=0x2d;
}

///////////////////////////////////////////////////////////////Data setup for delivery

void setByte1()
{
  int rez=((analogRead(13)*1.1)/273)*10;
  value[0]=(char)rez;
}

void setByte3(float tmp)
{
  bool gear=false;
  // int zec=(int)((tmp-(float)(int)tmp)*10);
  // if(zec<0)
  // {
  //   zec=0-zec;
  // }
  value[2]=0|lightsOn<<3 | gear<<4 | EngineState<<5 | locked<<6 | keylessEnabled<<7;
}

void setByte2()
{ 
  float tmp=temp.readTemperature();

  value[1]=((int)tmp+128);

  setByte3(tmp);
}


void setData()
{
  setByte1();
  setByte2();//apeleaza si setByte_3
}

/////////////////////////////////////////////////////////////////Action Functions

void lights_on()
{
  if(!lightsOn)
  {
    lightsOn=true;
    digitalWrite(25,HIGH);
  }
}

void lights_off() 
{
  if(lightsOn)
  {
    lightsOn=false;
    digitalWrite(25,LOW);
  }
}

void lock_key()
{
  locked=true;
  digitalWrite(32,HIGH);
  delay(150);
  digitalWrite(32,LOW);
  lights_off();  
}

void unlock_key()
{
  locked=false;
  digitalWrite(33,HIGH);
  delay(150);
  digitalWrite(33,LOW);
  if(analogRead(26)<1000)
    lights_on();
}

void lock()
{
  lock_key();
  overRide=true;
}

void unlock()
{
  unlock_key();
  overRide=true;
}

void engine()
{
  long int now=millis();
  if((now-lastEngineAction)>4000)
  { 
    if(EngineState) 
    {
      EngineState=false;
      digitalWrite(19,LOW);
      delay(250);
      digitalWrite(18,LOW);
      //keylessEnabled=true;
      lastEngineAction=now;
    }
    else
    {
      EngineState=true;
      digitalWrite(18,HIGH);
      delay(250);
      digitalWrite(19,HIGH);
      delay(1300);
      digitalWrite(21,HIGH);
      delay(1000);
      digitalWrite(21,LOW);
      //keylessEnabled=false;
      lights_on();
      lastEngineAction=now;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////Callbacks

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getAddress().equals(BLEAddress(TARGET_MAC_ADDRESS))) {
      lastAdvertisedUpdate=millis();
      if(keylessEnabled&&!EngineState)
      {
        if(!overRide)
        {
          int rssi=advertisedDevice.getRSSI();
          
          if((rssi<=lastRssi+5)&&(rssi>=lastRssi-5))
          {
            //Serial.println(rssi);
            if(rssi>-94)
            {
              if(locked)
                unlock_key(); 
            }
            if(rssi<-97)
            {
              if(!locked)
                lock_key();
            }
          }
          lastRssi=rssi;
        }
        globalRSSI=lastRssi;
      }
    }
  }
};

class MyCharacteristicCallback:public BLECharacteristicCallbacks
{
void onWrite(BLECharacteristic* data){
      std::string value=data->getValue();
      if(value.length()>0)
      {
        //Serial.print("Data from the client: ");
        //Serial.println(value[0]);

        if(value[0]=='A')
        {
          if(lightsOn)
            lights_off();
          else
            lights_on();
        }
        else if(value[0]=='B')
        {
          lock();
        } if(value[0]=='C')
        {
          unlock();
        }
        else if(value[0]=='D')
        {
          engine();
        }
        else if(value[0]=='E')
        {
          keylessEnabled=true;
        }
        else if(value[0]=='F')
        {
          keylessEnabled=false;
        }
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
      
      for(char i=0;i<6;++i)
      {
        if(param->connect.remote_bda[i]!=deviceAdr[i])
        {
          server->disconnect(server->getConnId());
          //Serial.println("caca , nu e voie !!");
          return;
        }
      }
      deviceConnected = true;
    }; 

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      overRide=false;
      globalRSSI=-100;
      BLEDevice::startAdvertising();
    } 

    
};

//////////////////////////////////////////////////////////////////////

void startServer()
{
  BLEDevice::init("ESP32");

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

  server=BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());
  
  service=server->createService(BLEUUID(SERVICE_UUID) , 32);
  caracteristic = service->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ |
                  BLECharacteristic::PROPERTY_WRITE|
                  BLECharacteristic::PROPERTY_NOTIFY |
                  BLECharacteristic::PROPERTY_INDICATE);
  caracteristic->addDescriptor(new BLE2902());
  delay(2000);
  caracteristicRecive=service->createCharacteristic(CHARACTERISTIC_UUID_2 , BLECharacteristic::PROPERTY_READ |
                  BLECharacteristic::PROPERTY_WRITE|
                  BLECharacteristic::PROPERTY_NOTIFY |
                  BLECharacteristic::PROPERTY_INDICATE);
  BLE2902 * desc= new BLE2902();
  desc->setNotifications(true);
  caracteristicRecive->addDescriptor(desc); 
  caracteristicRecive->setCallbacks(new MyCharacteristicCallback());

  service->start();

  BLEDevice::startAdvertising();
}

/////////////////////////////////////////////////////////////Interrupt

//void  startEng_interrupt()//IRAM_ATTR stocheaza fct in ram pt executie mai rapida
//{
  //engine();
//}

void setup() {
  //Serial.begin(9600);
  pinMode(33,OUTPUT);//unlock +++++++
  pinMode(32,OUTPUT); // lock     +++++++

  pinMode(18,OUTPUT);//accesorii  ++++++++
  pinMode(19,OUTPUT);//contact  ++++++++++
  pinMode(21,OUTPUT);//ignition +++++++++++

  pinMode(25,OUTPUT);//lights +++++++
  //gpio 26 analog input light sensor ++++++
  
  //pinMode(23,INPUT);// button start  Add fizical pullup

  //pinMode(22,INPUT_PULLUP);//safety switch gear  

  //gpio 13 analog input for voltage meter
  //gpio 14 temp  +++++

  initializeDevAdr();
  startServer();
}

void loop() {
  //Serial.println(analogRead(26));
  // btn=digitalRead(23);
  // if(btn!=lastBtn&&btn<=100)
  // {
  //   engine();
  //   if(locked&&EngineState)
  //   {
  //     lights_off();
  //   }
  // }
  // lastBtn=btn;
  pBLEScan->start(1, false);

  if (deviceConnected) {
    setData();
    caracteristic->setValue((uint8_t *)&value,3);
    caracteristic->notify(true);
  }
  
  pBLEScan->stop();
  pBLEScan->clearResults();
  if((millis()-lastAdvertisedUpdate)>7000)
    if(!locked)
      lock_key();
      globalRSSI=-100;
}
