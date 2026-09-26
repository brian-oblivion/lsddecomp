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

/* The rectangle Class866E8__InitFootprintSlot copies into rects[key] before
 * setting its element: no element (-1), the whole 20 x 20 cells from (0, 0). */
extern GridSlot866E8 gDefaultElemRateOffset;

/* The default "enable every element" spec table SetTargetAndBuildRates
 * passes to buildRateEntries: seven entries, every `flag` nonzero
 * (asm/data/76DC8.data.s). */
extern TargetSpec866E8 sDefaultTargetSpecs[7];

/* Indexed by TargetSpec866E8::key in Class866E8__BuildRateEntries: the
 * world offset of that neighbour's cellParent from the centre position. Bound unknown (`key` is the caller's
 * byte), so unsized. */
extern LongVec3 sRateOffsetTable[];

/* `key`-indexed bitmask table (`1 << key`) Class866E8__ComputeRateEntry tests
 * against ComputeRateFlags' result. 7 words in the data before
 * sRateEntryTable starts. */
extern const s32 sRateKeyMask[7];

/* `key`-indexed chunk-index steps to the seven chunks around a centre chunk
 * (ChunkNeighbourDelta, include/Class866E8.h), 7 entries
 * (0x800868A8-0x800868FC). */
extern const ChunkNeighbourDelta sRateEntryTable[7];

/* LbdFile::ownerKey-indexed remap, read signed by
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
 * A GsCOORDINATE2 (SceneNodeSub14) read as the element's origin: an
 * element's cellParent->coord2, reached through a cast. tx (+0x018) and tz
 * (+0x020) are unions because Class866E8__ComputeFootprintDescriptor reads
 * each whole (the cell) and, later, as its low halfword (the offset):
 * retail reloads at the narrower width. Class866E8__BuildRateEntries writes
 * all three and clears `flg` (+0x000).
 */
struct Unk14Obj {
    s32 unk0; /* +0x000, GsCOORDINATE2.flg */
    u8 pad4[0x18 - 0x4];

    union {
        s32 w;
        u16 h;
    } unk18; /* +0x018, tx */

    s32 unk1C; /* +0x01C, ty */

    union {
        s32 w;
        u16 h;
    } unk20; /* +0x020, tz */
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
    s32 unk0[5]; /* +0x000..+0x010, opaque (untouched by Class866E8__ComputeFootprintFromRotation) */
    s32 unk14; /* +0x014, Class866E8__ComputeFootprintFromRotation: zeroed before the call, then an in/out arg to ApplyMatrixLV */
    s32 unk18; /* +0x018, Class866E8__ComputeFootprintFromRotation: zeroed before the call */
    s32 unk1C; /* +0x01C, Class866E8__ComputeFootprintFromRotation: set to self->gridSpan before the call */
};

extern QueryTemplate866E8 D_8008E98C;

/* Library helpers (Class866E8__ComputeFootprintFromRotation's only call
 * site). `RotMatrix`'s first argument is the target's coord2->param->rotate.
 * `ApplyMatrixLV` is called with its 2nd and 3rd arguments pointing at the
 * SAME address (`&desc.unk14` passed twice) -- confirmed against the raw
 * disassembly (`$a1`/`$a2` both `sp+0x54`). */
extern void RotMatrix(void *arg0, QueryTemplate866E8 *arg1);
extern void ApplyMatrixLV(QueryTemplate866E8 *arg0, s32 *arg1,
                          s32 *arg2); /* arity-ok: this IS the callee's real signature (Sony libgte, 0x80015618 reads $a0 matrix / $a1 in / $a2 out); include/code_d294.h's unprototyped copy is round 19's deliberate frame-sizing shape, not a claim about arity */

/* The four scale steps (Ratio16[3], x/y/z) configureRateEntry picks for
 * `scaleStep`: y +1/64, +1/4 (rate > 0; flag 0, nonzero), -1/64, -1/4
 * (rate <= 0); x and z 0/1. */
extern Ratio16 D_8008699C[3];
extern Ratio16 D_800869A8[3];
extern Ratio16 D_800869B4[3];
extern Ratio16 D_800869C0[3];

/* 1/1, 1/1, 1/1: the scale Class866E8__ResetChildRate sets on every cell. */
extern Ratio16 D_800869CC[3];

/* -------------------------------------------------------------------
 * class_3bb8c_c additions below. Small sibling classes, each built by
 * its own New_X/ctor pair (allocator + base-chain + own-vtable-set, the
 * same shape as TimedTask__TimedTask in class_39e08.c). Named by their
 * vtable's address, same convention as Class866E8/TimedTask. The first
 * of them, NodeGuardedViewport (gNodeGuardedViewportMethods, a Viewport), is defined in
 * include/NodeGuardedViewport.h (round 87, track 4).
 * ------------------------------------------------------------------- */

extern void *BMemPMgrAlloc(s32 size);

/* GridCell (gGridCellMethods, a SceneNode) is defined in
 * include/GridCell.h (round 88, track 4). */

/* GetSceneNodeMethods and its table: include/SceneNode.h (track 4, round
 * 81). The local BaseCtorTableB_3bb8c_c view and the unprototyped getter that
 * lived here are gone; round 59 measured both arguments class_3bb8c_c.c passed
 * to the no-argument getter as zero-cost (docs/match-reports/GridCell__GridCell.md). */


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

/* Class86B60::saveCtrl is a TaskObjF (include/TaskObjF.h). */


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
 * the name field's attachToParent, TextRow +0x04C, as its LongVec3 offset;
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
    s32 unkC; /* +0x00C, compared against 9999999 */
    u8 pad10[0x2F4 - 0x010];
    s32 unk2F4; /* +0x2F4, zero-checked when unkC > 9999999 */
} SaveBlock678_3bb8c_c;

/*
 * First argument of CheckSaveScoreFlag: its one caller,
 * Class86B60__CommitNameEntry (class_3bb8c_d), passes its own self, so
 * +0x0BC is Class86B60::saveBlock (include/Class86B60.h).
 */
typedef struct Ctx678_3bb8c_c {
    u8 pad00[0x0BC];
    SaveBlock678_3bb8c_c *target; /* +0x0BC */
} Ctx678_3bb8c_c;

/*
 * Second argument of CheckSaveScoreFlag: holds a pointer at +0x018 to a small
 * result block whose word at +0x004 is the flag CheckSaveScoreFlag computes.
 */
typedef struct Result678_3bb8c_c {
    u8 pad00[0x018];
    s32 *block; /* +0x018, CheckSaveScoreFlag writes block[1] */
} Result678_3bb8c_c;

/* TaskObjF (gTaskObjFMethods, class_3bb8c_d/e/f/g) is include/TaskObjF.h
 * (track 4, round 89). */

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

extern DeviceName866E8 gMcDevicePath1; /* "bu10:" */
extern DeviceName866E8 gMcDevicePath0; /* "bu00:" */

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
extern s32 CopyMemcardIconTemplate(s32 arg0, s32 arg1); /* TaskObjF__WriteMemcardSaveFile's own retry-loop bracket; also called with (arg,0) after the retry loop gives up */

/* ObjM (gObjMMethods, class_3bb8c_k/_l/_m) is include/ObjM.h (track 4,
 * round 89). The views `ObjM` (class_3bb8c_m) and `Obj87034_3bb8c_l`, and
 * their helper views of the objects ObjM holds (FieldM14/18/34/50/7C,
 * ChildM_AC, ChildM114, ParamM, Obj14/StyleWorldObj/RegistrantObj_3bb8c_l)
 * are gone: those objects are the unified Class866E8, NodeGuardedViewport,
 * Class6E99C, TimBlockSrc, VabStreamObj, FrameClock, WBgm and TextRow. */

/* ObjM::styleConfig's pointee (include/ObjM.h): not a class, a plain
 * record, the same memory class_3bb8c_m's local StyleM describes
 * (D_80087424, which ApplyStyleConfig fills and RegisterStyleConfig
 * returns). What ObjM's methods do with each word: */
typedef struct Unk50Struct_3bb8c_l {
    s32 unk0;   /* +0x000, SetupSceneStyle: the Class866E8's setChildParams `dirs` */
    s32 unk4;   /* +0x004, SetupSceneStyle: setChildParams `colors` */
    s32 unk8;   /* +0x008, SetupSceneStyle: the Class866E8's setAmbientColor rgb (a pointer) */
    void *unkC; /* +0x00C, a colour: the viewport's setClearColor (EnterStyleSession); the TimBlockSrc's fadeAllEntries when unk14 is 2 (PollTimBlockLoad); StyleM's D_800872C4 entry */
    u8 pad10[0x014 - 0x010];
    s32 unk14; /* +0x014, selects unkC or unk18: PollTimBlockLoad against 2, EnterStyleSession against 1 */
    void *unk18; /* +0x018, a colour: setFarColor, or fadeAllEntries, when unk14 does not select unkC */
    s32 unk1C; /* +0x01C, EnterStyleSession: the viewport's setFogNear (StyleM: a D_8008730C value) */
} Unk50Struct_3bb8c_l;

/* ObjM__OnRegistrantEvent's own two helpers -- MATCHED, src/code_39094.c.
 * GetGridRecordAt(index, sub) reads both $a0 and $a1. ObjM__OnRegistrantEvent's
 * `code >= 0` branch leaves its own incoming `code` in $a1 at the jal (no
 * write to $a1 before it), so `code` IS the second argument: a non-negative
 * code is a linear cell index, a negative one sends x/y to GetGridRecordXY.
 * (Was declared K&R/unprototyped and called with one argument until round
 * 82's externcheck pass; the forwarded form is byte-identical.) */
extern s32 GetGridRecordAt(s32 index, s32 sub);
extern void GetGridRecordXY(s32 index, s32 x, s32 y);

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
    void (*slotC4)(ChildObj86ED0 *self, s32 arg1,
                   s32 arg2); /* +0x0C4, TextEntry__SetCharAt, textRow: (char, pos); TextRow__SetCellAt */
};

struct ChildObj86ED0 {
    ChildMethods86ED0 *methods; /* +0x000 */
};

/* TextEntry's `target` (+0x03C) -- an unrelated class (own vtable, unconnected to
 * gTextEntryMethods; TaskObjF passes its `sound`, a VabStreamObj), reached only through its own +0x080 slot by TextEntry__NotifyTarget.
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

/* ItemList (gItemListMethods, the 0x54-byte list selector whose methods
 * are in class_3bb8c_j and class_3bb8c_k) is include/ItemList.h (track 4,
 * round 89). */

/* gObjMMethods and GetObjMMethods: include/ObjM.h (track 4, round 89). */

#endif
