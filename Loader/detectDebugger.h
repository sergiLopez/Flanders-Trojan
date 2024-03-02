#ifndef DEBUGGER_DETECTION_H
#define DEBUGGER_DETECTION_H

#include <Windows.h>

typedef NTSTATUS(NTAPI* fnQueryProcInfo)(HANDLE, DWORD, PVOID, ULONG, PULONG);

enum PROCESSINFOCLASS {
    ProcessDebugPortIndicator = 7,
    ProcessDebugHandleIndicator = 30,
};

fnQueryProcInfo LoadQueryProcInfoFunction();
bool IsDebugPortPresent(fnQueryProcInfo ptrQueryProcInfo);
bool IsDebugHandlePresent(fnQueryProcInfo ptrQueryProcInfo);
bool DetectDebugger();

#endif 
