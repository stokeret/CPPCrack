#ifndef RSA_H
#define RSA_H

#include <string>
#include <openssl/rsa.h>

// File operations
bool saveToFile(const std::string& data, const std::string& filepath);
std::string readFromFile(const std::string& filepath);

// RSA key operations
RSA* rsagen(int keyLength = 2048);
bool saveRSAKey(RSA* key, const std::string& filepath, bool isPublic);
RSA* loadRSAKey(const std::string& filepath, bool isPublic);

// Encryption and decryption
std::string encrypt(const std::string& message, RSA* puk);
std::string decrypt(const std::string& encMessage, RSA* prk);

// Mode functions
void encryptMode();
void decryptMode();

#endif // RSA_H