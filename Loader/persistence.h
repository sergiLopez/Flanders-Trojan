#ifndef AUTOSTART_CONFIG_H
#define AUTOSTART_CONFIG_H

#include <windows.h>
#include <tchar.h>
#include <string>

bool configureAutoStart(LPCTSTR appName, LPCTSTR appPath);
bool checkAutoStartEnabled(LPCTSTR appName);
int setupPersistence();
void setupAndRunScheduledTask();

#endif 
