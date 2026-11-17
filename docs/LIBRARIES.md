# Modular Libraries Reference

## 1. Overview

`PlaygroundPkg` organizes functionalities into four distinct, self-contained libraries. Each library encapsulates a specific operational domain, exposing clean public interfaces under `Include/Library/`.

| Library | Header | Primary Focus |
| :--- | :--- | :--- |
| **ScreenLib** | `Include/Library/ScreenLib.h` | Safe console display, banner formatting, color output, and status reporting |
| **FunctionLib** | `Include/Library/FunctionLib.h` | General DXE helpers, input key polling (`wait4key`), stalls, and system control |
| **NvramLib** | `Include/Library/NvramLib.h` | NVRAM variable searching, reading, listing, and hex inspection |
| **EfiAnaliserLib** | `Include/Library/EfiAnaliserLib.h` | EFI System Partition discovery, file reading, and cryptographic SHA-256 analysis |

---

## 2. ScreenLib

`ScreenLib` centralizes console output, protecting drivers from null-pointer dereferences on `ConOut` while providing formatted and color-coded status reporting.

### Public API

```c
VOID EFIAPI ScreenClear (VOID);
VOID EFIAPI ScreenSetColor (IN UINTN Attribute);
VOID EFIAPI ScreenSetColors (IN UINTN ForegroundColor, IN UINTN BackgroundColor);
VOID EFIAPI ScreenSetBackgroundColor (IN UINTN BackgroundColor);
VOID EFIAPI ScreenSetForegroundColor (IN UINTN ForegroundColor);
UINTN EFIAPI ScreenGetBackgroundColor (VOID);
UINTN EFIAPI ScreenGetForegroundColor (VOID);
VOID EFIAPI ScreenClearWithColor (IN UINTN ForegroundColor, IN UINTN BackgroundColor);
VOID EFIAPI ScreenClearWithBackground (IN UINTN BackgroundColor);
VOID EFIAPI ScreenResetColor (VOID);
VOID EFIAPI ScreenResetColors (VOID);
VOID EFIAPI ScreenPrintLine (IN CHAR16 Character, IN UINTN Length);
VOID EFIAPI ScreenPrintLineWithColor (IN CHAR16 Character, IN UINTN Length, IN UINTN Attribute);
VOID EFIAPI ScreenPrintBanner (IN CONST CHAR16 *Title);
VOID EFIAPI ScreenPrintBannerWithColor (IN CONST CHAR16 *Title, IN UINTN Attribute);
VOID EFIAPI ScreenPrintMessage (IN CONST CHAR16 *Format, ...);
VOID EFIAPI ScreenPrintColorMessage (IN UINTN Attribute, IN CONST CHAR16 *Format, ...);
VOID EFIAPI ScreenPrintSuccess (IN CONST CHAR16 *Format, ...);
VOID EFIAPI ScreenPrintWarning (IN CONST CHAR16 *Format, ...);
VOID EFIAPI ScreenPrintError (IN CONST CHAR16 *Format, ...);
VOID EFIAPI ScreenPrintStatus (IN CONST CHAR16 *Category, IN CONST CHAR16 *Message, IN EFI_STATUS Status);
```

### Background Color Management

UEFI console attributes support background colors 0-7 (`EFI_BLACK`, `EFI_BLUE`, `EFI_GREEN`, `EFI_CYAN`, `EFI_RED`, `EFI_MAGENTA`, `EFI_BROWN`, `EFI_LIGHTGRAY`). `ScreenLib` retains the active background color across formatting calls and `ScreenClear` calls:

- **`ScreenClearWithColor(Foreground, Background)`**: Sets the active colors and clears the entire screen buffer to the specified background color.
- **`ScreenSetBackgroundColor(Background)`**: Adjusts only the background color for subsequent text output while keeping current foreground.
- **`ScreenResetColor()`**: Resets foreground text to default while preserving the current screen background.
- **`ScreenResetColors()`**: Restores standard UEFI platform console state (`EFI_LIGHTGRAY` on `EFI_BLACK`).

### Usage Example

```c
#include <Library/ScreenLib.h>

VOID DisplayFirmwareStatus (EFI_STATUS Status) {
  // Paint full screen with classic UEFI Blue background
  ScreenClearWithColor (EFI_WHITE, EFI_BLUE);

  ScreenPrintBannerWithColor (L"Platform Health Monitor", EFI_TEXT_ATTR (EFI_YELLOW, EFI_BLUE));

  ScreenPrintSuccess (L"Core services operational.\n");
  ScreenPrintWarning (L"Secondary network adapter unlinked.\n");

  ScreenPrintStatus (L"Security Engine", L"Firmware Integrity", Status);

  // Restore standard platform console defaults when finished
  ScreenResetColors ();
}
```

---

## 3. FunctionLib

`FunctionLib` provides core operational helpers for DXE drivers, notably blocking and timeout-driven keyboard input functions.

### Public API

```c
EFI_STATUS EFIAPI FunctionWait4Key (
  IN  CONST CHAR16  *Prompt OPTIONAL,
  OUT EFI_INPUT_KEY *Key OPTIONAL
  );

EFI_STATUS EFIAPI FunctionWait4KeyWithTimeout (
  IN  CONST CHAR16  *Prompt OPTIONAL,
  IN  UINTN         TimeoutSeconds,
  OUT EFI_INPUT_KEY *Key OPTIONAL
  );

VOID EFIAPI FunctionStall (
  IN UINTN Microseconds
  );

VOID EFIAPI FunctionDelaySeconds (
  IN UINTN Seconds
  );

VOID EFIAPI FunctionResetSystem (
  IN EFI_RESET_TYPE ResetType
  );

EFI_STATUS EFIAPI FunctionGetSystemBootTime (
  OUT EFI_TIME *Time
  );
```

### Function Details

- **`FunctionWait4Key`**: Displays a prompt (defaulting to `"Press any key to continue..."` if `Prompt` is `NULL`), flushes stale keystrokes, and pauses execution until a key is pressed. Used to pause screen-by-screen output.
- **`FunctionWait4KeyWithTimeout`**: Displays a prompt and utilizes an `EVT_TIMER` event in conjunction with `WaitForKey` to wait up to `TimeoutSeconds`. Returns `EFI_TIMEOUT` if the timer triggers before a key stroke occurs.

### Usage Example

```c
#include <Library/FunctionLib.h>
#include <Library/ScreenLib.h>

VOID PromptUserWithTimeout (VOID) {
  EFI_STATUS    Status;
  EFI_INPUT_KEY Key;

  ScreenPrintMessage (L"Press any key within 5 seconds to enter configuration...\n");
  Status = FunctionWait4KeyWithTimeout (NULL, 5, &Key);

  if (Status == EFI_SUCCESS) {
    ScreenPrintSuccess (L"Key detected: ScanCode=0x%04x Unicode='%c'\n", Key.ScanCode, Key.UnicodeChar);
  } else if (Status == EFI_TIMEOUT) {
    ScreenPrintMessage (L"Timeout expired. Proceeding with standard boot.\n");
  }
}
```

---

## 4. NvramLib

`NvramLib` manages UEFI Non-Volatile RAM variable interactions, allowing callers to locate, retrieve, enumerate, and inspect variable data.

### Public API

```c
BOOLEAN EFIAPI NvramVariableExists (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid
  );

EFI_STATUS EFIAPI NvramGetVariable (
  IN  CONST CHAR16   *VariableName,
  IN  CONST EFI_GUID *VendorGuid,
  OUT UINT32         *Attributes OPTIONAL,
  OUT UINTN          *DataSize,
  OUT VOID           **Data
  );

EFI_STATUS EFIAPI NvramSetVariable (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid,
  IN UINT32         Attributes,
  IN UINTN          DataSize,
  IN VOID           *Data
  );

EFI_STATUS EFIAPI NvramDeleteVariable (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid
  );

EFI_STATUS EFIAPI NvramFindVariable (
  IN  CONST CHAR16   *VariableName,
  IN  CONST EFI_GUID *VendorGuid OPTIONAL,
  OUT EFI_GUID       *FoundGuid OPTIONAL,
  OUT UINT32         *Attributes OPTIONAL,
  OUT UINTN          *DataSize OPTIONAL,
  OUT VOID           **Data OPTIONAL
  );

EFI_STATUS EFIAPI NvramListVariables (
  IN  CONST EFI_GUID *FilterGuid OPTIONAL,
  IN  CONST CHAR16   *NameSubstring OPTIONAL,
  OUT UINTN          *FoundCount OPTIONAL
  );

EFI_STATUS EFIAPI NvramDumpVariableHex (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid
  );

EFI_STATUS EFIAPI NvramGetBootCount (
  IN  CONST CHAR16   *VariableName OPTIONAL,
  IN  CONST EFI_GUID *VendorGuid OPTIONAL,
  OUT UINT32         *BootCount
  );

EFI_STATUS EFIAPI NvramIncrementBootCount (
  IN  CONST CHAR16   *VariableName OPTIONAL,
  IN  CONST EFI_GUID *VendorGuid OPTIONAL,
  OUT UINT32         *BootCount OPTIONAL
  );
```

### Function Details

- **`NvramIncrementBootCount`**: Reads the persistent `Playground` boot counter (or a custom variable), creates it with `EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS` if not yet present, increments the `UINT32` count by 1, persists it to flash storage, and returns the updated count.
- **`NvramGetBootCount`**: Reads the current value of the boot counter without incrementing.
- **`NvramFindVariable`**: If `VendorGuid` is omitted (`NULL`), searches iteratively across all registered vendor GUIDs using `GetNextVariableName`.
- **`NvramListVariables`**: Enumerates variables in NVRAM and applies optional GUID or substring filters (e.g., finding all `"Boot"` variables).

### Usage Example

```c
#include <Library/NvramLib.h>
#include <Guid/GlobalVariable.h>
#include <Guid/PlaygroundVariable.h>

VOID InspectAndCountBoots (VOID) {
  EFI_STATUS Status;
  UINT32     BootCount;
  UINTN      TotalMatches;

  // Increment and persist the boot counter across platform restarts
  Status = NvramIncrementBootCount (PLAYGROUND_VARIABLE_NAME, &gPlaygroundVariableGuid, &BootCount);
  if (!EFI_ERROR (Status)) {
    ScreenPrintSuccess (L"Machine boot count: #%d\n", BootCount);
  }

  // Check standard UEFI variable
  if (NvramVariableExists (L"BootOrder", &gEfiGlobalVariableGuid)) {
    NvramDumpVariableHex (L"BootOrder", &gEfiGlobalVariableGuid);
  }

  // List all variables containing the substring "Setup"
  TotalMatches = 0;
  NvramListVariables (NULL, L"Setup", &TotalMatches);
}
```

---

## 5. EfiAnaliserLib

`EfiAnaliserLib` validates files on the active EFI System Partition (ESP) and performs cryptographic SHA-256 hash calculations via `BaseCryptLib`.

### Public API

```c
EFI_STATUS EFIAPI EfiAnaliserFindEsp (
  OUT EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **FileSystem
  );

BOOLEAN EFIAPI EfiAnaliserFileExists (
  IN CONST CHAR16 *FilePath
  );

EFI_STATUS EFIAPI EfiAnaliserReadFile (
  IN  CONST CHAR16 *FilePath,
  OUT VOID         **FileBuffer,
  OUT UINTN        *FileSize
  );

EFI_STATUS EFIAPI EfiAnaliserCalculateSha256 (
  IN  CONST VOID *Data,
  IN  UINTN      DataSize,
  OUT UINT8      Hash[SHA256_DIGEST_SIZE]
  );

VOID EFIAPI EfiAnaliserFormatHashString (
  IN  CONST UINT8 Hash[SHA256_DIGEST_SIZE],
  OUT CHAR16      HashString[SHA256_STRING_BUFFER_SIZE]
  );

EFI_STATUS EFIAPI EfiAnaliserValidateFileHash (
  IN  CONST CHAR16 *FilePath,
  IN  CONST UINT8  ExpectedSha256[SHA256_DIGEST_SIZE],
  OUT BOOLEAN      *IsMatch
  );

EFI_STATUS EFIAPI EfiAnaliserValidateFileHashString (
  IN  CONST CHAR16 *FilePath,
  IN  CONST CHAR16 *ExpectedHashHex,
  OUT BOOLEAN      *IsMatch
  );

EFI_STATUS EFIAPI EfiAnaliserInspectFile (
  IN  CONST CHAR16 *FilePath,
  OUT UINTN        *FileSize OPTIONAL,
  OUT UINT8        Hash[SHA256_DIGEST_SIZE] OPTIONAL,
  OUT CHAR16       HashString[SHA256_STRING_BUFFER_SIZE] OPTIONAL
  );
```

### Usage Example

```c
#include <Library/EfiAnaliserLib.h>
#include <Library/ScreenLib.h>

VOID VerifyBootloaderIntegrity (VOID) {
  EFI_STATUS Status;
  UINTN      Size = 0;
  CHAR16     HexHash[SHA256_STRING_BUFFER_SIZE];
  BOOLEAN    Match = FALSE;

  CONST CHAR16 *TargetFile = L"\\EFI\\BOOT\\BOOTX64.EFI";
  CONST CHAR16 *ExpectedHash = L"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

  Status = EfiAnaliserInspectFile (TargetFile, &Size, NULL, HexHash);
  if (!EFI_ERROR (Status)) {
    ScreenPrintSuccess (L"File %s verified (%d bytes)\n", TargetFile, Size);
    ScreenPrintMessage (L"SHA-256: %s\n", HexHash);

    EfiAnaliserValidateFileHashString (TargetFile, ExpectedHash, &Match);
    ScreenPrintStatus (L"Signature Check", L"Golden Hash Match", Match ? EFI_SUCCESS : EFI_SECURITY_VIOLATION);
  } else {
    ScreenPrintError (L"File %s missing on EFI system partition.\n", TargetFile);
  }
}
```
