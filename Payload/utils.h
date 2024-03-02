// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <windows.h>

// Constantes
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

// Declaraciones de funciones
std::string base64_decode(const std::string& encoded_string);
std::string base64_encode(const unsigned char* bytes_to_encode, unsigned int in_len);
int CountFilesInDirectory(const std::string& directoryPath);
std::string getCurrentDateTime();
std::string ReadFileContent(const std::string& filePath);
bool SetRegistryValue(HKEY hKeyRoot, LPCSTR subKey, LPCSTR valueName, DWORD dataType, const BYTE* data, DWORD dataSize);
bool GetRegistryValue(HKEY hKeyRoot, LPCSTR subKey, LPCSTR valueName, DWORD dataType, BYTE* data, DWORD* dataSize);
void ExecuteCommandHidden(const char* cmd, std::string& output);

#endif // UTILS_H

