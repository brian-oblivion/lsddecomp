#ifndef CODE_171E0_H
#define CODE_171E0_H

#include "common.h"

/* Method table (25 slots per tools/classtable.py) for the class whose
 * constructor caller is new_class_6d3c8 (src unit code_1677c). Not yet named
 * or typed field-by-field -- only its address is needed here, by
 * func_800269E0, which hands it to new_class_6d3c8 so the constructor slot
 * (+0x008, func_80025FDC) can be fetched and called indirectly. See
 * CLAUDE.md's "Writing a class method" for the +0x008 constructor-slot
 * convention. */
extern s32 D_8006D3C8[];

/* Some class instance (a slot of D_8006D430's table, going by
 * classtable.py) with at least one flag word at offset 0x24, OR'd with 1 by
 * func_80026C88. The full layout is derived further down this file, once
 * the method table type it needs (UnkFlagsObjMethods_171e0) is declared. */
typedef struct UnkFlagsObj_171e0 UnkFlagsObj_171e0;

/* Method table (header 0x00000003) for a second class. Several of this
 * unit's own functions are its slots: func_800269F0 (slot 0, offset +0x004),
 * func_80026A50 (slot 1 / +0x008, i.e. its constructor by the same
 * convention), func_80026AB4 (slot 2 / +0x00C), plus func_80026B08,
 * func_80026C20, func_80026C80 and func_80026C88 further down the table.
 * func_80026C9C returns its address the same way func_800269E0 returns
 * D_8006D3C8's. */
extern s32 D_8006D430[];

/* A 3-word vector-like object, written wholesale by func_80026CE8. Only the
 * first three words are touched; nothing here says whether a further field
 * follows. */
typedef struct Vec3_171e0 {
    s32 x;
    s32 y;
    s32 z;
} Vec3_171e0;

/* BasicClass's own table (D_8006B58C-equivalent, returned by Get_vtable_BasicClass,
 * which lives in the still-uncarved code_8220 segment -- see also
 * include/class_16334.h, which independently derived the same two slots
 * from a different unit). Declared again here, under a unit-local name, so
 * this header does not need to include class_16334.h. Only the two slots
 * this unit calls are typed. */
typedef struct BasicClassMethods171e0 BasicClassMethods171e0;
struct BasicClassMethods171e0 {
    /* +0x00 */ s32 header;
    /* +0x04 */ void *unk04;
    /* +0x08 */ void *(*ctor)(void *self);
    /* +0x0C */ void *(*dtor)(void *self);
};
extern BasicClassMethods171e0 *Get_vtable_BasicClass(void);
extern void *func_80017B34(s32 size); /* one arg confirmed by new_class_6d3c8.md (code_1677c) */
extern void func_80017CFC(void *arg);
extern s32 strlen(char *s);

/* D_8006D430's own vtable, laid out by the same convention BasicClass uses
 * (header, an own-class slot at +0x004, ctor at +0x008, dtor at +0x00C --
 * confirmed by dumping the table's raw words directly from disk/SLPS_015.56
 * rather than trusting classtable.py's null-slot elision). +0x010..+0x03C
 * are the 13 slots inherited verbatim from BasicClass (BasicClass__func_*),
 * +0x03C..+0x040 and +0x044..+0x054 (bar +0x058..+0x064) are genuinely null
 * at this level -- i.e. this class declares slots it never itself
 * implements, for a subclass to override; func_80026B08 calls four of them
 * (+0x044/+0x048/+0x04C/+0x054) polymorphically without knowing or caring
 * that they resolve to nothing at this level. Only the slots this unit's
 * queued functions actually dispatch through are typed; the rest are
 * skipped rather than padded out to +0x0B0, since nothing here ever
 * instantiates this struct by value. */
typedef struct UnkFlagsObjMethods_171e0 UnkFlagsObjMethods_171e0;
struct UnkFlagsObjMethods_171e0 {
    /* +0x00 */ s32 header;
    /* +0x04 */ void *(*ownDtorChain)(UnkFlagsObj_171e0 *self); /* func_800269F0 */
    /* +0x08 */ void (*ctor)(UnkFlagsObj_171e0 *self);          /* func_80026A50 */
    /* +0x0C */ void *(*dtor)(UnkFlagsObj_171e0 *self);         /* func_80026AB4 */
    /* +0x10 */ u8 pad10[0x44 - 0x10];      /* inherited BasicClass slots + null slots */
    /* +0x44 */ void (*slot44)(UnkFlagsObj_171e0 *self, s32 arg1, s32 arg2, s32 arg3);
    /* +0x48 */ void (*slot48)(UnkFlagsObj_171e0 *self);
    /* +0x4C */ s32 (*slot4C)(UnkFlagsObj_171e0 *self, s32 arg1, s32 arg2);
    /* +0x50 */ u8 pad50[0x54 - 0x50];      /* null slot */
    /* +0x54 */ void (*slot54)(UnkFlagsObj_171e0 *self, void *arg1, s32 arg2);
    /* +0x58 */ u8 pad58[0x5C - 0x58];      /* func_80026B08's own slot, unused here */
    /* +0x5C */ void *(*slot5C)(UnkFlagsObj_171e0 *self);       /* func_80026C20, dispatched
                                                                  * indirectly even though this
                                                                  * class's own copy is known */
};

/* An instance of the D_8006D430 class. UnkFlagsObj_171e0's own name and the
 * unknown_value_0x24 field predate this unit's work (see func_80026C88);
 * fields below it are new, derived from func_80026A50 (the ctor, which
 * zeroes them), func_80026B08/func_80026C20 (unk0C/unk10/unk14/unk20) and
 * func_80026D88 (the unk40..unk74 block, a field-by-field copy -- retail
 * copies +0x40..+0x58 then jumps a 0xC-byte gap to +0x68..+0x74, so that gap
 * is left unnamed rather than guessed at). */
struct UnkFlagsObj_171e0 {
    /* +0x00 */ UnkFlagsObjMethods_171e0 *methods;
    /* +0x04 */ u8 pad04[0x0C - 0x04];       /* BasicClass instance fields; owned elsewhere */
    /* +0x0C */ s32 unk0C;                   /* saved/restored around the unk10 (re)alloc */
    /* +0x10 */ void *unk10;                 /* resource from func_80017B34; freed via func_80017CFC */
    /* +0x14 */ s32 unk14;                   /* unk10's allocation size */
    /* +0x18 */ u8 pad18[0x20 - 0x18];       /* unknown, 8 bytes */
    /* +0x20 */ u16 unk20;                   /* nonzero blocks the unk10 free in func_80026C20 */
    /* +0x22 */ u16 unk22;
    /* +0x24 */ s32 unknown_value_0x24;      /* flags; bit 0 set by func_80026C88 */
    /* +0x28 */ u16 unk28;
    /* +0x2A */ u16 unk2A;
    /* +0x2C */ u8 pad2C[0x40 - 0x2C];       /* unknown, 0x14 bytes */
    /* +0x40 */ s32 unk40;
    /* +0x44 */ s32 unk44;
    /* +0x48 */ s32 unk48;
    /* +0x4C */ s32 unk4C;
    /* +0x50 */ s32 unk50;
    /* +0x54 */ s32 unk54;
    /* +0x58 */ s32 unk58;
    /* +0x5C */ u8 pad5C[0x68 - 0x5C];       /* unknown, 0xC bytes; NOT copied by func_80026D88 */
    /* +0x68 */ s32 unk68;
    /* +0x6C */ s32 unk6C;
    /* +0x70 */ s32 unk70;
    /* +0x74 */ s32 unk74;
};

extern void *func_80026C9C(void); /* returns &D_8006D430, see src/code_171e0.c */

char *strcat(char *dest, char *src);

#endif
