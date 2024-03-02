#include <iostream>    
#include <windows.h>   
#include <tlhelp32.h>  
#include <tchar.h>     

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

bool TokenHasElevation(HANDLE tokenHandle) {

    TOKEN_ELEVATION tokenElevation;
    DWORD size;
    if (GetTokenInformation(tokenHandle, TokenElevation, &tokenElevation, sizeof(tokenElevation), &size)) {
        return tokenElevation.TokenIsElevated > 0;
    }

    return false;
}

bool EvaluateProcessElevation(HANDLE processHandle) {

    HANDLE tokenHandle;
    bool elevatedStatus = false;
    if (OpenProcessToken(processHandle, TOKEN_QUERY, &tokenHandle)) {
        elevatedStatus = TokenHasElevation(tokenHandle);
        CloseHandle(tokenHandle);
    }
    return elevatedStatus;
}

bool IsTargetProcessElevated(const wchar_t* targetProcessName, PROCESSENTRY32& processEntry) {

    if (_wcsicmp(targetProcessName, processEntry.szExeFile) == 0) {
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processEntry.th32ProcessID);
        if (process != NULL) {
            bool elevated = EvaluateProcessElevation(process);
            CloseHandle(process);
            return elevated;
        }
    }
    return false;

}


bool CheckHighPrivilegeProcess(const wchar_t* targetProcessName) {

    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnapshot == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to take a snapshots!!" << std::endl;
        return false;
    }
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(processSnapshot, &processInfo)) {
        std::wcerr << L"Failed to retrieve information on system proceses!" << std::endl;
        CloseHandle(processSnapshot);
        return false;
    }
    do {
        if (IsTargetProcessElevated(targetProcessName, processInfo)) {
            CloseHandle(processSnapshot);
            return true;
        }
    } while (Process32Next(processSnapshot, &processInfo));

    CloseHandle(processSnapshot);
    return false;
}
