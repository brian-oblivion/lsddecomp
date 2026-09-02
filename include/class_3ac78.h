#ifndef CLASS_3AC78_H
#define CLASS_3AC78_H

#include "common.h"

typedef struct Class866E8 Class866E8;
typedef struct Class866E8Methods Class866E8Methods;
typedef struct Class86668 Class86668;
typedef struct Class86668Methods Class86668Methods;

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
    /* +0x030 */ void (*slot30)(Class866E8 *self);                              /* BasicClass__func_182cc; called by func_8004AA6C */
    /* +0x034 */ u8 pad034[0x038 - 0x034];
    /* +0x038 */ void (*slot38)(Class866E8 *self);                              /* func_8004A984; called by func_8004B2D4 */
    /* +0x03C */ u8 pad03C[0x040 - 0x03C];
    /* +0x040 */ void (*slot40)(Class866E8 *self);                              /* func_8004AA10 (gp_rel-blocked, docs/research/gp-relative-blocker.md); called by func_8004B344 */
    /* +0x044 */ u8 pad044[0x080 - 0x044];
    /* +0x080 */ void (*slot80)(Class866E8 *self, s32 arg1, s32 arg2, s32 arg3); /* func_8001D4AC; called by func_8004A478 through Class86668::unk34 */
    /* +0x084 */ u8 pad084[0x0B8 - 0x084];
    /* +0x0B8 */ void *(*slotB8)(Class866E8 *self, s32 index);                  /* func_80042828; called by func_8004ACF8, returns an opaque child object */
    /* +0x0BC */ u8 pad0BC[0x0D0 - 0x0BC];
    /* +0x0D0 */ void (*slotD0)(Class866E8 *self, void *list, s32 count);        /* func_8004ADD8; called by func_8004AB88 */
    /* +0x0D4 */ u8 pad0D4[0x0F4 - 0x0D4];
    /* +0x0F4 */ void (*slotF4)(Class866E8 *self);                              /* func_8004B5BC; called by func_8004AB24 */
    /* +0x0F8 */ u8 pad0F8[0x100 - 0x0F8];
    /* +0x100 */ void (*slot100)(Class866E8 *self, void *arg1, s32 arg2);        /* func_8004BD14; called by func_8004A984 */
    /* +0x104 */ u8 pad104[0x12C - 0x104];
    /* +0x12C */ void (*slot12C)(Class866E8 *self, void *list, s32 count);       /* func_8004AEA4; called by func_8004ADD8 */
    /* +0x130 */ u8 pad130[0x13C - 0x130];
    /* +0x13C */ void (*slot13C)(Class866E8 *self);                             /* func_8004D028; called by func_8004AB24 */
    /* +0x140 */ u8 pad140[0x144 - 0x140];                                      /* func_8004D088 lives here (classtable.py D_800866E8); not dispatched by this round's functions */
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
    /* +0x068 */ s32 unk68;                  /* func_8004B344 arg1 */
    /* +0x06C */ u8 pad06C[0x070 - 0x06C];
    /* +0x070 */ s32 unk70;                  /* func_8004AB24: gates slotF4/slot13C dispatch */
    /* +0x074 */ s32 unk74;                  /* func_8004B32C arg1, stored raw */
    /* +0x078 */ s16 unk78;                  /* func_8004B32C: arg1 >> 12 */
    /* +0x07A */ s16 unk7A;                  /* func_8004B32C: arg1 >> 11 */
    /* +0x07C */ s16 unk7C;                  /* func_8004AFE0: (s8)arg1->unk2 - 1 */
    /* +0x07E */ s16 unk7E;                  /* func_8004AFE0: (s8)arg1->unk3 - 1 */
    /* +0x080 */ s32 unk80;                  /* func_8004AFE0 arg2, stored raw */
    /* +0x084 */ s32 unk84;                  /* func_8004AFE0 arg2, stored raw (same value as unk80) */
    /* +0x088 */ u8 pad088[0x0E8 - 0x088];
    /* +0x0E8 */ s32 unkE8;                  /* func_8004ADD0 arg1; func_8004ADD8 reads it back as a NUL-terminated s32 tag array -- true element type still s32, only usage differs per call site */
    /* +0x0EC */ u8 pad0EC[0x1C0 - 0x0EC];
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
 * Opaque descriptor buffer passed as func_8004AFE0's arg1 (and, out of this
 * round's scope, func_8004AEA4/func_8004B030's shared buffer). Populated by
 * a call through Class866E8Methods slot +0x110, which is not decompiled
 * anywhere yet, so only the two bytes func_8004AFE0 actually reads are
 * typed. Named after the convention in code_171e0.h (`Unk*Obj_<unit>`).
 */
typedef struct {
    u8 pad0[2];
    s8 unk2;
    s8 unk3;
} UnkArgObj_3ac78;

/*
 * Generic "object with a vtable pointer at offset 0" view, used only by
 * func_8004AB88 to read the low byte of another object's vtable header
 * word as a type tag (0x34 here). Several unrelated class tables share
 * that low byte (D_800878D4 and DREAMSYS_METHODS among them, per
 * docs/research/class-framework.md), so this reads as a family/base-class
 * check, not an exact-class check. True type of the pointed-to object is
 * unconfirmed; only the one byte this function reads is modeled here.
 */
typedef struct {
    s32 header;
} GenericMethodsHeader;

typedef struct {
    GenericMethodsHeader *methods;
} GenericObject;

#endif
