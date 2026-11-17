/** @file
  FunctionLib: Provides core utility functions for DXE drivers, including key input handling.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef FUNCTION_LIB_H_
#define FUNCTION_LIB_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

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
  );

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
  );

/**
  Stalls the CPU execution for a given number of microseconds.

  @param[in] Microseconds Number of microseconds to stall.
**/
VOID
EFIAPI
FunctionStall (
  IN UINTN Microseconds
  );

/**
  Delays execution for a given number of seconds.

  @param[in] Seconds      Number of seconds to delay.
**/
VOID
EFIAPI
FunctionDelaySeconds (
  IN UINTN Seconds
  );

/**
  Resets the system according to the specified reset type.

  @param[in] ResetType    Cold, Warm, Shutdown, or PlatformSpecific.
**/
VOID
EFIAPI
FunctionResetSystem (
  IN EFI_RESET_TYPE ResetType
  );

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
  );

#endif // FUNCTION_LIB_H_
