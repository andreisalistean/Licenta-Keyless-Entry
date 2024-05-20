#include "esp_log.h"
#include "mbedtls/aes.h"
#include <stdio.h>
#include <string.h>

uint8_t* encrypt_string( uint8_t *input, uint8_t *key, uint8_t *iv , uint8_t *output) 
{
  uint8_t key_copy[16] = {0};
    uint8_t iv_copy[16] = {0};
    

    memcpy(key_copy, key, 16);
    memcpy(iv_copy, iv, 16);
mbedtls_aes_context aes; //aes is simple variable of given type
mbedtls_aes_init(&aes);
mbedtls_aes_setkey_enc(&aes, key_copy, 128);//set key for encryption
//this is cryption function which encrypt data
mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv_copy,
(unsigned char *)input, (unsigned char *)output);
//print output hex buffer on console
//ESP_LOG_BUFFER_HEX("cbc_encrypt", output, strlen(input));
return output;
}

uint8_t* decrypt_string(char *input, uint8_t input_len, const unsigned char *key,  uint8_t *iv, uint8_t* dec)
{
    
    uint8_t key_copy[16] = {0};
    uint8_t iv_copy[16] = {0};
    

    memcpy(key_copy, key, 16);
    memcpy(iv_copy, iv, 16);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key_copy, 128);

    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, input_len, iv_copy, (unsigned char *)input, dec);

    return dec;
}


void setup()
{
  Serial.begin(9600);
}

void loop()
{
    uint8_t key [16]={0x43, 0x52,0x4f,0x41,0x5a,0x49,0x45,0x52,0x41,0x20,0x50,0x45,0x20,0x4e, 0x49,0x4c};
    uint8_t msg [16]={0x4d, 0x41,0x49,0x4e,0x45,0x20,0x50,0x4c,0x45,0x43,0x20,0x41,0x43,0x41, 0x53,0x41};
    uint8_t iv [16]={0x4c,0x45,0x43,0x20,0x41,0x43,0x41, 0x53,0x41, 0x4d, 0x41,0x49,0x4e,0x45,0x20,0x50};
    uint8_t output[16];
    uint8_t dec[16];
    long time=millis();
    encrypt_string(msg,key,iv, output);
    uint8_t* decc = decrypt_string((char *)output,16,key,iv, dec);

    time=millis()-time;
    Serial.println(time);
    for(int i=0;i<16;i++ )
    {
      Serial.print(msg[i]);
    }
Serial.println();
    for(int i=0;i<16;i++ )
    {
      Serial.println(output[i]);
    }
Serial.println();
    
    
    for(int i=0;i<16;i++ )
    {
      Serial.print(decc[i]);
    }
    Serial.println();
    


    while(1);
}