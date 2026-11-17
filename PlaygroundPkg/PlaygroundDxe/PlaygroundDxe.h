/** @file
  Header file for Playground DXE Driver.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef PLAYGROUND_DXE_H_
#define PLAYGROUND_DXE_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/GlobalVariable.h>

#include <Library/ScreenLib.h>
#include <Library/FunctionLib.h>
#include <Library/NvramLib.h>
#include <Library/EfiAnaliserLib.h>

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
  );

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
  );

#endif // PLAYGROUND_DXE_H_
