// Include necessary headers for Windows API functions and input/output operations.
#include <Windows.h>
#include <iostream>

// Define a function pointer type for NtQueryInformationProcess.
typedef NTSTATUS(NTAPI* fnQueryProcInfo)(HANDLE, DWORD, PVOID, ULONG, PULONG);

// Enum to represent specific process information classes for querying.
enum PROCESSINFOCLASS {
    ProcessDebugPortIndicator = 7, // Used to query the debug port.
    ProcessDebugHandleIndicator = 30, // Used to query the debug handle.
};

// Load the NtQueryInformationProcess function from NTDLL.DLL dynamically.
fnQueryProcInfo LoadQueryProcInfoFunction() {
    fnQueryProcInfo ptrQueryProcInfo = nullptr; // Initialize pointer to null.
    // Attempt to get the function address.
    ptrQueryProcInfo = (fnQueryProcInfo)GetProcAddress(GetModuleHandle(TEXT("NTDLL.DLL")), "NtQueryInformationProcess");
    return ptrQueryProcInfo; // Return the function pointer.
}

// Check if a debug port is present for the current process.
bool IsDebugPortPresent(fnQueryProcInfo ptrQueryProcInfo) {
    NTSTATUS queryStatus = NULL; // Status of the query.
    DWORD64 debugPortValue = NULL; // Variable to store the debug port value.

    // Query the process for a debug port.
    queryStatus = ptrQueryProcInfo(
        GetCurrentProcess(), // Handle to the current process.
        ProcessDebugPortIndicator, // Query class for debug port.
        &debugPortValue, // Output parameter for debug port value.
        sizeof(DWORD64), // Size of the output parameter.
        nullptr // Not using the return length.
    );

    // Check the status of the query.
    if (queryStatus != 0x0) {
        // Query failed; log the error.
        std::cerr << "\t[!] NtQueryInformationProcess [1] Failed With Status : 0x" << std::hex << queryStatus << std::endl;
        return false; // Return false indicating failure or no debug port present.
    }
    // If debugPortValue is not NULL, a debug port is present.
    if (debugPortValue != NULL) {
        return true; // Debug port is present.
    }
    return false; // No debug port is present.
}

// Check if a debug handle is present for the current process.
bool IsDebugHandlePresent(fnQueryProcInfo ptrQueryProcInfo) {
    NTSTATUS queryStatus = NULL; // Status of the query.
    DWORD64 debugHandleValue = NULL; // Variable to store the debug handle value.

    // Query the process for a debug handle.
    queryStatus = ptrQueryProcInfo(
        GetCurrentProcess(), // Handle to the current process.
        ProcessDebugHandleIndicator, // Query class for debug handle.
        &debugHandleValue, // Output parameter for debug handle value.
        sizeof(DWORD64), // Size of the output parameter.
        nullptr // Not using the return length.
    );

    // Check the status of the query; specific status codes may indicate different conditions.
    if (queryStatus != 0x0 && queryStatus != 0xC0000353) {
        // Query failed; log the error.
        std::cerr << "\tNtQueryInformationProcess failed With Status: 0x" << std::hex << queryStatus << std::endl;
        return false; // Return false indicating failure or no debug handle present.
    }
    // If debugHandleValue is not NULL, a debug handle is present.
    if (debugHandleValue != NULL) {
        return true; // Debug handle is present.
    }

    return false; // No debug handle is present.
}

// Main function to detect if the process is being debugged.
bool DetectDebugger() {
    // Load the NtQueryInformationProcess function.
    fnQueryProcInfo ptrQueryProcInfo = LoadQueryProcInfoFunction();
    // Check if the function was successfully loaded.
    if (ptrQueryProcInfo == nullptr) {
        // Failed to load the function; log the error.
        std::cerr << "\tGetProcAddress Failed With Error : " << GetLastError() << std::endl;
        return false; // Return false indicating failure to detect.
    }

    // Check for the presence of a debug port.
    if (IsDebugPortPresent(ptrQueryProcInfo)) {
        return true; // Debug port is present; debugger detected.
    }

    // Check for the presence of a debug handle.
    if (IsDebugHandlePresent(ptrQueryProcInfo)) {
        return true; // Debug handle is present; debugger detected.
    }

    return false; // No debugger detected.
}
