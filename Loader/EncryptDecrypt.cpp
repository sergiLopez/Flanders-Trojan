#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <iostream>
#include <fstream>

using namespace CryptoPP;



void EncryptFileAES(const std::string& filename, const std::string& encryptedFilename, const byte key[AES::DEFAULT_KEYLENGTH], const byte iv[AES::BLOCKSIZE]) {
    
    CBC_Mode<AES>::Encryption encryption;

    encryption.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv);

    FileSource fs(
        filename.c_str(),
        true,
        new StreamTransformationFilter(
            encryption,
            new FileSink(encryptedFilename.c_str())
        )
    );

}



void DecryptFileAES(const std::string& encryptedFilename, const std::string& decryptedFilename, const byte key[AES::DEFAULT_KEYLENGTH], const byte iv[AES::BLOCKSIZE]) {
    CBC_Mode<AES>::Decryption decryption;
    decryption.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv);

    FileSource fs(
        encryptedFilename.c_str(),
        true,
        new StreamTransformationFilter(
            decryption,
            new FileSink(decryptedFilename.c_str())
        )
    );
}



void ProcessEncryptionAES(const std::string& filename, const std::string& encryptedFilename, const std::string& decryptedFilename) {

    AutoSeededRandomPool rnd;

    byte key[AES::DEFAULT_KEYLENGTH];
    rnd.GenerateBlock(key, sizeof(key));

    byte iv[AES::BLOCKSIZE];
    rnd.GenerateBlock(iv, sizeof(iv));

    std::string hexKey, hexIV;
    HexEncoder encoderKey(new StringSink(hexKey));
    encoderKey.Put(key, sizeof(key));
    encoderKey.MessageEnd();

    HexEncoder encoderIV(new StringSink(hexIV));
    encoderIV.Put(iv, sizeof(iv));
    encoderIV.MessageEnd();

    std::cout << "Key: " << hexKey << std::endl;
    std::cout << "IV: " << hexIV << std::endl;

    EncryptFileAES(filename, encryptedFilename, key, iv);
    DecryptFileAES(encryptedFilename, decryptedFilename, key, iv);


}
