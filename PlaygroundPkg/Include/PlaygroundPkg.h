/** @file
  Master umbrella header for PlaygroundPkg.

  Includes all core GUIDs and library interfaces provided by PlaygroundPkg.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef PLAYGROUND_PKG_H_
#define PLAYGROUND_PKG_H_

#include <Uefi.h>

//
// Package Version
//
#define PLAYGROUND_PKG_VERSION_MAJOR  0
#define PLAYGROUND_PKG_VERSION_MINOR  1

//
// GUID Definitions
//
#include <Guid/PlaygroundVariable.h>

//
// Library Interfaces
//
#include <Library/ScreenLib.h>
#include <Library/FunctionLib.h>
#include <Library/NvramLib.h>
#include <Library/EfiAnaliserLib.h>

#endif // PLAYGROUND_PKG_H_
