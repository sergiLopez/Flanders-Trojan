#ifndef PROCESS_ELEVATION_H
#define PROCESS_ELEVATION_H

#include <windows.h>
#include <tlhelp32.h>

bool TokenHasElevation(HANDLE tokenHandle);
bool EvaluateProcessElevation(HANDLE processHandle);
bool IsTargetProcessElevated(const wchar_t* targetProcessName, PROCESSENTRY32& processEntry);
bool CheckHighPrivilegeProcess(const wchar_t* targetProcessName);

#endif 
