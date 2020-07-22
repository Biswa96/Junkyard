/*
 * This source file is part of Junkyard repository.
 * Licensed under the terms of the GNU General Public License v3 or later.
 * Copyright (C) Biswapriyo Nath
 * 
 * KillDWM.c: Terminate DWM process by suspending WinLogon process.
 * Options:
 * -s  Suspend winlogon process and terminate DWM.
 * -r  Resume winlogon process which revives DWM.
 *
 * Compile: gcc -Wall KillDwm.c -lntdll -ladvapi32
 * Requirements: Need administrator permission.
 * Related links: https://stackoverflow.com/a/11010508/8928481/
 */

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "ntdll.lib")
#endif

NTSYSCALLAPI NTSTATUS NTAPI NtSuspendProcess(HANDLE ProcessHandle);
NTSYSCALLAPI NTSTATUS NTAPI NtResumeProcess(HANDLE ProcessHandle);
NTSYSCALLAPI NTSTATUS NTAPI NtTerminateProcess(HANDLE ProcessHandle, NTSTATUS ExitStatus);

BOOL WINAPI EnablePrivilege(PCSTR PrivilegeName)
{
    BOOL bRes;
    HANDLE TokenHandle;
    LUID Luid;

    bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle);

    bRes = LookupPrivilegeValueA(NULL, PrivilegeName, &Luid);

    TOKEN_PRIVILEGES NewState;
    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = Luid;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bRes = AdjustTokenPrivileges(TokenHandle, FALSE, &NewState, sizeof NewState, NULL, NULL);

    return bRes;
}

void WINAPI GetPid(PCSTR ProcessName, PULONG ProcessId)
{
    PROCESSENTRY32 entry = { 0 };
    entry.dwSize = sizeof entry;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry))
    {
        while (Process32Next(snapshot, &entry))
        {
            if (stricmp(entry.szExeFile, ProcessName) == 0)
                *ProcessId = entry.th32ProcessID;
        }
    }

    CloseHandle(snapshot);
}


int WINAPI main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("No option provided\n");
        return 0;
    }

    ULONG ProcessId;
    HANDLE hProcess = NULL;
    NTSTATUS Status;

    EnablePrivilege("SeDebugPrivilege");

    if (!strcmp("-s", argv[1]))
    {
        GetPid("winlogon.exe", &ProcessId);
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
        Status = NtSuspendProcess(hProcess);
        printf("winlogon: PID: %ld Handle: 0x%p Status: 0x%lX\n", ProcessId, hProcess, Status);
        CloseHandle(hProcess);

        GetPid("dwm.exe", &ProcessId);
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
        Status = NtTerminateProcess(hProcess, 0);
        printf("dwm: PID: %ld Handle: 0x%p Status: 0x%lX\n", ProcessId, hProcess, Status);
        CloseHandle(hProcess);

        return 0;
    }
    else if (!strcmp("-r", argv[1]))
    {
        GetPid("winlogon.exe", &ProcessId);
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
        Status = NtResumeProcess(hProcess);
        printf("winlogon: PID: %ld Handle: 0x%p Status: 0x%lX\n", ProcessId, hProcess, Status);
        CloseHandle(hProcess);

        return 0;
    }
    else
        printf("Only -r and -s option available\n");

    return 0;
}
