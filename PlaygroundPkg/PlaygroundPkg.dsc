## @file
#  PlaygroundPkg Package Description File.
#
#  Copyright (c) 2026, Playground Contributors. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = Playground
  PLATFORM_GUID                  = a2674020-583c-488c-a078-029cf13599ef
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x0001001B
  OUTPUT_DIRECTORY               = Build/Playground
  SUPPORTED_ARCHITECTURES        = X64|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  # Basic EDK2 libraries
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  StackCheckLib|MdePkg/Library/StackCheckLibNull/StackCheckLibNull.inf

  # CryptoPkg library mappings for EfiAnaliserLib
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf

  # Playground custom libraries
  ScreenLib|PlaygroundPkg/Library/ScreenLib/ScreenLib.inf
  FunctionLib|PlaygroundPkg/Library/FunctionLib/FunctionLib.inf
  NvramLib|PlaygroundPkg/Library/NvramLib/NvramLib.inf
  EfiAnaliserLib|PlaygroundPkg/Library/EfiAnaliserLib/EfiAnaliserLib.inf

[Components]
  PlaygroundPkg/Library/ScreenLib/ScreenLib.inf
  PlaygroundPkg/Library/FunctionLib/FunctionLib.inf
  PlaygroundPkg/Library/NvramLib/NvramLib.inf
  PlaygroundPkg/Library/EfiAnaliserLib/EfiAnaliserLib.inf
  PlaygroundPkg/PlaygroundDxe/PlaygroundDxe.inf
