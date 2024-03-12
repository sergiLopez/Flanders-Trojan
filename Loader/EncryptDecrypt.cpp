// Include necessary headers from Crypto++ for AES, modes of operation, file operations, random number generation, and encoding.
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <iostream>
#include <fstream>

// Use the CryptoPP namespace to avoid prefixing CryptoPP:: on every type.
using namespace CryptoPP;

// Encrypts a file with AES in CBC mode.
void EncryptFileAES(const std::string& filename, const std::string& encryptedFilename, const byte key[AES::DEFAULT_KEYLENGTH], const byte iv[AES::BLOCKSIZE]) {
    // Set up AES encryption in CBC mode.
    CBC_Mode<AES>::Encryption encryption;
    // Initialize the encryption object with the key and IV.
    encryption.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv);

    // Create a FileSource to encrypt the file and write the output to a new file.
    FileSource fs(
        filename.c_str(), // Input filename.
        true, // Pump all data through the filter.
        new StreamTransformationFilter(
            encryption, // Encryption transformation.
            new FileSink(encryptedFilename.c_str()) // Output filename.
        )
    );
}

// Decrypts a file with AES in CBC mode.
void DecryptFileAES(const std::string& encryptedFilename, const std::string& decryptedFilename, const byte key[AES::DEFAULT_KEYLENGTH], const byte iv[AES::BLOCKSIZE]) {
    // Set up AES decryption in CBC mode.
    CBC_Mode<AES>::Decryption decryption;
    // Initialize the decryption object with the key and IV.
    decryption.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv);

    // Create a FileSource to decrypt the file and write the output to a new file.
    FileSource fs(
        encryptedFilename.c_str(), // Input filename.
        true, // Pump all data through the filter.
        new StreamTransformationFilter(
            decryption, // Decryption transformation.
            new FileSink(decryptedFilename.c_str()) // Output filename.
        )
    );
}

// Demonstrates the process of encrypting and then decrypting a file, generating a key and IV.
void ProcessEncryptionAES(const std::string& filename, const std::string& encryptedFilename, const std::string& decryptedFilename) {
    AutoSeededRandomPool rnd; // Random number generator for key and IV.

    byte key[AES::DEFAULT_KEYLENGTH]; // Buffer for the AES key.
    rnd.GenerateBlock(key, sizeof(key)); // Generate a random key.

    byte iv[AES::BLOCKSIZE]; // Buffer for the AES IV.
    rnd.GenerateBlock(iv, sizeof(iv)); // Generate a random IV.

    // Encode the key and IV in hexadecimal format for display.
    std::string hexKey, hexIV;
    HexEncoder encoderKey(new StringSink(hexKey));
    encoderKey.Put(key, sizeof(key));
    encoderKey.MessageEnd();

    HexEncoder encoderIV(new StringSink(hexIV));
    encoderIV.Put(iv, sizeof(iv));
    encoderIV.MessageEnd();

    // Output the key and IV to the console.
    std::cout << "Key: " << hexKey << std::endl;
    std::cout << "IV: " << hexIV << std::endl;

    // Encrypt and then decrypt the file.
    EncryptFileAES(filename, encryptedFilename, key, iv);
    DecryptFileAES(encryptedFilename, decryptedFilename, key, iv);
}
