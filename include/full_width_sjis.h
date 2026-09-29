/**
 * @file full_width_sjis.h
 * @brief Conversions between printable ASCII and its full-width Shift-JIS forms.
 *
 * Full-width Shift-JIS takes two bytes a character. The save title, the
 * text entry buffer and the item list's names are stored full-width and
 * edited or drawn one byte a character, so these convert between the two.
 * Defined in src/ui/full_width_sjis.c.
 */
#ifndef FULL_WIDTH_SJIS_H
#define FULL_WIDTH_SJIS_H

#include "common.h"

/**
 * @brief Converts a full-width Shift-JIS string back to ASCII.
 *
 * Drops each lead byte and maps the trail byte back to its ASCII
 * character, one byte out per two in.
 * @param dst the ASCII output; half the length of `src` plus the NUL.
 * @param src the NUL-terminated full-width string.
 * @return the address of the NUL written at the end of `dst`.
 */
extern u8 *DecodeFullWidthSjis(u8 *dst, u8 *src);

/**
 * @brief Converts an ASCII string to full-width Shift-JIS, two bytes a character.
 * @param dst the full-width output; twice the length of `src` plus the NUL.
 * @param src the NUL-terminated ASCII string.
 * @return the address of the NUL written at the end of `dst`.
 */
extern u8 *EncodeFullWidthSjis(u8 *dst, u8 *src);

/**
 * @brief Writes a number in decimal as full-width Shift-JIS.
 * @param dst      the full-width output.
 * @param value    the number.
 * @param width    the digit count to pad to on the left with '0'.
 * @param unpadded nonzero to write the digits as they are, without padding.
 */
extern void FormatFullWidthNumber(u8 *dst, s32 value, s32 width, s32 unpadded);

#endif
