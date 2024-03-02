#include "pch.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <ctime>
#include <vector>
#include <regex>
#include <setjmp.h>
#include <jpeglib.h>
#include <random>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iomanip>

#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>

#include <nlohmann/json.hpp>
#include <fmt/core.h>

#define _CRT_SECURE_NO_WARNINGS
using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace CryptoPP;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "comctl32.lib")

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";



std::string base64_decode(const std::string& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/')) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}


std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;
}

int CountFilesInDirectory(const std::string& directoryPath) {
    using std::filesystem::directory_iterator;
    return std::distance(directory_iterator(directoryPath), directory_iterator{});
}


std::string getCurrentDateTime() {
   
    auto now = std::chrono::system_clock::now();


    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = {};
    localtime_s(&tm, &time); 

   
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M");

  
    return ss.str();
}

std::string ReadFileContent(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;

    if (file) {
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    }
    else {
        std::cerr << "No se pudo abrir el archivo: " << filePath << std::endl;
        return "";
    }
}

bool SetRegistryValue(HKEY hKeyRoot, LPCSTR subKey, LPCSTR valueName, DWORD dataType, const BYTE* data, DWORD dataSize) {
    HKEY hKey;
    if (RegCreateKeyExA(hKeyRoot, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return false;
    }

    if (RegSetValueExA(hKey, valueName, 0, dataType, data, dataSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool GetRegistryValue(HKEY hKeyRoot, LPCSTR subKey, LPCSTR valueName, DWORD dataType, BYTE* data, DWORD* dataSize) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    if (RegQueryValueExA(hKey, valueName, NULL, &dataType, data, dataSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

void ExecuteCommandHidden(const char* cmd, std::string& output) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    HANDLE hRead, hWrite;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;  
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        std::cerr << "CreatePipe failed\n";
        return;
    }
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWrite;  
    si.hStdError = hWrite;   

    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")\n";
        CloseHandle(hWrite);
        CloseHandle(hRead);
        return;
    }

  
    CloseHandle(hWrite);

    Sleep(2000);  

    CHAR buffer[4096];
    DWORD bytesRead;

    output = "";  
    while (true) {
        if (!ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) || bytesRead == 0) {
            break;  
        }

        buffer[bytesRead] = '\0';
        output.append(buffer, bytesRead); 
    }

 
    WaitForSingleObject(pi.hProcess, INFINITE); 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hRead);  
}


