/** @file
  ScreenLib: Implements screen output, formatting, and console UI functions
  with comprehensive foreground and background color management.

  Copyright (c) 2026, Playground Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ScreenLib.h>

#define SCREEN_MAX_BUFFER_SIZE  1024

STATIC UINTN  mCurrentForeground = EFI_LIGHTGRAY;
STATIC UINTN  mCurrentBackground = EFI_BLACK;

/**
  Clears the console screen using the currently configured background color.
**/
VOID
EFIAPI
ScreenClear (
  VOID
  )
{
  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
    gST->ConOut->ClearScreen (gST->ConOut);
  }
}

/**
  Sets the text color/attribute for the console screen.
  If Attribute contains only foreground bits (<= 0x0F), the current background color is retained.

  @param[in] Attribute    Text attribute.
**/
VOID
EFIAPI
ScreenSetColor (
  IN UINTN Attribute
  )
{
  if (Attribute <= 0x0F) {
    mCurrentForeground = Attribute & 0x0F;
  } else {
    mCurrentForeground = Attribute & 0x0F;
    mCurrentBackground = (Attribute >> 4) & 0x07;
  }

  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
  }
}

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
  )
{
  mCurrentForeground = ForegroundColor & 0x0F;
  mCurrentBackground = BackgroundColor & 0x07;

  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
  }
}

/**
  Sets only the background color for subsequent console output and clearing,
  preserving the current foreground color.

  @param[in] BackgroundColor  Background color (0-7, e.g. EFI_BLUE, EFI_BLACK).
**/
VOID
EFIAPI
ScreenSetBackgroundColor (
  IN UINTN BackgroundColor
  )
{
  mCurrentBackground = BackgroundColor & 0x07;

  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
  }
}

/**
  Sets only the foreground color for subsequent console output,
  preserving the current background color.

  @param[in] ForegroundColor  Foreground color (0-15, e.g. EFI_WHITE, EFI_YELLOW).
**/
VOID
EFIAPI
ScreenSetForegroundColor (
  IN UINTN ForegroundColor
  )
{
  mCurrentForeground = ForegroundColor & 0x0F;

  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
  }
}

/**
  Gets the currently active background color (0-7).

  @return Current background color.
**/
UINTN
EFIAPI
ScreenGetBackgroundColor (
  VOID
  )
{
  return mCurrentBackground;
}

/**
  Gets the currently active foreground color (0-15).

  @return Current foreground color.
**/
UINTN
EFIAPI
ScreenGetForegroundColor (
  VOID
  )
{
  return mCurrentForeground;
}

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
  )
{
  ScreenSetColors (ForegroundColor, BackgroundColor);
  ScreenClear ();
}

/**
  Clears the entire console screen with a specified background color,
  preserving the current foreground color.

  @param[in] BackgroundColor  Background color (0-7).
**/
VOID
EFIAPI
ScreenClearWithBackground (
  IN UINTN BackgroundColor
  )
{
  ScreenSetBackgroundColor (BackgroundColor);
  ScreenClear ();
}

/**
  Resets the text color attribute to the active background and default foreground.
**/
VOID
EFIAPI
ScreenResetColor (
  VOID
  )
{
  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
  }
}

/**
  Resets all colors back to default UEFI platform state (Light Gray on Black).
**/
VOID
EFIAPI
ScreenResetColors (
  VOID
  )
{
  mCurrentForeground = EFI_LIGHTGRAY;
  mCurrentBackground = EFI_BLACK;
  ScreenResetColor ();
}

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
  )
{
  CHAR16 Buffer[128];
  UINTN  Index;

  if (Length >= sizeof (Buffer) / sizeof (CHAR16)) {
    Length = (sizeof (Buffer) / sizeof (CHAR16)) - 1;
  }

  for (Index = 0; Index < Length; Index++) {
    Buffer[Index] = Character;
  }
  Buffer[Length] = L'\0';

  ScreenPrintMessage (L"%s\n", Buffer);
}

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
  )
{
  UINTN EffectiveAttr;

  if (Attribute <= 0x0F) {
    EffectiveAttr = EFI_TEXT_ATTR (Attribute, mCurrentBackground);
  } else {
    EffectiveAttr = Attribute;
  }

  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EffectiveAttr);
  }

  ScreenPrintLine (Character, Length);
  ScreenResetColor ();
}

/**
  Prints a framed title banner to the console screen with default color.

  @param[in] Title        Title string to display in the banner.
**/
VOID
EFIAPI
ScreenPrintBanner (
  IN CONST CHAR16 *Title
  )
{
  ScreenPrintBannerWithColor (Title, EFI_TEXT_ATTR (EFI_CYAN, mCurrentBackground));
}

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
  )
{
  UINTN EffectiveAttr;

  if (Attribute <= 0x0F) {
    EffectiveAttr = EFI_TEXT_ATTR (Attribute, mCurrentBackground);
  } else {
    EffectiveAttr = Attribute;
  }

  if ((gST != NULL) && (gST->ConOut != NULL)) {
    gST->ConOut->SetAttribute (gST->ConOut, EffectiveAttr);
  }

  ScreenPrintMessage (L"\n+==============================================================+\n");
  ScreenPrintMessage (L"|  %-58s  |\n", (Title != NULL) ? Title : L"");
  ScreenPrintMessage (L"+==============================================================+\n\n");
  ScreenResetColor ();
}

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
  )
{
  VA_LIST Marker;
  CHAR16  Buffer[SCREEN_MAX_BUFFER_SIZE];

  if ((gST == NULL) || (gST->ConOut == NULL) || (Format == NULL)) {
    return;
  }

  VA_START (Marker, Format);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  gST->ConOut->OutputString (gST->ConOut, Buffer);
}

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
  )
{
  VA_LIST Marker;
  CHAR16  Buffer[SCREEN_MAX_BUFFER_SIZE];
  UINTN   EffectiveAttr;

  if ((gST == NULL) || (gST->ConOut == NULL) || (Format == NULL)) {
    return;
  }

  if (Attribute <= 0x0F) {
    EffectiveAttr = EFI_TEXT_ATTR (Attribute, mCurrentBackground);
  } else {
    EffectiveAttr = Attribute;
  }

  gST->ConOut->SetAttribute (gST->ConOut, EffectiveAttr);

  VA_START (Marker, Format);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  gST->ConOut->OutputString (gST->ConOut, Buffer);
  ScreenResetColor ();
}

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
  )
{
  VA_LIST Marker;
  CHAR16  Buffer[SCREEN_MAX_BUFFER_SIZE];

  if ((gST == NULL) || (gST->ConOut == NULL) || (Format == NULL)) {
    return;
  }

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGREEN, mCurrentBackground));
  gST->ConOut->OutputString (gST->ConOut, L"[+] ");
  ScreenResetColor ();

  VA_START (Marker, Format);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  gST->ConOut->OutputString (gST->ConOut, Buffer);
}

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
  )
{
  VA_LIST Marker;
  CHAR16  Buffer[SCREEN_MAX_BUFFER_SIZE];

  if ((gST == NULL) || (gST->ConOut == NULL) || (Format == NULL)) {
    return;
  }

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_YELLOW, mCurrentBackground));
  gST->ConOut->OutputString (gST->ConOut, L"[!] ");
  ScreenResetColor ();

  VA_START (Marker, Format);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  gST->ConOut->OutputString (gST->ConOut, Buffer);
}

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
  )
{
  VA_LIST Marker;
  CHAR16  Buffer[SCREEN_MAX_BUFFER_SIZE];

  if ((gST == NULL) || (gST->ConOut == NULL) || (Format == NULL)) {
    return;
  }

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTRED, mCurrentBackground));
  gST->ConOut->OutputString (gST->ConOut, L"[-] ");
  ScreenResetColor ();

  VA_START (Marker, Format);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  gST->ConOut->OutputString (gST->ConOut, Buffer);
}

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
  )
{
  if ((gST == NULL) || (gST->ConOut == NULL)) {
    return;
  }

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
  ScreenPrintMessage (L"  %-16s: %-32s [", (Category != NULL) ? Category : L"", (Message != NULL) ? Message : L"");

  if (!EFI_ERROR (Status)) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGREEN, mCurrentBackground));
    ScreenPrintMessage (L"  OK  ");
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTRED, mCurrentBackground));
    ScreenPrintMessage (L"FAILED");
  }

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (mCurrentForeground, mCurrentBackground));
  ScreenPrintMessage (L"]\n");
  ScreenResetColor ();
}
