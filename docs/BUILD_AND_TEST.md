# Build and Verification Guide

## 1. Prerequisites and Environment Setup

To compile `PlaygroundPkg` and assemble bootable OVMF images, ensure the following components are installed and accessible:

| Tool | Windows Requirement | Linux Requirement | Notes |
| :--- | :--- | :--- | :--- |
| **Compiler** | Visual Studio 2026 (or 18.x) | GCC (gcc/g++) or Clang | On Windows, MSVC toolchain is auto-detected via `vswhere.exe`. |
| **Assembler** | NASM (2.15+) | NASM (2.15+) | Must be in system `PATH`. |
| **ACPI Tool** | iASL | iASL (acpica-tools) | Must be in system `PATH`. |
| **Interpreter** | Python 3.8+ | Python 3.8+ | Used for build orchestrator (`build.py`). |
| **Emulator** | QEMU for Windows (`qemu-system-x86_64`) | QEMU (`qemu-system-x86_64`) | Required for executing `qemu.bat` / `qemu.sh`. |

---

## 2. Interactive Build Guide

Interactive wrappers are provided for Windows (`build.bat`) and Linux/POSIX (`build.sh`). Both scripts validate prerequisites, offer interactive menu selections, and run the underlying builder.

### Windows Execution

Run `build.bat` from Command Prompt or PowerShell:

```cmd
build.bat
```

The interactive menu prompts for:
1. **Build Target**: `[1] DEBUG (default)` or `[2] RELEASE`
2. **Build Type**: `[1] Incremental (default)` or `[2] Clean build`

### Linux Execution

Run `build.sh` from bash:

```bash
chmod +x build.sh
./build.sh
```

---

## 3. Command Line Interface (`build.py`)

For automated environments, CI pipelines, or direct terminal usage, invoke the streamlined `build.py` directly:

```
usage: build.py [-h] [--arch {X64,AARCH64}] [--toolchain TOOLCHAIN]
                [--target {DEBUG,RELEASE}] [-c]

Simplified EDK2 OVMF X64 Builder for PlaygroundPkg

options:
  -h, --help            show this help message and exit
  --arch {X64,AARCH64}  Platform Architecture (default: X64)
  --toolchain TOOLCHAIN EDK2 toolchain tag (default: VS2026 on Windows, GCC on Linux)
  --target {DEBUG,RELEASE} EDK2 build target (DEBUG, RELEASE). Default: DEBUG
  -c, --clean           Perform a clean build before building
```

### Examples

- **Build X64 DEBUG with default toolchain (VS2026 on Windows, GCC on Linux)**:
  ```bash
  python build.py --target DEBUG
  ```
- **Perform a Clean Build**:
  ```bash
  python build.py --target DEBUG --clean
  ```
- **Build X64 RELEASE**:
  ```bash
  python build.py --target RELEASE
  ```

---

## 4. Standalone Module Build

If you wish to compile only the driver and libraries without baking the full OVMF firmware ROM, run the standard EDK II build command pointing to `PlaygroundPkg.dsc`:

```cmd
cd edk2
call edksetup.bat
set PACKAGES_PATH=%CD%;%CD%\..
build -p ../PlaygroundPkg/PlaygroundPkg.dsc -a X64 -t VS2026 -b DEBUG
```

Output binaries (`PlaygroundDxe.efi` and library archives) are generated in:
`edk2/Build/Playground/DEBUG_VS2026/X64/`

---

## 5. Emulating in QEMU

Run `qemu.bat` on Windows or `qemu.sh` on Linux to launch the compiled OVMF image in QEMU:

```cmd
qemu.bat
```

On Linux:
```bash
chmod +x qemu.sh
./qemu.sh
```

### Execution Details:
- **Code Image**: `edk2\Build\OvmfX64\DEBUG_VS2026\FV\OVMF_CODE.fd`
- **Variable Store**: Copied from `OVMF_VARS.fd` to `OVMF-VARS.qemu.fd`
- **Debug Port**: Captures port `0x402` to `debug.log`
- **Virtual Disk**: Exposes the `esp` directory as a FAT drive (`-drive driver=vvfat,rw=on,dir=esp`)

When the machine reaches `ReadyToBoot`:
1. `PlaygroundDxe` executes `PlaygroundOnReadyToBoot()`.
2. The `ScreenLib` banner renders on the GTK console screen.
3. `NvramLib` queries firmware variables.
4. `EfiAnaliserLib` validates the EFI system partition.
5. `FunctionLib` prompts the user with an interactive countdown (`wait4key`).

---

## 6. Troubleshooting

- **Missing NASM or iASL**: Ensure `C:\Program Files\NASM` and `C:\Program Files\IASL` are in the system `PATH`.
- **VS2026 Detection**: If Visual Studio is installed in a non-standard location, set `VS2026_PREFIX` or verify `vswhere.exe` is present in `%ProgramFiles(x86)%\Microsoft Visual Studio\Installer`.
- **Console Assertion (`Console != NULL`)**: Occurs if `Print()` is called during early DXE phase. Always defer console output to the `ReadyToBoot` callback via `EfiCreateEventReadyToBootEx`.
