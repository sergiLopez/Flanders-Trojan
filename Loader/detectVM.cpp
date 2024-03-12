#include <Windows.h>
#include <Shlwapi.h>
#include <psapi.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <intrin.h>

// Ensure the Shlwapi.lib library is linked.
#pragma comment(lib, "Shlwapi.lib")

// Checks if the executable name contains more than three digits, which might indicate a randomized or generated file name, common in sandboxed environments.
BOOL ExeDigitsInNameCheck() {
    CHAR Path[MAX_PATH * 3];
    CHAR cName[MAX_PATH];
    DWORD dwNumberOfDigits = NULL;

    // Retrieve the file name of the executable containing the current process.
    if (!GetModuleFileNameA(NULL, Path, MAX_PATH * 3)) {
        return 0;
    }

    // Extract the file name from the path and copy it to cName.
    if (lstrlenA(PathFindFileNameA(Path)) < MAX_PATH)
        lstrcpyA(cName, PathFindFileNameA(Path));

    // Count the number of digits in the file name.
    for (int i = 0; i < lstrlenA(cName); i++) {
        if (isdigit(cName[i]))
            dwNumberOfDigits++;
    }

    // Return TRUE if more than three digits are found, indicating a potential sandbox environment.
    if (dwNumberOfDigits > 3) {
        return TRUE;
    }

    return FALSE;
}

// Callback function used by EnumDisplayMonitors to check each monitor's resolution.
BOOL CALLBACK ResolutionCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lpRect, LPARAM ldata) {
    int X = 0, Y = 0;
    MONITORINFO MI;
    MI.cbSize = sizeof(MONITORINFO);

    // Retrieve information about the monitor.
    if (!GetMonitorInfoW(hMonitor, &MI)) {
        return 0;
    }

    // Calculate the resolution of the monitor.
    X = MI.rcMonitor.right - MI.rcMonitor.left;
    Y = MI.rcMonitor.top - MI.rcMonitor.bottom;

    // Normalize the values in case they are negative.
    if (X < 0) X = -X;
    if (Y < 0) Y = -Y;

    // Set the SANDBOX flag if the resolution is less than 800x600, a common minimum resolution for a sandbox.
    if (X < 800 || Y < 600) 
        *((BOOL*)ldata) = TRUE;

    return 1;
}

// Checks if any monitor attached to the computer has a resolution less than 800x600.
BOOL CheckMachineResolution() {
    BOOL SANDBOX = 0;
    EnumDisplayMonitors(NULL, NULL, ResolutionCallback, (LPARAM)(&SANDBOX));
    return SANDBOX;
}

// Checks the number of processes running on the system to detect low-activity environments like sandboxes.
BOOL CheckMachineProcesses() {
    DWORD adwProcesses[1024];
    DWORD dwReturnLen = NULL, dwNmbrOfPids = NULL;

    // Enumerate all processes.
    if (!EnumProcesses(adwProcesses, sizeof(adwProcesses), &dwReturnLen)) {
        return 0;
    }

    // Calculate the number of process identifiers returned.
    dwNmbrOfPids = dwReturnLen / sizeof(DWORD);

    // Return TRUE if the number of processes is unusually low (< 50), indicating a sandbox.
    if (dwNmbrOfPids < 50)
        return 1;

    return 0;
}

// String identifiers for common virtual machine hypervisors.
constexpr int ID_LENGTH = 13;
const char* const VM_IDENTIFIERS[] = {
    "KVMKVMKVM\0\0\0",
    "VMwareVMware",
    "XenVMMXenVMM",
    "prl hyperv  ",
    "VBoxVBoxVBox"
};

// Retrieves information about the CPU using the __cpuid intrinsic function.
void RetrieveCPUInfo(int infoArray[4], int idFunction) {
    __cpuid(infoArray, idFunction);
}

// Verifies if a hypervisor is present based on CPUID information.
bool VerifyHypervisorPresence(const int infoArray[4]) {
    return infoArray[2] & (1 << 31);
}

// Acquires the vendor identifier string from the CPUID information.
void AcquireVendorIdentifier(char* vendorIdentifier, const int infoArray[4]) {
    memcpy(vendorIdentifier, &infoArray[1], 4); 
    memcpy(vendorIdentifier + 4, &infoArray[2], 4);
    memcpy(vendorIdentifier + 8, &infoArray[3], 4);
    vendorIdentifier[12] = '\0';
}

// Compares the CPU vendor identifier against known VM vendor identifiers.
bool CompareVendorIDs(const char* vendorIdentifier) {
    for (const auto& id : VM_IDENTIFIERS) {
        if (!memcmp(id, vendorIdentifier, ID_LENGTH))
            return true;
    }
    return false;
}

// Main function to detect if the system is running inside a virtual machine.
bool IsVirtualMachine() {
    int infoArray[4] = {};
    RetrieveCPUInfo(infoArray, 1);

    // Check for the presence of a hypervisor.
    if (!VerifyHypervisorPresence(infoArray))
        return false;

    // Retrieve the vendor identifier of the hypervisor.
    RetrieveCPUInfo(infoArray, 0x40000000);

    char vendorIdentifier[ID_LENGTH] = {};
    AcquireVendorIdentifier(vendorIdentifier, infoArray);

    // Compare the vendor identifier against known VM identifiers.
    return CompareVendorIDs(vendorIdentifier);
}
