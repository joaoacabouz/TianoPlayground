@echo off
setlocal
cd /d %~dp0

:: Select target (DEBUG or RELEASE)
echo [1] DEBUG (default)
echo [2] RELEASE
set /p TARGET_CHOICE="Select Target [1-2]: "
set "TARGET=DEBUG"
if "%TARGET_CHOICE%"=="2" set "TARGET=RELEASE"

set "CODE=%~dp0edk2\Build\OvmfX64\%TARGET%_VS2026\FV\OVMF_CODE.fd"
set "VARS_TEMPLATE=%~dp0edk2\Build\OvmfX64\%TARGET%_VS2026\FV\OVMF_VARS.fd"
set "VARS=%~dp0OVMF-VARS.qemu.fd"

if not exist "%CODE%" (
    echo Error: %CODE% not found. Build the project first.
    pause
    exit /b 1
)

:: Initialize persistent NVRAM variable store if missing
if not exist "%VARS%" copy /y "%VARS_TEMPLATE%" "%VARS%" >nul

:: Ensure simulated EFI System Partition (ESP) exists
if not exist "esp\EFI\BOOT" mkdir "esp\EFI\BOOT"
if not exist "esp\EFI\BOOT\BOOTX64.EFI" (
    if exist "edk2\Build\OvmfX64\%TARGET%_VS2026\X64\Shell.efi" (
        copy /y "edk2\Build\OvmfX64\%TARGET%_VS2026\X64\Shell.efi" "esp\EFI\BOOT\BOOTX64.EFI" >nul
    ) else (
        echo Placeholder UEFI Bootloader > "esp\EFI\BOOT\BOOTX64.EFI"
    )
)

:: Locate QEMU binary
set "QEMU=qemu-system-x86_64.exe"
where %QEMU% >nul 2>&1 || (
    if exist "C:\Program Files\qemu\qemu-system-x86_64.exe" (
        set "QEMU=C:\Program Files\qemu\qemu-system-x86_64.exe"
    )
)

:: Launch QEMU with OVMF firmware and simulated ESP
"%QEMU%" ^
    -machine q35,smm=on ^
    -m 512M ^
    -smp 2 ^
    -drive if=pflash,format=raw,readonly=on,file="%CODE%" ^
    -drive if=pflash,format=raw,file="%VARS%" ^
    -drive file=fat:rw:esp,format=raw ^
    -debugcon file:debug.log -global isa-debugcon.iobase=0x402 ^
    -serial stdio

pause
