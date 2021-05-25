/*
 * This source file is part of Junkyard repository.
 * Licensed under the terms of the GNU General Public License v3 or later.
 * Copyright (C) Biswapriyo Nath
 * 
 * HlslComp.c: Compile HLSL shader file.
 * Compile: gcc -Wall HlslComp.c -ld3dcompiler
 */

#include <windows.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <getopt.h>

void ShowUsage(char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -e, --entry <name>       Entrypoint name.\n");
    printf("  -h, --help               Show this usage information.\n");
    printf("  -i, --in <file>          Input shader file.\n");
    printf("  -o, --out <file>         Output header file.\n");
    printf("  -t, --target <profile>   Target profile.\n");
}

void Logging(LPCSTR Func, HRESULT hRes)
{
    LPSTR MsgBuf = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, hRes, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&MsgBuf, 0, NULL);
    printf("%s Error: %s\n", Func, MsgBuf);
    LocalFree(MsgBuf);
}

int main(int argc, char *argv[])
{
    LPWSTR InFileW = NULL;
    LPSTR InFileA = NULL, OutFile = NULL, EntryPoint = NULL, Target = NULL;
    BOOLEAN EntryOpt = FALSE, InOpt = FALSE, OutOpt = FALSE, TargetOpt = FALSE;

    const char shortopts[] = "e:hi:o:t:";
    const struct option longopts[] = {
        { "entry",  required_argument, NULL, 'e' },
        { "help",   0,                 NULL, 'h' },
        { "in",     required_argument, NULL, 'i' },
        { "out",    required_argument, NULL, 'o' },
        { "target", required_argument, NULL, 't' },
        { NULL,     0,                 NULL,  0  }
    };

    int ch = 0;
    while ((ch = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch (ch)
        {
            case 'e':
            {
                EntryPoint = _strdup(optarg);
                EntryOpt = TRUE;
                break;
            }
            case 'h':
            {
                ShowUsage(argv[0]);
                exit(0);
            }
            case 'i':
            {
                InFileA = _strdup(optarg);
                InOpt = TRUE;
                break;
            }
            case 'o':
            {
                OutFile = _strdup(optarg);
                OutOpt = TRUE;
                break;
            }
            case 't':
            {
                Target = _strdup(optarg);
                TargetOpt = TRUE;
                break;
            }
            default:
            {
                printf("Try '%s --help' for more information.\n", argv[0]);
                exit(0);
            }
        }
    }

    if (!EntryOpt || !InOpt || !OutOpt || !TargetOpt)
    {
        printf("Please provide all the options, try '%s --help' for more information.\n", argv[0]);
        exit(0);
    }

    HRESULT hRes;
    ID3DBlob *Contents = NULL, *Code = NULL, *ErrMsg = NULL, *Disassembly = NULL;
    FILE *file = NULL;

    int FileNameLen = MultiByteToWideChar(CP_UTF8, 0, InFileA, -1, 0, 0);
    InFileW = malloc(FileNameLen * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, InFileA, -1, InFileW, FileNameLen);

    hRes = D3DReadFileToBlob(InFileW, &Contents);
    if (hRes)
    {
        Logging("D3DReadFileToBlob", hRes);
        goto Cleanup;
    }

    hRes = D3DCompile2(
        Contents->lpVtbl->GetBufferPointer(Contents),
        Contents->lpVtbl->GetBufferSize(Contents),
        InFileA,
        NULL,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPoint,
        Target,
        0, 0, 0, NULL, 0,
        &Code,
        &ErrMsg);
    if (hRes)
    {
        Logging("D3DCompile2", hRes);
        goto Cleanup;
    }
    if (ErrMsg)
    {
        printf("D3DCompile2 Error: %s\n", (PBYTE)ErrMsg->lpVtbl->GetBufferPointer(ErrMsg));
        goto Cleanup;
    }

    fopen_s(&file, OutFile, "w");

    PBYTE OutBuf = Code->lpVtbl->GetBufferPointer(Code);
    SIZE_T OutLen = Code->lpVtbl->GetBufferSize(Code);

    hRes = D3DDisassemble(OutBuf, OutLen, 0, NULL, &Disassembly);

    fprintf(file, "#if 0\n");

    if (!hRes && Disassembly)
        fprintf(file, "%s", (PBYTE)Disassembly->lpVtbl->GetBufferPointer(Disassembly));
    else
        fprintf(file, "Disassembly failed\n");

    fprintf(file, "#endif\n\n");

    fprintf(file, "const BYTE g_%s[] =\n{", EntryPoint);

    SIZE_T i = 0;
    while (TRUE)
    {
        if (i % 6 == 0)
            fprintf(file, "\n    ");
        fflush(file);
        fprintf(file, "%3u", OutBuf[i]);
        i++;
        if (i >= OutLen)
            break;
        fflush(file);
        fprintf(file, ", ");
    }
    fflush(file);
    fprintf(file, "\n};\n");

Cleanup:
    if (file)
        fclose(file);
    if (InFileW)
        free(InFileW);
    if (Disassembly)
        Disassembly->lpVtbl->Release(Disassembly);
    if (Contents)
        Contents->lpVtbl->Release(Contents);
    if (Code)
        Code->lpVtbl->Release(Code);
    if (ErrMsg)
        ErrMsg->lpVtbl->Release(ErrMsg);
}
