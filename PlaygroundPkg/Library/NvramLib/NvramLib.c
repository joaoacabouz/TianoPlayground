/** @file
  NvramLib: Implements functions for finding, reading, listing, and querying NVRAM variables.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/NvramLib.h>

#define INITIAL_NAME_BUFFER_SIZE  128

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
  )
{
  EFI_STATUS Status;
  UINTN      DataSize;

  if ((gRT == NULL) || (VariableName == NULL) || (VendorGuid == NULL)) {
    return FALSE;
  }

  DataSize = 0;
  Status = gRT->GetVariable (
                  (CHAR16 *)VariableName,
                  (EFI_GUID *)VendorGuid,
                  NULL,
                  &DataSize,
                  NULL
                  );

  // If EFI_BUFFER_TOO_SMALL or EFI_SUCCESS, the variable exists
  return (Status == EFI_BUFFER_TOO_SMALL || Status == EFI_SUCCESS);
}

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
  )
{
  EFI_STATUS Status;
  UINTN      Size;
  VOID       *Buffer;
  UINT32     Attr;

  if ((gRT == NULL) || (VariableName == NULL) || (VendorGuid == NULL) || (DataSize == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Size = 0;
  Status = gRT->GetVariable (
                  (CHAR16 *)VariableName,
                  (EFI_GUID *)VendorGuid,
                  &Attr,
                  &Size,
                  NULL
                  );

  if (Status != EFI_BUFFER_TOO_SMALL) {
    return Status;
  }

  Buffer = AllocatePool (Size);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gRT->GetVariable (
                  (CHAR16 *)VariableName,
                  (EFI_GUID *)VendorGuid,
                  &Attr,
                  &Size,
                  Buffer
                  );

  if (EFI_ERROR (Status)) {
    FreePool (Buffer);
    return Status;
  }

  if (Attributes != NULL) {
    *Attributes = Attr;
  }
  *DataSize = Size;
  *Data     = Buffer;

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS Status;
  UINTN      NameBufferSize;
  CHAR16     *CurrentName;
  EFI_GUID   CurrentGuid;
  BOOLEAN    Found;

  if ((gRT == NULL) || (VariableName == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  // If VendorGuid is specified, test directly
  if (VendorGuid != NULL) {
    if (!NvramVariableExists (VariableName, VendorGuid)) {
      return EFI_NOT_FOUND;
    }
    if (FoundGuid != NULL) {
      CopyGuid (FoundGuid, VendorGuid);
    }
    if (DataSize != NULL && Data != NULL) {
      return NvramGetVariable (VariableName, VendorGuid, Attributes, DataSize, Data);
    }
    return EFI_SUCCESS;
  }

  // Iterate across all variables
  NameBufferSize = INITIAL_NAME_BUFFER_SIZE * sizeof (CHAR16);
  CurrentName    = AllocateZeroPool (NameBufferSize);
  if (CurrentName == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (&CurrentGuid, sizeof (EFI_GUID));
  Found = FALSE;

  while (TRUE) {
    UINTN Size = NameBufferSize;
    Status = gRT->GetNextVariableName (&Size, CurrentName, &CurrentGuid);

    if (Status == EFI_BUFFER_TOO_SMALL) {
      CHAR16 *NewBuffer = ReallocatePool (NameBufferSize, Size, CurrentName);
      if (NewBuffer == NULL) {
        FreePool (CurrentName);
        return EFI_OUT_OF_RESOURCES;
      }
      CurrentName    = NewBuffer;
      NameBufferSize = Size;
      Status         = gRT->GetNextVariableName (&Size, CurrentName, &CurrentGuid);
    }

    if (Status == EFI_NOT_FOUND) {
      break; // Reached end of variables
    }

    if (EFI_ERROR (Status)) {
      break;
    }

    if (StrCmp (CurrentName, VariableName) == 0) {
      Found = TRUE;
      if (FoundGuid != NULL) {
        CopyGuid (FoundGuid, &CurrentGuid);
      }
      break;
    }
  }

  FreePool (CurrentName);

  if (!Found) {
    return EFI_NOT_FOUND;
  }

  if ((DataSize != NULL) && (Data != NULL) && (FoundGuid != NULL)) {
    return NvramGetVariable (VariableName, FoundGuid, Attributes, DataSize, Data);
  }

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS Status;
  UINTN      NameBufferSize;
  CHAR16     *CurrentName;
  EFI_GUID   CurrentGuid;
  UINTN      MatchCount;
  UINT32     Attr;
  UINTN      DataSize;

  if (gRT == NULL) {
    return EFI_NOT_READY;
  }

  NameBufferSize = INITIAL_NAME_BUFFER_SIZE * sizeof (CHAR16);
  CurrentName    = AllocateZeroPool (NameBufferSize);
  if (CurrentName == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (&CurrentGuid, sizeof (EFI_GUID));
  MatchCount = 0;

  DEBUG ((DEBUG_INFO, "=== NVRAM Variable Listing ===\n"));

  while (TRUE) {
    UINTN Size = NameBufferSize;
    Status = gRT->GetNextVariableName (&Size, CurrentName, &CurrentGuid);

    if (Status == EFI_BUFFER_TOO_SMALL) {
      CHAR16 *NewBuffer = ReallocatePool (NameBufferSize, Size, CurrentName);
      if (NewBuffer == NULL) {
        FreePool (CurrentName);
        return EFI_OUT_OF_RESOURCES;
      }
      CurrentName    = NewBuffer;
      NameBufferSize = Size;
      Status         = gRT->GetNextVariableName (&Size, CurrentName, &CurrentGuid);
    }

    if (Status == EFI_NOT_FOUND) {
      break; // End of list
    }

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "[NvramLib] Error getting next variable: %r\n", Status));
      break;
    }

    // Filter by GUID if requested
    if ((FilterGuid != NULL) && !CompareGuid (&CurrentGuid, FilterGuid)) {
      continue;
    }

    // Filter by substring if requested
    if ((NameSubstring != NULL) && (StrStr (CurrentName, NameSubstring) == NULL)) {
      continue;
    }

    MatchCount++;

    // Query variable size and attributes
    Attr     = 0;
    DataSize = 0;
    gRT->GetVariable (CurrentName, &CurrentGuid, &Attr, &DataSize, NULL);

    DEBUG ((DEBUG_INFO, "  [%03d] %s (Size: %d bytes, GUID: %g)\n",
            MatchCount, CurrentName, DataSize, &CurrentGuid));
  }

  FreePool (CurrentName);

  if (FoundCount != NULL) {
    *FoundCount = MatchCount;
  }

  DEBUG ((DEBUG_INFO, "=== Total Matching NVRAM Variables: %d ===\n", MatchCount));

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS Status;
  UINTN      DataSize;
  VOID       *Data;
  UINT8      *ByteData;
  UINTN      Index;

  Status = NvramGetVariable (VariableName, VendorGuid, NULL, &DataSize, &Data);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "[NvramLib] Cannot dump variable %s: %r\n", VariableName, Status));
    return Status;
  }

  ByteData = (UINT8 *)Data;
  DEBUG ((DEBUG_INFO, "--- Hex Dump of %s (%d bytes) ---\n", VariableName, DataSize));

  for (Index = 0; Index < DataSize; Index++) {
    if ((Index > 0) && (Index % 16 == 0)) {
      DEBUG ((DEBUG_INFO, "\n"));
    }
    DEBUG ((DEBUG_INFO, "%02x ", ByteData[Index]));
  }
  DEBUG ((DEBUG_INFO, "\n--------------------------------\n"));

  FreePool (Data);
  return EFI_SUCCESS;
}

/**
  Sets or creates an NVRAM variable.

  @param[in] VariableName    Null-terminated Unicode variable name.
  @param[in] VendorGuid      Pointer to the vendor GUID.
  @param[in] Attributes      Variable attributes.
  @param[in] DataSize        Size of Data buffer in bytes.
  @param[in] Data            Pointer to data buffer. If DataSize is 0, deletes the variable.

  @retval EFI_SUCCESS        Variable was set successfully.
  @retval EFI_INVALID_PARAMETER Invalid parameter.
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
  )
{
  if ((gRT == NULL) || (VariableName == NULL) || (VendorGuid == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return gRT->SetVariable (
                (CHAR16 *)VariableName,
                (EFI_GUID *)VendorGuid,
                Attributes,
                DataSize,
                Data
                );
}

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
  )
{
  if ((gRT == NULL) || (VariableName == NULL) || (VendorGuid == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return gRT->SetVariable (
                (CHAR16 *)VariableName,
                (EFI_GUID *)VendorGuid,
                0,
                0,
                NULL
                );
}

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
  )
{
  EFI_STATUS     Status;
  CONST CHAR16   *Name;
  CONST EFI_GUID *Guid;
  UINTN          DataSize;
  VOID           *Data;

  if (BootCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Name = (VariableName != NULL) ? VariableName : PLAYGROUND_VARIABLE_NAME;
  Guid = (VendorGuid != NULL) ? VendorGuid : &gPlaygroundVariableGuid;

  Data     = NULL;
  DataSize = 0;

  Status = NvramGetVariable (Name, Guid, NULL, &DataSize, &Data);
  if (EFI_ERROR (Status) || (Data == NULL)) {
    return Status;
  }

  if (DataSize >= sizeof (UINT32)) {
    *BootCount = *(UINT32 *)Data;
  } else if (DataSize == sizeof (UINT16)) {
    *BootCount = *(UINT16 *)Data;
  } else if (DataSize == sizeof (UINT8)) {
    *BootCount = *(UINT8 *)Data;
  } else {
    *BootCount = 0;
  }

  FreePool (Data);
  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS  Status;
  CONST CHAR16 *Name;
  CONST EFI_GUID *Guid;
  UINT32      Count;
  UINTN       DataSize;
  VOID        *Data;
  UINT32      Attributes;

  Name = (VariableName != NULL) ? VariableName : PLAYGROUND_VARIABLE_NAME;
  Guid = (VendorGuid != NULL) ? VendorGuid : &gPlaygroundVariableGuid;

  Count    = 0;
  Data     = NULL;
  DataSize = 0;

  Status = NvramGetVariable (Name, Guid, &Attributes, &DataSize, &Data);
  if (!EFI_ERROR (Status) && (Data != NULL)) {
    if (DataSize >= sizeof (UINT32)) {
      Count = *(UINT32 *)Data;
    } else if (DataSize == sizeof (UINT16)) {
      Count = *(UINT16 *)Data;
    } else if (DataSize == sizeof (UINT8)) {
      Count = *(UINT8 *)Data;
    }
    FreePool (Data);
  } else {
    Count = 0;
  }

  Count++;

  Attributes = EFI_VARIABLE_NON_VOLATILE |
               EFI_VARIABLE_BOOTSERVICE_ACCESS |
               EFI_VARIABLE_RUNTIME_ACCESS;

  Status = NvramSetVariable (Name, Guid, Attributes, sizeof (UINT32), &Count);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[NvramLib] Failed to set boot count variable '%s': %r\n", Name, Status));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "[NvramLib] Boot count variable '%s' set to %d\n", Name, Count));

  if (BootCount != NULL) {
    *BootCount = Count;
  }

  return EFI_SUCCESS;
}
