#!/bin/bash
cd "$(dirname "$0")"

# Select target (DEBUG or RELEASE)
echo "[1] DEBUG (default)"
echo "[2] RELEASE"
read -r -p "Select Target [1-2]: " TARGET_CHOICE
TARGET="DEBUG"
[ "$TARGET_CHOICE" = "2" ] && TARGET="RELEASE"

CODE="edk2/Build/OvmfX64/${TARGET}_GCC/FV/OVMF_CODE.fd"
VARS_TEMPLATE="edk2/Build/OvmfX64/${TARGET}_GCC/FV/OVMF_VARS.fd"
VARS="OVMF-VARS.qemu.fd"

# Fall back to any available toolchain build directory if GCC not found
if [ ! -f "$CODE" ]; then
    MATCH=$(ls -1 edk2/Build/OvmfX64/${TARGET}_*/FV/OVMF_CODE.fd 2>/dev/null | head -n 1)
    if [ -n "$MATCH" ]; then
        CODE="$MATCH"
        VARS_TEMPLATE="${MATCH%OVMF_CODE.fd}OVMF_VARS.fd"
    fi
fi

if [ ! -f "$CODE" ]; then
    echo "Error: $CODE not found. Build the project first."
    exit 1
fi

# Initialize persistent NVRAM variable store if missing
if [ ! -f "$VARS" ]; then
    cp "$VARS_TEMPLATE" "$VARS"
fi

# Ensure simulated EFI System Partition (ESP) exists
mkdir -p esp/EFI/BOOT
if [ ! -f "esp/EFI/BOOT/BOOTX64.EFI" ]; then
    SHELL_BIN=$(ls -1 edk2/Build/OvmfX64/${TARGET}_*/X64/Shell.efi 2>/dev/null | head -n 1)
    if [ -n "$SHELL_BIN" ]; then
        cp "$SHELL_BIN" "esp/EFI/BOOT/BOOTX64.EFI"
    else
        echo "Placeholder UEFI Bootloader" > "esp/EFI/BOOT/BOOTX64.EFI"
    fi
fi

# Locate QEMU binary
if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "Error: 'qemu-system-x86_64' is required but not found in PATH."
    exit 1
fi

# Launch QEMU with OVMF firmware and simulated ESP
qemu-system-x86_64 \
    -machine q35,smm=on \
    -m 512M \
    -smp 2 \
    -drive if=pflash,format=raw,readonly=on,file="$CODE" \
    -drive if=pflash,format=raw,file="$VARS" \
    -drive file=fat:rw:esp,format=raw \
    -debugcon file:debug.log -global isa-debugcon.iobase=0x402 \
    -serial stdio
