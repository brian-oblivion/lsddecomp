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
 * (func_8001CAF4). Round 2 (func_8001D0EC, func_8001CE30, func_8001D1A4)
 * filled in most of the rest of this layout:
 *   +0x000  a flag/state word: 1 after func_8001CE30 (the ctor's own init
 *           hook) runs, 0 again after func_8001D0EC's "attach" and after
 *           func_8001CEB4/func_8001D008 (still queued, but their own tails
 *           both end `self->unk14->unk0 = 0` per their disassembly) do
 *           their work -- reads like a "pending update" flag.
 *   +0x018..+0x020  a Vec3 (x,y,z), written wholesale by func_8001D0EC from
 *           its optional 3rd argument (or zeroed if that argument is NULL).
 *   +0x044  a second heap block (0x28 bytes, alloc'd by the ctor).
 *   +0x048  a flag/back-reference word: zeroed by the ctor and by
 *           func_8001D1A4 (slot +0x050), set to `obj->unk14` by
 *           func_8001D0EC's "attach".
 * +0x004..+0x018 and +0x024..+0x044 are still unknown -- not padded out
 * field-by-field, since nothing this unit's chosen functions touch reads
 * them. */
struct Class6B5CCSub14 {
    s32 unk0;                    /* +0x000, state/pending flag -- see above */
    u8 pad04[0x018 - 0x004];
    s32 unk18;                   /* +0x018, Vec3.x */
    s32 unk1C;                   /* +0x01C, Vec3.y */
    s32 unk20;                   /* +0x020, Vec3.z */
    u8 pad24[0x044 - 0x024];
    void *unk44;  /* +0x044, a second heap block (0x28 bytes, alloc'd by the ctor) */
    s32 unk48;    /* +0x048, zeroed by the ctor and by func_8001D1A4 (slot +0x050);
                   * set to `obj->unk14` by func_8001D0EC's "attach" */
};

/* A plain 3-word vector, used only as func_8001D0EC's optional 3rd
 * argument (copied wholesale into Class6B5CCSub14::unk18/unk1C/unk20). */
typedef struct Vec3_d294 {
    s32 x;
    s32 y;
    s32 z;
} Vec3_d294;

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
    /* +0x010/+0x014, both round-2 finds (func_8001CC48/func_8001CCB4):
     * a `(self, other)` pair this class's own +0x010/+0x014 overrides
     * (func_8001CC48/func_8001CCB4) forward to unconditionally, after/before
     * their own extra work. Real BasicClass-level meaning unknown from
     * this unit alone. */
    void (*slot10)(void *self, void *other); /* +0x010 */
    void (*slot14)(void *self, void *other); /* +0x014 */
    void (*slot18)(void *self);              /* +0x018, func_8001CD20's forward target */
};

extern BasicClassMethodsD294 *func_80018390(void);
extern void *func_80017B34(s32 size);
extern void func_80017CFC(void *arg);

/* A second small class, only ever seen through self->unkC (see
 * Class6B5CCObj below) and through func_8001D0EC's 2nd argument -- an
 * "owner" this class can register/unregister with. Two slots seen so far,
 * `slot10`/`slot14` (func_8001D0EC/func_8001D1A4's call sites), both
 * `(owner, Class6B5CCObj *self)` -- read as an attach/detach pair. `unk14`
 * is a plain data field (func_8001D0EC reads it into
 * `self->unk14->unk48`); real meaning unknown. Real class identity
 * unknown -- named for the field it lives behind, per this unit's own
 * convention (see Class6B5CCSub14, also field-named). */
typedef struct UnkOwner_d294 UnkOwner_d294;
typedef struct UnkOwnerMethods_d294 UnkOwnerMethods_d294;
struct UnkOwnerMethods_d294 {
    u8 pad0[0x010];
    void (*slot10)(UnkOwner_d294 *owner, Class6B5CCObj *self); /* +0x010, "attach" */
    void (*slot14)(UnkOwner_d294 *owner, Class6B5CCObj *self); /* +0x014, "detach" */
};
struct UnkOwner_d294 {
    UnkOwnerMethods_d294 *methods; /* +0x000 */
    u8 pad04[0x014 - 0x004];
    s32 unk14;                     /* +0x014 */
};

/* A generic "just enough to dispatch" view of some OTHER class, used where
 * this unit's queued functions read another object's own vtable header tag
 * or call one specific slot without knowing (or needing) the rest of that
 * class's layout. Same shape/convention as class_3bb8c.h's
 * `GenericTagInst_3bb8c_c`. */
typedef struct GenericMethods_d294 GenericMethods_d294;
typedef struct GenericObj_d294 GenericObj_d294;
struct GenericMethods_d294 {
    s32 header;                  /* +0x000, low nibble is a class-tag; func_8001CC48/CCB4 compare it against 9 */
    u8 pad004[0x050 - 0x004];
    void (*slot50)(GenericObj_d294 *self); /* +0x050, func_8001D204's call target */
};
struct GenericObj_d294 {
    GenericMethods_d294 *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    void *unkC;                    /* +0x00C, func_8001D280 (still queued) compares this to a Class6B5CCObj* */
};

/* Class6B5CC's own table (D_8006B5CC). Only the slots this unit's chosen
 * functions actually dispatch through are typed; see the file banner above
 * for the full slot census from classtable.py. */
struct Class6B5CCMethods {
    s32 header;                              /* +0x000 */
    void *unk04;                              /* +0x004, BasicClass__func_17eb0, inherited, unused here */
    void *(*ctor)(void *self);                /* +0x008, func_8001CAF4 (this unit) */
    void  (*dtor)(void *self);                /* +0x00C, func_8001CBA4 (this unit) */
    u8 pad010[0x040 - 0x010];
    void (*slot40)(Class6B5CCObj *self);      /* +0x040, func_8001CE30 (this unit) */
    /* +0x044/+0x048, a `(self, s32 flag, void *data)` pair -- func_8001CE30
     * calls both with flag=1 and `data` pointing at a 3-entry table of
     * {s16,s16} pairs (D_8006B684/D_8006B690, 0xC bytes each). func_8001D008
     * (slot +0x048's own occupant, still queued) confirms the `data` shape:
     * it reads three such pairs via func_8001EC84. */
    void (*slot44)(Class6B5CCObj *self, s32 flag, void *data); /* +0x044, func_8001CEB4 -- still queued */
    void (*slot48)(Class6B5CCObj *self, s32 flag, void *data); /* +0x048, func_8001D008 -- still queued */
    u8 pad04C[0x050 - 0x04C];
    void (*slot50)(Class6B5CCObj *self);      /* +0x050, func_8001D1A4 (this unit) */
    void (*slot54)(Class6B5CCObj *self);      /* +0x054, func_8001D204 (this unit) */
    /* +0x058, func_8001D280 (still queued). func_8001D204's own call site
     * establishes its signature: writes an output entry pointer and an
     * output "more remain" flag through its 2nd/3rd arguments. */
    void (*slot58)(Class6B5CCObj *self, GenericObj_d294 **outEntry, s32 *outCont); /* +0x058 */
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
    /* +0x004..+0x00C: BasicClass instance fields, owned by whatever unit
     * decompiles BasicClass itself -- EXCEPT +0x004, read directly by
     * func_8001D280 (this unit) as a list-head pointer (seeded into its
     * search cursor whenever the caller hasn't found an entry yet). Split
     * out of the opaque blob for that one reason; still don't know
     * BasicClass's own name for it. */
    void *unk4;
    u8 unk8[0x00C - 0x008];
    /* +0x00C, an owner back-reference. RETYPED round 2 from an untyped
     * `void *` to `UnkOwner_d294 *` once func_8001D0EC (this unit's own
     * "attach" -- sets `self->unkC = obj` and calls `obj->methods->slot10`)
     * and func_8001D1A4 (the "detach", `self->unkC->methods->slot14`)
     * were both matched: both dispatch through the exact same 2-slot
     * shape, `UnkOwnerMethods_d294`, at the exact same slot offsets. Not
     * just a plausible guess -- the two call sites agree byte-for-byte on
     * what lives at +0x010/+0x014 of whatever this points to. */
    UnkOwner_d294 *unkC;
    u32 unk10;                  /* +0x010, a packed bit-flags word -- see func_8001EDAC below */
    Class6B5CCSub14 *unk14;     /* +0x014, the ctor's 0x50-byte allocation */
    s32 unk18;                  /* +0x018, zeroed by the ctor */
    u8 unk1C[0x020 - 0x01C];    /* unknown; not touched by this unit's chosen functions */
    s32 unk20;                  /* +0x020, zeroed by the ctor. func_8001CC48's still-queued
                                  * forward target func_8001E770 (code_d294_b.s) stores its
                                  * own 2nd argument into this offset -- i.e. unk20 is
                                  * genuinely a pointer once that function runs, not just an
                                  * always-zero s32. Left untyped `s32` here since nothing
                                  * THIS unit's chosen functions dereference through it; a
                                  * later carve of code_d294_b should retype it. */
    s32 unk24;                  /* +0x024, zeroed by func_8001CE30 (this unit) */
};

/* func_8001EC84 (asm/code_d294_b.s, still uncarved): reads a `{s16 whole;
 * s16 frac;}`-shaped pair at the given pointer and returns a 20.12
 * fixed-point value (whole << 12 | frac's own division-derived low bits) --
 * read off its own disassembly (a `div` by the pair's own two fields, not
 * decompiled here). func_8001D008 (still queued) applies it three times in
 * a row, at offsets +0x0, +0x4, +0x8 of its 3rd argument -- i.e. that
 * argument is a 3-entry table of these pairs (an angle-like x/y/z triple,
 * degrees-and-fraction each). Declared here with a `void *` argument since
 * this unit's chosen functions only ever pass the pointer through, never
 * dereference the pair themselves. */
extern s32 func_8001EC84(void *pair);

/* D_8006B684/D_8006B690 (rodata): two 3-entry, 0xC-byte tables in the shape
 * func_8001EC84 reads (see above) -- func_8001CE30's own literal `data`
 * arguments to slot +0x044/+0x048. Declared as opaque byte blobs since
 * nothing this unit's chosen functions read out of them directly (only
 * their address is taken and forwarded). */
extern u8 D_8006B684[0xC];
extern u8 D_8006B690[0xC];

/* func_8001E770/func_8001E7B0 (asm/code_d294_b.s, still uncarved), and
 * func_80012838 (asm/psyq_2258.s, Psy-Q library, not game code): three
 * helpers this unit's own +0x010/+0x014/+0x018/+0x040 overrides forward
 * into. func_8001E7B0's whole body is `self->unk18 = 0; self->unk20 = 0;`
 * (MEASURED, two `sw $zero` stores, no branches) -- confirms unk18/unk20
 * above independently of the ctor. func_8001E770 and func_80012838 are
 * NOT decompiled here (out of this carve's scope); declared only with the
 * argument shape their call sites need. */
extern void func_8001E7B0(Class6B5CCObj *self);
extern void func_8001E770(Class6B5CCObj *self, GenericObj_d294 *other);
extern void func_80012838(s32 arg0, void *dest);

/* func_800183A0 (asm/code_8220_b.s, a DIFFERENT still-uncarved unit): a
 * generic intrusive-list "pop next" step. Given `out` and `cursor`
 * (both `T **`), if `*cursor` is non-NULL: `*out = (*cursor)->unk4`
 * (the node's own "next" field) and `*cursor = (*cursor)->unk0` (some
 * other per-node link -- NOT necessarily the same "next" field, going by
 * its own disassembly). If `*cursor` is NULL, `*out = NULL`. Declared
 * here typed to func_8001D280's own call site (the only caller reachable
 * from this unit) rather than generically. */
extern void func_800183A0(GenericObj_d294 **out, GenericObj_d294 **cursor);

extern Class6B5CCObj *func_8001CA94(void);
void *func_8001CAF4(Class6B5CCObj *self);
void func_8001CBA4(Class6B5CCObj *self);
void func_8001CC48(Class6B5CCObj *self, GenericObj_d294 *other);
void func_8001CCB4(Class6B5CCObj *self, GenericObj_d294 *other);
void func_8001CD20(Class6B5CCObj *self);
void func_8001CE30(Class6B5CCObj *self);
Class6B5CCObj *func_8001D0EC(Class6B5CCObj *self, UnkOwner_d294 *obj, Vec3_d294 *vec);
Class6B5CCObj *func_8001D1A4(Class6B5CCObj *self);
void func_8001D204(Class6B5CCObj *self);
void func_8001D280(Class6B5CCObj *self, GenericObj_d294 **entry, GenericObj_d294 **cursor);

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

/* Round 12 (code_d294_b): four more self->unk10 bitfield siblings, same
 * family as the five above. See src/code_d294_b.c for the per-function
 * shift/width/return-type notes. */
u32 func_8001D424(Class6B5CCObj *self, u32 a1);
s32 func_8001D450(Class6B5CCObj *self, s32 a1);

#endif
