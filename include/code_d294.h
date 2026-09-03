#ifndef CODE_D294_H
#define CODE_D294_H

#include "common.h"

/* code_d294: a FRESH CARVE (round 10), the first 20-function slice of a
 * 55-function segment never carved before. `tools/classtable.py D_8006B5CC`
 * shows this unit's own method table starts at D_8006B5CC and its slots,
 * in order, ARE this unit's functions: +0x008 func_8001CAF4 (ctor),
 * +0x00C func_8001CBA4 (dtor), +0x010 func_8001CC48, +0x014 func_8001CCB4,
 * +0x018 func_8001CD20, [+0x01C..+0x038 seven slots inherited verbatim from
 * BasicClass, D_8006B58C], +0x038 func_8001CD60 (override), +0x03C null,
 * +0x040 func_8001CE30, +0x044 func_8001CEB4, +0x048 func_8001D008,
 * +0x04C func_8001D0EC, +0x050 func_8001D1A4, +0x054 func_8001D204,
 * +0x058 func_8001D280, +0x05C func_8001D33C (already-matched no-op stub),
 * +0x060 func_8001D344, +0x064 func_8001D374, +0x068 func_8001D3A0,
 * +0x06C func_8001D3CC, +0x070 func_8001D3F8, and continuing past this
 * unit's slice into the next carve (code_d294_b) up to +0x0B4
 * func_8001E4A4 (45 slots total). This is proof, not a guess -- `--vs
 * D_8006B58C` confirms the class overrides 5 BasicClass slots (+0x008,
 * +0x00C, +0x010, +0x014, +0x018), inherits 7 verbatim (+0x01C..+0x038),
 * then overrides +0x038 and adds everything from +0x040 on as new virtuals.
 *
 * The class is unnamed -- no hypothesis about its purpose has surfaced yet
 * (no suggestive strings/symbol names reachable from this slice). Named
 * `Class6B5CC` after its table address, following this project's
 * established convention for classes discovered by table address rather
 * than by a plausible role (see Class6D3C8, Class86AA0, Class65650).
 *
 * func_8001CA94 is NOT a table slot -- it is the `New_Class6B5CC` allocator
 * wrapper (allocates 0x44 bytes, calls the ctor via `func_8001E57C()->ctor`,
 * frees and returns NULL on ctor failure). func_8001E57C is a plain
 * no-argument getter, `lui/addiu %hi/%lo(D_8006B5CC); jr $ra` -- MEASURED,
 * see include/class_3bb8c.h's own note on this exact symbol for the
 * "arity/signature is per-call-site, not a callee property" precedent this
 * follows: other units call the same func_8001E57C symbol with a different
 * argument count and are equally byte-exact. Declared here with 0 arguments
 * and a Class6B5CCMethods* return type, matching only THIS unit's call site
 * (func_8001CA94).
 */

typedef struct Class6B5CCObj Class6B5CCObj;
typedef struct Class6B5CCMethods Class6B5CCMethods;
typedef struct Class6B5CCSub14 Class6B5CCSub14;

/* self->unk14's target: a 0x50-byte block allocated by the ctor
 * (func_8001CAF4). Only two fields are touched by this unit's queued
 * functions: +0x044 (a second, independently allocated 0x28-byte block)
 * and +0x048 (a flag/count word, zeroed by both the ctor and func_8001D1A4
 * i.e. slot +0x050). The gap between them and everything before +0x044 is
 * unknown -- not padded out, since nothing here reads it. */
struct Class6B5CCSub14 {
    u8 pad0[0x044];
    void *unk44;  /* +0x044, a second heap block (0x28 bytes, alloc'd by the ctor) */
    s32 unk48;    /* +0x048, zeroed by the ctor and by func_8001D1A4 (slot +0x050) */
};

/* This unit's own minimal, local view of the shared BasicClass ancestor
 * table (D_8006B58C, returned by func_80018390, which lives in the
 * still-uncarved code_8220 segment) -- same shape and same "ctor at +0x008,
 * dtor at +0x00C universally" convention already established independently
 * in include/class_16334.h and include/code_171e0.h. Declared again here,
 * under a unit-local name, per this project's policy of NOT unifying
 * independent local views of the same table into one shared header. */
typedef struct BasicClassMethodsD294 BasicClassMethodsD294;
struct BasicClassMethodsD294 {
    s32 header;               /* +0x000 */
    void *unk04;               /* +0x004 */
    void *(*ctor)(void *self); /* +0x008 */
    void *(*dtor)(void *self); /* +0x00C */
};

extern BasicClassMethodsD294 *func_80018390(void);
extern void *func_80017B34(s32 size);
extern void func_80017CFC(void *arg);

/* Class6B5CC's own table (D_8006B5CC). Only the slots this unit's chosen
 * functions actually dispatch through are typed; see the file banner above
 * for the full slot census from classtable.py. */
struct Class6B5CCMethods {
    s32 header;                              /* +0x000 */
    void *unk04;                              /* +0x004, BasicClass__func_17eb0, inherited, unused here */
    void *(*ctor)(void *self);                /* +0x008, func_8001CAF4 (this unit) */
    void  (*dtor)(void *self);                /* +0x00C, func_8001CBA4 (this unit) */
    u8 pad010[0x040 - 0x010];
    void (*slot40)(Class6B5CCObj *self);      /* +0x040, func_8001CE30 -- still queued */
    u8 pad044[0x050 - 0x044];
    void (*slot50)(Class6B5CCObj *self);      /* +0x050, func_8001D1A4 -- still queued */
    void (*slot54)(Class6B5CCObj *self);      /* +0x054, func_8001D204 -- still queued */
    u8 pad058[0x05C - 0x058];
    /* +0x05C, func_8001D33C -- already matched as a no-op `void(void)`
     * body, but THIS call site (func_8001CBA4's dtor) passes it 2 args
     * (self, 0). Both are right about their own codegen: the callee body
     * ignores every argument, so the caller's arity is unconstrained. Same
     * "per-call-site signature" precedent as func_8001E57C above. */
    void (*slot5C)(Class6B5CCObj *self, s32 arg1);
};

/* MEASURED: func_8001E57C's whole body is `lui/addiu %hi/%lo(D_8006B5CC);
 * jr $ra` (asm/code_d294_b.s) -- a plain no-argument getter for this unit's
 * own vtable. See the file banner for the cross-unit precedent on why this
 * local 0-argument declaration doesn't need to agree with other units'. */
extern Class6B5CCMethods *func_8001E57C(void);

struct Class6B5CCObj {
    Class6B5CCMethods *methods; /* +0x000 */
    u8 unk04[0x00C - 0x004];    /* BasicClass instance fields; owned by whatever unit decompiles BasicClass itself */
    /* +0x00C, a pointer field: zeroed by the ctor, read/cleared by
     * func_8001D1A4 (slot +0x050, still queued) which treats it as
     * `((SomeObj *)self->unkC)->methods->slot14(self)` when non-NULL --
     * some kind of owner/parent back-reference. Real target type unknown;
     * left untyped (void *) since nothing this unit's chosen functions
     * touch dereferences it. */
    void *unkC;
    u32 unk10;                  /* +0x010, a packed bit-flags word -- see func_8001EDAC below */
    Class6B5CCSub14 *unk14;     /* +0x014, the ctor's 0x50-byte allocation */
    s32 unk18;                  /* +0x018, zeroed by the ctor */
    u8 unk1C[0x020 - 0x01C];    /* unknown; not touched by this unit's chosen functions */
    s32 unk20;                  /* +0x020, zeroed by the ctor */
};

extern Class6B5CCObj *func_8001CA94(void);
void *func_8001CAF4(Class6B5CCObj *self);
void func_8001CBA4(Class6B5CCObj *self);

/* func_8001EDAC (asm/code_d294_b.s, the NEXT slice, still uncarved): a
 * generic packed-bitfield accessor. Given a word pointer, a bit SHIFT, a
 * bit WIDTH and a VALUE, it clears WIDTH bits at bit-offset SHIFT in *word,
 * ORs in (value << shift), and returns the PREVIOUS contents of that
 * bitfield (shifted back down to bit 0). MEASURED from its own disassembly
 * (asm/code_d294_b.s @ func_8001EDAC): a `while` loop builds `(1 << width)
 * - 1` one bit at a time (i.e. computes a WIDTH-bit mask, not a
 * `(1<<width)-1` closed form -- retail's own source apparently spelled it
 * as the loop), then shifts that mask into position, clears/sets, and
 * shifts the old value back down. Five of this unit's own functions
 * (func_8001D344/D374/D3A0/D3CC/D3F8) are thin wrappers around this,
 * always over `&self->unk10`, at five non-overlapping bit positions
 * (shift 3 width 3, shift 6 width 1, shift 28 width 2, shift 30 width 1,
 * shift 31 width 1) -- i.e. self->unk10 is a packed flags/small-fields
 * register and these five functions are its per-field setters. */
extern u32 func_8001EDAC(u32 *word, s32 shift, s32 width, u32 value);

s32 func_8001D344(Class6B5CCObj *self, s32 a1);
u32 func_8001D374(Class6B5CCObj *self, s32 a1);
u32 func_8001D3A0(Class6B5CCObj *self, u32 a1);
u32 func_8001D3CC(Class6B5CCObj *self, s32 a1);
u32 func_8001D3F8(Class6B5CCObj *self, u32 a1);

#endif
