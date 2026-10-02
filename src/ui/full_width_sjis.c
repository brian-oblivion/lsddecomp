/*
 * The full-width Shift-JIS string helpers (include/full_width_sjis.h), free
 * functions over plain byte buffers that the save title, the text entry
 * buffer and the item list's names go through, in ROM order:
 * DecodeFullWidthSjis, EncodeFullWidthSjis and FormatFullWidthNumber.
 */
#include "common.h"
#include "full_width_sjis.h"
#include <libgte.h>
#include <strings.h>

/* The full-width Shift-JIS forms of printable ASCII, as these three convert
 * them: two bytes a character, a lead byte and ASCII + SJIS_TRAIL_OFFSET as
 * the trail, one more from SJIS_TRAIL_GAP up because Shift-JIS never uses
 * 0x7F as a trail byte. */
#define SJIS_LEAD_SYMBOL 0x81  /* the lead for space and the symbols below '0' */
#define SJIS_LEAD_ALNUM 0x82   /* the lead from '0' up: digits and Latin letters */
#define SJIS_TRAIL_OFFSET 0x1F /* ASCII + this is the trail below the gap */
#define SJIS_TRAIL_GAP 0x7F    /* the trail value Shift-JIS skips */
#define SJIS_TRAIL_SPACE 0x40 /* the full-width space 0x8140's trail: ' ' + SJIS_TRAIL_OFFSET + 1 */

/* `d` and `dst` are two cursors over one buffer; `special` holds
 * SJIS_TRAIL_SPACE. */
/* MATCHING: `special` is set before `d` is copied from `dst`, retail's order. */
u8 *DecodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 special;
    u32 c;
    u32 v;
    u32 peek;

    if (*src++ != 0) {
        special = SJIS_TRAIL_SPACE;
        d = dst;
        do {
            d++;
            c = *src;
            dst++;
            if (c <= SJIS_TRAIL_GAP && c != special) {
                v = c - SJIS_TRAIL_OFFSET;
            } else {
                v = c - (SJIS_TRAIL_OFFSET + 1);
            }
            src++;
            d[-1] = v;
            peek = *src;
            src++;
        } while (peek != 0);
    }
    *dst = 0;
    return dst;
}

/* MATCHING: the `d = dst; dst++;` cursor pairs and the `trail` copy of `c` give
 * retail's code; `*dst++` and `c` alone compile differently. */
u8 *EncodeFullWidthSjis(u8 *dst, u8 *src) {
    u8 *d;
    u32 c;
    u32 v;
    u32 lead;
    u32 trail;

    if (*src != 0) {
        do {
            d = dst;
            dst++;
            c = *src;
            if (c >= '0') {
                lead = SJIS_LEAD_ALNUM;
            } else {
                lead = SJIS_LEAD_SYMBOL;
            }
            *d = lead;
            d = dst;
            dst++;
            c = *src;
            trail = c;
            if (trail < SJIS_TRAIL_GAP - SJIS_TRAIL_OFFSET && trail != ' ') {
                v = trail + SJIS_TRAIL_OFFSET;
            } else {
                v = trail + (SJIS_TRAIL_OFFSET + 1);
            }
            src++;
            *d = v;
        } while (*src != 0);
    }
    *dst = 0;
    return dst;
}

/* Sony's itoa (libc2): the decimal digits of `n`, in the library's own buffer.
 * psyz's libc.h renames it, away from the host C library's itoa. */
#ifdef PLATFORM_PC
#include <libc.h>
#endif
extern char *itoa(int n);

/* MATCHING: the declaration order text/fill/padded and `fill` set in two
 * statements give retail's code. */
void FormatFullWidthNumber(u8 *dst, s32 value, s32 width, s32 unpadded) {
    char text[width + 1];
    s32 fill;
    char padded[width + 1];

    fill = strlen(strcpy(text, itoa(value)));
    fill = width - fill;
    if (unpadded == 0) {
        memset((unsigned char *)padded, '0', width);
        strcpy(&padded[fill], text);
    }
    EncodeFullWidthSjis(dst, (u8 *)(unpadded != 0 ? text : padded));
}
