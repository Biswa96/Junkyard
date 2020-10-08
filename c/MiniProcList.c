/*
 * This source file is part of Junkyard repository.
 * Licensed under the terms of the GNU General Public License v3 or later.
 * Copyright (C) 2020 Biswapriyo Nath.
 * 
 * MiniProcList.c: List all minimal processes based on handle count.
 * Compile: gcc MiniProcList.c -lntdll
 * Related links: https://github.com/thinkcz/pico-toolbox.git
 */

#include <windows.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma comment(lib, "ntdll.lib")
#endif

#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004)
#endif

typedef long KPRIORITY;

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING;

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemProcessInformation = 5
} SYSTEM_INFORMATION_CLASS;

/* Extracted from combase.pdb */
typedef struct _SYSTEM_PROCESS_INFORMATION
{
    /* 0x0000 */ ULONG NextEntryOffset;
    /* 0x0004 */ ULONG NumberOfThreads;
    /* 0x0008 */ LARGE_INTEGER WorkingSetPrivateSize;
    /* 0x0010 */ ULONG HardFaultCount;
    /* 0x0014 */ ULONG NumberOfThreadsHighWatermark;
    /* 0x0018 */ ULONGLONG CycleTime;
    /* 0x0020 */ LARGE_INTEGER CreateTime;
    /* 0x0028 */ LARGE_INTEGER UserTime;
    /* 0x0030 */ LARGE_INTEGER KernelTime;
    /* 0x0038 */ UNICODE_STRING ImageName;
    /* 0x0048 */ KPRIORITY BasePriority;
    /* 0x004C */ ULONG Padding1;
    /* 0x0050 */ ULONGLONG UniqueProcessId;
    /* 0x0058 */ ULONGLONG InheritedFromUniqueProcessId;
    /* 0x0060 */ ULONG HandleCount;
    /* 0x0064 */ ULONG SessionId;
    /* 0x0068 */ ULONG_PTR UniqueProcessKey;
    /* 0x0070 */ SIZE_T PeakVirtualSize;
    /* 0x0078 */ SIZE_T VirtualSize;
    /* 0x0080 */ ULONG PageFaultCount;
    /* 0x0084 */ ULONG Padding2;
    /* 0x0088 */ SIZE_T PeakWorkingSetSize;
    /* 0x0090 */ SIZE_T WorkingSetSize;
    /* 0x0098 */ SIZE_T QuotaPeakPagedPoolUsage;
    /* 0x00A0 */ SIZE_T QuotaPagedPoolUsage;
    /* 0x00A8 */ SIZE_T QuotaPeakNonPagedPoolUsage;
    /* 0x00B0 */ SIZE_T QuotaNonPagedPoolUsage;
    /* 0x00B8 */ SIZE_T PagefileUsage;
    /* 0x00C0 */ SIZE_T PeakPagefileUsage;
    /* 0x00C8 */ SIZE_T PrivatePageCount;
    /* 0x00D0 */ LARGE_INTEGER ReadOperationCount;
    /* 0x00D8 */ LARGE_INTEGER WriteOperationCount;
    /* 0x00E0 */ LARGE_INTEGER OtherOperationCount;
    /* 0x00E8 */ LARGE_INTEGER ReadTransferCount;
    /* 0x00F0 */ LARGE_INTEGER WriteTransferCount;
    /* 0x00F8 */ LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION; /* size: 0x0100 */

C_ASSERT(sizeof(SYSTEM_PROCESS_INFORMATION) == 0x100);

NTSTATUS NTAPI NtQuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength);

int main(void)
{
    NTSTATUS Status;
    void *Buffer = NULL;
    ULONG size = 0, total = 0;
    SYSTEM_PROCESS_INFORMATION spi, *pspi;
    memset(&spi, 0, sizeof spi);

    Status = NtQuerySystemInformation(SystemProcessInformation, &spi, sizeof spi, &size);
    if (Status != STATUS_INFO_LENGTH_MISMATCH)
    {
        wprintf(L"Error: 0x%08X\n", Status);
        return 1;
    }

    Buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    Status = NtQuerySystemInformation(SystemProcessInformation, Buffer, size, &size);
    if (Status != 0)
    {
        wprintf(L"Error: 0x%08X\n", Status);
        return 1;
    }

    pspi = (SYSTEM_PROCESS_INFORMATION *)Buffer;

    wprintf(L"\n   PID  Process\n");
    while (pspi)
{
        if (pspi->HandleCount == 0) /* If minimal */
        {
            if (pspi->ImageName.Length != 0)
            {
                wprintf(L"%6llu  %ls\n", pspi->UniqueProcessId, pspi->ImageName.Buffer);
                ++total;
            }
        }

        if (!pspi->NextEntryOffset)
            break;

        pspi = (SYSTEM_PROCESS_INFORMATION *)((char *)pspi + pspi->NextEntryOffset);
    }

    wprintf(L"\nTotal minimal processes: %lu\n", total);
    HeapFree(GetProcessHeap(), 0, Buffer);

    return 0;
}
