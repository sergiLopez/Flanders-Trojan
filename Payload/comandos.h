// commands.h
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <sstream>
#include <Windows.h> 

// Declaración de las funciones que se utilizan dentro de executeCommands y que no están definidas aquí.
void ExecuteCommandHidden(const std::string& command, std::string& output);
bool GetRegistryValue(HKEY hKey, const std::string& subKey, const std::string& value, DWORD type, BYTE* data, DWORD* dataSize);

// Declaración de executeCommands
void executeCommands();

#endif // COMMANDS_H
