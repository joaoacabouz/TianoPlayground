/** @file
  ScreenLib: Provides screen output, message formatting, and console UI functions
  with full foreground and background color management.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SCREEN_LIB_H_
#define SCREEN_LIB_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

/**
  Clears the console screen using the currently configured background color.
**/
VOID
EFIAPI
ScreenClear (
  VOID
  );

/**
  Sets the text color/attribute for the console screen.
  If Attribute contains only foreground bits (<= 0x0F), the current background color is retained.

  @param[in] Attribute    Text attribute (e.g., EFI_TEXT_ATTR (EFI_LIGHTGREEN, EFI_BLUE)).
**/
VOID
EFIAPI
ScreenSetColor (
  IN UINTN Attribute
  );

/**
  Sets both foreground and background colors for console output.

  @param[in] ForegroundColor  Foreground color (0-15, e.g. EFI_WHITE, EFI_YELLOW).
  @param[in] BackgroundColor  Background color (0-7, e.g. EFI_BLUE, EFI_BLACK).
**/
VOID
EFIAPI
ScreenSetColors (
  IN UINTN ForegroundColor,
  IN UINTN BackgroundColor
  );

/**
  Sets only the background color for subsequent console output and clearing,
  preserving the current foreground color.

  @param[in] BackgroundColor  Background color (0-7, e.g. EFI_BLUE, EFI_BLACK).
**/
VOID
EFIAPI
ScreenSetBackgroundColor (
  IN UINTN BackgroundColor
  );

/**
  Sets only the foreground color for subsequent console output,
  preserving the current background color.

  @param[in] ForegroundColor  Foreground color (0-15, e.g. EFI_WHITE, EFI_YELLOW).
**/
VOID
EFIAPI
ScreenSetForegroundColor (
  IN UINTN ForegroundColor
  );

/**
  Gets the currently active background color (0-7).

  @return Current background color.
**/
UINTN
EFIAPI
ScreenGetBackgroundColor (
  VOID
  );

/**
  Gets the currently active foreground color (0-15).

  @return Current foreground color.
**/
UINTN
EFIAPI
ScreenGetForegroundColor (
  VOID
  );

/**
  Clears the entire console screen with a specified background color and sets the foreground color.
  All subsequent output uses this background color.

  @param[in] ForegroundColor  Foreground color (0-15).
  @param[in] BackgroundColor  Background color (0-7).
**/
VOID
EFIAPI
ScreenClearWithColor (
  IN UINTN ForegroundColor,
  IN UINTN BackgroundColor
  );

/**
  Clears the entire console screen with a specified background color,
  preserving the current foreground color.

  @param[in] BackgroundColor  Background color (0-7).
**/
VOID
EFIAPI
ScreenClearWithBackground (
  IN UINTN BackgroundColor
  );

/**
  Resets the text color attribute to the active background and default foreground (Light Gray or White).
**/
VOID
EFIAPI
ScreenResetColor (
  VOID
  );

/**
  Resets all colors back to default UEFI platform state (Light Gray foreground on Black background).
**/
VOID
EFIAPI
ScreenResetColors (
  VOID
  );

/**
  Prints a decorative horizontal line of specified character and length.

  @param[in] Character    Character to repeat.
  @param[in] Length       Number of characters.
**/
VOID
EFIAPI
ScreenPrintLine (
  IN CHAR16 Character,
  IN UINTN  Length
  );

/**
  Prints a decorative horizontal line of specified character and length with a custom color.

  @param[in] Character    Character to repeat.
  @param[in] Length       Number of characters.
  @param[in] Attribute    Text attribute.
**/
VOID
EFIAPI
ScreenPrintLineWithColor (
  IN CHAR16 Character,
  IN UINTN  Length,
  IN UINTN  Attribute
  );

/**
  Prints a framed title banner to the console screen.

  @param[in] Title        Title string to display in the banner.
**/
VOID
EFIAPI
ScreenPrintBanner (
  IN CONST CHAR16 *Title
  );

/**
  Prints a framed title banner to the console screen with a custom color attribute.

  @param[in] Title        Title string to display in the banner.
  @param[in] Attribute    Text attribute (e.g. EFI_TEXT_ATTR (EFI_YELLOW, EFI_BLUE)).
**/
VOID
EFIAPI
ScreenPrintBannerWithColor (
  IN CONST CHAR16 *Title,
  IN UINTN        Attribute
  );

/**
  Prints a formatted string to the console screen safely.

  @param[in] Format       Unicode format string.
  @param[in] ...          Variable arguments.
**/
VOID
EFIAPI
ScreenPrintMessage (
  IN CONST CHAR16 *Format,
  ...
  );

/**
  Prints a formatted string to the console screen in a specific color.

  @param[in] Attribute    Text attribute for the message.
  @param[in] Format       Unicode format string.
  @param[in] ...          Variable arguments.
**/
VOID
EFIAPI
ScreenPrintColorMessage (
  IN UINTN        Attribute,
  IN CONST CHAR16 *Format,
  ...
  );

/**
  Prints a success message prefixed with an indicator '[+]' using active background.

  @param[in] Format       Unicode format string.
  @param[in] ...          Variable arguments.
**/
VOID
EFIAPI
ScreenPrintSuccess (
  IN CONST CHAR16 *Format,
  ...
  );

/**
  Prints a warning message prefixed with an alert indicator '[!]' using active background.

  @param[in] Format       Unicode format string.
  @param[in] ...          Variable arguments.
**/
VOID
EFIAPI
ScreenPrintWarning (
  IN CONST CHAR16 *Format,
  ...
  );

/**
  Prints an error message prefixed with an indicator '[-]' using active background.

  @param[in] Format       Unicode format string.
  @param[in] ...          Variable arguments.
**/
VOID
EFIAPI
ScreenPrintError (
  IN CONST CHAR16 *Format,
  ...
  );

/**
  Prints a categorized status line with formatted result using active background.

  @param[in] Category     Category or component name.
  @param[in] Message      Description message.
  @param[in] Status       EFI_STATUS to reflect success or failure.
**/
VOID
EFIAPI
ScreenPrintStatus (
  IN CONST CHAR16 *Category,
  IN CONST CHAR16 *Message,
  IN EFI_STATUS   Status
  );

#endif // SCREEN_LIB_H_
