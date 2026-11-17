/** @file
  EfiAnaliserLib: Validates presence of EFI applications/files on EFI partitions
  and verifies their cryptographic SHA-256 hashes.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef EFI_ANALISER_LIB_H_
#define EFI_ANALISER_LIB_H_

#include <Uefi.h>
#include <Guid/FileInfo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#define SHA256_DIGEST_SIZE         32
#define SHA256_STRING_BUFFER_SIZE  65

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Validates a file on the EFI partition against an expected 64-character hex string.

  @param[in]  FilePath        Path to the file on the EFI partition.
  @param[in]  ExpectedHashHex Expected 64-character hex string.
  @param[out] IsMatch         Set to TRUE if the hash matches, FALSE otherwise.

  @retval EFI_SUCCESS         File was read and hash compared successfully.
  @retval Other               Error reading file.
**/
EFI_STATUS
EFIAPI
EfiAnaliserValidateFileHashString (
  IN  CONST CHAR16 *FilePath,
  IN  CONST CHAR16 *ExpectedHashHex,
  OUT BOOLEAN      *IsMatch
  );

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
  );

#endif // EFI_ANALISER_LIB_H_
