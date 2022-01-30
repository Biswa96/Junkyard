# Windows PE AArch64 in Qemu

The procedure will show how to run Windows PE AArch64 in Qemu. But first we need
some files to download and prepare.

## Prerequisites:

Here are the list of files required for the procedure. Create a folder to store
all these. For example, `C:\WinPE` is used here.

* Windows Assessment and Deployment Kit (ADK) deployment tools and ADK Windows PE Add-ons.
Install ADK and PE ADD-ons using the setup executable or ISO files.
  - Official link: https://docs.microsoft.com/en-us/windows-hardware/get-started/adk-install
  - ADK ISO: https://software-download.microsoft.com/download/sg/22000.1.210604-1628.co_release_amd64fre_ADK.iso
  - ADK PE Add-ons ISO: https://software-download.microsoft.com/download/sg/22000.1.210604-1628.co_release_amd64fre_adkwinpeaddons.iso

* VirtIO drivers.
  - Official link: https://docs.fedoraproject.org/en-US/quick-docs/creating-windows-virtual-machines-using-virtio-drivers/
  - Repository: https://github.com/virtio-win/virtio-win-pkg-scripts/blob/master/README.md
  - Direct link: https://fedorapeople.org/groups/virt/virtio-win/direct-downloads/

* UEFI firmware. Use any one of these.
  - Arch package (edk2-armvirt): https://archlinux.org/packages/extra/any/edk2-armvirt/
  - Debian package (qemu-efi-aarch64): https://packages.debian.org/search?keywords=qemu-efi-aarch64
  - Ubuntu package (qemu-efi-aarch64): https://packages.ubuntu.com/search?keywords=qemu-efi-aarch64

* Run these commands in Command Prompt as administrator to create a VHD file.
There will be a WinPE.vhd file in `C:\WinPE` folder containing two partitions,
`Y:` will be EFI partition (500 MB) and `X:` will be system partition (100 GB).
The file will be dynamically allocated.

```cmd
diskpart
create vdisk file=C:\WinPE\WinPE.vhd maximum=102400 type=expandable
select vdisk file=C:\WinPE\WinPE.vhd
attach vdisk
clean
convert GPT
create partition primary size=500
format quick fs=fat32 label=EFI
assign letter=Y
create partition primary
format fs=ntfs quick label=OS
assign letter=X
exit
```

## Procedure:

1. From start menu, run Deployment and Imaging Tools Environment **as administrator**.
Execute the commands in later steps in this Command Prompt window.

2. Create a local copy of WinPE files.

```
copype arm64 C:\WinPE\WinPE_arm64
```

3. Mount the boot.wim file to modify.

```
mkdir C:\WinPE\mount
dism /Mount-Image /ImageFile:C:\WinPE\WinPE_arm64\media\sources\boot.wim /Index:1 /MountDir:C:\WinPE\mount
```

4. Extract all the ARM64 drivers from the previously downloaded VirtIO ISO file
in `C:\WinPE\arm64` folder. Install the drivers in that mounted WinPE.

```
dism /Image:C:\WinPE\mount /Add-Driver /Driver:C:\WinPE\arm64 /Recurse
```

5. Unmount the modified WinPE image.

```
dism /Unmount-Image /MountDir:C:\WinPE\mount /Commit
```

6. Apply the WinPE image to the mounted VHD file.

```
dism /Apply-Image /ImageFile:C:\WinPE\WinPE_arm64\media\sources\boot.wim /Index:1 /ApplyDir:X:\
```

7. Install EFI files to the EFI partition of mounted VHD file.

```
bcdboot X:\Windows /s Y: /f UEFI
```

8. Detach the mounted VHD file.

```
diskpart
select vdisk file=C:\WinPE\WinPE.vhd
detach vdisk
exit
```

9. Extract QEMU_CODE.fd and QEMU_VARS.fd files from previously downloaded UEFI
firmware package.

10. Run Qemu in C:\WinPE folder. I am using Qemu package from MSYS2/mingw-w64.
The command should not depend on the host OS. The commands allocates 2 CPU cores
and 1 GB memory.

```
qemu-system-aarch64 \
-M virt,virtualization=true \
-accel tcg,thread=multi \
-smp 2 \
-m 1024 \
-cpu cortex-a72 \
-serial stdio \
-device ramfb \
-device qemu-xhci \
-device usb-tablet \
-device usb-kbd \
-drive file=QEMU_CODE.fd,format=raw,if=pflash,index=0,readonly=on \
-drive file=QEMU_VARS.fd,format=raw,if=pflash,index=1 \
-device virtio-blk,drive=system \
-drive if=none,id=system,format=vpc,file=WinPE.vhd
```

If there is no output reboot the VM and press ESC to select correct boot order.
The output should be something like this.

```
BdsDxe: loading Boot0002 "UEFI Misc Device 2" from PciRoot(0x0)/Pci(0x3,0x0)
BdsDxe: starting Boot0002 "UEFI Misc Device 2" from PciRoot(0x0)/Pci(0x3,0x0)
```

At first boot, wpeinit process will be executed. Let it run. For customization,
edit `X:\Windows\System32\startnet.cmd` file.

## Article links:

* [Create bootable Windows PE media](https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/winpe-create-usb-bootable-drive)
* [Add and Remove Drivers to an offline Windows Image](https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/add-and-remove-drivers-to-an-offline-windows-image)
* [WinPE: Install on a hard drive (Flat boot or Non-RAM)](https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/winpe-install-on-a-hard-drive--flat-boot-or-non-ram)
* [WinPE: Mount and Customize](https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/winpe-mount-and-customize)
* https://kitsunemimi.pw/notes/posts/running-windows-10-for-arm64-in-a-qemu-virtual-machine.html
* https://github.com/jeremyd2019/winautoconfig/wiki/Windows-ARM64-VMs-with-QEMU
* https://withinrafael.com/2018/02/12/boot-arm64-builds-of-windows-10-in-qemu/
