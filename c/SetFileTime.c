/*
 * This source file is part of Junkyard repository.
 * Licensed under the terms of the GNU General Public License v3 or later.
 * Copyright (C) 2020 Biswapriyo Nath
 * 
 * SetFileTime: Set all timestamp of a file to 1 Jan 1601.
 * Options: SetFileTime.exe [file or folder full path]
 * Compile: gcc SetFileTime.c -lntdll
 */

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma comment (lib, "ntdll.lib")
#endif

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationFile(
    _In_ HANDLE FileHandle,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationFile(
    _In_ HANDLE FileHandle,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass
    );

NTSTATUS SetTimeIndividual(PWSTR ToRead, PWSTR ToWrite)
{
    HANDLE hFile;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION FileInfo = { 0 };

    if (ToRead && *ToRead)
    {
        hFile = CreateFileW(
            ToRead,
            FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);

        Status = NtQueryInformationFile(
            hFile,
            &IoStatusBlock,
            &FileInfo, sizeof FileInfo, 4); /* FileBasicInformation */

        CloseHandle(hFile);
    }
    else
    {
        FileInfo.CreationTime.LowPart = FileInfo.CreationTime.HighPart = 1;
        FileInfo.LastAccessTime.LowPart = FileInfo.LastAccessTime.HighPart = 1;
        FileInfo.LastWriteTime.LowPart = FileInfo.LastWriteTime.HighPart = 1;
        FileInfo.ChangeTime.LowPart = FileInfo.ChangeTime.HighPart = 1;
        FileInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }

    hFile = CreateFileW(
        ToWrite,
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    Status = NtSetInformationFile(
        hFile,
        &IoStatusBlock,
        &FileInfo, sizeof FileInfo, 4); /* FileBasicInformation */

    CloseHandle(hFile);
    return Status;
}

void WINAPI SetTimeRecursive(PWSTR FdPath)
{
    WIN32_FIND_DATAW fileInfo;
    wchar_t Path[MAX_PATH], FolderInitialPath[MAX_PATH];
    wchar_t wildCard[] = L"\\*.*";

    wcscpy(Path, FdPath);
    wcscpy(FolderInitialPath, FdPath);
    wcscat(FolderInitialPath, wildCard);

    HANDLE hFile = FindFirstFileW(FolderInitialPath, &fileInfo);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fileInfo.cFileName[0] != '.')
            {
                wcscpy(Path, FdPath);
                wcscat(Path, L"\\");
                wcscat(Path, fileInfo.cFileName);
                if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {   // Folder
                    SetTimeIndividual(NULL, Path);
                    SetTimeRecursive(Path);
                }
                else
                {   // File
                    SetTimeIndividual(NULL, Path);
                }
            }
        } while (FindNextFileW(hFile, &fileInfo));

        FindClose(hFile);
    }
}

int WINAPI main(void)
{
    int wargc;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

    if (wargc < 2)
    {
        wprintf(L"No path specified\n");
        return 0;
    }

    if (wargc == 3)
    {
        wprintf(L"Read from: %ls\n", wargv[1]);
        wprintf(L"Write to: %ls\n", wargv[2]);

        SetTimeIndividual(wargv[1], wargv[2]);
        return 0;
    }

    SetTimeIndividual(NULL, wargv[1]);
    SetTimeRecursive(wargv[1]);
    return 0;
}
