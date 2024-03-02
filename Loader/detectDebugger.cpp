#include <Windows.h>
#include <iostream>

typedef NTSTATUS(NTAPI* fnQueryProcInfo)(HANDLE, DWORD, PVOID, ULONG, PULONG);

enum PROCESSINFOCLASS {
    ProcessDebugPortIndicator = 7,
    ProcessDebugHandleIndicator = 30,
};

fnQueryProcInfo LoadQueryProcInfoFunction() {

    fnQueryProcInfo ptrQueryProcInfo = nullptr;
    ptrQueryProcInfo = (fnQueryProcInfo)GetProcAddress(GetModuleHandle(TEXT("NTDLL.DLL")), "NtQueryInformationProcess");
    return ptrQueryProcInfo;

}

bool IsDebugPortPresent(fnQueryProcInfo ptrQueryProcInfo) {

    NTSTATUS queryStatus = NULL;
    DWORD64 debugPortValue = NULL;

    queryStatus = ptrQueryProcInfo(
        GetCurrentProcess(),
        ProcessDebugPortIndicator,
        &debugPortValue,
        sizeof(DWORD64),
        nullptr
    );


    if (queryStatus != 0x0) {
        std::cerr << "\t[!] NtQueryInformationProcess [1] Failed With Status : 0x" << std::hex << queryStatus << std::endl;
        return false;
    }
    if (debugPortValue != NULL) {
        return true;
    }
    return false;

}

bool IsDebugHandlePresent(fnQueryProcInfo ptrQueryProcInfo) {

    NTSTATUS queryStatus = NULL;
    DWORD64 debugHandleValue = NULL;

    queryStatus = ptrQueryProcInfo(
        GetCurrentProcess(),
        ProcessDebugHandleIndicator,
        &debugHandleValue,
        sizeof(DWORD64),
        nullptr
    );

    if (queryStatus != 0x0 && queryStatus != 0xC0000353) {
        std::cerr << "\tNtQueryInformationProcess failed With Status: 0x" << std::hex << queryStatus << std::endl;
        return false;
    }
    if (debugHandleValue != NULL) {
        return true;
    }

    return false;

}

bool DetectDebugger() {
    fnQueryProcInfo ptrQueryProcInfo = LoadQueryProcInfoFunction();
    if (ptrQueryProcInfo == nullptr) {
        std::cerr << "\tGetProcAddress Failed With Error : " << GetLastError() << std::endl;
        return false;
    }

    if (IsDebugPortPresent(ptrQueryProcInfo)) {
        return true;
    }

    if (IsDebugHandlePresent(ptrQueryProcInfo)) {
        return true;
    }

    return false;
}
