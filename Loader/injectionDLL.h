
#ifndef MILIBRERIA_H
#define MILIBRERIA_H

#include <Windows.h>

BOOL InjectLibraryIntoProcess(HANDLE processHandle, LPWSTR libraryName);
BOOL RetrieveProcessHandle(LPWSTR targetProcessName, DWORD* processId, HANDLE* handleOfProcess);
int InjectDLL();

#endif 
