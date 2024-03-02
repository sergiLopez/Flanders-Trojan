#ifndef VM_CHECK_H
#define VM_CHECK_H

#include <Windows.h>

BOOL ExeDigitsInNameCheck();
BOOL CheckMachineResolution();
BOOL CheckMachineProcesses();
bool IsVirtualMachine();

constexpr int ID_LENGTH = 13;
extern const char* const VM_IDENTIFIERS[];

#endif 