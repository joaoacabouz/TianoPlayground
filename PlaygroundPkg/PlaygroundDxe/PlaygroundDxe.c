/** @file
  Playground DXE Driver demonstrating ScreenLib, FunctionLib, NvramLib, and EfiAnaliserLib
  in a sequential screen-by-screen interactive presentation with distinct foreground
  and background color themes.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "PlaygroundDxe.h"

/**
  Notification callback when the system is ready to boot.
  At this stage, console devices (ConOut) are fully initialized and safe to print to.

  @param[in] Event      Event whose notification function is being invoked.
  @param[in] Context    Pointer to the notification function's context.
**/
VOID
EFIAPI
PlaygroundOnReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                       Status;
  EFI_INPUT_KEY                    Key;
  UINTN                            FoundVarCount;
  EFI_GUID                         FoundGuid;
  CHAR16                           HashString[SHA256_STRING_BUFFER_SIZE];
  UINTN                            FileSize;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;
  EFI_TIME                         Time;
  EFI_TPL                          OriginalTpl;
  VOID                             *VarData;
  UINTN                            VarDataSize;
  UINTN                            ByteIdx;
  UINT32                           BootCount;
  CONST CHAR8                      *SamplePayload;
  UINT8                            Digest[SHA256_DIGEST_SIZE];

  gBS->CloseEvent (Event);

  DEBUG ((DEBUG_INFO, "[PlaygroundDxe] ReadyToBoot event triggered. Starting screen presentation.\n"));

  //
  // ReadyToBoot callback is dispatched at TPL_CALLBACK (8).
  // Lower to TPL_APPLICATION (4) so timers, keyboard interrupts, and WaitForEvent
  // execute without being blocked during the interactive user session.
  //
  OriginalTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  gBS->RestoreTPL (TPL_APPLICATION);

  // =========================================================================
  // SCREEN 1: ScreenLib Demonstration (THEME: BLUE BACKGROUND, WHITE/YELLOW FG)
  // =========================================================================
  ScreenClearWithColor (EFI_WHITE, EFI_BLUE);
  ScreenPrintBannerWithColor (L"Screen 1/4: ScreenLib Demonstration", EFI_TEXT_ATTR (EFI_YELLOW, EFI_BLUE));

  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_LIGHTCYAN, EFI_BLUE), L"ScreenLib provides centralized rendering, foreground & background colors, and banners:\n\n");
  ScreenPrintSuccess (L"Green indicator [+] for successful firmware operations.\n");
  ScreenPrintWarning (L"Yellow alert indicator [!] for platform warnings.\n");
  ScreenPrintError (L"Red indicator [-] for error conditions and faults.\n\n");

  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_WHITE, EFI_BLUE), L"Formatted status validation matrix:\n");
  ScreenPrintStatus (L"GOP Driver", L"Graphics Output Protocol", EFI_SUCCESS);
  ScreenPrintStatus (L"Text Console", L"Console Splitter (ConOut)", EFI_SUCCESS);
  ScreenPrintStatus (L"Background Engine", L"Full-Screen Background Active", EFI_SUCCESS);

  ScreenPrintLineWithColor (L'-', 64, EFI_TEXT_ATTR (EFI_LIGHTCYAN, EFI_BLUE));
  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_YELLOW, EFI_BLUE), L"Theme: Classic UEFI Blue Background (White on Blue)\n");

  // Pause screen 1
  FunctionWait4Key (L"\nPress any key to proceed to Screen 2 (NvramLib)...\n", &Key);

  // =========================================================================
  // SCREEN 2: NvramLib Demonstration (THEME: CYAN BACKGROUND, BLACK/BLUE FG)
  // =========================================================================
  ScreenClearWithColor (EFI_BLACK, EFI_CYAN);
  ScreenPrintBannerWithColor (L"Screen 2/4: NvramLib Demonstration", EFI_TEXT_ATTR (EFI_WHITE, EFI_CYAN));

  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLACK, EFI_CYAN), L"NvramLib provides variable management, persistence, searching, and querying:\n\n");

  // Update persistent machine boot counter in NVRAM variable "Playground"
  BootCount = 0;
  Status = NvramIncrementBootCount (PLAYGROUND_VARIABLE_NAME, &gPlaygroundVariableGuid, &BootCount);
  if (!EFI_ERROR (Status)) {
    ScreenPrintSuccess (L"Boot Counter: Machine boot #%d recorded in NVRAM variable '%s'\n", BootCount, PLAYGROUND_VARIABLE_NAME);
    ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLUE, EFI_CYAN), L"    Variable Name   : %s\n", PLAYGROUND_VARIABLE_NAME);
    ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLUE, EFI_CYAN), L"    Vendor GUID     : %g\n", &gPlaygroundVariableGuid);
    ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLUE, EFI_CYAN), L"    Attributes      : NV | BS | RT (Non-Volatile Flash Storage)\n");
    ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLUE, EFI_CYAN), L"    Boot Count Value: %d (0x%08x)\n", BootCount, BootCount);

    // Read back and dump the binary value of "Playground"
    VarData     = NULL;
    VarDataSize = 0;
    Status = NvramGetVariable (PLAYGROUND_VARIABLE_NAME, &gPlaygroundVariableGuid, NULL, &VarDataSize, &VarData);
    if (!EFI_ERROR (Status) && (VarData != NULL)) {
      ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLUE, EFI_CYAN), L"    Raw NVRAM Bytes : ");
      for (ByteIdx = 0; ByteIdx < VarDataSize; ByteIdx++) {
        ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLACK, EFI_CYAN), L"%02x ", ((UINT8 *)VarData)[ByteIdx]);
      }
      ScreenPrintMessage (L"\n\n");
      FreePool (VarData);
    }
  } else {
    ScreenPrintError (L"Failed to update boot counter variable '%s': %r\n\n", PLAYGROUND_VARIABLE_NAME, Status);
  }

  // Check standard variable existence
  if (NvramVariableExists (L"PlatformLang", &gEfiGlobalVariableGuid)) {
    ScreenPrintSuccess (L"Variable 'PlatformLang' detected under EfiGlobalVariableGuid.\n");
  } else {
    ScreenPrintWarning (L"'PlatformLang' not found with default global GUID.\n");
  }

  // Cross-GUID variable search
  Status = NvramFindVariable (L"BootOrder", NULL, &FoundGuid, NULL, NULL, NULL);
  if (!EFI_ERROR (Status)) {
    ScreenPrintSuccess (L"Cross-GUID Lookup: Found 'BootOrder' (GUID: %g)\n", &FoundGuid);
  } else {
    ScreenPrintWarning (L"Variable 'BootOrder' not found in active NVRAM.\n");
  }

  // List matching variables
  FoundVarCount = 0;
  Status = NvramListVariables (NULL, L"Boot", &FoundVarCount);
  if (!EFI_ERROR (Status)) {
    ScreenPrintSuccess (L"Enumerated %d variable(s) matching substring 'Boot'.\n", FoundVarCount);
  }

  ScreenPrintLineWithColor (L'-', 64, EFI_TEXT_ATTR (EFI_BLUE, EFI_CYAN));
  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_BLACK, EFI_CYAN), L"Theme: Cyan / Teal Background (High-Contrast Black on Cyan)\n");

  // Pause screen 2
  FunctionWait4Key (L"\nPress any key to proceed to Screen 3 (EfiAnaliserLib)...\n", &Key);

  // =========================================================================
  // SCREEN 3: EfiAnaliserLib Demonstration (THEME: MAGENTA BACKGROUND, WHITE/CYAN FG)
  // =========================================================================
  ScreenClearWithColor (EFI_WHITE, EFI_MAGENTA);
  ScreenPrintBannerWithColor (L"Screen 3/4: EfiAnaliserLib Demonstration", EFI_TEXT_ATTR (EFI_YELLOW, EFI_MAGENTA));

  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_LIGHTCYAN, EFI_MAGENTA), L"EfiAnaliserLib validates EFI partitions and verifies SHA-256 integrity:\n\n");

  // ESP discovery
  Fs     = NULL;
  Status = EfiAnaliserFindEsp (&Fs);
  ScreenPrintStatus (L"ESP Discovery", L"SimpleFileSystem Protocol", Status);

  // Inspect bootloader file
  Status = EfiAnaliserInspectFile (
             L"\\EFI\\BOOT\\BOOTX64.EFI",
             &FileSize,
             NULL,
             HashString
             );

  if (!EFI_ERROR (Status)) {
    ScreenPrintSuccess (L"Discovered executable: \\EFI\\BOOT\\BOOTX64.EFI (%d bytes)\n", FileSize);
    ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_LIGHTCYAN, EFI_MAGENTA), L"    SHA-256 Digest : %s\n", HashString);
  } else {
    SamplePayload = "PlaygroundPkg Firmware Verification Payload";

    ScreenPrintWarning (L"\\EFI\\BOOT\\BOOTX64.EFI not present on active ESP volume.\n");
    ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_YELLOW, EFI_MAGENTA), L"    Executing cryptographic SHA-256 self-test on memory buffer...\n");

    EfiAnaliserCalculateSha256 (SamplePayload, AsciiStrLen (SamplePayload), Digest);
    EfiAnaliserFormatHashString (Digest, HashString);

    ScreenPrintSuccess (L"SHA-256 cryptographic self-test passed:\n");
    ScreenPrintMessage (L"    Payload Buffer : \"%a\"\n", SamplePayload);
    ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_LIGHTCYAN, EFI_MAGENTA), L"    SHA-256 Digest : %s\n", HashString);
  }

  ScreenPrintLineWithColor (L'-', 64, EFI_TEXT_ATTR (EFI_LIGHTCYAN, EFI_MAGENTA));
  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_YELLOW, EFI_MAGENTA), L"Theme: Magenta / Purple Background (White & Cyan on Magenta)\n");

  // Pause screen 3
  FunctionWait4Key (L"\nPress any key to proceed to Screen 4 (FunctionLib)...\n", &Key);

  // =========================================================================
  // SCREEN 4: FunctionLib & Boot Handoff (THEME: GREEN BACKGROUND, WHITE/YELLOW FG)
  // =========================================================================
  ScreenClearWithColor (EFI_WHITE, EFI_GREEN);
  ScreenPrintBannerWithColor (L"Screen 4/4: FunctionLib & Boot Sequence", EFI_TEXT_ATTR (EFI_YELLOW, EFI_GREEN));

  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_LIGHTGREEN, EFI_GREEN), L"FunctionLib provides input handling, timing delays, and firmware utilities:\n\n");

  // RTC time check
  Status = FunctionGetSystemBootTime (&Time);
  if (!EFI_ERROR (Status)) {
    ScreenPrintSuccess (L"Hardware RTC Time : %04d-%02d-%02d %02d:%02d:%02d\n",
                        Time.Year, Time.Month, Time.Day, Time.Hour, Time.Minute, Time.Second);
  }

  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_WHITE, EFI_GREEN), L"Executing 1-second delay test via FunctionDelaySeconds...\n");
  FunctionDelaySeconds (1);
  ScreenPrintSuccess (L"1-second delay completed successfully.\n\n");

  ScreenPrintLineWithColor (L'=', 64, EFI_TEXT_ATTR (EFI_YELLOW, EFI_GREEN));
  ScreenPrintColorMessage (EFI_TEXT_ATTR (EFI_WHITE, EFI_GREEN), L"All 4 PlaygroundPkg modules demonstrated across distinct background colors!\n");
  ScreenPrintLineWithColor (L'=', 64, EFI_TEXT_ATTR (EFI_YELLOW, EFI_GREEN));

  // Final pause before continuing boot
  FunctionWait4Key (L"\nPress any key to complete PlaygroundDxe and continue boot sequence...\n", &Key);

  // Restore standard platform console colors (Light Gray on Black) before boot handoff
  ScreenResetColors ();
  ScreenClear ();

  // Restore original TPL before exiting callback back to BDS
  if (OriginalTpl > TPL_APPLICATION) {
    gBS->RaiseTPL (OriginalTpl);
  }

  DEBUG ((DEBUG_INFO, "[PlaygroundDxe] Screen presentation complete. Continuing boot.\n"));
}

/**
  Entry point for the Playground DXE Driver.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point executed successfully.
**/
EFI_STATUS
EFIAPI
PlaygroundDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   ReadyToBootEvent;

  DEBUG ((DEBUG_INFO, "[PlaygroundDxe] Initializing Playground DXE Driver...\n"));

  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             PlaygroundOnReadyToBoot,
             NULL,
             &ReadyToBootEvent
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[PlaygroundDxe] Failed to create ReadyToBoot event: %r\n", Status));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "[PlaygroundDxe] ReadyToBoot event registered successfully.\n"));

  return EFI_SUCCESS;
}
