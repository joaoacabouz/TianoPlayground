# TianoPlayground

![EDK2](https://img.shields.io/badge/EDK2-Tianocore-blue)
![Architecture](https://img.shields.io/badge/Architecture-X64%20%7C%20AARCH64-orange)
![Build Target](https://img.shields.io/badge/Target-DEBUG%20%7C%20RELEASE-success)
![Toolchains](https://img.shields.io/badge/Toolchains-VS2026%20%7C%20GCC%20%7C%20CLANGPDB-informational)
![License](https://img.shields.io/badge/License-BSD--2--Clause--Patent-green)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)

---

## Overview

**TianoPlayground** is a modular development workspace for UEFI firmware programming based on the TianoCore EDK II framework. It features **`PlaygroundPkg`**, an extensible UEFI package providing a clean DXE driver (`PlaygroundDxe`) coupled with a suite of reusable, feature-oriented libraries:

- **ScreenLib**: Centralized console UI, banners, foreground and background color formatting, and safe screen printing.
- **FunctionLib**: General DXE operational helpers, timing delays, reset logic, and interactive user key input (`wait4key`) with timeout.
- **NvramLib**: NVRAM variable management, persistence, machine boot counter tracking (`Playground`), querying, enumeration, and hex viewing.
- **EfiAnaliserLib**: EFI System Partition inspection, binary file loading, and cryptographic SHA-256 integrity verification.

The repository includes dynamic firmware synthesis tooling (`build.py`), enabling developers to build and test firmware modules inside complete OVMF (X64) and ArmVirtQemu (AARCH64) ROM images without permanently modifying upstream EDK II files.

---

## Project Structure

```
TianoPlayground/
|-- PlaygroundPkg/                 # Custom UEFI Package
|   |-- Include/
|   |   |-- Guid/
|   |   |   `-- PlaygroundVariable.h   # Playground GUID & variable definitions
|   |   `-- Library/
|   |       |-- ScreenLib.h        # Screen output and formatting interface
|   |       |-- FunctionLib.h      # DXE helpers & wait4key interface
|   |       |-- NvramLib.h         # NVRAM discovery, persistence & boot counter
|   |       `-- EfiAnaliserLib.h   # EFI partition & SHA-256 hash interface
|   |-- Library/
|   |   |-- ScreenLib/             # ScreenLib implementation & INF
|   |   |-- FunctionLib/           # FunctionLib implementation & INF
|   |   |-- NvramLib/              # NvramLib implementation & INF
|   |   `-- EfiAnaliserLib/        # EfiAnaliserLib implementation & INF
|   |-- PlaygroundDxe/             # Main DXE Driver implementation
|   |   |-- PlaygroundDxe.c
|   |   |-- PlaygroundDxe.h
|   |   `-- PlaygroundDxe.inf
|   |-- PlaygroundPkg.dec          # Package Declaration File
|   `-- PlaygroundPkg.dsc          # Platform Description File
|-- docs/                          # Project Documentation
|   |-- ARCHITECTURE.md            # Architecture, DXE lifecycle & injection model
|   |-- LIBRARIES.md               # Complete API specifications & usage examples
|   `-- BUILD_AND_TEST.md          # Setup, build steps, and QEMU emulation
|-- edk2/                          # Upstream TianoCore EDK II submodule
|-- build.bat                      # Windows interactive build launcher
|-- build.sh                       # Linux interactive build launcher
|-- build.py                       # Python dynamic platform builder
|-- qemu.bat                       # Windows QEMU launcher for OVMF firmware
|-- qemu.sh                        # Linux QEMU launcher for OVMF firmware
`-- README.md                      # Project root documentation
```

---

## Modular Libraries

Each library in `PlaygroundPkg` is designed as an independent feature module:

### ScreenLib
Protects against uninitialized console assertions during early boot and formats visual console output:
- Formatted title banners (`ScreenPrintBanner`)
- Color-coded status lines (`ScreenPrintSuccess`, `ScreenPrintWarning`, `ScreenPrintError`)
- Table-aligned status validation (`ScreenPrintStatus`)
- Direct text attribute and screen clearing controls

### FunctionLib
Provides essential execution and input utilities for DXE drivers:
- Indefinite blocking key wait (`FunctionWait4Key`)
- Timeout-driven key wait (`FunctionWait4KeyWithTimeout`) using relative timer events
- CPU stall (`FunctionStall`) and delay seconds (`FunctionDelaySeconds`)
- System resets (`FunctionResetSystem`) and hardware RTC retrieval (`FunctionGetSystemBootTime`)

### NvramLib
Simplifies NVRAM variable management via runtime services:
- Existence checks (`NvramVariableExists`)
- Dynamic variable allocation and retrieval (`NvramGetVariable`)
- Cross-GUID searching by variable name (`NvramFindVariable`)
- Variable table enumeration and substring filtering (`NvramListVariables`)
- Binary hex dumps (`NvramDumpVariableHex`)

### EfiAnaliserLib
Verifies the integrity of boot files and binaries stored on the EFI System Partition:
- Active ESP volume discovery (`EfiAnaliserFindEsp`)
- File existence verification on partition (`EfiAnaliserFileExists`)
- Complete binary file ingestion into memory pools (`EfiAnaliserReadFile`)
- Cryptographic SHA-256 calculation (`EfiAnaliserCalculateSha256`) via `BaseCryptLib`
- File hash validation against binary digests and hex strings (`EfiAnaliserValidateFileHashString`)

---

## Quick Start

### 1. Requirements

Ensure the following tools are installed and present in your system `PATH`:
- **Windows**: Visual Studio 2026 (or 18.x), NASM, iASL, Python 3.8+, QEMU.
- **Linux**: GCC / Clang, NASM, iASL, Python 3.8+, QEMU.

### 2. Building the Firmware

Use the interactive launcher:

```cmd
# Windows
build.bat

# Linux
./build.sh
```

Or invoke `build.py` directly from the command line:

```bash
# Build X64 DEBUG OVMF image
python build.py --arch X64 --target DEBUG

# Clean build
python build.py --arch X64 --target DEBUG --clean
```

### 3. Running in QEMU

Run `qemu.bat` (Windows) or `qemu.sh` (Linux) to launch the compiled OVMF firmware containing `PlaygroundDxe` and all libraries:

```cmd
qemu.bat
```

On Linux:
```bash
chmod +x qemu.sh
./qemu.sh
```

The driver executes upon reaching the `ReadyToBoot` phase, printing the console banner, verifying NVRAM variables, inspecting the EFI partition, and prompting for interactive key input.

---

## Documentation Links

For comprehensive technical documentation, refer to:

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md): Deep dive into UEFI DXE lifecycle, ReadyToBoot event patterns, and dynamic platform injection.
- [docs/LIBRARIES.md](docs/LIBRARIES.md): Complete API function signatures, parameters, return codes, and code examples for all four libraries.
- [docs/BUILD_AND_TEST.md](docs/BUILD_AND_TEST.md): Detailed toolchain setup, standalone module compilation, and QEMU debugging instructions.

---

## License

This project and `PlaygroundPkg` are licensed under the [BSD-2-Clause-Patent License](LICENSE) (SPDX: `BSD-2-Clause-Patent`), the standard open-source license for TianoCore EDK II projects with explicit hardware patent grants. See [PlaygroundPkg/License.txt](PlaygroundPkg/License.txt) for the package license declaration.
