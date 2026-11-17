/** @file
  GUID and definitions for PlaygroundPkg NVRAM variables.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef PLAYGROUND_VARIABLE_H_
#define PLAYGROUND_VARIABLE_H_

#define PLAYGROUND_VARIABLE_GUID \
  { 0x4f123b7e, 0x51c2, 0x4d68, { 0x89, 0xb2, 0x3d, 0x8a, 0x6e, 0x1f, 0x47, 0x90 } }

#define PLAYGROUND_VARIABLE_NAME  L"Playground"

extern EFI_GUID gPlaygroundVariableGuid;

#endif // PLAYGROUND_VARIABLE_H_
