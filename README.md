# Junkyard

Just for fun. Recipe to destroy your PC in one minute.

## List of files

* c: C files
  - KillDwm: Terminate DWM process by suspending WinLogon process.
  - MiniProcList: List all minimal processes based on handle count.

* sh: shell scripts
  - Build_LLVM_Mingw_AArch64: Build [llvm-mingw] for Windows 10 AArch64 only.

[llvm-mingw]: https://github.com/mstorsjo/llvm-mingw.git

* TypeLibrary: Generates type library files (.til) for Hex Rays Decompiler
using Type Information Library Utility provided in IDA Pro SDK.

  - ntddk.bat: generates ntddk.til
  - ntifs.bat: generates ntifs.til
  - wdm.bat: generates wdm.til
  - windows.bat: generates windows.til
  - winnt.bat: generates winnt.til

## License

Junkyard is licensed under the GNU General Public License v3 or later.
A full copy of the license is provided in [LICENSE](LICENSE).
