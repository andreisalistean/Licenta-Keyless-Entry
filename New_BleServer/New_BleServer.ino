#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "mbedtls/aes.h"
#include <stdio.h>
#include<string>
#include "esp_log.h"
#include "sdkconfig.h"
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gattc_api.h>
#include <esp_gatt_common_api.h>// ESP32 BLE
#include "BLEClient.h"
#include "BLEService.h"
#include "GeneralUtils.h"
#include <sstream>
#include <unordered_set>
#include "BLEDevice.h"
#include "esp32-hal-log.h"
//#include <FreeRTOS.h>
//#include <semphr.h>

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


SemaphoreHandle_t serialMutex;
uint8_t output[16];

//////////////////////////////////////////////////////Crypting

uint8_t key [16]={0x43, 0x52,0x4f,0x41,0x5a,0x49,0x45,0x52,0x41,0x20,0x50,0x45,0x20,0x4e, 0x49,0x4c};
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

/////////////////////////////////////////////////////Global Variable

char value[3]={0};

static void my_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  if(xSemaphoreTake(serialMutex, portMAX_DELAY))
    {
      output[8]=(byte)(-param->read_rssi_cmpl.rssi);

      for(short i=0;i<16;i++)
      {
        Serial.write(output[i]);
        output[i]=0x00;
      }
        delay(200);
      if(Serial.available()>0)
      {
        {
          for(short i=0;i<3;i++)  
          {
            value[i]=Serial.read(); 
          }
        }
      }
      caracteristic->setValue((uint8_t *)&value,3);
      caracteristic->notify(true);
      
      xSemaphoreGive(serialMutex);
    }  
}

class MyCharacteristicCallback:public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic* data,esp_ble_gatts_cb_param_t* param){
    decrypt(data->getData(),key,iv,output);  
    ::esp_ble_gap_read_rssi(param->read.bda);
  }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
      if(xSemaphoreTake(serialMutex, portMAX_DELAY))
      {
        output[0]=0x96;
        output[1]=0x5A;
        for(short i=0;i<16;i++)
          Serial.write(output[i]);
        xSemaphoreGive(serialMutex);
      }  
    }; 

    void onDisconnect(BLEServer* pServer) {
     if(xSemaphoreTake(serialMutex, portMAX_DELAY))
      {
        output[0]=0x96;
        output[1]=0xF0; 
        for(short i=0;i<16;i++)
          Serial.write(output[i]);
        xSemaphoreGive(serialMutex);
      }  
      BLEDevice::startAdvertising();
    } 

    
};

void startServer()
{
  BLEDevice::init("ESP32");
  BLEDevice::setCustomGapHandler(my_gap_event_handler);
  
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
  serialMutex=xSemaphoreCreateMutex();
  Serial.begin(115200);
  startServer();
}

void loop(){}

