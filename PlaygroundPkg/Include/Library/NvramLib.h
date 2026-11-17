/** @file
  NvramLib: Provides functions for finding, reading, listing, querying,
  and managing UEFI NVRAM variables, including machine boot counting.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef NVRAM_LIB_H_
#define NVRAM_LIB_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Guid/PlaygroundVariable.h>

/**
  Checks if a specific NVRAM variable exists.

  @param[in] VariableName    Null-terminated Unicode variable name.
  @param[in] VendorGuid      Pointer to the vendor GUID.

  @retval TRUE               The variable exists.
  @retval FALSE              The variable was not found or error occurred.
**/
BOOLEAN
EFIAPI
NvramVariableExists (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid
  );

/**
  Retrieves the contents of an NVRAM variable, allocating memory for the caller.

  @param[in]  VariableName    Null-terminated Unicode variable name.
  @param[in]  VendorGuid      Pointer to the vendor GUID.
  @param[out] Attributes      Optional pointer to store variable attributes.
  @param[out] DataSize        Pointer to store the retrieved size in bytes.
  @param[out] Data            Pointer to store the allocated data buffer.

  @retval EFI_SUCCESS         Variable was found and retrieved.
  @retval EFI_NOT_FOUND       Variable does not exist.
  @retval EFI_OUT_OF_RESOURCES Memory allocation failed.
  @retval Other               Error from GetVariable.
**/
EFI_STATUS
EFIAPI
NvramGetVariable (
  IN  CONST CHAR16   *VariableName,
  IN  CONST EFI_GUID *VendorGuid,
  OUT UINT32         *Attributes OPTIONAL,
  OUT UINTN          *DataSize,
  OUT VOID           **Data
  );

/**
  Sets or creates an NVRAM variable.

  @param[in] VariableName    Null-terminated Unicode variable name.
  @param[in] VendorGuid      Pointer to the vendor GUID.
  @param[in] Attributes      Variable attributes (e.g., EFI_VARIABLE_NON_VOLATILE |
                             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS).
  @param[in] DataSize        Size of Data buffer in bytes.
  @param[in] Data            Pointer to data buffer. If DataSize is 0, deletes the variable.

  @retval EFI_SUCCESS        Variable was set successfully.
  @retval EFI_NOT_READY      Runtime services not available.
  @retval Other              Error from SetVariable.
**/
EFI_STATUS
EFIAPI
NvramSetVariable (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid,
  IN UINT32         Attributes,
  IN UINTN          DataSize,
  IN VOID           *Data
  );

/**
  Deletes an NVRAM variable.

  @param[in] VariableName    Null-terminated Unicode variable name.
  @param[in] VendorGuid      Pointer to the vendor GUID.

  @retval EFI_SUCCESS        Variable was deleted.
  @retval Other              Error deleting variable.
**/
EFI_STATUS
EFIAPI
NvramDeleteVariable (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid
  );

/**
  Searches for an NVRAM variable by name. If VendorGuid is NULL, searches
  across all vendor GUIDs registered in NVRAM.

  @param[in]  VariableName    Null-terminated Unicode variable name to find.
  @param[in]  VendorGuid      Optional vendor GUID. If NULL, searches all GUIDs.
  @param[out] FoundGuid       Optional pointer to receive the matched GUID.
  @param[out] Attributes      Optional pointer to receive variable attributes.
  @param[out] DataSize        Optional pointer to receive data size.
  @param[out] Data            Optional pointer to receive allocated data.

  @retval EFI_SUCCESS         Variable was found.
  @retval EFI_NOT_FOUND       Variable was not found.
  @retval Other               Error during lookup.
**/
EFI_STATUS
EFIAPI
NvramFindVariable (
  IN  CONST CHAR16   *VariableName,
  IN  CONST EFI_GUID *VendorGuid OPTIONAL,
  OUT EFI_GUID       *FoundGuid OPTIONAL,
  OUT UINT32         *Attributes OPTIONAL,
  OUT UINTN          *DataSize OPTIONAL,
  OUT VOID           **Data OPTIONAL
  );

/**
  Iterates through all NVRAM variables, optionally filtering by vendor GUID
  and/or substring match in the variable name, and prints their details.

  @param[in]  FilterGuid      Optional GUID to filter variables. If NULL, lists all.
  @param[in]  NameSubstring   Optional Unicode substring to filter names.
  @param[out] FoundCount      Optional pointer to receive the total matching count.

  @retval EFI_SUCCESS         Variable enumeration completed.
  @retval Other               Error during enumeration.
**/
EFI_STATUS
EFIAPI
NvramListVariables (
  IN  CONST EFI_GUID *FilterGuid OPTIONAL,
  IN  CONST CHAR16   *NameSubstring OPTIONAL,
  OUT UINTN          *FoundCount OPTIONAL
  );

/**
  Prints a formatted hex dump of the variable's binary contents.

  @param[in] VariableName    Null-terminated Unicode variable name.
  @param[in] VendorGuid      Pointer to the vendor GUID.

  @retval EFI_SUCCESS        Variable was retrieved and dumped.
  @retval Other              Error retrieving variable.
**/
EFI_STATUS
EFIAPI
NvramDumpVariableHex (
  IN CONST CHAR16   *VariableName,
  IN CONST EFI_GUID *VendorGuid
  );

/**
  Reads the machine boot counter stored in the specified NVRAM variable.

  @param[in]  VariableName    Optional variable name. If NULL, defaults to PLAYGROUND_VARIABLE_NAME (L"Playground").
  @param[in]  VendorGuid      Optional vendor GUID. If NULL, defaults to gPlaygroundVariableGuid.
  @param[out] BootCount       Pointer to receive the current boot count.

  @retval EFI_SUCCESS         Boot count retrieved successfully.
  @retval EFI_NOT_FOUND       Variable does not exist yet.
  @retval Other               Error retrieving variable.
**/
EFI_STATUS
EFIAPI
NvramGetBootCount (
  IN  CONST CHAR16   *VariableName OPTIONAL,
  IN  CONST EFI_GUID *VendorGuid OPTIONAL,
  OUT UINT32         *BootCount
  );

/**
  Increments and updates the persistent boot counter stored in the specified NVRAM variable.
  If the variable does not exist, it creates the variable with Non-Volatile attributes
  and initializes it to 1. If it exists, it increments the stored count by 1 and persists it.

  @param[in]  VariableName    Optional variable name. If NULL, defaults to PLAYGROUND_VARIABLE_NAME (L"Playground").
  @param[in]  VendorGuid      Optional vendor GUID. If NULL, defaults to gPlaygroundVariableGuid.
  @param[out] BootCount       Optional pointer to receive the updated boot count.

  @retval EFI_SUCCESS         Boot counter incremented and persisted in NVRAM.
  @retval Other               Error reading or writing the NVRAM variable.
**/
EFI_STATUS
EFIAPI
NvramIncrementBootCount (
  IN  CONST CHAR16   *VariableName OPTIONAL,
  IN  CONST EFI_GUID *VendorGuid OPTIONAL,
  OUT UINT32         *BootCount OPTIONAL
  );

#endif // NVRAM_LIB_H_
