#ifndef CLASS_6D3C8_H
#define CLASS_6D3C8_H

#include "common.h"

/*
 * The class allocated by new_class_6d3c8 / constructed by func_80025FDC.
 * Method table is D_8006D3C8 (25 slots, see tools/classtable.py 0x8006D3C8
 * --vs 0x8006B58C): it derives from BasicClass (D_8006B58C, 14 slots),
 * overriding the constructor (+0x008) and destructor (+0x00C), and adding
 * ten slots of its own starting at +0x040. No FirecatFG name survives for
 * this class (only new_class_6d3c8 itself is named in the symbol file), so
 * fields are named by offset until real names are known.
 *
 * Only the slots this unit's functions actually call through are given
 * concrete field types; the rest stay opaque `void *` so the struct keeps
 * the right size/offsets without requiring every method to be typed up
 * front.
 */

typedef struct Class6D3C8 Class6D3C8;

typedef struct Class6D3C8Methods {
    s32 header;                                            /* +0x000 */
    void *unk04;                                            /* +0x004 BasicClass__func_17eb0 */
    Class6D3C8 *(*ctor)(Class6D3C8 *self, void *arg);       /* +0x008 func_80025FDC */
    void *unk0C;                                            /* +0x00C func_8003B024 (dtor override) */
    void *unk10;                                            /* +0x010 BasicClass__func_17f98 */
    void *unk14;                                            /* +0x014 BasicClass__func_17ff0 */
    void *unk18;                                            /* +0x018 BasicClass__func_18040 */
    void *unk1C;                                            /* +0x01C BasicClass__func_180bc */
    void *unk20;                                            /* +0x020 BasicClass__func_180fc */
    void *unk24;                                            /* +0x024 BasicClass__func_1811c */
    void *unk28;                                            /* +0x028 BasicClass__func_1813c */
    void *unk2C;                                            /* +0x02C BasicClass__func_1816c */
    void *unk30;                                            /* +0x030 BasicClass__func_182cc */
    void *unk34;                                            /* +0x034 BasicClass__func_18350 */
    void *unk38;                                            /* +0x038 BasicClass__func_18358 */
    void *unk3C;                                            /* +0x03C null slot */
    void (*slot40)(void);                                   /* +0x040 func_800260A4 */
    void *slot44;                                           /* +0x044 func_80026108 */
    void *unk48;                                            /* +0x048 func_8003B108 */
    void *unk4C;                                            /* +0x04C func_8003B110 */
    void *slot50;                                           /* +0x050 func_80026170 */
    void *slot54;                                           /* +0x054 func_80026348 */
    void *slot58;                                           /* +0x058 func_80026410 */
    void (*slot5C)(void);                                   /* +0x05C func_80026690 */
    void *slot60;                                           /* +0x060 func_80026698 */
    void *slot64;                                           /* +0x064 func_80026900 */
} Class6D3C8Methods;

/* Object size is 0x2C (from the allocator call in new_class_6d3c8). Field
 * offsets below are only the ones observed so far in func_80025FDC. */
struct Class6D3C8 {
    Class6D3C8Methods *methods;    /* +0x00 */
    u8 unk04[0x1C];                 /* +0x04 .. +0x1F, not yet decoded */
    void *arg;                      /* +0x20 the constructor's `arg` parameter */
    s32 unk24;                      /* +0x24 */
    void *dreamSys;                 /* +0x28 result of New_DreamSys() */
};

extern Class6D3C8Methods D_8006D3C8;

#endif
