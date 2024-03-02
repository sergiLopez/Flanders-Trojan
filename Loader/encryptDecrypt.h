// EncryptionUtils.h
#ifndef ENCRYPTION_UTILS_H
#define ENCRYPTION_UTILS_H

#include <cryptopp/aes.h>
#include <string>
#include <vector>

using namespace CryptoPP;

void XORProcess(const byte* input, byte* output, size_t size, const byte* key, size_t keySize);
void EncryptFileXOR(const std::string& filename, const std::string& encryptedFilename, const byte* key, size_t keySize);
void DecryptFileXOR(const std::string& encryptedFilename, const std::string& decryptedFilename, const byte* key, size_t keySize);

void EncryptFileAES(const std::string& filename, const std::string& encryptedFilename, const byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], const byte iv[CryptoPP::AES::BLOCKSIZE]);
void DecryptFileAES(const std::string& encryptedFilename, const std::string& decryptedFilename, const byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], const byte iv[CryptoPP::AES::BLOCKSIZE]);

void ProcessEncryptionAES(const std::string& filename, const std::string& encryptedFilename, const std::string& decryptedFilename);

#endif 
