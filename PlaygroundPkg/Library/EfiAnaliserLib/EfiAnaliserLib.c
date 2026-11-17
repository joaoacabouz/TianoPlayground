/** @file
  EfiAnaliserLib: Implements EFI partition file inspection and cryptographic SHA-256 validation.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/EfiAnaliserLib.h>
#include <Library/BaseCryptLib.h>

/**
  Discovers an active EFI partition (SimpleFileSystem protocol instance).

  @param[out] FileSystem      Pointer to store the discovered protocol instance.

  @retval EFI_SUCCESS         Filesystem was found.
  @retval EFI_NOT_FOUND       No accessible EFI filesystem found.
  @retval Other               Error locating filesystem.
**/
EFI_STATUS
EFIAPI
EfiAnaliserFindEsp (
  OUT EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **FileSystem
  )
{
  EFI_STATUS                       Status;
  UINTN                            HandleCount;
  EFI_HANDLE                       *HandleBuffer;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;

  if (FileSystem == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HandleCount  = 0;
  HandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR (Status) || (HandleCount == 0)) {
    DEBUG ((DEBUG_WARN, "[EfiAnaliserLib] No SimpleFileSystem handles found: %r\n", Status));
    if (HandleBuffer != NULL) {
      FreePool (HandleBuffer);
    }
    return EFI_NOT_FOUND;
  }

  // Open protocol on the first handle
  Status = gBS->HandleProtocol (
                  HandleBuffer[0],
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&Fs
                  );

  FreePool (HandleBuffer);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[EfiAnaliserLib] Failed to open SimpleFileSystem protocol: %r\n", Status));
    return Status;
  }

  *FileSystem = Fs;
  return EFI_SUCCESS;
}

/**
  Checks if a specific file exists on the EFI partition.

  @param[in] FilePath         Path to the file (e.g., L"\\EFI\\BOOT\\BOOTX64.EFI").

  @retval TRUE                File exists and can be opened.
  @retval FALSE               File does not exist or cannot be accessed.
**/
BOOLEAN
EFIAPI
EfiAnaliserFileExists (
  IN CONST CHAR16 *FilePath
  )
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;
  EFI_FILE_PROTOCOL                *Root;
  EFI_FILE_PROTOCOL                *File;

  if (FilePath == NULL) {
    return FALSE;
  }

  Status = EfiAnaliserFindEsp (&Fs);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Status = Fs->OpenVolume (Fs, &Root);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Status = Root->Open (Root, &File, (CHAR16 *)FilePath, EFI_FILE_MODE_READ, 0);
  if (!EFI_ERROR (Status)) {
    File->Close (File);
    Root->Close (Root);
    return TRUE;
  }

  Root->Close (Root);
  return FALSE;
}

/**
  Reads the entire contents of a file from the EFI partition into an allocated buffer.

  @param[in]  FilePath        Path to the file on the EFI partition.
  @param[out] FileBuffer      Pointer to receive the allocated file content buffer.
  @param[out] FileSize        Pointer to receive the size of the file in bytes.

  @retval EFI_SUCCESS         File was read successfully.
  @retval EFI_NOT_FOUND       File was not found.
  @retval EFI_OUT_OF_RESOURCES Memory allocation failed.
  @retval Other               Error during file read.
**/
EFI_STATUS
EFIAPI
EfiAnaliserReadFile (
  IN  CONST CHAR16 *FilePath,
  OUT VOID         **FileBuffer,
  OUT UINTN        *FileSize
  )
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;
  EFI_FILE_PROTOCOL                *Root;
  EFI_FILE_PROTOCOL                *File;
  EFI_FILE_INFO                    *FileInfo;
  UINTN                            InfoSize;
  UINTN                            Size;
  VOID                             *Buffer;

  if ((FilePath == NULL) || (FileBuffer == NULL) || (FileSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EfiAnaliserFindEsp (&Fs);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Fs->OpenVolume (Fs, &Root);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Root->Open (Root, &File, (CHAR16 *)FilePath, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    Root->Close (Root);
    return Status;
  }

  // Query file info to determine size
  InfoSize = 0;
  FileInfo = NULL;
  Status   = File->GetInfo (File, &gEfiFileInfoGuid, &InfoSize, NULL);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FileInfo = AllocatePool (InfoSize);
    if (FileInfo == NULL) {
      File->Close (File);
      Root->Close (Root);
      return EFI_OUT_OF_RESOURCES;
    }

    Status = File->GetInfo (File, &gEfiFileInfoGuid, &InfoSize, FileInfo);
  }

  if (EFI_ERROR (Status) || (FileInfo == NULL)) {
    if (FileInfo != NULL) {
      FreePool (FileInfo);
    }
    File->Close (File);
    Root->Close (Root);
    return EFI_DEVICE_ERROR;
  }

  Size = (UINTN)FileInfo->FileSize;
  FreePool (FileInfo);

  Buffer = AllocatePool (Size);
  if (Buffer == NULL) {
    File->Close (File);
    Root->Close (Root);
    return EFI_OUT_OF_RESOURCES;
  }

  // Read entire file content
  Status = File->Read (File, &Size, Buffer);
  File->Close (File);
  Root->Close (Root);

  if (EFI_ERROR (Status)) {
    FreePool (Buffer);
    return Status;
  }

  *FileBuffer = Buffer;
  *FileSize   = Size;
  return EFI_SUCCESS;
}

/**
  Calculates the SHA-256 digest of a memory buffer.

  @param[in]  Data            Pointer to the data buffer.
  @param[in]  DataSize        Size of data in bytes.
  @param[out] Hash            32-byte array to receive the SHA-256 digest.

  @retval EFI_SUCCESS         Hash was calculated.
  @retval EFI_INVALID_PARAMETER Data or Hash is NULL.
**/
EFI_STATUS
EFIAPI
EfiAnaliserCalculateSha256 (
  IN  CONST VOID *Data,
  IN  UINTN      DataSize,
  OUT UINT8      Hash[SHA256_DIGEST_SIZE]
  )
{
  BOOLEAN Success;

  if ((Data == NULL && DataSize > 0) || (Hash == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Success = Sha256HashAll (Data, DataSize, Hash);
  return Success ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

/**
  Formats a 32-byte binary SHA-256 digest into a 64-character null-terminated Unicode hex string.

  @param[in]  Hash            32-byte SHA-256 digest.
  @param[out] HashString      Unicode buffer of at least 65 CHAR16 elements.
**/
VOID
EFIAPI
EfiAnaliserFormatHashString (
  IN  CONST UINT8 Hash[SHA256_DIGEST_SIZE],
  OUT CHAR16      HashString[SHA256_STRING_BUFFER_SIZE]
  )
{
  UINTN Index;

  if ((Hash == NULL) || (HashString == NULL)) {
    return;
  }

  for (Index = 0; Index < SHA256_DIGEST_SIZE; Index++) {
    UnicodeSPrint (
      HashString + (Index * 2),
      (SHA256_STRING_BUFFER_SIZE - (Index * 2)) * sizeof (CHAR16),
      L"%02x",
      (UINTN)Hash[Index]
      );
  }
  HashString[SHA256_DIGEST_SIZE * 2] = L'\0';
}

/**
  Validates a file on the EFI partition against an expected binary SHA-256 digest.

  @param[in]  FilePath        Path to the file on the EFI partition.
  @param[in]  ExpectedSha256  Expected 32-byte SHA-256 digest.
  @param[out] IsMatch         Set to TRUE if the hash matches, FALSE otherwise.

  @retval EFI_SUCCESS         File was read and hash compared successfully.
  @retval EFI_NOT_FOUND       File was not found.
  @retval Other               Error reading file.
**/
EFI_STATUS
EFIAPI
EfiAnaliserValidateFileHash (
  IN  CONST CHAR16 *FilePath,
  IN  CONST UINT8  ExpectedSha256[SHA256_DIGEST_SIZE],
  OUT BOOLEAN      *IsMatch
  )
{
  EFI_STATUS Status;
  VOID       *Buffer;
  UINTN      Size;
  UINT8      CalculatedHash[SHA256_DIGEST_SIZE];

  if ((FilePath == NULL) || (ExpectedSha256 == NULL) || (IsMatch == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *IsMatch = FALSE;

  Status = EfiAnaliserReadFile (FilePath, &Buffer, &Size);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EfiAnaliserCalculateSha256 (Buffer, Size, CalculatedHash);
  FreePool (Buffer);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  *IsMatch = (CompareMem (CalculatedHash, ExpectedSha256, SHA256_DIGEST_SIZE) == 0);
  return EFI_SUCCESS;
}

/**
  Validates a file on the EFI partition against an expected 64-character hex string.

  @param[in]  FilePath        Path to the file on the EFI partition.
  @param[in]  ExpectedHashHex Expected 64-character hex string.
  @param[out] IsMatch         Set to TRUE if the hash matches, FALSE otherwise.

  @retval EFI_SUCCESS         File was read and hash compared successfully.
  @retval EFI_NOT_FOUND       File was not found.
  @retval Other               Error reading file.
**/
EFI_STATUS
EFIAPI
EfiAnaliserValidateFileHashString (
  IN  CONST CHAR16 *FilePath,
  IN  CONST CHAR16 *ExpectedHashHex,
  OUT BOOLEAN      *IsMatch
  )
{
  EFI_STATUS Status;
  VOID       *Buffer;
  UINTN      Size;
  UINT8      CalculatedHash[SHA256_DIGEST_SIZE];
  CHAR16     CalculatedString[SHA256_STRING_BUFFER_SIZE];
  UINTN      Index;

  if ((FilePath == NULL) || (ExpectedHashHex == NULL) || (IsMatch == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *IsMatch = FALSE;

  Status = EfiAnaliserReadFile (FilePath, &Buffer, &Size);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EfiAnaliserCalculateSha256 (Buffer, Size, CalculatedHash);
  FreePool (Buffer);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  EfiAnaliserFormatHashString (CalculatedHash, CalculatedString);

  // Compare case-insensitively
  *IsMatch = TRUE;
  for (Index = 0; Index < (SHA256_DIGEST_SIZE * 2); Index++) {
    CHAR16 C1 = CalculatedString[Index];
    CHAR16 C2 = ExpectedHashHex[Index];

    if ((C1 >= L'a') && (C1 <= L'f')) {
      C1 = C1 - L'a' + L'A';
    }
    if ((C2 >= L'a') && (C2 <= L'f')) {
      C2 = C2 - L'a' + L'A';
    }

    if (C1 != C2) {
      *IsMatch = FALSE;
      break;
    }
  }

  return EFI_SUCCESS;
}

/**
  Inspects a file on the EFI partition, reporting its presence, size, and SHA-256 hash.

  @param[in]  FilePath        Path to the file on the EFI partition.
  @param[out] FileSize        Optional pointer to receive the file size.
  @param[out] Hash            Optional 32-byte array to receive binary digest.
  @param[out] HashString      Optional 65-character buffer to receive hex string.

  @retval EFI_SUCCESS         File was found and analyzed.
  @retval EFI_NOT_FOUND       File does not exist.
  @retval Other               Error reading file.
**/
EFI_STATUS
EFIAPI
EfiAnaliserInspectFile (
  IN  CONST CHAR16 *FilePath,
  OUT UINTN        *FileSize OPTIONAL,
  OUT UINT8        Hash[SHA256_DIGEST_SIZE] OPTIONAL,
  OUT CHAR16       HashString[SHA256_STRING_BUFFER_SIZE] OPTIONAL
  )
{
  EFI_STATUS Status;
  VOID       *Buffer;
  UINTN      Size;
  UINT8      Digest[SHA256_DIGEST_SIZE];

  if (FilePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_INFO, "[EfiAnaliserLib] Inspecting file: %s\n", FilePath));

  Status = EfiAnaliserReadFile (FilePath, &Buffer, &Size);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "[EfiAnaliserLib] File not found or read error: %s (%r)\n", FilePath, Status));
    return Status;
  }

  Status = EfiAnaliserCalculateSha256 (Buffer, Size, Digest);
  FreePool (Buffer);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[EfiAnaliserLib] Hash calculation failed: %r\n", Status));
    return Status;
  }

  if (FileSize != NULL) {
    *FileSize = Size;
  }
  if (Hash != NULL) {
    CopyMem (Hash, Digest, SHA256_DIGEST_SIZE);
  }
  if (HashString != NULL) {
    EfiAnaliserFormatHashString (Digest, HashString);
    DEBUG ((DEBUG_INFO, "[EfiAnaliserLib] File: %s | Size: %d bytes | SHA256: %s\n", FilePath, Size, HashString));
  }

  return EFI_SUCCESS;
}
