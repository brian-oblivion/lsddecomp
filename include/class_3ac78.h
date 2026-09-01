#ifndef CLASS_3AC78_H
#define CLASS_3AC78_H

#include "common.h"

/*
 * The class carved into src/class_3ac78.c. No FirecatFG name survives for
 * it -- no "New_"/"Constructor" symbol exists anywhere near this address
 * range in config/symbols.slps01556.lsdde.txt -- so it is named by its own
 * method table's address, same fallback Class6D3C8.h/code_55dd4.h document
 * for the same situation.
 *
 * Method table is D_800866E8 (80 slots, header word 0x114), resolved with
 * tools/classtable.py (never by counting):
 *   .venv/bin/python3 tools/classtable.py D_800866E8
 *   .venv/bin/python3 tools/classtable.py D_800866E8 --vs D_800878D4
 * BasicClass (D_8006B58C, 14 slots) -> the intermediate class at D_800878D4
 * (59 slots, header 0x34 -- the SAME shared middle class DreamSys and
 * Class65650 both sit under, see docs/research/class-framework.md and
 * code_55dd4.h) -> this class (D_800866E8: overrides the intermediate
 * class's ctor at +0x008 with func_8004A534 and its dtor at +0x00C with
 * func_8004A7C0, keeps everything else inherited verbatim, and adds
 * +0x0F0..+0x140 of its own). The table's tail (from +0x0E4 on) holds
 * addresses past this unit's own carve (e.g. +0x140 = func_8004D088, which
 * lives in class_3bb8c) -- a class's vtable is not required to sit inside
 * one carved unit, and this one doesn't.
 *
 * Only the slots this unit's queued functions actually dispatch through
 * are typed; everything else stays opaque padding so the struct keeps the
 * right size/offsets without requiring every method to be typed up front
 * (same policy as Entity.h's EntityMethods/BasicClassMethods).
 */

typedef struct Class866E8 Class866E8;
typedef struct Class866E8Methods Class866E8Methods;

struct Class866E8Methods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;                                        /* BasicClass__func_17eb0, inherited */
    /* +0x008 */ Class866E8 *(*ctor)(Class866E8 *self, void *arg1, void *arg2); /* func_8004A534 -- own ctor override */
    /* +0x00C */ void (*dtor)(Class866E8 *self);                      /* func_8004A7C0 -- own dtor override */
    /* +0x010 */ u8 pad10[0x38 - 0x010];
    /* +0x038 */ void (*slot38)(Class866E8 *self);                    /* func_8004A984 -- called by func_8004B2D4 when (self->unk36 & 0x80) != 0 */
    /* +0x03C */ u8 pad3C[0x40 - 0x03C];
    /* +0x040 */ void (*slot40)(Class866E8 *self);                    /* func_8004AA10 (gp_rel-blocked, see docs/research/gp-relative-blocker.md) -- called by func_8004B344 */
    /* +0x044 */ u8 pad44[0x0D0 - 0x044];
    /* +0x0D0 */ void (*slotD0)(Class866E8 *self, void *arg1);        /* func_8004ADD8 -- called by func_8004AB88 when arg1's own class has header byte 0x34 */
    /* +0x0D4 */ u8 pad0D4[0x0F4 - 0x0D4];
    /* +0x0F4 */ void (*slotF4)(Class866E8 *self);                    /* func_8004B5BC (outside this unit) -- called by func_8004AB24 */
    /* +0x0F8 */ u8 pad0F8[0x13C - 0x0F8];
    /* +0x13C */ void (*slot13C)(Class866E8 *self);                   /* func_8004D028 (outside this unit) -- called by func_8004AB24 */
};

struct Class866E8 {
    /* +0x000 */ Class866E8Methods *methods;
    /* Everything between is not yet named by this unit's queued work. */
    /* +0x004 */ u8 pad004[0x036 - 0x004];
    /* +0x036 */ u16 unk36;   /* flag bit 0x80 gates the func_8004B2D4 dispatch of slot38 */
    /* +0x038 */ u8 pad038[0x060 - 0x038];
    /* +0x060 */ s32 unk60;   /* set by func_8004ADC4(self, arg1, arg2) */
    /* +0x064 */ s32 unk64;   /* set by func_8004ADC4(self, arg1, arg2) */
    /* +0x068 */ s32 unk68;   /* set by func_8004B344(self, arg1) after dispatching slot40 */
    /* +0x06C */ u8 pad06C[0x070 - 0x06C];
    /* +0x070 */ void *unk70; /* non-NULL guards the func_8004AB24 dispatch of slotF4/slot13C */
    /* +0x074 */ s32 unk74;   /* set whole by func_8004B32C(self, arg1) */
    /* +0x078 */ s16 unk78;   /* = (s16)(arg1 >> 12), set by func_8004B32C */
    /* +0x07A */ s16 unk7A;   /* = (s16)(arg1 >> 11), set by func_8004B32C */
    /* +0x07C */ s16 unk7C;   /* = (s8)arg1[2] - 1, set by func_8004AFE0 */
    /* +0x07E */ s16 unk7E;   /* = (s8)arg1[3] - 1, set by func_8004AFE0 */
    /* +0x080 */ s32 unk80;   /* = arg2, set by func_8004AFE0 */
    /* +0x084 */ s32 unk84;   /* = arg2, set by func_8004AFE0 */
    /* +0x088 */ u8 pad088[0x0E8 - 0x088];
    /* +0x0E8 */ void *unkE8; /* linked-list-style head pointer, set by func_8004ADD0(self, arg1) */
    /* +0x0EC */ u8 pad0EC[0x1C0 - 0x0EC];
    /* +0x1C0 */ u8 pad1C0[0x1E8 - 0x1C0]; /* embedded sub-object, shape unknown -- func_8004B31C only returns its address; 0x1E8 is func_8004A4C8's own allocation size (New_-style wrapper) */
};

/* The overlapping unit's own accessor for this class's vtable -- matched
 * elsewhere (asm/class_3bb8c.s, func_8004D244, a plain lui/addiu
 * address-of, not gp_rel). Declared here (not defined) the same way
 * func_80057C84 is declared in code_55dd4.h/DreamSys.h for their own
 * shared base-class accessor. */
extern Class866E8Methods *func_8004D244(void);

extern void *func_80017B34(s32 size);

/* A second, smaller sibling class also implemented in this unit
 * (func_8004A478's own `self`). Its vtable is D_80086668/D_800865C8 --
 * both 0x1F230/0x230-headed tables that share the identical function
 * pointer at their own +0x070 slot, func_8004A478 -- distinct from
 * Class866E8Methods above (D_800866E8 does not use func_8004A478 at all).
 * Only the one field func_8004A478 reaches is named. */
typedef struct Class86668 Class86668;
typedef struct Class86668Methods Class86668Methods;

struct Class86668Methods {
    /* +0x000 */ u8 pad00[0x080];
    /* +0x080 */ void (*slot80)(Class86668 *self, s32 arg1, s32 arg2, s32 arg3); /* called by func_8004A478 */
};

struct Class86668 {
    /* +0x000 */ Class86668Methods *methods;
};

/* func_8004A4B8's own return value -- the smaller sibling class's OWN
 * vtable, D_80086668 (28 slots, header 0x230, asm/data/76DC8.data.s),
 * still undecompiled data. */
extern Class86668Methods D_80086668;

/* A generic view of "some other object", used only where all that is
 * needed is the byte sitting at the low byte of ITS OWN vtable's header
 * word -- an inline class/type check, the same shape as TaggedObj in
 * code_55dd4.h except the tag lives behind one more pointer indirection
 * (on the vtable, not the object itself). */
typedef struct AnyObj AnyObj;
typedef struct AnyObjMethods AnyObjMethods;
struct AnyObjMethods {
    /* +0x000 */ u8 headerLowByte;
};
struct AnyObj {
    /* +0x000 */ AnyObjMethods *methods;
};

typedef struct Class3AC78Sub34 Class3AC78Sub34;
struct Class3AC78Sub34 {
    /* +0x000 */ u8 pad00[0x34];
    /* +0x034 */ Class86668 *child; /* func_8004A478 dispatches through child->methods->slot80 when non-NULL */
};

/* Outside this unit; called with its result discarded at the end of
 * func_8004AFE0. */
extern void func_8004C93C(Class866E8 *self);

#endif
