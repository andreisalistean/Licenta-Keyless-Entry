#include <DHT11.h>

DHT11 temp(16);
uint8_t data[16];
uint8_t status[3];
uint8_t token[4]={0x76,0x09,0x0e,0x23};
int btn=0;
int lastBtn=0;
uint8_t RSSI=100;
uint8_t counter=0;

bool engineIsOn=false;
bool lockOverride=false;
bool deviceConnected=false;
bool locked=false;
bool firstLockSent=false;
bool keylessEnabled=true;
bool lightsOn=false;
bool gear=false;

void lights_on(){
  lightsOn=true;
  digitalWrite(12,HIGH);
}

void lights_off(){
  lightsOn=false;
  digitalWrite(12,LOW);
}

void engine()
{
  if(engineIsOn)
  {
    //digitalWrite(20,HIGH);
    engineIsOn=false;
    lockOverride=false;
    digitalWrite(8,LOW);
    delay(300);
    digitalWrite(6,LOW);
  }
  else{
        //digitalWrite(20,LOW);
    engineIsOn=true;
    digitalWrite(6,HIGH);
    delay(250);
    digitalWrite(8,HIGH);
    delay(1300);
    digitalWrite(10,HIGH);
    delay(1000);
    digitalWrite(10,LOW);
    lights_on();
  }
}


void lights(){
  if(lightsOn) lights_off();
  else lights_on();
}
void lock(){
  locked=true;
  digitalWrite(18,HIGH);
  delay(200);
  digitalWrite(18,LOW);
  if(!engineIsOn) lights_off(); 
}

void unlock(){
  locked=false;
  digitalWrite(20,HIGH);
  delay(200);
  digitalWrite(20,LOW);
  if(analogRead(A0)<500)  
    lights_on();
}

bool checkToken()
{
  for(short i=0;i<4;i++)
    if(data[i+1]!=token[i]) return false;
  return true;
}

void handleAction()
{
  if(data[6]==0x11)
  {
    //digitalWrite(18,HIGH);
    RSSI=data[8];
    if(RSSI>84)
    {
      if(firstLockSent)
        if(!locked&&!lockOverride&&keylessEnabled) lock();
      firstLockSent=true;
    }
    else if (RSSI<78) 
    {
      if(locked&&!lockOverride&&keylessEnabled) unlock();
      firstLockSent=false;
    }
  }
  else if (data[6]==0x78)
  {
    // digitalWrite(18,HIGH);
    // delay(300);
    // digitalWrite(18,LOW);
    switch(data[7]){
      case 0xAA: lights(); break;
      case 0xFC: lock(); lockOverride=true; break;
      case 0x49: unlock(); lockOverride=true; break;
      case 0x1B: engine(); break; 
      case 0xC7: keylessEnabled=true; break;
      case 0x9D: keylessEnabled=false; break;
      default:break;
    }
  }
}

void handlerServerMessage()
{
  if(data[1]==0xF0)// disconnect
  {
    deviceConnected=false;
    lockOverride=false;
    RSSI=100;
    counter=0;
    if(!locked)
      lock();
  }
  else if (data[1]==0x5A)//connect
    deviceConnected=true;
}

void setData(){
  status[0]=(analogRead(A3)*25)/1023;
  //status[1]=((int)temp.readTemperature()+128);
  status[2]=0|lightsOn<<3 | gear<<4 | engineIsOn<<5 | locked<<6 | keylessEnabled<<7;
}

void setup()
{
  Serial.begin(115200);
  pinMode(6,OUTPUT);//acc
  pinMode(8,OUTPUT);//con
  pinMode(10,OUTPUT);//start
  pinMode(18,OUTPUT);//lock
  pinMode(20,OUTPUT);//unlock
  pinMode(12,OUTPUT);//lights
  pinMode(14,INPUT);//btnStart
}

void loop()
{
  btn=digitalRead(14);
  if(btn==HIGH&&btn!=lastBtn&&RSSI<69&&deviceConnected)
  {
    delay(500);
    engine();
  }
  lastBtn=btn;
  if(Serial.available()>15)
  {
    setData();
    for(short i=0;i<3;i++)  
      Serial.write(status[i]);
    for(short i=0;i<16;i++)
    {
      data[i]=Serial.read();
    }
    if(data[0]==0x21)//phone data
    {
      //digitalWrite(12,HIGH);
      if(checkToken())
      {//digitalWrite(20,HIGH);
        handleAction();
      }
      
    }else if(data[0]==0x96)
    {
      handlerServerMessage();
    }
    
  }
}