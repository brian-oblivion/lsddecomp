#ifndef CLASS_3BB8C_H
#define CLASS_3BB8C_H

#include "common.h"
#include "BasicClass.h"
#include "TaskCore.h"
#include "Class866E8.h"

/*
 * class_3bb8c.c and class_3bb8c_b.c hold the back half of Class866E8 (the
 * grid manager, gClass866E8Methods), which is declared once, in
 * include/Class866E8.h (track 4, round 89). What stays here is that class's
 * data (the rate and footprint tables) and the helper views its two units
 * read through casts.
 */
typedef struct Unk14Obj Unk14Obj;
typedef struct QueryTemplate866E8 QueryTemplate866E8;

/* Constant `Unk54Struct` (unk0=-1, unk4=0, unk8=0x140014) whole-struct-copied
 * by Class866E8__InitFootprintSlot into rects[key]. */
extern Unk54Struct gDefaultElemRateOffset;

/* The default "enable every element" spec table SetTargetAndBuildRates
 * passes to buildRateEntries: seven entries, every `flag` nonzero
 * (asm/data/76DC8.data.s). */
extern TargetSpec866E8 sDefaultTargetSpecs[7];

/* Indexed by TargetSpec866E8::key in Class866E8__BuildRateEntries, 0xC
 * stride, read as three plain words. Bound unknown (`key` is the caller's
 * byte), so unsized. */
extern Unk54Struct sRateOffsetTable[];

/* `key`-indexed bitmask table (`1 << key`) Class866E8__ComputeRateEntry tests
 * against ComputeRateFlags' result. 7 words in the data before
 * sRateEntryTable starts. */
extern const s32 sRateKeyMask[7];

/* `key`-indexed, three words each: ComputeRateEntry uses `unk0 * divisor`
 * plus `unk4` or `unk8` (by its `flag`), or `unk4` alone when `unk0` is 0.
 * 7 entries (0x800868A8-0x800868FC). */
extern const Unk54Struct sRateEntryTable[7];

/* Class81940::ownerKey-indexed remap, read signed by
 * Class866E8__UpdateFootprintTracking: exactly 8 bytes in the data
 * (01 02 03 00 04 05 06 00) before sDefaultTargetSpecs. The byte (0..6) is
 * UpdateFootprintTracking's return value and the index into
 * sFootprintResultPtrTable. */
extern const s8 sFootprintResultRemap[8];

/* 7 pointers, the first NULL, the rest to the 4-word (seven 2-byte
 * TargetSpec866E8s, padded) tables D_80086914..D_80086964:
 * UpdateFootprintTracking passes the selected one to buildRateEntries as its
 * spec table, as SetTargetAndBuildRates passes sDefaultTargetSpecs. */
extern TargetSpec866E8 *sFootprintResultPtrTable[7];

/*
 * A GsCOORDINATE2 (Class6B5CCSub14) read as the element's origin: an
 * element's cellParent->coord2, reached through a cast. tx (+0x018) and tz
 * (+0x020) are unions because Class866E8__ComputeFootprintDescriptor reads
 * each whole (the cell) and, later, as its low halfword (the offset):
 * retail reloads at the narrower width. Class866E8__BuildRateEntries writes
 * all three and clears `flg` (+0x000).
 */
struct Unk14Obj {
    s32 unk0;                         /* +0x000, GsCOORDINATE2.flg */
    u8 pad4[0x18 - 0x4];
    union { s32 w; u16 h; } unk18;   /* +0x018, tx */
    s32 unk1C;                        /* +0x01C, ty */
    union { s32 w; u16 h; } unk20;    /* +0x020, tz */
};

/*
 * Template struct copied wholesale by Class866E8__ComputeFootprintFromRotation from the constant
 * global `D_8008E98C` into a stack-local descriptor, then partially
 * overwritten (`unk14`/`unk18` zeroed, `unk1C` set from `self->gridSpan`)
 * before being handed to two uncarved library helpers
 * (`RotMatrix`/`ApplyMatrixLV`) as an in/out parameter block. Field
 * meaning beyond "8 words, offsets 0x00-0x1C" is unestablished; the first
 * five words are read/written only as the opaque whole-struct copy.
 */
struct QueryTemplate866E8 {
    s32 unk0[5];                    /* +0x000..+0x010, opaque (untouched by Class866E8__ComputeFootprintFromRotation) */
    s32 unk14;                      /* +0x014, Class866E8__ComputeFootprintFromRotation: zeroed before the call, then an in/out arg to ApplyMatrixLV */
    s32 unk18;                      /* +0x018, Class866E8__ComputeFootprintFromRotation: zeroed before the call */
    s32 unk1C;                      /* +0x01C, Class866E8__ComputeFootprintFromRotation: set to self->gridSpan before the call */
};

extern QueryTemplate866E8 D_8008E98C;

/* Library helpers (Class866E8__ComputeFootprintFromRotation's only call
 * site). `RotMatrix`'s first argument is the target's coord2->param->rotate.
 * `ApplyMatrixLV` is called with its 2nd and 3rd arguments pointing at the
 * SAME address (`&desc.unk14` passed twice) -- confirmed against the raw
 * disassembly (`$a1`/`$a2` both `sp+0x54`). */
extern void RotMatrix(void *arg0, QueryTemplate866E8 *arg1);
extern void ApplyMatrixLV(QueryTemplate866E8 *arg0, s32 *arg1, s32 *arg2); /* arity-ok: this IS the callee's real signature (Sony libgte, 0x80015618 reads $a0 matrix / $a1 in / $a2 out); include/code_d294.h's unprototyped copy is round 19's deliberate frame-sizing shape, not a claim about arity */

/* The four static EntryDesc866E8 entries (0xC apart) configureRateEntry
 * picks for `rateEntry` by (rate > 0, flag != 0). */
extern EntryDesc866E8 D_8008699C;
extern EntryDesc866E8 D_800869A8;
extern EntryDesc866E8 D_800869B4;
extern EntryDesc866E8 D_800869C0;

/* 3-word block Class866E8__ResetChildRate passes to every cell's updateScale
 * (the same 0xC stride as the four above, plausibly a fifth entry). */
extern s32 D_800869CC[3];

/* -------------------------------------------------------------------
 * class_3bb8c_c additions below. Small sibling classes, each built by
 * its own New_X/ctor pair (allocator + base-chain + own-vtable-set, the
 * same shape as Class86668__Class86668 in class_39e08.c). Named by their
 * vtable's address, same convention as Class866E8/Class86668. The first
 * of them, Class869D8 (gClass869D8Methods, a Viewport), is defined in
 * include/Class869D8.h (round 87, track 4).
 * ------------------------------------------------------------------- */

extern void *BMemPMgrAlloc(s32 size);

/* Class86AA0 (gClass86AA0Methods, a Class6B5CC) is defined in
 * include/Class86AA0.h (round 88, track 4). */

/* GetClass6B5CCMethods and its table: include/Class6B5CC.h (track 4, round
 * 81). The local BaseCtorTableB_3bb8c_c view and the unprototyped getter that
 * lived here are gone; round 59 measured both arguments class_3bb8c_c.c passed
 * to the no-argument getter as zero-cost (docs/match-reports/Class86AA0__Class86AA0.md). */


/* Class86B60 (gClass86B60Methods, a TaskCore) is defined in
 * include/Class86B60.h (round 88, track 4). Two views of OTHER classes
 * its methods call stay here, named for the Class86B60 field that holds
 * them: */

/* The class of IntermediateBaseInitArgs::unk0 (a BasicClass * there), as
 * Class86B60__OnDeinit calls its +0x078; TaskCore__OnDeinit makes the same
 * call through code_2c054.h's TaskTextObj. */
typedef struct Class86B60UnkC0ObjMethods_3bb8c_d Class86B60UnkC0ObjMethods_3bb8c_d;
typedef struct Class86B60UnkC0Obj_3bb8c_d Class86B60UnkC0Obj_3bb8c_d;

struct Class86B60UnkC0ObjMethods_3bb8c_d {
    u8 pad000[0x078];
    /* +0x078, Class86B60__OnDeinit's own call: `(childObj, &self->unk93,
     * tableEntry)`, where `tableEntry` walks a fixed external table
     * (`D_80086DAC`, stride 0xC) starting fresh each call to this
     * function. */
    void (*slot78)(Class86B60UnkC0Obj_3bb8c_d *self, void *arg1, void *arg2);
};

struct Class86B60UnkC0Obj_3bb8c_d {
    Class86B60UnkC0ObjMethods_3bb8c_d *methods; /* +0x000 */
};

/* Class86B60::saveCtrl's class: New_TaskObjF's object (gTaskObjFMethods,
 * class id 0xB), as Class86B60's methods call it. A view of TaskObjF, not
 * of Class86B60; include/Class86B60.h refers to it by tag. */
typedef struct Class86B60UnkACObjMethods_3bb8c_d Class86B60UnkACObjMethods_3bb8c_d;
typedef struct Class86B60UnkACObj_3bb8c_d Class86B60UnkACObj_3bb8c_d;

struct Class86B60UnkACObjMethods_3bb8c_d {
    u8 pad000[0x004];
    void (*release)(Class86B60UnkACObj_3bb8c_d *self); /* +0x004, Class86B60__Finalize */
    u8 pad008[0x06C - 0x008];
    /* +0x06C, Class86B60__BeginMemcardSave's own call: `(self, D_8008A9D0, &D_80086D6C,
     * self->initArgs->unk4, self->unk10, self->unk14, self->sound)` -- 7
     * arguments, the last three on the stack. Every pointer beyond `self`
     * is forwarded opaquely (never dereferenced by this slot's own
     * caller), so all stay `void *`/`s32 *` placeholders. */
    void (*slot6C)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, s32 *arg2,
                   void *arg3, void *arg4, void *arg5, void *arg6); /* +0x06C */
    void (*slot70)(Class86B60UnkACObj_3bb8c_d *self); /* +0x070, Class86B60__EndMemcardSave */
    /* +0x074, Class86B60__UpdateMemcardSaveStatus's own 2nd call: `(self, D_8008AA10, D_8008AA18,
     * self->saveBlock, self->saveBlockSize)` -- the two middle arguments are the
     * VALUES of two `.sdata` globals loaded via `%gp_rel` (not their
     * addresses), each holding a pointer into the still-uncarved rodata
     * block at `D_80011434` (verified in `asm/data/1C34.rodata.s`: no
     * dlabel exists at either byte offset, so they cannot be referenced by
     * name and are forwarded as opaque `void *`). */
    void (*slot74)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, void *arg2, s32 arg3, s32 arg4); /* +0x074 */
    /* +0x078, Class86B60__UpdateMemcardSaveWithIcon's own call: `(self, D_8008AA10, D_8008AA18,
     * 0xD, 3, self->iconHandle, self->saveBlock, self->saveBlockSize)` --
     * eight arguments, the last four on the stack. `arg5` is
     * `Class86B60::iconHandle` (a TimImage), forwarded opaquely (never
     * dereferenced by this slot's own caller), so kept `void *`. */
    void (*slot78)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, void *arg2,
                   s32 arg3, s32 arg4, void *arg5, s32 arg6, s32 arg7); /* +0x078 */
};

struct Class86B60UnkACObj_3bb8c_d {
    Class86B60UnkACObjMethods_3bb8c_d *methods; /* +0x000 */
};


/* Class86B60's menu description (include/TaskCore.h): Class86B60__Class86B60
 * passes &D_80086D44 as TaskCore's ctor's `target` and again to setTarget.
 * 0x28 bytes in asm/data/76DC8.data.s, a path word, three zero words, the two
 * colour triples and four pointers: the TaskCoreTarget layout. RETYPED from a
 * placeholder `s32` in round 88 (address-of only; no byte change). */
extern TaskCoreTarget D_80086D44;

/* "ETC\ETCSE" (asm/data/1C34.rodata.s), Class86B60__Class86B60's
 * soundBankPath for TaskCore's ctor. RETYPED from a placeholder `s32` in
 * round 88 (address-of only); the ctor casts away the const for the
 * ctor's `char *`. */
extern const char D_800114DC[];

/* "ETC\TITLE.TIM" (asm/data/1C34.rodata.s), Class86B60__Reset's path for
 * setSubHandle. RETYPED from a placeholder `s32` in round 88 (address-of
 * only). */
extern const char D_800114E8[];

/* Class86B60__BeginMemcardSave's own path string, passed to New_TimImage -- a real
 * dlabel (`asm/data/1C34.rodata.s`: "CARD\FILEICN1.TIM"), so this is the
 * ONLY correct spelling (CLAUDE.md: never re-write a string splat has
 * already emitted as a symbol). */
extern const char D_800114F8[];

/* Address-of only in this unit -- Class86B60__OnDeinit walks it with an
 * explicit 0xC-byte stride, passing each entry's address on to
 * `Class86B60UnkC0ObjMethods_3bb8c_d::slot78`, but never dereferences it
 * itself. Placeholder s32 type; real element layout unknown. */
extern s32 D_80086DAC;

/* Address-of only in this unit (Class86B60__ForwardToNameField passes &D_8008A9B4 to
 * the name field's attachToParent, TextRow +0x04C, as its Vec3_d294 offset;
 * the words are -4, -23, ...). Placeholder s32 type, cast at the call. */
extern s32 D_8008A9B4;

/* VALUE-of, not address-of, in this unit -- Class86B60__UpdateMemcardSaveStatus reaches these
 * through `%gp_rel` loads of the .sdata globals themselves, forwarding
 * whatever they hold. Each holds a pointer into the still-uncarved rodata
 * block at `D_80011434` (`asm/data/1C34.rodata.s`: 0x80011464 and
 * 0x8001149C respectively, neither with its own dlabel), so they cannot
 * be spelled by the address they point to and are typed opaque `void *`
 * instead.
 *
 * `D_8008AA18` is also read by round 43's `Class86B60__CreateNameField`, which
 * `strcpy`s INTO `(char *)D_8008AA18 + 0x18` and reads it with `strlen` --
 * both require the RUNTIME value to be a writable buffer, not the .rodata
 * address the ROM image happens to initialise it to. Nothing in this unit
 * ever reassigns it, so whatever sets the real (writable) value is outside
 * this unit's own ground; the ROM-image value above is a placeholder only. */
extern void *D_8008AA10;
extern void *D_8008AA18;

/* Same VALUE-of `%gp_rel` pattern, read (and its buffer formatted into via
 * FormatFullWidthNumber) by round 45's `FormatNumberIntoBuffer` (src/class_3bb8c_c.c). Holds
 * `D_8008AA1C` in the ROM image -- the "7654321" placeholder string
 * (`asm/data/7B12C.sdata.s`) -- so, like `D_8008AA18` above, this is a
 * writable-buffer placeholder rather than the real runtime value. */
extern void *D_8008AA24;

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `Class86B60__CreateNameField`
 * as `strcpy`'s SOURCE argument. Holds `0x80011474` in the ROM image
 * (immediately past `D_8008AA10`'s own "BISLPS-01556xxx" string, i.e. the
 * start of the font-glyph word table in `D_80011434`) -- likely also a
 * placeholder for the same reason `D_8008AA18` is, since a font-glyph
 * table is not plausible `strcpy` input. */
extern void *D_8008AA14;

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `Class86B60__BeginMemcardSave`
 * as `Class86B60UnkACObjMethods_3bb8c_d::slot6C`'s own `arg1`. Holds
 * `0x80011454` in the ROM image -- the "BISLPS-01556" string in
 * `D_80011434`, again with no `dlabel` of its own. */
extern void *D_8008A9D0;

/* Address-of only, round 43's `Class86B60__BeginMemcardSave`
 * (`Class86B60UnkACObjMethods_3bb8c_d::slot6C`'s own `arg2`) -- a real
 * 16-entry pointer table (`asm/data/76DC8.data.s`, `D_8008AA0C` down to
 * `D_8008A9D4` then a NULL terminator), reached only by its own address
 * here, never walked. Placeholder `s32` type since only the address is
 * taken. */
extern s32 D_80086D6C;

/* Class86B60__TickNameFieldCursor's own rolling byte index (0/1/2, wraps to 0 at 3) into
 * that function's own 3-byte stack buffer -- declared in the ROM image
 * as a full `.word` (`asm/data/7B12C.sdata.s`), but accessed only via
 * `lbu`/`sb` here, so `u8` is the correct C type for this unit's own
 * reference regardless of the underlying storage's full width. */
extern u8 D_8008AA28;

/* Class86B60__TickNameFieldCursor's own rolling word counter (wraps to 0 at 0x101). */
extern s32 D_8008AA2C;

/* TaskObjF__TaskObjF's own one-shot init guard: read, then unconditionally
 * incremented, before its own body's InitCARD/StartCARD/_bu_init calls
 * run only when the PRE-increment value was 0 (i.e. only on the very
 * first construction of this class). */
extern s32 D_8008AA30;

/* Still raw asm in this unit (gp-relative-blocked, see
 * docs/match-reports/FormatNumberIntoBuffer.md) -- not this round's function, but
 * Class86B60__Class86B60 calls it with one forwarded s32 argument (the return
 * value of dreamSys->methods->slot1A0); return value unused there. */
extern void FormatNumberIntoBuffer(s32 arg0);

/*
 * The block CheckSaveScoreFlag reads: its caller's Class86B60::saveBlock
 * (the DreamSys's getSaveBlock, &saveMagic). +0x00C from saveMagic is
 * DreamSys's totalFlasbackUnlockScore (include/DreamSys.h); +0x2F4 is
 * unnamed there.
 */
typedef struct SaveBlock678_3bb8c_c {
    u8 pad00[0x00C];
    s32 unkC;                                   /* +0x00C, compared against 9999999 */
    u8 pad10[0x2F4 - 0x010];
    s32 unk2F4;                                 /* +0x2F4, zero-checked when unkC > 9999999 */
} SaveBlock678_3bb8c_c;

/*
 * First argument of CheckSaveScoreFlag: its one caller,
 * Class86B60__CommitNameEntry (class_3bb8c_d), passes its own self, so
 * +0x0BC is Class86B60::saveBlock (include/Class86B60.h).
 */
typedef struct Ctx678_3bb8c_c {
    u8 pad00[0x0BC];
    SaveBlock678_3bb8c_c *target;               /* +0x0BC */
} Ctx678_3bb8c_c;

/*
 * Second argument of CheckSaveScoreFlag: holds a pointer at +0x018 to a small
 * result block whose word at +0x004 is the flag CheckSaveScoreFlag computes.
 */
typedef struct Result678_3bb8c_c {
    u8 pad00[0x018];
    s32 *block;                                 /* +0x018, CheckSaveScoreFlag writes block[1] */
} Result678_3bb8c_c;

/*
 * New_TaskObjF's own New_X allocator target -- yet another small
 * sibling class (same shape as `Class869D8`/`Class86AA0`/`Class86B60`
 * above): pool-allocate a fixed 0x84-byte block, and if it succeeds,
 * construct it through this table's own `ctor` slot at `+0x008`. Kept
 * fully opaque (no instance type at all) since New_TaskObjF never
 * dereferences the allocation itself, only forwards it.
 */
typedef struct GenericCtorTable_3bb8c_d GenericCtorTable_3bb8c_d;
struct GenericCtorTable_3bb8c_d {
    u8 pad000[0x008];
    void (*ctor)(void *self, void *arg1, void *arg2); /* +0x008, New_TaskObjF's own call; IS TaskObjF__TaskObjF -- see that function's own, more precise (s32, s32) local declaration in class_3bb8c_d.c, kept separate per the project's independent-arities convention since nothing here type-checks the two against each other */
    u8 pad00C[0x040 - 0x00C];
    /* +0x040, TaskObjF__TaskObjF's own last call, forwarding its own 3rd
     * parameter verbatim; class_3bb8c_e's independent view (round 14,
     * this same real object) names the concrete function `TaskObjF__SetCardSlot`,
     * still uncarved there. Round 78: renamed slot40 -> setCardSlot
     * (classtable-confirmed occupant TaskObjF__SetCardSlot). */
    void (*setCardSlot)(void *self, s32 arg1);
};

extern GenericCtorTable_3bb8c_d gTaskObjFMethods;
extern GenericCtorTable_3bb8c_d *GetTaskObjFMethods(void); /* returns &gTaskObjFMethods; matched in class_3bb8c_g */

/*
 * The object instance itself -- established this round by TaskObjF__TaskObjF,
 * which IS this class's own constructor (verified: `tools/classtable.py
 * 0x80086DC4` places it at the table's own +0x008 ctor slot). Kept
 * minimal (only the one field this unit's ctor writes) since nothing else
 * here dereferences it -- the fuller shape belongs to class_3bb8c_e's own
 * independent view of the SAME real object (`Node3bb8cE`,
 * src/class_3bb8c_e.c), which this unit does not include (per this
 * project's established multiple-independent-local-views convention).
 */
typedef struct GenericCtorObj_3bb8c_d GenericCtorObj_3bb8c_d;
struct GenericCtorObj_3bb8c_d {
    GenericCtorTable_3bb8c_d *methods; /* +0x000, TaskObjF__TaskObjF: self->methods = GetTaskObjFMethods() -- the base-ctor-chain "sets self->methods directly to this table's own pointer" pattern already seen for Class86B60/Class86B60__Class86B60 */
};

/*
 * Class86E00 -- NOT a separate class (round 73 head correction). The name
 * came from reading `tools/classtable.py 0x80086E00` as a table start, but
 * nothing in the image addresses 0x80086E00: it is +0x03C INSIDE
 * `gTaskObjFMethods` (0x80086DC4, 44 slots, header word 0xB), whose +0x064..
 * +0x078 are the `TaskObjF__*` methods of class_3bb8c_f and whose +0x07C..
 * +0x0B0 are class_3bb8c_g's. So `Class86E00_3bb8c_g` below is class_3bb8c_g's
 * unit-local view of `TaskObjF`, and its methods are named `TaskObjF__*`.
 * Slot offsets quoted as "Class86E00 +N" in older comments are 0x3C short of
 * the real table offset. Track 4 merges these views into one TaskObjF.
 *
 * class_3bb8c_g is the first unit to write any of THIS class's own
 * methods -- specifically slots `+0x048` and up. Slots `+0x004..+0x03C`
 * belong to `class_3bb8c_e`/`class_3bb8c_f`, this round's other two
 * runners sharing this header; they are left fully opaque here since no
 * function in this unit ever dispatches through them.
 *
 * Every type below is suffixed `_3bb8c_g`, INCLUDING the class name
 * itself -- unlike `Class86B60`/`Class869D8`/`Class86AA0` above, which
 * are single-owner. This table is reached by three units at once this
 * round, so an unsuffixed `Class86E00` here could collide at merge with
 * a same-named, differently-shaped definition from `class_3bb8c_e` or
 * `class_3bb8c_f` with no conflict marker to catch it -- the exact
 * round-13 hazard this shared header's rules exist to prevent.
 *
 * Only the offsets this unit's own functions touch are given concrete
 * types; everything else stays opaque padding.
 */
typedef struct Class86E00_3bb8c_g Class86E00_3bb8c_g;
typedef struct Class86E00Methods_3bb8c_g Class86E00Methods_3bb8c_g;

/*
 * self->childC's pointee. TaskObjF__SetChildFlag8 is the function that PROVES this
 * is a pointer (dereferences its `+0x080` vtable slot) -- before that
 * function was read, `childC` looked like a plain `s32` value forwarded
 * opaquely to `Class86E00SubObj_3bb8c_g::slot4C`'s 3rd argument, which is
 * why that parameter is typed with this pointer rather than `s32` below.
 */
typedef struct Class86E00Unk6CObj_3bb8c_g Class86E00Unk6CObj_3bb8c_g;
typedef struct Class86E00Unk6CObjMethods_3bb8c_g Class86E00Unk6CObjMethods_3bb8c_g;

struct Class86E00Unk6CObjMethods_3bb8c_g {
    u8 pad000[0x080];
    /* +0x080, TaskObjF__SetChildFlag8's own call: `(self, arg1, 0x7F, 0x7F)`,
     * `arg1` forwarded verbatim from TaskObjF__SetChildFlag8's own 2nd parameter. */
    void (*slot80)(Class86E00Unk6CObj_3bb8c_g *self, s32 arg1, s32 arg2, s32 arg3);
};

struct Class86E00Unk6CObj_3bb8c_g {
    Class86E00Unk6CObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * self->childA's and self->childB's shared pointee -- two parallel fields
 * of the SAME sub-object shape (TaskObjF__AttachChildA/TaskObjF__DetachChildA exercise
 * `childA`; TaskObjF__AttachChildB/TaskObjF__DetachChildB exercise `childB` the identical
 * way), each independently attached via `Class86E00Methods_3bb8c_g::
 * slot10` and torn down via a fixed `slot50`/`slot48`/`release` sequence.
 */
typedef struct Class86E00SubObj_3bb8c_g Class86E00SubObj_3bb8c_g;
typedef struct Class86E00SubObjMethods_3bb8c_g Class86E00SubObjMethods_3bb8c_g;

struct Class86E00SubObjMethods_3bb8c_g {
    u8 pad000[0x004];
    void (*release)(Class86E00SubObj_3bb8c_g *self); /* +0x004, TaskObjF__DetachChildA/TaskObjF__DetachChildB */
    u8 pad008[0x044 - 0x008];
    /* +0x044, TaskObjF__AttachChildA/TaskObjF__AttachChildB's own call: `(self, childReady)`
     * from the OWNING `Class86E00_3bb8c_g`. */
    void (*slot44)(Class86E00SubObj_3bb8c_g *self, s32 arg1);
    void (*slot48)(Class86E00SubObj_3bb8c_g *self); /* +0x048, TaskObjF__DetachChildA/TaskObjF__DetachChildB */
    /* +0x04C, TaskObjF__AttachChildA/TaskObjF__AttachChildB's own call:
     * `(self, unk60, unk64, childC)` from the OWNING `Class86E00_3bb8c_g`. */
    void (*slot4C)(Class86E00SubObj_3bb8c_g *self, s32 a1, s32 a2, Class86E00Unk6CObj_3bb8c_g *a3);
    void (*slot50)(Class86E00SubObj_3bb8c_g *self); /* +0x050, TaskObjF__DetachChildA/TaskObjF__DetachChildB */
};

struct Class86E00SubObj_3bb8c_g {
    Class86E00SubObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * self->cardIcon's pointee -- a DIFFERENT sub-object from
 * `Class86E00SubObj_3bb8c_g` above. It shares the same `+0x004` slot
 * offset only because every BasicClass-family table keeps a slot there
 * (see `BasicClassMethods::release` in code_8220.h) -- the USAGE differs:
 * TaskObjF__TickCardIcon assigns this call's RETURN VALUE back into `cardIcon` (an
 * "advance" pattern), where `Class86E00SubObj_3bb8c_g::release`'s callers
 * (TaskObjF__DetachChildA/TaskObjF__DetachChildB) discard the return and unconditionally
 * null the field afterward instead. Different enough to keep separate
 * rather than unify.
 */
typedef struct Class86E00Unk70Obj_3bb8c_g Class86E00Unk70Obj_3bb8c_g;
typedef struct Class86E00Unk70ObjMethods_3bb8c_g Class86E00Unk70ObjMethods_3bb8c_g;

struct Class86E00Unk70ObjMethods_3bb8c_g {
    u8 pad000[0x004];
    /* +0x004, TaskObjF__TickCardIcon's own call: return value stored back into
     * `Class86E00_3bb8c_g::cardIcon` itself. */
    Class86E00Unk70Obj_3bb8c_g *(*slot4)(Class86E00Unk70Obj_3bb8c_g *self);
    u8 pad008[0x04C - 0x008];
    /* +0x04C, TaskObjF__LoadCardIcon's own call on a FRESH `cardIcon` right after
     * assigning it: `(self, self->childReady, &D_8008AA94)`. `childReady` is
     * forwarded verbatim -- kept as the owning struct's established
     * bare `s32` reading of that field, not retyped to a pointer here. */
    void (*slot4C)(Class86E00Unk70Obj_3bb8c_g *self, s32 arg1, void *arg2);
};

struct Class86E00Unk70Obj_3bb8c_g {
    Class86E00Unk70ObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * TaskObjF__OnItemSelected's own `arg1` -- a third, unrelated small object, reached
 * only through its own `+0x09C` slot, whose return value is stored into
 * `Class86E00_3bb8c_g::selectedItem`.
 */
typedef struct GenericSlot9CObj_3bb8c_g GenericSlot9CObj_3bb8c_g;
typedef struct GenericSlot9CMethods_3bb8c_g GenericSlot9CMethods_3bb8c_g;

struct GenericSlot9CMethods_3bb8c_g {
    u8 pad000[0x09C];
    void *(*slot9C)(GenericSlot9CObj_3bb8c_g *self); /* +0x09C, TaskObjF__OnItemSelected */
};

struct GenericSlot9CObj_3bb8c_g {
    GenericSlot9CMethods_3bb8c_g *methods; /* +0x000 */
};

struct Class86E00Methods_3bb8c_g {
    u8 pad000[0x010];
    /* +0x010, TaskObjF__AttachChildA/TaskObjF__AttachChildB's own first call: `(self,
     * subObj)`, registering/attaching whichever of `childA`/`childB` that
     * function owns. */
    void (*slot10)(Class86E00_3bb8c_g *self, Class86E00SubObj_3bb8c_g *arg1);
    u8 pad014[0x030 - 0x014];
    /* +0x030, TaskObjF__SetState's own first call, every path: `(self,
     * arg1)` where `arg1` may already have been forced to `0x17`. */
    void (*slot30)(Class86E00_3bb8c_g *self, s32 arg1);
    u8 pad034[0x050 - 0x034];
    /* +0x050, TaskObjF__SetState's own `arg1==0x13` case: no extra arguments,
     * return value picks between two literal replacement codes. */
    s32 (*slot50)(Class86E00_3bb8c_g *self);
    u8 pad054[0x058 - 0x054];
    /* +0x058, TaskObjF__SetState's own `arg1==0x14` case, only when
     * `*(u8 *)self->unk40 == 0`: `(self, unk40, unk30, unk34)`. */
    void (*slot58)(Class86E00_3bb8c_g *self, s32 a1, char *a2, s32 a3);
    u8 pad05C[0x064 - 0x05C];
    /* +0x064, TaskObjF__SetState's own `arg1==0x15` case: `(self, unk40,
     * unk54, unk58)`, return value picks between two literal
     * replacement codes (same shape as `slot68` just below). */
    s32 (*slot64)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3);
    /* +0x068, TaskObjF__SetState's own `arg1==0x14` case, unconditional:
     * `(self, unk40, unk44, unk4C, unk50, unk54, unk58)` -- the same
     * six fields as `slot78` below MINUS `unk48`, not a subset call to
     * that slot. Return value picks between two literal replacement
     * codes, same shape as `slot64` above. */
    s32 (*slot68)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
    u8 pad06C[0x074 - 0x06C];
    /* +0x074, TaskObjF__AdvanceState's own `self->secondaryMode==1` case: 4 extra
     * arguments (`unk40`, `unk44`, `unk54`, `unk58`), same shape as
     * `slot78` just below but with only the last two of that call's
     * trailing four. */
    void (*slot74)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3, s32 a4);
    /* +0x078, TaskObjF__OnCommand's own call for its `arg2==2` case: 7 extra
     * arguments, the last four passed on the stack (`unk4C`, promoted
     * from its native `u8` to a full word, then `unk50`/`unk54`/`unk58`). */
    void (*slot78)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3,
                    s32 a4, s32 a5, s32 a6, s32 a7);
    /* +0x07C, the shared tail call of TaskObjF__ForceIdleFromState/TaskObjF__TickStateDelay/
     * TaskObjF__OnCommand/TaskObjF__OnItemSelected: `(self, literal state code)`. */
    void (*slot7C)(Class86E00_3bb8c_g *self, s32 arg1);
    /* +0x080, TaskObjF__SetState's own 3rd call, every path: `(self, arg1)`,
     * same `arg1` value as `slot30` above. */
    void (*slot80)(Class86E00_3bb8c_g *self, s32 arg1);
    /* +0x084, TaskObjF__SetState's own 2nd call, every path: `(self)`. */
    void (*slot84)(Class86E00_3bb8c_g *self);
    u8 pad088[0x08C - 0x088];
    void (*slot8C)(Class86E00_3bb8c_g *self, s32 arg1); /* +0x08C, TaskObjF__ForceIdleFromState */
    void (*slot90)(Class86E00_3bb8c_g *self); /* +0x090, TaskObjF__OnNotify's `arg2==0x19` case */
    void (*slot94)(Class86E00_3bb8c_g *self); /* +0x094, TaskObjF__OnNotify's `arg2==0x17` case */
    u8 pad098[0x09C - 0x098];
    /* +0x09C, TaskObjF__SetState's own `arg1==0x11` case: `(self)`, no
     * return value read. */
    void (*slot9C)(Class86E00_3bb8c_g *self);
    void (*slotA0)(Class86E00_3bb8c_g *self); /* +0x0A0, TaskObjF__OnCommand's own first call, both cases */
    u8 pad0A4[0x0A8 - 0x0A4];
    /* +0x0A8, TaskObjF__SetState's own `arg1==0x12` case: `(self)`, same
     * shape as `slot9C` above. */
    void (*slotA8)(Class86E00_3bb8c_g *self);
    void (*slotAC)(Class86E00_3bb8c_g *self); /* +0x0AC, TaskObjF__OnItemSelected's own 2nd call, both cases */
};

struct Class86E00_3bb8c_g {
    Class86E00Methods_3bb8c_g *methods; /* +0x000 */
    u8 pad004[0x024 - 0x004];
    /* +0x024, TaskObjF__AdvanceState's own secondary dispatch code (nested inside
     * the `state`-driven switch's shared `2`/`4`/`0xA`/`0xE` case),
     * tested against literals `2` and `1`. */
    s32 secondaryMode;
    s32 state;   /* +0x028, dispatch/state code tested by several functions */
    /* +0x02C, TaskObjF__SetState's own loop bound: frees `self->unk38[0..
     * unk2C)` when tearing down (see `unk38`'s own comment below). */
    s32 unk2C;
    /* +0x030, TaskObjF__AdvanceState's own `strcpy` source into `self->unk40`,
     * in its `self->state==0xE` sub-case. */
    char *unk30;
    /* +0x034, TaskObjF__SetState's own `slot58` arg3, forwarded verbatim
     * alongside `unk30` above. */
    s32 unk34;
    void *unk38; /* +0x038, TaskObjF__AttachChildB: forwarded opaquely to `New_Class86F88`'s arg0 */
    /* +0x03C, TaskObjF__AdvanceState's own `self->state==0xE` sub-case: base of a
     * pointer array indexed by `(s32)self->selectedItem`, `strcat`ed onto
     * `self->unk40` -- same shape as `unk38` just below, indexed the
     * same way for `self->unk44`'s own `strcpy`. */
    void *unk3C;
    s32 unk40;   /* +0x040, TaskObjF__AttachChildA/TaskObjF__OnCommand */
    s32 unk44;   /* +0x044, TaskObjF__AttachChildA/TaskObjF__OnCommand */
    s32 unk48;   /* +0x048, TaskObjF__AttachChildA/TaskObjF__OnCommand */
    u8 unk4C;    /* +0x04C, TaskObjF__OnCommand: read `lbu`, promoted to a full word for `slot78`'s call */
    u8 pad04D[0x050 - 0x04D];
    s32 unk50;   /* +0x050, TaskObjF__OnCommand */
    s32 unk54;   /* +0x054, TaskObjF__OnCommand */
    s32 unk58;   /* +0x058, TaskObjF__OnCommand */
    s32 waitCounter;   /* +0x05C, TaskObjF__TickStateDelay: incremented, capped at 6 */
    s32 unk60;   /* +0x060, forwarded to `childA`/`childB`'s own `slot4C` arg1 */
    s32 unk64;   /* +0x064, forwarded to `childA`/`childB`'s own `slot4C` arg2 */
    /* +0x068, a readiness gate checked alongside `unk60` in four
     * functions (TaskObjF__AttachChildA/TaskObjF__DetachChildA/TaskObjF__AttachChildB/
     * TaskObjF__DetachChildB) -- both must be non-zero before the body runs.
     * Kept a bare `s32`; never dereferenced in this unit. */
    s32 childReady;
    /* +0x06C, forwarded to `childA`/`childB`'s own `slot4C` arg3.
     * TaskObjF__SetChildFlag8 proves this is a pointer (dereferences its `+0x080`
     * vtable slot), not the plain `s32` it looked like from the slot4C
     * call site alone -- retyped here, same size, no layout change. */
    Class86E00Unk6CObj_3bb8c_g *childC;
    Class86E00Unk70Obj_3bb8c_g *cardIcon; /* +0x070, TaskObjF__TickCardIcon */
    /* +0x074, a one-shot flag set to 1 by TaskObjF__AttachChildA/TaskObjF__AttachChildB
     * right after attaching `childA`/`childB`, and consumed (guarding a
     * teardown callback) by TaskObjF__DetachChildA/TaskObjF__DetachChildB. */
    s32 childAttached;
    struct TextEntry *childA;         /* +0x078, TaskObjF__AttachChildA/TaskObjF__DetachChildA: New_TextEntry (include/TextEntry.h) */
    Class86E00SubObj_3bb8c_g *childB; /* +0x07C, TaskObjF__AttachChildB/TaskObjF__DetachChildB */
    void *selectedItem; /* +0x080, TaskObjF__OnItemSelected: set from `arg1->methods->slot9C(arg1)`'s return */
};

/* New_Class86F88 (TaskObjF__AttachChildB's childB) is declared in
 * include/Class86F88.h. */

/* New_TextEntry (TaskObjF__AttachChildA's childA) is declared in
 * include/TextEntry.h. */

/*
 * class_3bb8c_f: a SEPARATE class from Class866E8 (include/Class866E8.h) -- no evidence unifies
 * them (distinct field layouts), and BuildMemcardPath below writes raw bytes
 * over its own object's first 6 bytes, which would corrupt Class866E8's own
 * vtable pointer if the two were the same type. This is this unit's own
 * BasicClass-derived (docs/research/class-framework.md, include/code_8220.h)
 * task-ish object: it dispatches through the INHERITED BasicClass
 * addChild/removeChild slots at +0x010/+0x014 (Get_vtable_BasicClass()'s own table
 * establishes those two slots' exact signatures) and adds its own slots
 * from +0x044 on. No FirecatFG name survives, so fields are named by
 * offset. Only the slots/fields this unit's functions actually touch are
 * given concrete types; the rest stays opaque padding. `child`'s type is
 * kept `void *` here (not `BasicClass *`) to avoid pulling in
 * include/code_8220.h just for a parameter type nothing in this unit
 * dereferences.
 */
typedef struct TaskObjF TaskObjF;
typedef struct TaskObjFMethods TaskObjFMethods;

struct TaskObjFMethods {
    s32 header;                                                /* +0x000 */
    void *unk04;                                                /* +0x004 */
    void *ctor;                                                  /* +0x008 */
    void *unk0C;                                                  /* +0x00C */
    void (*addChild)(TaskObjF *self, void *child);                 /* +0x010, TaskObjF__Init (x2) */
    void (*removeChild)(TaskObjF *self, void *child);               /* +0x014, TaskObjF__Deinit (x2) */
    void *unk18, *unk1C, *unk20, *unk24, *unk28, *unk2C, *unk30, *unk34, *unk38; /* inherited BasicClass slots, untouched by this unit */
    u8 pad3C[0x044 - 0x03C];
    void (*slot44)(TaskObjF *self);                                   /* +0x044, TaskObjF__Validate */
    s32 (*slot48)(TaskObjF *self);                                     /* +0x048, TaskObjF__Validate */
    s32 (*slot4C)(TaskObjF *self, s32 *out1, s32 *out2, s32 *out3);      /* +0x04C, TaskObjF__Validate */
    u8 pad50[0x054 - 0x050];
    s32 (*slot54)(TaskObjF *self, s32 a1, s32 a2);                          /* +0x054, TaskObjF__func_8004F8A4 */
    u8 pad58[0x05C - 0x058];
    s32 (*slot5C)(TaskObjF *self, void *a1, void *a2, s32 a3, s32 a4);        /* +0x05C, TaskObjF__func_8004F638 */
    s32 (*slot60)(TaskObjF *self, s32 a1, s32 a2);                              /* +0x060, TaskObjF__func_8004F8A4 */
    u8 pad64[0x07C - 0x064];
    s32 (*slot7C)(TaskObjF *self, s32 a1);                                        /* +0x07C, TaskObjF__func_8004F638/TaskObjF__func_8004F8A4/TaskObjF__Validate */
    u8 pad80[0x088 - 0x080];
    void (*slot88)(TaskObjF *self, void *arg1, s32 arg2);                          /* +0x088, TaskObjF__Notify */
    u8 pad8C[0x098 - 0x08C];
    void (*slot98)(TaskObjF *self, void *arg1, s32 arg2);                            /* +0x098, TaskObjF__Notify */
    u8 pad9C[0x0A4 - 0x09C];
    void (*slotA4)(TaskObjF *self, void *arg1, s32 arg2);                              /* +0x0A4, TaskObjF__Notify */
    u8 padA8[0x0B0 - 0x0A8];
    void (*slotB0)(TaskObjF *self, void *arg1, s32 arg2);                                /* +0x0B0, TaskObjF__Notify */
};

struct TaskObjF {
    TaskObjFMethods *methods;   /* +0x000 */
    u8 pad04[0x00C - 0x004];     /* BasicClass::children/parentRefs, untouched by this unit */
    s32 cardSlot;                  /* +0x00C, TaskObjF__TryReadMemcardFile: passed as BuildMemcardPath's "selector" (device slot 0/1) -- RENAMED round 60 (was unk0C) */
    u8 pad10[0x014 - 0x010];
    s32 events[4];                  /* +0x014, TaskObjF__ForEachEvent (walks all 4, early-exit)/TaskObjF__WaitForReadyEvent (passes &events[0], count 4) -- RENAMED round 60 (was field14): 4 kernel event descriptors, corroborated cross-unit by class_3bb8c_e.c's TaskObjF__OpenEvents, which fills the identical offset via OpenEvent() then passes the same object to this unit's own EnableEvents wrapper (see TaskObjF__EnableEvents's report) */
    s32 opMode;                       /* +0x024, RENAMED round 60 (was unk24): distinguishes which of this class's two operations is active -- TaskObjF__func_8004F638 sets 1, TaskObjF__func_8004F8A4 sets 2, TaskObjF__Validate reads (==1?); the values' exact meaning is not established */
    s32 statusCode;                    /* +0x028, RENAMED round 60 (was unk28): TaskObjF__Init/TaskObjF__func_8004F638 clear or set it, TaskObjF__func_8004F8A4/TaskObjF__Validate read it and dispatch it through slot7C -- a status/completion code, not confirmed to be error-only */
    s32 bufCount;                       /* +0x02C, RENAMED round 60 (was unk2C): TaskObjF__func_8004F638 (slot5C's return)/TaskObjF__FreeUnusedBuffers/TaskObjF__FreeBuffers (loop bound over bufArray) -- the number of bufArray entries actually in use */
    s32 unk30;                          /* +0x030, TaskObjF__Init (arg1)/TaskObjF__func_8004F638 (slot5C's arg3) */
    s32 unk34;                           /* +0x034, TaskObjF__Init (arg2)/TaskObjF__func_8004F638 (slot5C's stack arg4) */
    void **bufArray;                      /* +0x038, RENAMED round 60 (was unk38): a 16-entry pointer array allocated by TaskObjF__AllocBuffers, torn down by TaskObjF__FreeBuffers, walked by TaskObjF__FreeUnusedBuffers; also TaskObjF__func_8004F638's slot5C arg1 */
    void *scratchBuf;                       /* +0x03C, RENAMED round 60 (was unk3C): a single buffer allocated by TaskObjF__AllocBuffers, freed by TaskObjF__FreeBuffers; also TaskObjF__func_8004F638's slot5C arg2 */
    s32 unk40;                                /* +0x040, TaskObjF__func_8004F638 (arg1)/TaskObjF__func_8004F8A4 (arg1, forwarded to slot54 as its own arg2) */
    s32 unk44;                                 /* +0x044, TaskObjF__func_8004F638 (arg2)/TaskObjF__func_8004F8A4 (arg2) */
    s32 unk48;                                  /* +0x048, TaskObjF__func_8004F8A4 (arg3) */
    u8 unk4C;                                     /* +0x04C, TaskObjF__func_8004F8A4's 5th (byte) arg; also forwarded live to slot60's arg1 */
    u8 pad4D[0x050 - 0x04D];
    s32 unk50;                                      /* +0x050, TaskObjF__func_8004F8A4's 6th arg */
    s32 unk54;                                       /* +0x054, TaskObjF__func_8004F638 (arg3)/TaskObjF__func_8004F8A4's 7th arg */
    s32 unk58;                                        /* +0x058, TaskObjF__func_8004F638's 5th/stack arg/TaskObjF__func_8004F8A4's 8th arg; also forwarded live to slot60's arg2 */
    u8 pad5C[0x060 - 0x05C];
    s32 unk60;                                          /* +0x060, TaskObjF__Deinit: removeChild's arg */
    s32 unk64;                                            /* +0x064, TaskObjF__Deinit: removeChild's arg */
    s32 unk68;                                             /* +0x068, TaskObjF__Init (arg6)/TaskObjF__Deinit (cleared) */
    s32 unk6C;                                              /* +0x06C, TaskObjF__Init (arg7)/TaskObjF__Deinit (cleared) */
    s32 unk70;                                                /* +0x070, TaskObjF__Init (cleared) */
};

/* The three library callbacks TaskObjF__EnableEvents/TaskObjF__DisableEvents/TaskObjF__TestEvents
 * forward into TaskObjF__ForEachEvent are EnableEvent/DisableEvent/TestEvent, now
 * linked from the Psy-Q objects libapi/a12, libapi/a13 and libapi/a11.
 * Their declarations live in src/class_3bb8c_f.c, the only unit that uses
 * them: a prototype for a function a Sony object defines does not belong in
 * a header 21 units include, where it would one day collide with the real
 * KERNEL.H. (The old comment here called them "SPU routines" -- they are
 * kernel event-queue calls; only their neighbours in the block are libspu.)
 * TestEvent is also the validity check WaitForReadyEvent uses on its own array
 * argument. */

/* Generic "find the first of up to `count` entries for which
 * TestEvent accepts it, retrying the whole array forever if none
 * qualify yet" helper -- TaskObjF__WaitForReadyEvent calls it on this unit's own
 * TaskObjF::events (count 4). D_80086E78 is a small lookup table indexed
 * by the winning slot; bound unknown from this unit alone, left unsized.
 * (Comment updated round 60: the callback was `func_800390F4` before round
 * 34 linked it as Sony's own `TestEvent`; `field14` renamed to `events`.) */
extern s32 D_80086E78[];
extern s32 WaitForReadyEvent(s32 *arr, s32 count);

/* The generic pool allocator/free pair, already established the same way
 * by include/code_8220.h, include/code_55dd4.h etc -- `BMemPMgrFree`
 * returning `void *` (not `void`) matches TaskObjF__FreeUnusedBuffers's own use here,
 * which stores its return value back into the freed slot. */
/* BMemPMgrAlloc/BMemPMgrFree already declared above in this header. */
extern void *BMemPMgrFree(void *ptr);

/* A fixed 6-byte memory-card device-name template ("bu00:"/"bu10:", PS-X
 * BIOS device names -- asm/data/7B008.sdata.s). An all-`s8` struct
 * (natural alignment 1) so the whole-struct assignment in BuildMemcardPath
 * reproduces retail's unaligned lwl/lwr + byte-store copy, the same idiom
 * already documented for `Descriptor10` above. */
typedef struct DeviceName866E8 {
    s8 b0, b1, b2, b3, b4, b5;
} DeviceName866E8;

extern DeviceName866E8 gMcDevicePath1;   /* "bu10:" */
extern DeviceName866E8 gMcDevicePath0;   /* "bu00:" */

/* This project's own strcat (matched elsewhere, src/code_171e0.c) --
 * BuildMemcardPath is this unit's only caller. */
extern char *strcat(char *dest, char *src);


/* ROUND 34: five of the six prototypes that used to sit here were the PSX
 * BIOS file trampolines, and they are Sony's -- `open`/`read`/`lseek`/
 * `close`/`delete`, now linked from libapi/a50,a52,a51,a54,a69. They are gone
 * from this SHARED header deliberately, not lost. CLAUDE.md's rule: a
 * prototype for a function ANOTHER unit defines -- here, a Sony object --
 * belongs in the `.c` that calls it. Under Sony's names that matters more,
 * not less: `open`/`read`/`close` are generic enough that a future
 * include/psyq prototype (measured 2026-09-12: none of the shipped headers
 * declares them today, only comments and O_* macros) would collide in
 * whichever of the eleven including units pulled both in first, and the two
 * real callers already disagree about the first argument's type.
 * Each caller now carries its own local `extern` with its own argument shape
 * -- see src/class_3bb8c_f.c and src/class_3bb8c_e.c.
 *
 * CopyMemcardIconTemplate stays: it is game code, defined in src/class_3bb8c_g.c
 * (MATCHED round 45, 60/60 words -- was gp_rel-blocked, resolved round 42). */
extern s32 CopyMemcardIconTemplate(s32 arg0, s32 arg1);                /* TaskObjF__WriteMemcardSaveFile's own retry-loop bracket; also called with (arg,0) after the retry loop gives up */

/* ObjM (gObjMMethods, class_3bb8c_k/_l/_m) is include/ObjM.h (track 4,
 * round 89). The class_3bb8c_m view `ObjM` and its helper views of the
 * objects ObjM holds (FieldM14/18/34/50/7C, ChildM_AC, ChildM114, ParamM)
 * are gone: those objects are the unified Class866E8, Class869D8,
 * Class6E99C, VabStreamObj, FrameClock, WBgm and TextRow. */

/*
 * class_3bb8c_l -- the class whose method table is gObjMMethods (53 slots,
 * resolved with tools/classtable.py 0x80087034). This unit is the FIRST to
 * write any of this class's own methods, but sibling units class_3bb8c_k
 * and class_3bb8c_m are being carved/worked in the SAME round and may
 * independently reach the SAME table -- per round 14's learning ("suffix
 * new type names with your unit" when other runners are live on the same
 * header), every type below carries the `_3bb8c_l` suffix so a later merge
 * cannot collide on a name. Only the slots/fields this unit's own chosen
 * functions actually dispatch through are given concrete types; everything
 * else stays opaque padding.
 */
typedef struct Obj87034_3bb8c_l Obj87034_3bb8c_l;

/* self->styleConfig's pointee: a plain (non-vtable) record, read directly by
 * ObjM__PollTimBlockLoad via ordinary field offsets, never through a methods
 * pointer -- so it is NOT another Obj87034_3bb8c_l, just an opaque
 * 3-field descriptor. */
typedef struct Unk50Struct_3bb8c_l {
    s32 unk0;      /* +0x000, round 45's ObjM__SetupSceneStyle: forwarded opaquely to Obj14Methods_3bb8c_l::slotC4's arg2 */
    s32 unk4;      /* +0x004, round 45's ObjM__SetupSceneStyle: forwarded opaquely to Obj14Methods_3bb8c_l::slotC4's arg3 */
    s32 unk8;      /* +0x008, round 45's ObjM__SetupSceneStyle: forwarded opaquely to Obj14Methods_3bb8c_l::slotBC's arg1 */
    void *unkC;   /* +0x00C, ObjM__PollTimBlockLoad (address taken, forwarded opaquely) */
    u8 pad10[0x014 - 0x010];
    s32 unk14;    /* +0x014, ObjM__PollTimBlockLoad: discriminant compared against 2; also ObjM__EnterStyleSession: discriminant compared against 1 */
    void *unk18;  /* +0x018, ObjM__PollTimBlockLoad (address taken, forwarded opaquely) */
    void *unk1C;  /* +0x01C, ObjM__EnterStyleSession (address taken, forwarded opaquely) */
} Unk50Struct_3bb8c_l;

/* Whatever self->unk14 points to: an object of some OTHER, unidentified
 * class -- it has its own methods pointer at +0x000 (ObjM__TickStyle
 * dispatches +0x10C on it) AND a plain u16 field at +0x1B4 (ObjM__PollTimBlockLoad
 * reads it directly). Offset +0x10C happens to coincide with a DreamSys
 * vtable offset, but DreamSys's own occupant there (DreamSys__SetSoundObj) takes
 * one s32 argument while this call site passes two -- different arities,
 * so this is a different class, not DreamSys; left unnamed. */
typedef struct Obj14Methods_3bb8c_l {
    u8 pad000[0x0BC];
    /* +0x0BC, round 45's ObjM__SetupSceneStyle: `(self, self->styleConfig->unk8, 0)`. */
    void (*slotBC)(void *self, s32 arg1, s32 arg2); /* +0x0BC */
    u8 pad0C0[0x0C4 - 0x0C0];
    /* +0x0C4, round 45's ObjM__SetupSceneStyle: `(self, 3, self->styleConfig->unk0,
     * self->styleConfig->unk4)`. */
    void (*slotC4)(void *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0C4 */
    u8 pad0C8[0x0CC - 0x0C8];
    /* +0x0CC, round 45's ObjM__SetupSceneStyle: `(self, &D_8008710C)`. */
    void (*slotCC)(void *self, void *arg1); /* +0x0CC */
    u8 pad0D0[0x0DC - 0x0D0];
    /* +0x0DC, round 45's ObjM__SetupSceneStyle: `(self, self->unk48)` on the
     * OWNING Obj87034_3bb8c_l. */
    void (*slotDC)(void *self, s32 arg1); /* +0x0DC */
    /* +0x0E0, round 45's ObjM__SetupSceneStyle: `(self, GetStageGridDimensions(
     * self->unk38))`. */
    void (*slotE0)(void *self, void *arg1); /* +0x0E0 */
    u8 pad0E4[0x0EC - 0x0E4];
    void (*slotEC)(void *self);                        /* +0x0EC, ObjM__EnterStyleSession */
    u8 padF0[0x10C - 0x0F0];
    void *(*slot10C)(void *self, s32 arg1, s32 arg2); /* +0x10C, ObjM__TickStyle */
    u8 pad110[0x134 - 0x110];
    void (*slot134)(void *self, void *arg1);           /* +0x134, ObjM__InitStyleAndWorld */
} Obj14Methods_3bb8c_l;
typedef struct Obj14_3bb8c_l {
    Obj14Methods_3bb8c_l *methods; /* +0x000 */
    u8 pad04[0x1B4 - 0x004];
    u16 unk1B4;                     /* +0x1B4, ObjM__PollTimBlockLoad */
} Obj14_3bb8c_l;

/* What self->world points to (and what its +0x0AC returns): an
 * unidentified class dispatched through style/world-setup slots. Until
 * round 88 this type was also self->target's, as DreamSysObj_3bb8c_l; the
 * target is the game's DreamSys (track 4: its +0x04C/+0x050/+0x0EC/+0x0F0/
 * +0x0F8/+0x0FC/+0x104/+0x108/+0x1A0/+0x200 calls are DreamSys occupants,
 * and +0x044/+0x164 are Actor's state and DreamSys's currentStage), and
 * those slots left with it. Nothing ties the world to DreamSys. */
typedef struct StyleWorldMethods_3bb8c_l {
    u8 pad00[0x054];
    /* +0x054, round 45's ObjM__SetupSceneStyle: `(self, val)`, `val` a small
     * derived integer (`(*obj->methods->slot7C(obj, 0)) / 2 * 5 / 3 +
     * D_8008AB34`, `obj` being `*(void **)self->unkC`). */
    void (*slot54)(void *self, s32 arg1); /* +0x054 */
    u8 pad58[0x060 - 0x058];
    void (*slot60)(void *self, s32 arg1);          /* +0x060, ObjM__EnterStyleSession */
    void (*slot64)(void *self, void *arg1);        /* +0x064, ObjM__EnterStyleSession */
    void (*slot68)(void *self, void *arg1);        /* +0x068, ObjM__EnterStyleSession */
    void (*slot6C)(void *self, void *arg1);        /* +0x06C, ObjM__EnterStyleSession */
    void (*slot70)(void *self, void *arg1, void *arg2, void *arg3, s32 arg4); /* +0x070, ObjM__InitStyleAndWorld: (target, &D_8008715C, &D_80087168, 0) */
    void (*slot74)(void *self);          /* +0x074, ObjM__ExitSceneStyle */
    u8 pad78[0x0AC - 0x078];
    /* Returns another object of this view, dispatched through slotF0/slotD4
     * (elaborated tag: the typedef is not in scope yet). */
    struct StyleWorldObj_3bb8c_l *(*slotAC)(void *self);    /* +0x0AC, ObjM__EnterStyleSession */
    void (*slotB0)(void *self, s32 arg1);          /* +0x0B0, ObjM__EnterStyleSession */
    void (*slotB4)(void *self, s32 arg1);          /* +0x0B4, ObjM__EnterStyleSession */
    u8 padB8[0x0D4 - 0x0B8];
    void (*slotD4)(void *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0D4, ObjM__EnterStyleSession (on slotAC's result) */
    u8 padD8[0x0F0 - 0x0D8];
    s32 (*slotF0)(void *self, s32 *outBuf, s32 arg2); /* +0x0F0, ObjM__EnterStyleSession (on slotAC's result, as (s32 *)ret, 0 or 3) */
} StyleWorldMethods_3bb8c_l;
typedef struct StyleWorldObj_3bb8c_l {
    StyleWorldMethods_3bb8c_l *methods;
} StyleWorldObj_3bb8c_l;

/* Whatever arg1->unkC points to in ObjM__AttachTarget -- a registration sink
 * of some kind (arg1->unkC->methods->slotC8(arg1->unkC, callback,
 * userdata) reads like "subscribe `callback` for `userdata`"). Only the
 * one slot this unit calls through is named. */
typedef struct RegistrantMethods_3bb8c_l {
    u8 pad00[0x0C8];
    void (*slotC8)(void *self, void (*callback)(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3), Obj87034_3bb8c_l *userdata); /* +0x0C8, ObjM__AttachTarget */
} RegistrantMethods_3bb8c_l;
typedef struct RegistrantObj_3bb8c_l {
    RegistrantMethods_3bb8c_l *methods;
} RegistrantObj_3bb8c_l;

typedef struct Obj87034Methods_3bb8c_l {
    s32 header;                                                    /* +0x000 */
    void (*slot04)(Obj87034_3bb8c_l *self);                        /* +0x004, BasicClass generic (func_80017EB0); dispatched directly by ObjM__PollTimBlockLoad on its `other` argument */
    u8 pad08[0x010 - 0x008];
    void (*slot10)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x010, ObjM__AttachTarget */
    void (*slot14)(Obj87034_3bb8c_l *self, void *arg1);            /* +0x014, ObjM__DetachTarget/ObjM__ExitSceneStyle */
    u8 pad18[0x030 - 0x018];
    void (*slot30)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x030, ObjM__EnterState4 (STALLED 28/71 -- offset/signature observed directly from the disassembly, reliable independent of the stall; see docs/match-reports/ObjM__EnterState4.md) */
    u8 pad34[0x048 - 0x034];
    void (*slot48)(Obj87034_3bb8c_l *self);                        /* +0x048, ObjM__DetachTarget/ObjM__TeardownStyle (via self->unk54) */
    u8 pad4C[0x05C - 0x04C];
    void (*slot5C)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x05C, ObjM__InitStyleAndWorld (arg1 is PickVariant's return value, forwarded opaquely) */
    u8 pad60[0x074 - 0x060];
    void (*slot74)(Obj87034_3bb8c_l *self);                        /* +0x074, ObjM__DispatchPadEvent (event 0x21) */
    u8 pad78[0x07C - 0x078];
    void (*slot7C)(Obj87034_3bb8c_l *self, void *arg1);            /* +0x07C, ObjM__PollTimBlockLoad */
    void (*slot80)(Obj87034_3bb8c_l *self);                        /* +0x080, ObjM__PollTimBlockLoad */
    void (*slot84)(Obj87034_3bb8c_l *self);                        /* +0x084, ObjM__TeardownStyle */
    void (*slot88)(Obj87034_3bb8c_l *self);                        /* +0x088, ObjM__PollTimBlockLoad */
    void (*slot8C)(Obj87034_3bb8c_l *self);                        /* +0x08C, ObjM__Update */
    u8 pad90[0x094 - 0x090];
    void (*slot94)(Obj87034_3bb8c_l *self);                        /* +0x094, ObjM__OnDreamSysNotify (event/code 0xA, dense switch) */
    void (*slot98)(Obj87034_3bb8c_l *self);                        /* +0x098, ObjM__OnDreamSysNotify (event/code 0xC) */
    void (*slot9C)(Obj87034_3bb8c_l *self);                        /* +0x09C, ObjM__EnterState5; ALSO ObjM__OnDreamSysNotify (event/code 0xD) */
    void (*slotA0)(Obj87034_3bb8c_l *self);                        /* +0x0A0, ObjM__OnDreamSysNotify (event/code 0xE) */
    void (*slotA4)(Obj87034_3bb8c_l *self);                        /* +0x0A4, ObjM__OnDreamSysNotify (event/code 0xF) */
    void (*slotA8)(Obj87034_3bb8c_l *self);                        /* +0x0A8, ObjM__OnDreamSysNotify (event/code 0x10) */
    void (*slotAC)(Obj87034_3bb8c_l *self);                        /* +0x0AC, ObjM__OnDreamSysNotify (event/code 0x11) */
    u8 padB0[0x0C0 - 0x0B0];
    void (*slotC0)(Obj87034_3bb8c_l *self);                        /* +0x0C0, ObjM__DispatchPadEvent (event 0xC) */
    void (*slotC4)(Obj87034_3bb8c_l *self);                        /* +0x0C4, ObjM__DispatchPadEvent (event 0x2C)/ObjM__TogglePause */
    void (*slotC8)(Obj87034_3bb8c_l *self);                        /* +0x0C8, ObjM__DispatchPadEvent (event 0x16) */
    u8 padCC[0x0D0 - 0x0CC];
    void (*slotD0)(Obj87034_3bb8c_l *self);                        /* +0x0D0, ObjM__Update/ObjM__TogglePause */
    void (*slotD4)(Obj87034_3bb8c_l *self);                        /* +0x0D4, ObjM__ExitSceneStyle/ObjM__TogglePause */
} Obj87034Methods_3bb8c_l;

struct Obj87034_3bb8c_l {
    Obj87034Methods_3bb8c_l *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    RegistrantObj_3bb8c_l *unkC;      /* +0x00C, ObjM__AttachTarget's `arg1->unkC` */
    s32 unk10;                        /* +0x010, ObjM__EnterStyleSession */
    Obj14_3bb8c_l *unk14;              /* +0x014, ObjM__PollTimBlockLoad/ObjM__TickStyle/ObjM__ExitSceneStyle */
    StyleWorldObj_3bb8c_l *world;         /* +0x018, ObjM__ExitSceneStyle; cached into `cachedWorld` by ObjM__InitStyleAndWorld -- an object distinct from `target`, dispatched through style/world-setup slots (slot74/slotAC/slot60/slot64/slot6C/slot68/slotB0/slotB4) */
    s32 unk1C;                           /* +0x01C, ObjM__Update: incremented once per call */
    s32 phase;                            /* +0x020, ObjM__EnterState6: written 6 (a state/phase tag; also written 4 by ObjM__EnterState4 and 5 by ObjM__EnterState5; read by ObjM__OnDreamSysNotify, which is 0-gated) */
    u8 pad24[0x034 - 0x024];
    s32 unk34;                            /* +0x034, round 45's ObjM__SetupSceneStyle: forwarded opaquely to SetDreamAuxWorld's own arg3 */
    void *unk38;                          /* +0x038, ObjM__OnRegistrantEvent: forwarded opaquely to GetGridRecordAt/GetGridRecordXY */
    struct DreamSys *target;            /* +0x03C, many functions in this unit: the game's DreamSys (include/DreamSys.h), which SetDreamAuxWorld installs as gDreamAuxWorld */
    s32 unk40;                             /* +0x040, ObjM__EnterStyleSession */
    s32 unk44;                              /* +0x044, ObjM__EnterStyleSession */
    s32 unk48;                               /* +0x048, ObjM__InitStyleAndWorld: set from arg1, or 0xA000 if arg1==0 */
    s32 unk4C;                                /* +0x04C, ObjM__InitStyleAndWorld: set from arg3 (only when self->unk38 != 0) */
    Unk50Struct_3bb8c_l *styleConfig;             /* +0x050, ObjM__PollTimBlockLoad; set from RegisterStyleConfig's return in ObjM__InitStyleAndWorld (or an explicit `arg2` override) */
    Obj87034_3bb8c_l *unk54;                 /* +0x054, ObjM__TeardownStyle */
    Obj87034_3bb8c_l *pendingOther;                  /* +0x058, ObjM__OnTag1Notify: forwarded as ObjM__PollTimBlockLoad's `other` */
    u8 pad5C[0x060 - 0x05C];
    s32 hasTarget;                                 /* +0x060, ObjM__PollTimBlockLoad: has-a-target gate, cleared after detaching */
    s32 unk64;                                  /* +0x064, ObjM__PollTimBlockLoad: set to 1 */
    s32 attached;                                   /* +0x068, ObjM__PollTimBlockLoad/ObjM__DispatchPadEvent/ObjM__Update: zero-checked gate */
    s32 unk6C;                                    /* +0x06C, ObjM__InitStyleAndWorld: out-parameter address passed to RegisterStyleConfig, own type unknown */
    u8 pad70[0x078 - 0x070];
    StyleWorldObj_3bb8c_l *cachedWorld;                    /* +0x078, ObjM__InitStyleAndWorld: cached copy of self->world */
    u8 pad7C[0x080 - 0x07C];
    s32 unk80;                                    /* +0x080, ObjM__PollTimBlockLoad (on `other`)/ObjM__Update/ObjM__TogglePause: zero-checked gate */
};

/* ObjM_3bb8c_m's parent class, Class86668 (gClass86668Methods), is declared in
 * include/Class86668.h (track 4, round 84); class_3bb8c_k and class_3bb8c_l
 * include it for their base-table calls. The unit-local
 * `BaseMethods87034_3bb8c_l` view the round-15 head note here described is
 * gone. */

/* ObjM__OnRegistrantEvent's own two helpers -- MATCHED, src/code_39094.c.
 * GetGridRecordAt(index, sub) reads both $a0 and $a1. ObjM__OnRegistrantEvent's
 * `code >= 0` branch leaves its own incoming `code` in $a1 at the jal (no
 * write to $a1 before it), so `code` IS the second argument: a non-negative
 * code is a linear cell index, a negative one sends x/y to GetGridRecordXY.
 * (Was declared K&R/unprototyped and called with one argument until round
 * 82's externcheck pass; the forwarded form is byte-identical.) */
extern s32 GetGridRecordAt(void *arg0, s32 sub);
extern void GetGridRecordXY(void *arg0, s32 arg1, s32 arg2);

/* ObjM__TickStyle's own helper -- MATCHED, src/class_3bb8c_n.c. Typed purely
 * from this call site's own register usage. */
extern void TickStyle(void *arg0, void *arg1, s32 arg2);

/* ObjM__TeardownStyle's own helpers -- MATCHED, src/class_3bb8c_n.c (StyleTeardown)
 * and src/code_4cd08.c (TickDreamAuxSlots2), both matched, called with no arguments
 * and their return values unused. */
extern void TickDreamAuxSlots2(void);
extern void StyleTeardown(void);


/*
 * TextEntry (gTextEntryMethods, class_3bb8c_i/j) is declared in
 * include/TextEntry.h (FINISHING-PLAN track 4, round 87). What stays here are
 * two helper views of objects it HOLDS, which are not its class: the
 * resources `cursorSprite`/`textRow`/`panelSprite` and the TIM handles
 * (ChildObj86ED0), and `target` (TargetObj86ED0). The names are kept.
 */
/* TextEntry's `textRow` (a TextRow, include/TextRow.h): a view of the slots
 * its calls use, not one class. The TIM handles TextEntry__LoadCardResources
 * and TaskObjF__LoadCardIcon load through New_TimImage used this view too
 * until round 88; they are `TimImage *` now (include/TimImage.h). */
typedef struct ChildObj86ED0 ChildObj86ED0;
typedef struct ChildMethods86ED0 ChildMethods86ED0;
struct ChildMethods86ED0 {
    u8 pad000[0x004];
    void *(*release)(ChildObj86ED0 *self); /* +0x004, TextEntry__ReleaseCardResources */
    u8 pad008[0x04C - 0x008];
    /* +0x04C, textRow: (parent, &D_8008AAD4), the attachToParent shape
     * ScreenSprite's slot has (include/ScreenSprite.h). */
    void (*slot4C)(ChildObj86ED0 *self, void *arg1, void *arg2);
    u8 pad050[0x078 - 0x050];
    void (*slot78)(ChildObj86ED0 *self); /* +0x078; no accessor since round 88 (it was the TIM handles' TimImage__Upload) */
    u8 pad07C[0x0B8 - 0x07C];
    void (*slotB8)(ChildObj86ED0 *self, void *arg1); /* +0x0B8, TextEntry__LoadCardResources, textRow */
    u8 pad0BC[0x0C4 - 0x0BC];
    void (*slotC4)(ChildObj86ED0 *self, s32 arg1, s32 arg2); /* +0x0C4, TextEntry__SetCharAt, textRow: (char, pos); TextRow__SetCellAt */
};
struct ChildObj86ED0 {
    ChildMethods86ED0 *methods; /* +0x000 */
};

/* TextEntry's `target` (+0x03C) -- an unrelated class (own vtable, unconnected to
 * gTextEntryMethods; TaskObjF passes its childC), reached only through its own +0x080 slot by TextEntry__NotifyTarget.
 * Field meaning beyond that slot is unestablished. */
typedef struct TargetObj86ED0 TargetObj86ED0;
typedef struct TargetMethods86ED0 TargetMethods86ED0;
struct TargetMethods86ED0 {
    u8 pad000[0x080];
    /* +0x080, TextEntry__NotifyTarget: `self->methods->slot80(self, arg1, 0x60, 0x60)`.
     * 3 args, not 2 -- confirmed against this project's established
     * self->methods->slot80(self, arg1, 0x60, 0x60) idiom seen at several
     * other call sites (src/class_3bb8c_k.c, src/code_2cc8c.c,
     * src/class_3bb8c_g.c, src/code_55dd4.c), all forwarding a caller-
     * supplied arg1 alongside a repeated literal. TextEntry__NotifyTarget itself
     * takes that arg1 as its own second parameter and forwards it
     * unchanged (same register, no move instruction). */
    void (*slot80)(TargetObj86ED0 *self, s32 arg1, s32 arg2, s32 arg3);
};
struct TargetObj86ED0 {
    TargetMethods86ED0 *methods; /* +0x000 */
};

/* HEAD NOTE round 15: alpha's `extern char *DecodeFullWidthSjis(char *dest, char
 * *src);` moved into src/class_3bb8c_i.c -- FOURTH instance of the
 * shared-header prototype rule this round. DecodeFullWidthSjis is still
 * INCLUDE_ASM in code_2cc8c_f, so NOBODY has its definition and every
 * declaration is a call-site typing; class_3bb8c_i reads it as
 * `char *(char *, char *)` and class_3bb8c_j as `void (void *, void *)`.
 * Both units are byte-exact with their own reading, because the return
 * type of an INCLUDE_ASM callee only affects the CALLER's codegen. Two
 * call-site typings of one undefined function are exactly the case that
 * must stay unit-local. */

/* Class86F88 (gClass86F88Methods, the 0x54-byte list selector whose methods
 * are in class_3bb8c_j and class_3bb8c_k) is include/Class86F88.h (track 4,
 * round 89). */

/* gObjMMethods and GetObjMMethods: include/ObjM.h (track 4, round 89). */

#endif
