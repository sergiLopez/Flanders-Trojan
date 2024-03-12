#include <iostream>    // Include for input/output streaming.
#include <windows.h>   // Include for Windows-specific functions and data types.
#include <tlhelp32.h>  // Include for functions like CreateToolhelp32Snapshot.
#include <tchar.h>     // Include for Unicode and ANSI compatibility.

// Link with the Windows Socket 2 Library and Common Controls Library.
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

// Function to check if a given token has elevation privileges.
bool TokenHasElevation(HANDLE tokenHandle) {
    TOKEN_ELEVATION tokenElevation; // Structure to hold the elevation information.
    DWORD size; // Variable to receive the size of the structure.
    // Retrieve token elevation information.
    if (GetTokenInformation(tokenHandle, TokenElevation, &tokenElevation, sizeof(tokenElevation), &size)) {
        // Return true if the token has elevation privileges.
        return tokenElevation.TokenIsElevated > 0;
    }
    // Return false if the function fails or the token is not elevated.
    return false;
}

// Function to evaluate if a process, identified by its handle, has elevated privileges.
bool EvaluateProcessElevation(HANDLE processHandle) {
    HANDLE tokenHandle; // Handle to the process's token.
    bool elevatedStatus = false; // Status of elevation, default to false.
    // Open the process token with query access.
    if (OpenProcessToken(processHandle, TOKEN_QUERY, &tokenHandle)) {
        // Check if the token has elevation privileges.
        elevatedStatus = TokenHasElevation(tokenHandle);
        // Close the token handle.
        CloseHandle(tokenHandle);
    }
    // Return the elevation status.
    return elevatedStatus;
}

// Function to check if a target process, identified by name, is running with elevated privileges.
bool IsTargetProcessElevated(const wchar_t* targetProcessName, PROCESSENTRY32& processEntry) {
    // Compare the target process name with the current process entry's name.
    if (_wcsicmp(targetProcessName, processEntry.szExeFile) == 0) {
        // Open the target process with limited query information.
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processEntry.th32ProcessID);
        if (process != NULL) {
            // Evaluate the process's elevation status.
            bool elevated = EvaluateProcessElevation(process);
            // Close the process handle.
            CloseHandle(process);
            // Return the elevation status.
            return elevated;
        }
    }
    // Return false if the target process is not found or cannot be opened.
    return false;
}

// Function to check the system for a high privilege process, identified by name.
bool CheckHighPrivilegeProcess(const wchar_t* targetProcessName) {
    // Take a snapshot of all processes in the system.
    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnapshot == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to take a snapshots!!" << std::endl;
        return false;
    }
    PROCESSENTRY32 processInfo; // Structure to hold process entry information.
    processInfo.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process encountered in the snapshot.
    if (!Process32First(processSnapshot, &processInfo)) {
        std::wcerr << L"Failed to retrieve information on system processes!" << std::endl;
        CloseHandle(processSnapshot);
        return false;
    }
    do {
        // Check if the current process is the target and if it's elevated.
        if (IsTargetProcessElevated(targetProcessName, processInfo)) {
            // If found and elevated, close the snapshot handle and return true.
            CloseHandle(processSnapshot);
            return true;
        }
        // Move to the next process entry in the snapshot.
    } while (Process32Next(processSnapshot, &processInfo));

    // Close the snapshot handle and return false if the target process is not found or not elevated.
    CloseHandle(processSnapshot);
    return false;
}
