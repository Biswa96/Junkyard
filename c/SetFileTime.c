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

NTSTATUS NTAPI NtSetInformationFile(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass);

NTSTATUS SetTimeIndividual(PWSTR FileName)
{
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION FileInfo = { 0 };
    FileInfo.CreationTime.LowPart = FileInfo.CreationTime.HighPart = 1;
    FileInfo.LastAccessTime.LowPart = FileInfo.LastAccessTime.HighPart = 1;
    FileInfo.LastWriteTime.LowPart = FileInfo.LastWriteTime.HighPart = 1;
    FileInfo.ChangeTime.LowPart = FileInfo.ChangeTime.HighPart = 1;
    FileInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

    HANDLE hFile = CreateFileW(
        FileName,
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    NTSTATUS Status = NtSetInformationFile(
        hFile,
        &IoStatusBlock,
        &FileInfo, sizeof FileInfo, 4); /*FileBasicInformation */

    Status = NtClose(hFile);
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
                    SetTimeIndividual(Path);
                    SetTimeRecursive(Path);
                }
                else
                {   // File
                    SetTimeIndividual(Path);
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
        printf("No path specified\n");
        return 0;
    }

    SetTimeIndividual(wargv[1]);
    SetTimeRecursive(wargv[1]);
    return 0;
}
