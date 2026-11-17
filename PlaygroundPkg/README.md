# PlaygroundPkg

A modular EDK II package providing foundational DXE drivers and reusable firmware libraries for UEFI development and platform experimentation.

---

## Package Overview

`PlaygroundPkg` is designed for modularity, safety, and reusability across UEFI development environments (including OVMF and hardware platforms). It provides four independent libraries and an interactive demonstration DXE driver:

- **ScreenLib**: Centralized console UI management, safe formatted text rendering, banners, status line validation matrices, and comprehensive foreground and background color controls.
- **FunctionLib**: General-purpose DXE operational helpers, including blocking key wait (`FunctionWait4Key`), timeout-driven key wait (`FunctionWait4KeyWithTimeout`), CPU stalls, delays, RTC hardware time querying, and system resets.
- **NvramLib**: High-level UEFI Runtime Services variable management, existence testing, dynamic buffer allocation, variable enumeration, name substring searching, cross-GUID lookups, hex dumping, and persistent machine boot counting.
- **EfiAnaliserLib**: Storage volume analysis, EFI System Partition (ESP) SimpleFileSystem discovery, file presence testing, binary ingestion, and cryptographic SHA-256 integrity verification via `CryptoPkg`.
- **PlaygroundDxe**: A demonstration DXE driver showcasing the four libraries during the `ReadyToBoot` phase through a multi-screen presentation with distinct visual background themes.

---

## Directory Layout

```
PlaygroundPkg/
|-- Include/
|   |-- Guid/
|   |   `-- PlaygroundVariable.h       # GUID and variable definitions
|   |-- Library/
|   |   |-- ScreenLib.h                # Screen output and color management API
|   |   |-- FunctionLib.h              # DXE helpers & key input API
|   |   |-- NvramLib.h                 # NVRAM variable and boot counter API
|   |   `-- EfiAnaliserLib.h           # EFI partition and SHA-256 API
|   `-- PlaygroundPkg.h                # Package umbrella header
|-- Library/
|   |-- ScreenLib/                     # ScreenLib implementation & INF
|   |-- FunctionLib/                   # FunctionLib implementation & INF
|   |-- NvramLib/                      # NvramLib implementation & INF
|   `-- EfiAnaliserLib/                # EfiAnaliserLib implementation & INF
|-- PlaygroundDxe/                     # DXE driver implementation & INF
|-- PlaygroundPkg.dec                  # Package declaration file
|-- PlaygroundPkg.dsc                  # Package description file
|-- License.txt                        # BSD 2-Clause Patent License
`-- README.md                          # Package documentation
```

---

## Library Specifications

### ScreenLib
- **Class**: `ScreenLib`
- **Supported Module Types**: `DXE_DRIVER`, `DXE_RUNTIME_DRIVER`, `UEFI_APPLICATION`, `UEFI_DRIVER`
- **Header**: `Include/Library/ScreenLib.h`
- **Key APIs**:
  - `ScreenClear()` / `ScreenClearWithColor()` / `ScreenClearWithBackground()`
  - `ScreenSetColor()` / `ScreenSetColors()` / `ScreenSetForegroundColor()` / `ScreenSetBackgroundColor()`
  - `ScreenGetForegroundColor()` / `ScreenGetBackgroundColor()`
  - `ScreenResetColor()` / `ScreenResetColors()`
  - `ScreenPrintBanner()` / `ScreenPrintBannerWithColor()`
  - `ScreenPrintMessage()` / `ScreenPrintColorMessage()`
  - `ScreenPrintSuccess()` / `ScreenPrintWarning()` / `ScreenPrintError()`
  - `ScreenPrintStatus()` / `ScreenPrintLine()` / `ScreenPrintLineWithColor()`

### FunctionLib
- **Class**: `FunctionLib`
- **Supported Module Types**: `DXE_DRIVER`, `DXE_RUNTIME_DRIVER`, `UEFI_APPLICATION`, `UEFI_DRIVER`
- **Header**: `Include/Library/FunctionLib.h`
- **Key APIs**:
  - `FunctionWait4Key(Prompt, Key)`: Pauses execution until a key is pressed. Automatically manages TPL to avoid `EFI_UNSUPPORTED` during event callbacks.
  - `FunctionWait4KeyWithTimeout(Prompt, TimeoutSeconds, Key)`: Waits for user key press with an asynchronous timer event and polling fallback.
  - `FunctionStall(Microseconds)`: Stalls execution via `gBS->Stall`.
  - `FunctionDelaySeconds(Seconds)`: Delay loop for second-scale intervals.
  - `FunctionResetSystem(ResetType)`: System cold, warm, or shutdown reset.
  - `FunctionGetSystemBootTime(Time)`: Queries hardware RTC time via `gRT->GetTime`.

### NvramLib
- **Class**: `NvramLib`
- **Supported Module Types**: `DXE_DRIVER`, `DXE_RUNTIME_DRIVER`, `UEFI_APPLICATION`, `UEFI_DRIVER`
- **Header**: `Include/Library/NvramLib.h`
- **Key APIs**:
  - `NvramVariableExists(Name, Guid)`: Non-destructive existence check.
  - `NvramGetVariable(Name, Guid, Attr, Size, Data)`: Allocates pool memory and retrieves variable content.
  - `NvramSetVariable(Name, Guid, Attr, Size, Data)`: Sets or updates variable.
  - `NvramDeleteVariable(Name, Guid)`: Removes variable from NVRAM.
  - `NvramFindVariable(Name, Guid, FoundGuid, Attr, Size, Data)`: Finds a variable, optionally scanning across all GUIDs.
  - `NvramListVariables(FilterGuid, NameSubstring, FoundCount)`: Iterates and filters NVRAM variables.
  - `NvramDumpVariableHex(Name, Guid)`: Formatted hex dump to debug log.
  - `NvramGetBootCount(Name, Guid, BootCount)`: Reads persistent machine boot count.
  - `NvramIncrementBootCount(Name, Guid, BootCount)`: Atomically updates boot count with non-volatile attributes.

### EfiAnaliserLib
- **Class**: `EfiAnaliserLib`
- **Supported Module Types**: `DXE_DRIVER`, `DXE_RUNTIME_DRIVER`, `UEFI_APPLICATION`, `UEFI_DRIVER`
- **Header**: `Include/Library/EfiAnaliserLib.h`
- **Key APIs**:
  - `EfiAnaliserFindEsp(FileSystem)`: Locates active `EFI_SIMPLE_FILE_SYSTEM_PROTOCOL`.
  - `EfiAnaliserFileExists(FilePath)`: Checks whether a file exists on the partition.
  - `EfiAnaliserReadFile(FilePath, FileBuffer, FileSize)`: Loads file content into an allocated buffer.
  - `EfiAnaliserCalculateSha256(Data, DataSize, Hash)`: Generates 32-byte SHA-256 digest via `BaseCryptLib`.
  - `EfiAnaliserFormatHashString(Hash, HashString)`: Converts binary digest into a 64-character Unicode hex string.
  - `EfiAnaliserValidateFileHash(FilePath, ExpectedSha256, IsMatch)`: Validates file against binary digest.
  - `EfiAnaliserValidateFileHashString(FilePath, ExpectedHashHex, IsMatch)`: Validates file against hex string.
  - `EfiAnaliserInspectFile(FilePath, FileSize, Hash, HashString)`: High-level inspection returning size and hash.

---

## Integration Guide

### Including PlaygroundPkg in a Platform DSC

Add the package libraries to your platform's `[LibraryClasses]` section:

```ini
[LibraryClasses]
  ScreenLib|PlaygroundPkg/Library/ScreenLib/ScreenLib.inf
  FunctionLib|PlaygroundPkg/Library/FunctionLib/FunctionLib.inf
  NvramLib|PlaygroundPkg/Library/NvramLib/NvramLib.inf
  EfiAnaliserLib|PlaygroundPkg/Library/EfiAnaliserLib/EfiAnaliserLib.inf
```

Include the driver in the `[Components]` section:

```ini
[Components]
  PlaygroundPkg/PlaygroundDxe/PlaygroundDxe.inf
```

### Including PlaygroundDxe in a Platform FDF

Add the driver component to your firmware volume (e.g., `[FV.DXEFV]`):

```ini
INF PlaygroundPkg/PlaygroundDxe/PlaygroundDxe.inf
```

---

## License

This package is licensed under the **BSD 2-Clause Patent License**. See [License.txt](file:///c:/Users/JoaoAcabou/Desktop/TianoPlayground/PlaygroundPkg/License.txt) for complete license text.
