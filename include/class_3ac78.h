#ifndef CLASS_3AC78_H
#define CLASS_3AC78_H

#include "common.h"

typedef struct Class866E8 Class866E8;
typedef struct Class866E8Methods Class866E8Methods;
typedef struct Class86668 Class86668;
typedef struct Class86668Methods Class86668Methods;
typedef struct UnkListObj_3ac78 UnkListObj_3ac78;
typedef struct UnkChildMethods_3ac78 UnkChildMethods_3ac78;
typedef struct UnkChildObj_3ac78 UnkChildObj_3ac78;
typedef struct UnkSlotChildMethods_3ac78 UnkSlotChildMethods_3ac78;
typedef struct UnkSlotChildObj_3ac78 UnkSlotChildObj_3ac78;
typedef struct UnkSlotListObj_3ac78 UnkSlotListObj_3ac78;
typedef struct UnkSlotEntry_3ac78 UnkSlotEntry_3ac78;
typedef struct HistoryEntry_3ac78 HistoryEntry_3ac78;
typedef struct HistoryBlock_3ac78 HistoryBlock_3ac78;
typedef struct UnkPtr68Obj_3ac78 UnkPtr68Obj_3ac78;
typedef struct UnkArgObj_3ac78 UnkArgObj_3ac78;

/*
 * One entry of Class866E8::unkEC[7] (func_8004ABD0). 0x1C bytes; only the
 * three fields that function touches are typed. Its own two pointer
 * members' full types (UnkSlotChildObj_3ac78, UnkSlotListObj_3ac78) are
 * defined further down -- fine, since a pointer only needs the forward
 * `typedef struct X X;` above, not the full body. This struct itself has
 * to come before Class866E8 below because Class866E8::unkEC is an ARRAY
 * member, which (unlike a pointer) needs a COMPLETE type.
 */
struct UnkSlotEntry_3ac78 {
    u16 unk0;                        /* zeroed at the top of each loop pass */
    u8 pad2[0x4 - 0x2];
    UnkSlotChildObj_3ac78 *unk4;
    UnkSlotListObj_3ac78 *unk8;
    u8 padC[0x1C - 0xC];
};

/*
 * One entry of Class866E8::unk8C.e[3] (func_8004AEA4/func_8004B030). 0x10
 * bytes. func_8004B030 (STALLED, see docs/match-reports/func_8004B030.md)
 * writes element [0]'s fields directly (unk0 = a call's return pointer,
 * unk4/6/8/A = four s16 values); func_8004AEA4 (matched) treats the WHOLE
 * 3-element block opaquely, saving and restoring it around a pair of
 * calls via whole-struct assignment. Both readings are consistent with
 * the SAME 0x10-byte element layout; only the top-level field name
 * changed (previously modeled as five separate top-level Class866E8
 * fields, unk8C/unk90/unk92/unk94/unk96 -- see func_8004B030.md's
 * "Update" note for the correction).
 */
struct HistoryEntry_3ac78 {
    void *unk0;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    u8 padC[0x10 - 0xC];
};

/*
 * Class866E8::unk8C as a whole, 0x30 bytes (3 x HistoryEntry_3ac78).
 * Wrapped in a struct (rather than left as a bare array field) so it can
 * be copied with a plain `=` -- GCC 2.6.3 compiles a whole-struct
 * assignment to a batched 4-word-per-iteration block move, which is
 * exactly what func_8004AEA4's save/restore pair does. An indexed loop
 * translation does NOT reproduce this codegen; see
 * docs/match-reports/func_80025E1C.md for the earlier-established
 * precedent this follows.
 */
struct HistoryBlock_3ac78 {
    HistoryEntry_3ac78 e[3];
};

/*
 * Opaque target of Class866E8::unk68 (func_8004AEA4 dereferences it and
 * reads only the field at +0x4). func_8004B344 (matched) only ever
 * STORES a raw word there and never dereferences it itself, so retyping
 * its own `arg1` parameter to this pointer type changes nothing about
 * its compiled bytes -- confirmed after the retype, still byte-exact.
 */
struct UnkPtr68Obj_3ac78 {
    u8 pad0[0x4];
    s32 unk4;
};

/*
 * Class866E8 -- constructed by func_8004A4C8 (New_Class866E8: allocates
 * 0x1E8 bytes, gets the vtable via func_8004D244, calls ctor slot +0x008).
 * Vtable is D_800866E8 (80 slots, header 0x114), resolved with
 * tools/classtable.py D_800866E8. Ctor is func_8004A534 (177 words) and
 * dtor is func_8004A7C0 (128 words); both out of this round's scope.
 *
 * No FirecatFG name survives for this class (only anonymous func_ symbols
 * in the symbol file), so it is named by its vtable address, same
 * convention as Class6D3C8.h. Only the slots and fields this round's
 * functions actually reach are typed; the rest stays opaque padding.
 */
struct Class866E8Methods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;                                                   /* BasicClass__func_17eb0 */
    /* +0x008 */ void (*ctor)(Class866E8 *self, s32 arg1, s32 arg2);            /* func_8004A534; called by func_8004A4C8 */
    /* +0x00C */ void *dtor;                                                    /* func_8004A7C0; not dispatched by this round's functions */
    /* +0x010 */ u8 pad010[0x030 - 0x010];
    /* +0x030 */ void (*slot30)(Class866E8 *self, s32 arg1);                    /* BasicClass__func_182cc; called by func_8004AA6C */
    /* +0x034 */ u8 pad034[0x038 - 0x034];
    /* +0x038 */ void (*slot38)(Class866E8 *self);                              /* func_8004A984; called by func_8004B2D4 */
    /* +0x03C */ u8 pad03C[0x040 - 0x03C];
    /* +0x040 */ void (*slot40)(Class866E8 *self);                              /* func_8004AA10 (gp_rel-blocked, docs/research/gp-relative-blocker.md); called by func_8004B344 */
    /* +0x044 */ u8 pad044[0x080 - 0x044];
    /* +0x080 */ void (*slot80)(Class866E8 *self, s32 arg1, s32 arg2, s32 arg3); /* func_8001D4AC; called by func_8004A478 through Class86668::unk34 */
    /* +0x084 */ u8 pad084[0x088 - 0x084];
    /* +0x088 */ void (*slot88)(Class866E8 *self, s32 arg1, void *arg2, s32 arg3); /* func_8004AA6C; called by func_8004ABD0 with a 4th arg AA6C's own body never reads */
    /* +0x08C */ u8 pad08C[0x0B8 - 0x08C];
    /* +0x0B8 */ UnkChildObj_3ac78 *(*slotB8)(Class866E8 *self, s32 index);      /* func_80042828; called by func_8004ACF8 */
    /* +0x0BC */ u8 pad0BC[0x0D0 - 0x0BC];
    /* +0x0D0 */ void (*slotD0)(Class866E8 *self, void *list, s32 count);        /* func_8004ADD8; called by func_8004AB88 */
    /* +0x0D4 */ u8 pad0D4[0x0F4 - 0x0D4];
    /* +0x0F4 */ void (*slotF4)(Class866E8 *self);                              /* func_8004B5BC; called by func_8004AB24 */
    /* +0x0F8 */ u8 pad0F8[0x100 - 0x0F8];
    /* +0x100 */ void (*slot100)(Class866E8 *self, void *arg1, s32 arg2);        /* func_8004BD14; called by func_8004A984 */
    /* +0x104 */ u8 pad104[0x108 - 0x104];
    /* +0x108 */ void (*slot108)(Class866E8 *self, UnkSlotEntry_3ac78 *entry);   /* func_8004C0AC; called by func_8004ABD0 */
    /* +0x10C */ u8 pad10C[0x110 - 0x10C];
    /* +0x110 */ s32 (*slot110)(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2); /* func_8004C1C0; called by func_8004AEA4 */
    /* +0x114 */ u8 pad114[0x124 - 0x114];
    /* +0x124 */ void *(*slot124)(Class866E8 *self, void *arg1);                /* func_8004C5D0; called by func_8004B030 */
    /* +0x128 */ u8 pad128[0x12C - 0x128];
    /* +0x12C */ void (*slot12C)(Class866E8 *self, void *list, s32 count);       /* func_8004AEA4; called by func_8004ADD8 */
    /* +0x130 */ u8 pad130[0x13C - 0x130];
    /* +0x13C */ void (*slot13C)(Class866E8 *self);                             /* func_8004D028; called by func_8004AB24 */
    /* +0x140 */ void (*slot140)(Class866E8 *self);                             /* func_8004D088; called by func_8004ABD0 */
};

/* Object size is 0x1E8, from func_8004A4C8's allocator call. Field offsets
 * below are only the ones this round's functions touch. */
struct Class866E8 {
    /* +0x000 */ Class866E8Methods *methods;
    /* +0x004 */ u8 pad004[0x036 - 0x004];
    /* +0x036 */ u16 flags36;                /* bit 0x80 tested by func_8004B2D4 */
    /* +0x038 */ u8 pad038[0x060 - 0x038];
    /* +0x060 */ s32 unk60;                  /* func_8004ADC4 arg1 */
    /* +0x064 */ s32 unk64;                  /* func_8004ADC4 arg2 */
    /* +0x068 */ UnkPtr68Obj_3ac78 *unk68;   /* func_8004B344 arg1, stored raw; func_8004AEA4 dereferences ->unk4 */
    /* +0x06C */ u8 pad06C[0x070 - 0x06C];
    /* +0x070 */ s32 unk70;                  /* func_8004AB24: gates slotF4/slot13C dispatch */
    /* +0x074 */ s32 unk74;                  /* func_8004B32C arg1, stored raw */
    /* +0x078 */ s16 unk78;                  /* func_8004B32C: arg1 >> 12 */
    /* +0x07A */ s16 unk7A;                  /* func_8004B32C: arg1 >> 11 */
    /* +0x07C */ s16 unk7C;                  /* func_8004AFE0: (s8)arg1->unk2 - 1 */
    /* +0x07E */ s16 unk7E;                  /* func_8004AFE0: (s8)arg1->unk3 - 1 */
    /* +0x080 */ s32 unk80;                  /* func_8004AFE0 arg2, stored raw */
    /* +0x084 */ s32 unk84;                  /* func_8004AFE0 arg2, stored raw (same value as unk80) */
    /* +0x088 */ s32 unk88;                  /* func_8004B030 (STALLED): always set to 1; func_8004AEA4 saves/restores it around a pair of calls */
    /* +0x08C */ HistoryBlock_3ac78 unk8C;   /* func_8004AEA4: saved/restored via whole-struct assignment; func_8004B030 (STALLED) writes element [0]'s fields directly -- see HistoryEntry_3ac78 */
    /* +0x0BC */ u8 pad0BC_tail[0x0E8 - 0x0BC];
    /* +0x0E8 */ s32 unkE8;                  /* func_8004ADD0 arg1; func_8004ADD8 reads it back as a NUL-terminated s32 tag array -- true element type still s32, only usage differs per call site */
    /* +0x0EC */ UnkSlotEntry_3ac78 unkEC[7]; /* func_8004ABD0: 7 x 0x1C-byte slots, walked 0..6 */
    /* +0x1B0 */ u8 pad1B0[0x1B4 - 0x1B0];
    /* +0x1B4 */ s16 unk1B4;                 /* func_8004ABD0: zeroed after the loop */
    /* +0x1B6 */ u8 pad1B6[0x1B8 - 0x1B6];
    /* +0x1B8 */ s32 unk1B8;                 /* func_8004ABD0: zeroed after the loop */
    /* +0x1BC */ UnkListObj_3ac78 *unk1BC;   /* func_8004AA6C arg2, stored raw */
    /* +0x1C0 */ u8 unk1C0[0x1E8 - 0x1C0];   /* address-of only, returned by func_8004B31C; real element type unknown */
};

/* Get-vtable helper for Class866E8. Still raw asm: it lives in class_3bb8c,
 * an uncarved monolithic segment, not yet a carved src/ unit anywhere. */
extern Class866E8Methods *func_8004D244(void);

/* The generic allocator, established already in DreamSys.h/Entity.h/etc. */
extern void *func_80017B34(s32 size);

/*
 * Class86668 -- vtable D_80086668 (28 slots, header 0x230), resolved with
 * tools/classtable.py D_80086668. Its own last slot (+0x070) is
 * func_8004A478, which is the only reason this class is visible from this
 * unit at all: func_8004A478 dispatches through a Class866E8 instance held
 * at Class86668::unk34. Everything else about Class86668 is unknown.
 */
struct Class86668Methods {
    /* +0x000 */ s32 header;
    /* +0x004 */ u8 pad004[0x070 - 0x004];
};

struct Class86668 {
    /* +0x000 */ Class86668Methods *methods;
    /* +0x004 */ u8 pad004[0x034 - 0x004];
    /* +0x034 */ Class866E8 *unk34;          /* func_8004A478 dispatches through this, guarded by a NULL check */
};

extern Class86668Methods D_80086668;

/*
 * Opaque descriptor buffer passed as func_8004AFE0's arg1, func_8004AEA4's
 * stack-local `buf` (passed on to slot110/func_8004AFE0/func_8004B030),
 * and func_8004B030's arg1 (STALLED). Populated by a call through
 * Class866E8Methods slot +0x110 (func_8004C1C0, not decompiled anywhere
 * yet), so only the bytes those functions actually read are typed. Named
 * after the convention in code_171e0.h (`Unk*Obj_<unit>`).
 */
struct UnkArgObj_3ac78 {
    u8 pad0[2];
    s8 unk2;
    s8 unk3;
    u8 pad4[0x28 - 0x4];
    void *unk28;   /* func_8004B030 (STALLED): reread and forwarded to self->methods->slot124 */
};

/*
 * Generic "object with a vtable pointer at offset 0" view, used only by
 * func_8004AB88 to read the low byte of another object's vtable header
 * word as a type tag (0x34 here). Several unrelated class tables share
 * that low byte (D_800878D4 and DREAMSYS_METHODS among them, per
 * docs/research/class-framework.md), so this reads as a family/base-class
 * check, not an exact-class check. True type of the pointed-to object is
 * unconfirmed; only the one byte this function reads is modeled here.
 *
 * unk04 is the shared "BasicClass" base-method slot at +0x004 -- the same
 * slot Class866E8Methods documents as `BasicClass__func_17eb0`. Its exact
 * semantics are unknown; func_8004AA6C calls it on a GenericObject and
 * stores the (pointer) return value back where the object came from,
 * suggesting a release-and-replace pattern, but that is not confirmed.
 */
typedef struct {
    s32 header;
    void *(*unk04)(void *self);
} GenericMethodsHeader;

typedef struct {
    GenericMethodsHeader *methods;
} GenericObject;

/*
 * Opaque list/args object passed as func_8004AA6C's arg2 (stashed at
 * self->unk1BC) and as func_8004AEA4/func_8004ADD8's "list" argument.
 * func_8004AEA4 additionally reaches +0x0C (a flag: nonzero selects
 * `unk14->unk38`, computed as raw pointer arithmetic -- `unk14` points to
 * something bigger than a GenericObject, and +0x38 is a field within it
 * this unit never otherwise names) as the gate for that computation.
 */
struct UnkListObj_3ac78 {
    u8 pad0[0xC];
    s32 unk0C;               /* func_8004AEA4: nonzero selects the unk14->unk38 branch */
    u8 pad10[0x14 - 0x10];
    GenericObject *unk14;    /* func_8004AA6C: replaced via ->methods->unk04(unk14) when non-NULL. func_8004AEA4: (u8*)unk14 + 0x38 computed (not dereferenced) when unk0C != 0 */
};

/*
 * Opaque child object returned by Class866E8Methods::slotB8. Class unknown;
 * only the two adjacent method slots func_8004ACF8 dispatches through are
 * typed.
 */
struct UnkChildMethods_3ac78 {
    u8 pad0[0x44];
    void (*slot44)(UnkChildObj_3ac78 *self, s32 arg1, s32 arg2);
    void (*slot48)(UnkChildObj_3ac78 *self, s32 arg1, s32 arg2);
};

struct UnkChildObj_3ac78 {
    UnkChildMethods_3ac78 *methods;
};

/*
 * Opaque "child" object referenced by each Class866E8::unkEC[] slot entry
 * (func_8004ABD0). Class unknown; only the two dispatched slots are typed.
 * A DIFFERENT class from UnkChildObj_3ac78 above (different offsets), kept
 * as its own type rather than reusing that one.
 */
struct UnkSlotChildMethods_3ac78 {
    u8 pad0[0x74];
    void (*slot74)(UnkSlotChildObj_3ac78 *self);
    u8 pad78[0x84 - 0x78];
    void (*slot84)(UnkSlotChildObj_3ac78 *self);
};

struct UnkSlotChildObj_3ac78 {
    UnkSlotChildMethods_3ac78 *methods;
};

/*
 * Opaque "list" object referenced by each Class866E8::unkEC[] slot entry
 * (func_8004ABD0). Only field +0x2C (a GenericObject*, refreshed through
 * its own ->methods->unk04 base-class slot exactly like
 * func_8004AA6C's arg2->unk14) is typed.
 */
struct UnkSlotListObj_3ac78 {
    u8 pad0[0x2C];
    GenericObject *unk2C;
};

#endif
