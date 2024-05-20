#ifndef CRYPTO_AES_h
 #define CRYPTO_AES_h
  
 #include "BlockCipher.h"

 class AESTiny128;
 class AESTiny256;
 class AESSmall128;
 class AESSmall256;
  
 class AESCommon : public BlockCipher
 {
 public:
     virtual ~AESCommon();
  
     size_t blockSize() const;
  
     void encryptBlock(uint8_t *output, const uint8_t *input);
     void decryptBlock(uint8_t *output, const uint8_t *input);
  
     void clear();
  
 protected:
     AESCommon();
  
     uint8_t rounds;
     uint8_t *schedule;
  
     static void subBytesAndShiftRows(uint8_t *output, const uint8_t *input);
     static void inverseShiftRowsAndSubBytes(uint8_t *output, const uint8_t *input);
     static void mixColumn(uint8_t *output, uint8_t *input);
     static void inverseMixColumn(uint8_t *output, const uint8_t *input);
     static void keyScheduleCore(uint8_t *output, const uint8_t *input, uint8_t iteration);
     static void applySbox(uint8_t *output, const uint8_t *input);
     friend class AESTiny128;
     friend class AESTiny256;
     friend class AESSmall128;
     friend class AESSmall256;
 };
  
 class AES128 : public AESCommon
 {
 public:
     AES128();
     virtual ~AES128();
  
     size_t keySize() const;
  
     bool setKey(const uint8_t *key, size_t len);
  
 private:
     uint8_t sched[176];
 };
  
 class AES192 : public AESCommon
 {
 public:
     AES192();
     virtual ~AES192();
  
     size_t keySize() const;
  
     bool setKey(const uint8_t *key, size_t len);
  
 private:
     uint8_t sched[208];
 };
  
 class AES256 : public AESCommon
 {
 public:
     AES256();
     virtual ~AES256();
  
     size_t keySize() const;
  
     bool setKey(const uint8_t *key, size_t len);
  
 private:
     uint8_t sched[240];
 };
  
 class AESTiny256 : public BlockCipher
 {
 public:
     AESTiny256();
     virtual ~AESTiny256();
  
     size_t blockSize() const;
     size_t keySize() const;
  
     bool setKey(const uint8_t *key, size_t len);
  
     void encryptBlock(uint8_t *output, const uint8_t *input);
     void decryptBlock(uint8_t *output, const uint8_t *input);
  
     void clear();
  
 private:
     uint8_t schedule[32];
 };
  
 class AESSmall256 : public AESTiny256
 {
 public:
     AESSmall256();
     virtual ~AESSmall256();
  
     bool setKey(const uint8_t *key, size_t len);
  
     void decryptBlock(uint8_t *output, const uint8_t *input);
  
     void clear();
  
 private:
     uint8_t reverse[32];
 };
  
 class AESTiny128 : public BlockCipher
 {
 public:
     AESTiny128();
     virtual ~AESTiny128();
  
     size_t blockSize() const;
     size_t keySize() const;
  
     bool setKey(const uint8_t *key, size_t len);
  
     void encryptBlock(uint8_t *output, const uint8_t *input);
     void decryptBlock(uint8_t *output, const uint8_t *input);
  
     void clear();
  
 private:
     uint8_t schedule[16];
 };
  
 class AESSmall128 : public AESTiny128
 {
 public:
     AESSmall128();
     virtual ~AESSmall128();
  
     bool setKey(const uint8_t *key, size_t len);
  
     void decryptBlock(uint8_t *output, const uint8_t *input);
  
     void clear();
  
 private:
     uint8_t reverse[16];
 };

 #endif