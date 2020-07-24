:: This source file is part of Junkyard repository.
:: Licensed under the terms of the GNU General Public License v3 or later.
:: Copyright (C) Biswapriyo Nath
::
:: winnt.bat: Generates winnt.til type library for Hex Rays Decompiler

@echo off
set ver=10.0.19041.0
set folder=%ProgramFiles(x86)%\Windows Kits\10\Include\%ver%
D:\IDA\tilib64.exe -c ^
-Cc1 ^
-Ci4 ^
-Cl4 ^
-Cvr ^
-D_WIN32 ^
-D_AMD64_ ^
-DAMD64 ^
-D_M_AMD64 ^
-DMSC_NOOPT ^
-DDBG=1 ^
-DDEPRECATED_DDK_FNUCTIONS=1 ^
-D_MSC_VER=1926 ^
-D_MSC_FULL_VER=192628807 ^
-DWINNT=1 ^
-DWINVER=_WIN32_WINNT ^
-D_WIN32_WINNT=0x0A00 ^
-D_WIN32_IE=0x0A00 ^
-DNTDDI_VERSION=WDK_NTDDI_VERSION ^
-DWDK_NTDDI_VERSION=NTDDI_WIN10_FE ^
-DNTDDI_WIN10_FE=0x0A00000A ^
-D_inline=inline ^
-D__inline=inline ^
-D__forceinline=inline ^
-D__volatile=volatile ^
-Dbool=uint8_t ^
-DSIZE_T=size_t ^
-DPSIZE_T=size_t* ^
-D"PCSTR=const char*" ^
-D"PCWSTR=const wchar_t*" ^
-D"ULONG=unsigned long" ^
-h"%folder%\um\winnt.h" ^
-I"%folder%\cppwinrt\winrt" ^
-I"%folder%\km" ^
-I"%folder%\km\crt" ^
-I"%folder%\shared" ^
-I"%folder%\ucrt" ^
-I"%folder%\um" ^
-I"%folder%\winrt" ^
-e ^
winnt.til
