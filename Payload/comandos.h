// commands.h
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <sstream>
#include <Windows.h> 

// Declaraci�n de las funciones que se utilizan dentro de executeCommands y que no est�n definidas aqu�.
void ExecuteCommandHidden(const std::string& command, std::string& output);
bool GetRegistryValue(HKEY hKey, const std::string& subKey, const std::string& value, DWORD type, BYTE* data, DWORD* dataSize);

// Declaraci�n de executeCommands
void executeCommands();

#endif // COMMANDS_H
