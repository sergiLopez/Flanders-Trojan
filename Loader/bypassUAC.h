#ifndef REGISTRY_CONFIG_H
#define REGISTRY_CONFIG_H

#include <windows.h>

LSTATUS SetRegValue(HKEY keyHandle, const wchar_t* valueTitle, const wchar_t* valueData);
int ConfigureRegistry();
int InitiateElevatedProc();
int OverrideUAC();

#endif 

