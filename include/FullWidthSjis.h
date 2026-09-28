#ifndef FULLWIDTHSJIS_H
#define FULLWIDTHSJIS_H

/*
 * Conversions between printable ASCII and its full-width Shift-JIS forms,
 * two bytes a character (src/ui/ScreenWidgets.c). The save title, the text
 * entry buffer and the item list's names are stored full-width and edited
 * or drawn one byte a character.
 */

#include "common.h"

/* Full-width Shift-JIS back to ASCII. Returns the address of the NUL it
 * writes. */
extern u8 *DecodeFullWidthSjis(u8 *dst, u8 *src);

/* ASCII to full-width Shift-JIS. Returns the address of the NUL it writes. */
extern u8 *EncodeFullWidthSjis(u8 *dst, u8 *src);

/* Writes `value` in decimal, full-width, into `dst`: padded on the left with
 * '0' to `width` digits, or as it is when `unpadded` is set. */
extern void FormatFullWidthNumber(u8 *dst, s32 value, s32 width, s32 unpadded);

#endif
