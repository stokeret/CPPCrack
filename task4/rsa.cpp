#include "rsa.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <openssl/pem.h>
#include <openssl/err.h>

// Function to save data to a file
bool saveToFile(const std::string& data, const std::string& filepath) {
    std::filesystem::path path(filepath);
    if (!std::filesystem::exists(path.parent_path())) {
        std::cerr << "Directory does not exist: " << path.parent_path() << std::endl;
        return false;
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (file.is_open()) {
        file.write(data.c_str(), data.length());
        file.close();
        std::cout << "Data saved to " << filepath << std::endl;
        return true;
    } else {
        std::cerr << "Unable to open file: " << filepath << std::endl;
        return false;
    }
}

// Function to read data from a file
std::string readFromFile(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "File does not exist: " << filepath << std::endl;
        return "";
    }
    
    std::ifstream file(filepath, std::ios::binary);
    std::string data;
    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        data.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&data[0], data.size());
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filepath << std::endl;
    }
    return data;
}

// Function used for RSA key pair generation
RSA* rsagen(int keyLength) {
    RSA* rsa = RSA_new();
    BIGNUM* bne = BN_new();
    BN_set_word(bne, RSA_F4);
    RSA_generate_key_ex(rsa, keyLength, bne, nullptr);
    BN_free(bne);
    return rsa;
}

// Function to save RSA key to a file
bool saveRSAKey(RSA* key, const std::string& filepath, bool isPublic) {
    std::filesystem::path path(filepath);
    if (!std::filesystem::exists(path.parent_path())) {
        std::cerr << "Directory does not exist: " << path.parent_path() << std::endl;
        return false;
    }
    
    FILE* file = fopen(filepath.c_str(), "wb");
    if (file) {
        int result;
        if (isPublic) {
            result = PEM_write_RSAPublicKey(file, key);
        } else {
            result = PEM_write_RSAPrivateKey(file, key, nullptr, nullptr, 0, nullptr, nullptr);
        }
        fclose(file);
        if (result == 1) {
            std::cout << (isPublic ? "Public" : "Private") << " key saved to " << filepath << std::endl;
            return true;
        } else {
            std::cerr << "Failed to write " << (isPublic ? "public" : "private") << " key to file" << std::endl;
            return false;
        }
    } else {
        std::cerr << "Unable to open file: " << filepath << std::endl;
        return false;
    }
}

// Function to load RSA key from a file
RSA* loadRSAKey(const std::string& filepath, bool isPublic) {
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "File does not exist: " << filepath << std::endl;
        return nullptr;
    }
    
    FILE* file = fopen(filepath.c_str(), "rb");
    RSA* key = nullptr;
    if (file) {
        if (isPublic) {
            key = PEM_read_RSAPublicKey(file, nullptr, nullptr, nullptr);
        } else {
            key = PEM_read_RSAPrivateKey(file, nullptr, nullptr, nullptr);
        }
        fclose(file);
        if (!key) {
            std::cerr << "Failed to read " << (isPublic ? "public" : "private") << " key from file" << std::endl;
        }
    } else {
        std::cerr << "Unable to open file: " << filepath << std::endl;
    }
    return key;
}

// Function used for the encryption of a user inputted message using the RSA public key (puk)
std::string encrypt(const std::string& message, RSA* puk) {
    int rsaSize = RSA_size(puk);
    std::vector<unsigned char> encrypted(rsaSize);

    int encL = RSA_public_encrypt(
        static_cast<int>(message.length()),
        reinterpret_cast<const unsigned char*>(message.c_str()),
        encrypted.data(),
        puk,
        RSA_PKCS1_PADDING
    );

    if (encL == -1) {
        std::cerr << "Encryption failed" << std::endl;
        return "";
    }

    return std::string(reinterpret_cast<char*>(encrypted.data()), encL);
}

// Function used for the decryption of the message using the RSA private key (prk)
std::string decrypt(const std::string& encMessage, RSA* prk) {
    int rsaSize = RSA_size(prk);
    std::vector<unsigned char> decrypted(rsaSize);

    int decL = RSA_private_decrypt(
        static_cast<int>(encMessage.length()),
        reinterpret_cast<const unsigned char*>(encMessage.c_str()),
        decrypted.data(),
        prk,
        RSA_PKCS1_PADDING
    );

    if (decL == -1) {
        std::cerr << "Decryption failed" << std::endl;
        return "";
    }

    return std::string(reinterpret_cast<char*>(decrypted.data()), decL);
}

void encryptMode() {
    RSA* rsaKP = rsagen();

    std::string plaintext;
    std::cout << "Enter message to encrypt: ";
    std::getline(std::cin, plaintext);

    std::string encMessage = encrypt(plaintext, rsaKP);
    if (!encMessage.empty()) {
        std::string encFilepath;
        std::cout << "Enter filepath to save encrypted data: ";
        std::getline(std::cin, encFilepath);
        saveToFile(encMessage, encFilepath);

        std::string pubKeyFilepath;
        std::cout << "Enter filepath to save public key: ";
        std::getline(std::cin, pubKeyFilepath);
        if (saveRSAKey(rsaKP, pubKeyFilepath, true)) {
            std::cout << "Public key saved successfully." << std::endl;
        } else {
            std::cerr << "Failed to save public key." << std::endl;
        }

        std::string privKeyFilepath;
        std::cout << "Enter filepath to save private key: ";
        std::getline(std::cin, privKeyFilepath);
        if (saveRSAKey(rsaKP, privKeyFilepath, false)) {
            std::cout << "Private key saved successfully." << std::endl;
        } else {
            std::cerr << "Failed to save private key." << std::endl;
        }
    }

    RSA_free(rsaKP);
}

void decryptMode() {
    std::string privKeyFilepath, encFilepath;
    std::cout << "Enter filepath of private key: ";
    std::getline(std::cin, privKeyFilepath);
    
    RSA* prk = loadRSAKey(privKeyFilepath, false);
    if (prk) {
        std::cout << "Enter filepath of encrypted data: ";
        std::getline(std::cin, encFilepath);
        
        std::string encMessage = readFromFile(encFilepath);
        if (!encMessage.empty()) {
            std::string decMessage = decrypt(encMessage, prk);
            if (!decMessage.empty()) {
                std::cout << "Decrypted message: " << decMessage << std::endl;
            }
        }
        RSA_free(prk);
    } else {
        std::cerr << "Failed to load private key." << std::endl;
    }
}

int main() {
    OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS, NULL);

    int choice;
    do {
        std::cout << "Choose operation:\n1. Encrypt\n2. Decrypt\n3. Exit\nEnter choice: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                encryptMode();
                break;
            case 2:
                decryptMode();
                break;
            case 3:
                std::cout << "Exiting program." << std::endl;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    } while (choice != 3);

    return 0;
}