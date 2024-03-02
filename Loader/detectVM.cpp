#include <Windows.h>
#include <Shlwapi.h>
#include <psapi.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <intrin.h>

#pragma comment(lib, "Shlwapi.lib")


BOOL ExeDigitsInNameCheck() {

    CHAR Path[MAX_PATH * 3];
    CHAR cName[MAX_PATH];
    DWORD dwNumberOfDigits = NULL;

    if (!GetModuleFileNameA(NULL, Path, MAX_PATH * 3)) {
      
        return 0;
    }
    if (lstrlenA(PathFindFileNameA(Path)) < MAX_PATH)
        lstrcpyA(cName, PathFindFileNameA(Path));

    for (int i = 0; i < lstrlenA(cName); i++) {
        if (isdigit(cName[i]))
            dwNumberOfDigits++;
    }

    if (dwNumberOfDigits > 3) {
        return TRUE;
    }

    return FALSE;
}



BOOL CALLBACK ResolutionCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lpRect, LPARAM ldata) {
    
    int X = 0, Y = 0;
    MONITORINFO MI;
    MI.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfoW(hMonitor, &MI)) {
        return 0;
    }
    X = MI.rcMonitor.right - MI.rcMonitor.left;
    Y = MI.rcMonitor.top - MI.rcMonitor.bottom;

    if (X < 0) X = -X;
    if (Y < 0) Y = -Y;
    if (X < 800 || Y < 600) 
        *((BOOL*)ldata) = TRUE;

    return 1;
}

BOOL CheckMachineResolution() {
    BOOL SANDBOX = 0;
    EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)ResolutionCallback, (LPARAM)(&SANDBOX));
    return SANDBOX;
}

BOOL CheckMachineProcesses() {

    DWORD adwProcesses[1024];
    DWORD dwReturnLen = NULL, dwNmbrOfPids = NULL;

    if (!EnumProcesses(adwProcesses, sizeof(adwProcesses), &dwReturnLen)) {
        return 0;
    }

    dwNmbrOfPids = dwReturnLen / sizeof(DWORD);

    if (dwNmbrOfPids < 50)
        return 1;

    return 0;
}


constexpr int ID_LENGTH = 13;
const char* const VM_IDENTIFIERS[] = {
    "KVMKVMKVM\0\0\0",
    "VMwareVMware",
    "XenVMMXenVMM",
    "prl hyperv  ",
    "VBoxVBoxVBox"
};

void RetrieveCPUInfo(int infoArray[4], int idFunction) {
    __cpuid(infoArray, idFunction);
}

bool VerifyHypervisorPresence(const int infoArray[4]) {
    return infoArray[2] & (1 << 31);
}

void AcquireVendorIdentifier(char* vendorIdentifier, const int infoArray[4]) {
    memcpy(vendorIdentifier, &infoArray[1], 4); 
    memcpy(vendorIdentifier + 4, &infoArray[2], 4);
    memcpy(vendorIdentifier + 8, &infoArray[3], 4);
    vendorIdentifier[12] = '\0';
}

bool CompareVendorIDs(const char* vendorIdentifier) {
    for (const auto& id : VM_IDENTIFIERS) {
        if (!memcmp(id, vendorIdentifier, ID_LENGTH))
            return true;
    }
    return false;
}

bool IsVirtualMachine() {

    int infoArray[4] = {};
    RetrieveCPUInfo(infoArray, 1);

    if (!VerifyHypervisorPresence(infoArray))
        return false;

    RetrieveCPUInfo(infoArray, 0x40000000);

    char vendorIdentifier[ID_LENGTH] = {};
    AcquireVendorIdentifier(vendorIdentifier, infoArray);

    return CompareVendorIDs(vendorIdentifier);

}

