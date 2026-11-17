/** @file
  FunctionLib: Implements core utility functions for DXE drivers.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/FunctionLib.h>

/**
  Displays a prompt message ("Press any key to continue..." by default) and pauses
  execution until the user presses a key.

  @param[in]  Prompt      Optional custom prompt message. If NULL, defaults to
                          "Press any key to continue...".
  @param[out] Key         Optional pointer to store the pressed key.

  @retval EFI_SUCCESS     A key was pressed.
  @retval EFI_NOT_READY   Console input/output is not available.
  @retval Other           Error status from firmware service.
**/
EFI_STATUS
EFIAPI
FunctionWait4Key (
  IN  CONST CHAR16  *Prompt OPTIONAL,
  OUT EFI_INPUT_KEY *Key OPTIONAL
  )
{
  EFI_STATUS    Status;
  UINTN         Index;
  EFI_INPUT_KEY InputKey;
  CONST CHAR16  *Message;
  EFI_TPL       OriginalTpl;

  if ((gST == NULL) || (gST->ConIn == NULL)) {
    DEBUG ((DEBUG_WARN, "[FunctionLib] ConIn is not available.\n"));
    return EFI_NOT_READY;
  }

  // Display prompt message
  Message = (Prompt != NULL) ? Prompt : L"\nPress any key to continue...\n";
  if (gST->ConOut != NULL) {
    gST->ConOut->OutputString (gST->ConOut, (CHAR16 *)Message);
  }

  // Flush any pending/stale keystrokes before waiting
  while (gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey) == EFI_SUCCESS) {
  }

  //
  // In UEFI, WaitForEvent requires TPL_APPLICATION (4). When called inside an
  // event notification (e.g. ReadyToBoot callback at TPL_CALLBACK 8),
  // WaitForEvent returns EFI_UNSUPPORTED immediately.
  // We temporarily lower the TPL to TPL_APPLICATION to wait for user input,
  // then restore the original TPL upon return.
  //
  OriginalTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  gBS->RestoreTPL (TPL_APPLICATION);

  Status = EFI_UNSUPPORTED;
  if (gST->ConIn->WaitForKey != NULL) {
    Status = gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
    if (!EFI_ERROR (Status)) {
      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
    }
  }

  // If WaitForEvent failed or is unsupported, fall back to active polling
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "[FunctionLib] WaitForEvent returned %r, falling back to polling...\n", Status));
    while (TRUE) {
      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
      if (Status == EFI_SUCCESS) {
        break;
      }
      gBS->Stall (10000); // Stall 10ms
    }
  }

  // Restore the caller's original TPL
  if (OriginalTpl > TPL_APPLICATION) {
    gBS->RaiseTPL (OriginalTpl);
  }

  if (Key != NULL) {
    *Key = InputKey;
  }

  return EFI_SUCCESS;
}

/**
  Displays a prompt message and waits for a user key press with a specified timeout.

  @param[in]  Prompt          Optional custom prompt message.
  @param[in]  TimeoutSeconds  Number of seconds to wait before timing out.
  @param[out] Key             Optional pointer to store the pressed key.

  @retval EFI_SUCCESS         A key was pressed within the timeout period.
  @retval EFI_TIMEOUT         The timeout expired without a key press.
  @retval EFI_NOT_READY       Console input is not available.
  @retval Other               Error status from firmware service.
**/
EFI_STATUS
EFIAPI
FunctionWait4KeyWithTimeout (
  IN  CONST CHAR16  *Prompt OPTIONAL,
  IN  UINTN         TimeoutSeconds,
  OUT EFI_INPUT_KEY *Key OPTIONAL
  )
{
  EFI_STATUS    Status;
  UINTN         Index;
  EFI_EVENT     Events[2];
  EFI_EVENT     TimerEvent;
  EFI_INPUT_KEY InputKey;
  CONST CHAR16  *Message;
  EFI_TPL       OriginalTpl;

  if ((gST == NULL) || (gST->ConIn == NULL)) {
    DEBUG ((DEBUG_WARN, "[FunctionLib] ConIn is not ready for input.\n"));
    return EFI_NOT_READY;
  }

  // Display prompt message
  Message = (Prompt != NULL) ? Prompt : L"\nPress any key to continue...\n";
  if (gST->ConOut != NULL) {
    gST->ConOut->OutputString (gST->ConOut, (CHAR16 *)Message);
  }

  // Flush any stale keystrokes
  while (gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey) == EFI_SUCCESS) {
  }

  // If timeout is 0, check immediately
  if (TimeoutSeconds == 0) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
    if (!EFI_ERROR (Status) && (Key != NULL)) {
      *Key = InputKey;
    }
    return Status;
  }

  // Lower TPL to TPL_APPLICATION so timer and WaitForEvent work properly
  OriginalTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  gBS->RestoreTPL (TPL_APPLICATION);

  // Create timer event
  Status = gBS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &TimerEvent);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[FunctionLib] Failed to create timer event: %r\n", Status));
    if (OriginalTpl > TPL_APPLICATION) {
      gBS->RaiseTPL (OriginalTpl);
    }
    return Status;
  }

  // Set relative timer: 100ns units (1 second = 10,000,000 * 100ns)
  Status = gBS->SetTimer (
                  TimerEvent,
                  TimerRelative,
                  MultU64x32 (10000000ULL, (UINT32)TimeoutSeconds)
                  );
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (TimerEvent);
    if (OriginalTpl > TPL_APPLICATION) {
      gBS->RaiseTPL (OriginalTpl);
    }
    return Status;
  }

  Events[0] = gST->ConIn->WaitForKey;
  Events[1] = TimerEvent;

  Status = gBS->WaitForEvent (2, Events, &Index);
  gBS->CloseEvent (TimerEvent);

  if (!EFI_ERROR (Status) && (Index == 0)) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
    if (!EFI_ERROR (Status) && (Key != NULL)) {
      *Key = InputKey;
    }
  } else if (!EFI_ERROR (Status) && (Index == 1)) {
    Status = EFI_TIMEOUT;
  } else if (EFI_ERROR (Status)) {
    // Fallback polling loop if WaitForEvent failed
    UINTN ElapsedMs = 0;
    UINTN MaxMs = TimeoutSeconds * 1000;
    Status = EFI_TIMEOUT;
    while (ElapsedMs < MaxMs) {
      if (gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey) == EFI_SUCCESS) {
        if (Key != NULL) {
          *Key = InputKey;
        }
        Status = EFI_SUCCESS;
        break;
      }
      gBS->Stall (20000); // 20ms
      ElapsedMs += 20;
    }
  }

  // Restore original TPL
  if (OriginalTpl > TPL_APPLICATION) {
    gBS->RaiseTPL (OriginalTpl);
  }

  return Status;
}

/**
  Stalls the CPU execution for a given number of microseconds.

  @param[in] Microseconds Number of microseconds to stall.
**/
VOID
EFIAPI
FunctionStall (
  IN UINTN Microseconds
  )
{
  if (gBS != NULL) {
    gBS->Stall (Microseconds);
  }
}

/**
  Delays execution for a given number of seconds.

  @param[in] Seconds      Number of seconds to delay.
**/
VOID
EFIAPI
FunctionDelaySeconds (
  IN UINTN Seconds
  )
{
  UINTN Index;

  for (Index = 0; Index < Seconds; Index++) {
    FunctionStall (1000000); // 1,000,000 microseconds = 1 second
  }
}

/**
  Resets the system according to the specified reset type.

  @param[in] ResetType    Cold, Warm, Shutdown, or PlatformSpecific.
**/
VOID
EFIAPI
FunctionResetSystem (
  IN EFI_RESET_TYPE ResetType
  )
{
  if (gRT != NULL) {
    DEBUG ((DEBUG_INFO, "[FunctionLib] Resetting system (Type=%d)...\n", ResetType));
    gRT->ResetSystem (ResetType, EFI_SUCCESS, 0, NULL);
  }
}

/**
  Retrieves the current firmware/hardware RTC time.

  @param[out] Time        Pointer to EFI_TIME structure to populate.

  @retval EFI_SUCCESS     Time was retrieved successfully.
  @retval Other           Error retrieving time.
**/
EFI_STATUS
EFIAPI
FunctionGetSystemBootTime (
  OUT EFI_TIME *Time
  )
{
  if ((gRT == NULL) || (Time == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return gRT->GetTime (Time, NULL);
}
