# Architecture and Design

## 1. Overview

TianoPlayground provides an isolated, modular development environment built on top of the TianoCore EDK II framework. The central package, `PlaygroundPkg`, is structured to demonstrate clean UEFI DXE driver development, modular library separation, and dynamic platform firmware integration.

```
+-------------------------------------------------------------------+
|                        TianoPlayground Root                       |
|                                                                   |
|  +--------------------+   +-------------------+   +------------+  |
|  |   PlaygroundPkg    |   |     edk2 Core     |   | Build Host |  |
|  | - PlaygroundDxe    |   | - OvmfPkg (X64)   |   | - build.py |  |
|  | - ScreenLib        |   | - ArmVirtPkg (ARM)|   | - build.bat|  |
|  | - FunctionLib      |   | - MdePkg / Crypto |   | - build.sh |  |
|  | - NvramLib         |   |                   |   |            |  |
|  | - EfiAnaliserLib   |   |                   |   |            |  |
|  +--------------------+   +-------------------+   +------------+  |
+-------------------------------------------------------------------+
```

---

## 2. Dynamic Platform Injection

Standard EDK II development often requires directly modifying platform DSC and FDF files inside package directories (such as `OvmfPkgX64.dsc` and `OvmfPkgX64.fdf`). TianoPlayground decouples custom packages from upstream EDK II by employing dynamic in-memory/temporary injection through `build.py`:

1. **DSC Parsing**: `build.py` parses `PlaygroundPkg.dsc`, extracting declared `[LibraryClasses]` and `[Components]`.
2. **Template Synthesis**:
   - `edk2/OvmfPkg/OvmfPkgX64.dsc` (or `ArmVirtQemu.dsc`) is loaded into memory.
   - Custom include directories, library classes, and components are appended to the synthetic DSC.
   - The `FLASH_DEFINITION` tag is redirected to a custom FDF.
3. **FDF Injection**:
   - The platform FDF is loaded, and all DXE drivers in `PlaygroundPkg` are inserted directly before `[FV.FVMAIN_COMPACT]` (inside `DXEFV`).
4. **Execution**: The EDK II `build` utility runs against the generated temporary DSC.
5. **Cleanup**: Upon build completion (or error), the temporary DSC and FDF files are automatically deleted, leaving the working copy and submodules clean.

```
[PlaygroundPkg.dsc] ---> parse_playground_dsc()
                                  |
                                  v
[edk2 OvmfPkgX64.dsc]  ---> Inject Components & Libraries ---> [OvmfPkgX64Custom.dsc]
[edk2 OvmfPkgX64.fdf]  ---> Inject Drivers into DXEFV   ---> [OvmfPkgX64Custom.fdf]
                                                                     |
                                                                     v
                                                            edk2 build tool
                                                                     |
                                                                     v
                                                            OVMF.fd / OVMF_CODE.fd
```

---

## 3. UEFI DXE Lifecycle and Console Deferred Execution

### The Early DXE Constraint
During the early DXE dispatcher phase, drivers execute based on their Dependency Expression (`[Depex]`). When `[Depex] TRUE` is evaluated, the DXE core executes the driver entry point immediately upon dispatch.

At this early stage:
- The Console Splitter, Graphics Output Protocol (GOP), and text consoles (ConOut) are not yet initialized or connected by BDS (Boot Device Selection).
- `gST->ConOut` is `NULL`.
- Directly invoking `Print()` during entry point execution results in an assertion failure:
  ```
  ASSERT ...\UefiLibPrint.c: Console != NULL
  ```

### The ReadyToBoot Pattern
To display screen output safely while retaining standard DXE driver architecture, `PlaygroundDxe` utilizes the deferred `ReadyToBoot` event pattern:

```
[DXE Phase Start]
       |
       v
PlaygroundDxeEntryPoint()
       |
       +--> Log initialization via DEBUG ((DEBUG_INFO, ...)) (writes to debug I/O port 0x402)
       |
       +--> Register ReadyToBoot notification via EfiCreateEventReadyToBootEx()
       |
       v
[DXE Dispatch Continues]
       |
       v
[BDS Phase: Connect Consoles, Enumerate Disks, Prepare Boot]
       |
       v
Signal gEfiEventReadyToBootGuid
       |
       v
PlaygroundOnReadyToBoot() Callback Fires
       |
       +--> Close Event (one-time execution)
       |
       +--> Console is fully active: ConOut != NULL
       |
       +--> Execute ScreenLib, NvramLib, EfiAnaliserLib, FunctionLib safely
```

---

## 4. Package Structure and Organization

`PlaygroundPkg` follows strict EDK II package specification standards:

- **`PlaygroundPkg.dec`**:
  Defines the package name (`PlaygroundPkg`), unique package GUID, public include path (`Include`), and declarations for `ScreenLib`, `FunctionLib`, `NvramLib`, and `EfiAnaliserLib`.
- **`PlaygroundPkg.dsc`**:
  Specifies supported architectures (`X64`, `AARCH64`), defines library class mappings for standard MdePkg/CryptoPkg components and custom libraries, and lists build targets.
- **`Include/Library/`**:
  Contains public C headers exposed by each module for consumption by DXE drivers, runtime drivers, or UEFI applications.
- **`Library/`**:
  Contains the implementation and `.inf` metadata for each library feature.
- **`PlaygroundDxe/`**:
  The reference DXE driver incorporating and executing all modular libraries.
