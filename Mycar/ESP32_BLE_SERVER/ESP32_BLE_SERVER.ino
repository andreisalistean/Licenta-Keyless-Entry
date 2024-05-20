#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <DHT11.h>
#include "esp_log.h"
#include "mbedtls/aes.h"
#include <stdio.h>

#define TARGET_MAC_ADDRESS "4a:86:30:fb:83:2d"
#define SERVICE_UUID        "5daea2b4-1912-4839-8c9d-7657f074fa84"
#define CHARACTERISTIC_UUID "df847cb0-69aa-43e1-8aa9-1fad496c60b5"//send
#define CHARACTERISTIC_UUID_2 "f61ad6ec-247b-44e9-98e1-f2a61b9836ea"//receive


/////////////////////////////////////////////Server pointers
BLEServer *server;
BLEService *service;
BLECharacteristic *caracteristic;
BLECharacteristic *caracteristicRecive;

//////////////////////////////////////////////////////Crypting

uint8_t key [16]={0x43, 0x52,0x4f,0x41,0x7a,0x49,0x45,0x52,0x21,0x20,0x50,0x15,0xa0,0x4e, 0x4b,0x4c};
uint8_t iv [16]={0x4c,0x45,0x43,0x20,0x41,0x43,0x41, 0x53,0x41, 0x4d, 0x41,0x49,0x4e,0x45,0x20,0x50};

uint8_t* encrypt( uint8_t *input, uint8_t *key, uint8_t *iv , uint8_t *output) 
{
  uint8_t key_copy[16] = {0};
  uint8_t iv_copy[16] = {0}; 

  memcpy(key_copy, key, 16);
  memcpy(iv_copy, iv, 16);

  mbedtls_aes_context aes; 
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key_copy, 128);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv_copy,(unsigned char *)input, output);

return output;
}

uint8_t* decrypt(uint8_t*input,  const unsigned char *key,  uint8_t *iv, uint8_t* dec)
{
    
    uint8_t key_copy[16] = {0};
    uint8_t iv_copy[16] = {0};
    

    memcpy(key_copy, key, 16);
    memcpy(iv_copy, iv, 16);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key_copy, 128);

    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16, iv_copy, (unsigned char *)input, dec);

    return dec;
}

/////////////////////////////////////////////////////Global Variables

DHT11 temp(14);
char value[3]={0};
int RSSI=100;


bool inRange=true;
bool overRide=false;
bool deviceConnected=false;
bool keylessEnabled=true;
bool engineIsOn=false;
bool ovverride=false;
bool locked=true;
bool lightsOn=false;

byte TOKEN[4];

///////////////////////////////////////////////////////////////Data setup for delivery

void setByte1()
{
  int rez=((analogRead(13)*1.1)/273)*10;
  value[0]=(char)rez;
}

void setByte3(float tmp)
{
  bool gear=false;
  
  value[2]=0|lightsOn<<3 | gear<<4 | engineIsOn<<5 | locked<<6 | keylessEnabled<<7;
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
void lights_off()
{
  lightsOn=false;
    digitalWrite(25,LOW);
}

void lights_on()
{
  lightsOn=true;
    digitalWrite(25,HIGH);
}
void lights() 
{
  if(lightsOn)
  {
    lights_off();
  }
  else {
    lights_on();
  }
}



void lock()
{
  locked=true;
  digitalWrite(32,HIGH);
  delay(150);
  digitalWrite(32,LOW);
  lights_off();  
}

void unlock()
{
  locked=false;
  digitalWrite(33,HIGH);
  delay(150);
  digitalWrite(33,LOW);
  if(analogRead(26)<1000)
    lights_on();
}



void engine()
{
  if(engineIsOn) 
  {
    digitalWrite(19,LOW);
    delay(300);
    digitalWrite(18,LOW);
    engineIsOn=false;
    ovverride=false;
  }
  else
  {
    engineIsOn=true;
    digitalWrite(18,HIGH);
    delay(250);
    digitalWrite(19,HIGH);
    delay(1300);
    digitalWrite(21,HIGH);
    delay(1000);
    digitalWrite(21,LOW);
    lights_on();
  }  
  
}

void sendToken()
{
  long token = random(0,2147483640);
    uint8_t package[16];
    TOKEN[0]=token>>0;
    TOKEN[1]=token>>8;
    TOKEN[2]=token>>16;
    TOKEN[4]=token>>24;


    package[0]=0x55;
    for(short i =0;i<4;i++)
    {
      package[i+1]=TOKEN[i];
    }

    for(short i =5;i<16;i++)
    {
      package[i]=0xFF;
    }
    uint8_t output[16];
    uint8_t* rez=encrypt(package,key,iv,output);
    caracteristic->setValue(rez,16);
    caracteristic->notify(true);
    
}

void handleAction(uint8_t value)
{
  switch(value){
    case 0xAA: lights(); break;
    case 0xFC: lock(); ovverride=true; break;
    case 0x49: unlock(); ovverride=true; break;
    case 0x1B: engine(); break; 
    case 0xC7: keylessEnabled=true; break;
    case 0x9D: keylessEnabled=false; break;

    default:break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////Callbacks

class MyCharacteristicCallback:public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic* data){
    uint8_t output[16];

    uint8_t* rez=decrypt(data->getData(),key,iv,output);

    for(short i=0;i<4;i++)
    {
      if(TOKEN[i]!=rez[i])
        return;
    }
    if(rez[5]==0x47)//ACTION
    {
      handleAction(rez[6]);//for lock/unlock put ovverride manualy when aclling the acction
    }
    else if(rez[5]==0xA4)//RSSI
    {
       RSSI=rez[6];
       if(!engineIsOn&&!ovverride)
       {
        if(RSSI<95)//UNLOCK
        {
          if(locked) 
            unlock();
        }
        else if(RSSI>96)//LOCK
        {
            if(!locked)
              lock();
        }
       }
    }

  }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
      sendToken();
      deviceConnected = true;
    }; 

    void onDisconnect(BLEServer* pServer) {
      for(short i=0;i<4;i++)
      {
        TOKEN[i]=0x00;
      }
      deviceConnected = false;
      overRide=false;
      RSSI=100;
      BLEDevice::startAdvertising();
    } 

    
};

//////////////////////////////////////////////////////////////////////

void startServer()
{
  BLEDevice::init("ESP32");

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

void setup() {
  //Serial.begin(9600);
  pinMode(33,OUTPUT);//unlock +++++++
  pinMode(32,OUTPUT); // lock     +++++++

  pinMode(18,OUTPUT);//accesorii  ++++++++
  pinMode(19,OUTPUT);//contact  ++++++++++
  pinMode(21,OUTPUT);//ignition +++++++++++

  pinMode(25,OUTPUT);//lights +++++++
  //gpio 26 analog input light sensor ++++++
  
  pinMode(23,INPUT);// button start  Add fizical pullup

  //pinMode(22,INPUT_PULLUP);//safety switch gear  

  //gpio 13 analog input for voltage meter
  //gpio 14 temp  +++++

  startServer();
}

void loop() {

  if (deviceConnected) {
    setData();
    caracteristic->setValue((uint8_t *)&value,3);
    caracteristic->notify(true);
  }
}
