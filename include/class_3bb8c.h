#ifndef CLASS_3BB8C_H
#define CLASS_3BB8C_H

#include "common.h"
#include "BasicClass.h"
#include "TaskCore.h"
#include "StageMap.h"
#include "DrawSystem.h"

/*
 * class_3bb8c.c and class_3bb8c_b.c hold the back half of StageMap (the
 * grid manager, gStageMapMethods), which is declared once, in
 * include/StageMap.h (track 4, round 89). What stays here is that class's
 * data (the rate and footprint tables) and the helper views its two units
 * read through casts.
 */
typedef struct SplitCoord2 SplitCoord2;

/* The rectangle StageMap__InitFootprintSlot copies into rects[key] before
 * setting its element: no element (-1), the whole 20 x 20 cells from (0, 0). */
extern CellRect gFullSlotRect;

/* The default "enable every element" spec table SetTargetAndLoadChunks
 * passes to buildRateEntries: seven entries, every `flag` nonzero
 * (asm/data/76DC8.data.s). */
extern ChunkSlotSpec sDefaultTargetSpecs[7];

/* Indexed by ChunkSlotSpec::key in StageMap__LoadChunksAround: the
 * world offset of that neighbour's cellParent from the centre position. Bound unknown (`key` is the caller's
 * byte), so unsized. */
extern LongVec3 sNeighbourOffsets[];

/* `key`-indexed bitmask table (`1 << key`) StageMap__ComputeChunkLoadEntry tests
 * against ComputeNeighbourMask' result. 7 words in the data before
 * sChunkNeighbourDeltas starts. */
extern const s32 sNeighbourBits[7];

/* `key`-indexed chunk-index steps to the seven chunks around a centre chunk
 * (ChunkNeighbourDelta, include/StageMap.h), 7 entries
 * (0x800868A8-0x800868FC). */
extern const ChunkNeighbourDelta sChunkNeighbourDeltas[7];

/* LbdFile::ownerKey-indexed remap, read signed by
 * StageMap__UpdateFootprintTracking: exactly 8 bytes in the data
 * (01 02 03 00 04 05 06 00) before sDefaultTargetSpecs. The byte (0..6) is
 * UpdateFootprintTracking's return value and the index into
 * sFootprintResultPtrTable. */
extern const s8 sFootprintResultRemap[8];

/* 7 pointers, the first NULL, the rest to the 4-word (seven 2-byte
 * ChunkSlotSpecs, padded) tables D_80086914..D_80086964:
 * UpdateFootprintTracking passes the selected one to buildRateEntries as its
 * spec table, as SetTargetAndLoadChunks passes sDefaultTargetSpecs. */
extern ChunkSlotSpec *sFootprintResultPtrTable[7];

/*
 * A GsCOORDINATE2 (SceneNodeSub14) read as the element's origin: an
 * element's cellParent->coord2, reached through a cast. tx (+0x018) and tz
 * (+0x020) are unions because StageMap__ComputeFootprintDescriptor reads
 * each whole (the cell) and, later, as its low halfword (the offset):
 * retail reloads at the narrower width. StageMap__LoadChunksAround writes
 * all three and clears `flg` (+0x000).
 */
struct SplitCoord2 {
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

/* The four scale steps (Ratio16[3], x/y/z) startScaleRamp picks for
 * `scaleStep`: y +1/64, +1/4 (rate > 0; flag 0, nonzero), -1/64, -1/4
 * (rate <= 0); x and z 0/1. */
extern Ratio16 D_8008699C[3];
extern Ratio16 D_800869A8[3];
extern Ratio16 D_800869B4[3];
extern Ratio16 D_800869C0[3];

/* 1/1, 1/1, 1/1: the scale StageMap__ResetCellScale sets on every cell. */
extern Ratio16 D_800869CC[3];

/* -------------------------------------------------------------------
 * class_3bb8c_c additions below. Small sibling classes, each built by
 * its own New_X/ctor pair (allocator + base-chain + own-vtable-set, the
 * same shape as TimedTask__TimedTask in class_39e08.c). Named by their
 * vtable's address, same convention as StageMap/TimedTask. The first
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


/* TitleMenu (gTitleMenuMethods, a TaskCore) is defined in
 * include/TitleMenu.h (round 88, track 4). */

/* TitleMenu::saveCtrl is a TaskObjF (include/TaskObjF.h). */


/* TitleMenu's menu description (include/TaskCore.h): TitleMenu__TitleMenu
 * passes &D_80086D44 as TaskCore's ctor's `target` and again to setTarget.
 * 0x28 bytes in asm/data/76DC8.data.s, a path word, three zero words, the two
 * colour triples and four pointers: the TaskCoreTarget layout. RETYPED from a
 * placeholder `s32` in round 88 (address-of only; no byte change). */
extern TaskCoreTarget D_80086D44;

/* "ETC\ETCSE" (asm/data/1C34.rodata.s), TitleMenu__TitleMenu's
 * soundBankPath for TaskCore's ctor. RETYPED from a placeholder `s32` in
 * round 88 (address-of only); the ctor casts away the const for the
 * ctor's `char *`. */
extern const char D_800114DC[];

/* "ETC\TITLE.TIM" (asm/data/1C34.rodata.s), TitleMenu__Reset's path for
 * setSubHandle. RETYPED from a placeholder `s32` in round 88 (address-of
 * only). */
extern const char D_800114E8[];

/* TitleMenu__BeginCardAccess's own path string, passed to New_TimImage -- a real
 * dlabel (`asm/data/1C34.rodata.s`: "CARD\FILEICN1.TIM"), so this is the
 * ONLY correct spelling (CLAUDE.md: never re-write a string splat has
 * already emitted as a symbol). */
extern const char D_800114F8[];

/* The two 320 x 240 display buffers, stacked in VRAM at y 0 and y 240:
 * TitleMenu__OnDeinit clears each with the DrawSystem's clearImage. */
extern DrawRect D_80086DAC[2];

/* Address-of only in this unit (TitleMenu__AttachSaveTitle passes &D_8008A9B4 to
 * the name field's attachToParent, TextRow +0x04C, as its LongVec3 offset;
 * the words are -4, -23, ...). Placeholder s32 type, cast at the call. */
extern s32 D_8008A9B4;

/* VALUE-of, not address-of, in this unit -- TitleMenu__LoadFromCard reaches these
 * through `%gp_rel` loads of the .sdata globals themselves, forwarding
 * whatever they hold. Each holds a pointer into the still-uncarved rodata
 * block at `D_80011434` (`asm/data/1C34.rodata.s`: 0x80011464 and
 * 0x8001149C respectively, neither with its own dlabel), so they cannot
 * be spelled by the address they point to and are typed opaque `void *`
 * instead.
 *
 * `D_8008AA18` is also read by round 43's `TitleMenu__CreateSaveTitle`, which
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

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `TitleMenu__CreateSaveTitle`
 * as `strcpy`'s SOURCE argument. Holds `0x80011474` in the ROM image
 * (immediately past `D_8008AA10`'s own "BISLPS-01556xxx" string, i.e. the
 * start of the font-glyph word table in `D_80011434`) -- likely also a
 * placeholder for the same reason `D_8008AA18` is, since a font-glyph
 * table is not plausible `strcpy` input. */
extern void *D_8008AA14;

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `TitleMenu__BeginCardAccess`
 * as `TitleMenuUnkACObjMethods_3bb8c_d::slot6C`'s own `arg1`. Holds
 * `0x80011454` in the ROM image -- the "BISLPS-01556" string in
 * `D_80011434`, again with no `dlabel` of its own. */
extern void *D_8008A9D0;

/* Address-of only, round 43's `TitleMenu__BeginCardAccess`
 * (`TitleMenuUnkACObjMethods_3bb8c_d::slot6C`'s own `arg2`) -- a real
 * 16-entry pointer table (`asm/data/76DC8.data.s`, `D_8008AA0C` down to
 * `D_8008A9D4` then a NULL terminator), reached only by its own address
 * here, never walked. Placeholder `s32` type since only the address is
 * taken. */
extern s32 D_80086D6C;

/* TitleMenu__CycleSaveTitleColor's own rolling byte index (0/1/2, wraps to 0 at 3) into
 * that function's own 3-byte stack buffer -- declared in the ROM image
 * as a full `.word` (`asm/data/7B12C.sdata.s`), but accessed only via
 * `lbu`/`sb` here, so `u8` is the correct C type for this unit's own
 * reference regardless of the underlying storage's full width. */
extern u8 D_8008AA28;

/* TitleMenu__CycleSaveTitleColor's own rolling word counter (wraps to 0 at 0x101). */
extern s32 D_8008AA2C;

/* TaskObjF__TaskObjF's own one-shot init guard: read, then unconditionally
 * incremented, before its own body's InitCARD/StartCARD/_bu_init calls
 * run only when the PRE-increment value was 0 (i.e. only on the very
 * first construction of this class). */
extern s32 D_8008AA30;

/* Still raw asm in this unit (gp-relative-blocked, see
 * docs/match-reports/FormatNumberIntoBuffer.md) -- not this round's function, but
 * TitleMenu__TitleMenu calls it with one forwarded s32 argument (the return
 * value of dreamSys->methods->slot1A0); return value unused there. */
extern void FormatNumberIntoBuffer(s32 arg0);

/*
 * The save block DreamSys's getSaveBlock returns (TitleMenu::saveBlock):
 * DreamSys from `saveMagic` (+0x178) on, so each field here is the DreamSys
 * field (include/DreamSys.h) 0x178 bytes further in. CheckSaveScoreFlag
 * reads the two below.
 */
typedef struct DreamSaveBlock {
    u8 pad00[0x00C];
    s32 totalFlasbackUnlockScore; /* +0x00C, DreamSys +0x184: FLASHBACK unlocks past 9999999 */
    u8 pad10[0x2F4 - 0x010];
    s32 amountFlashbacksAvailable; /* +0x2F4, DreamSys +0x46C: ... and only with one stored */
} DreamSaveBlock;

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
typedef struct McDevicePath {
    s8 b0, b1, b2, b3, b4, b5;
} McDevicePath;

extern McDevicePath gMcDevicePath1; /* "bu10:" */
extern McDevicePath gMcDevicePath0; /* "bu00:" */

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
 * are gone: those objects are the unified StageMap, NodeGuardedViewport,
 * FadeBox, TimBlockSrc, VabStreamObj, FrameClock, WBgm and TextRow. */

/* ObjM::styleConfig's pointee (include/ObjM.h): not a class, a plain
 * record, the same memory class_3bb8c_m's local StyleM describes
 * (D_80087424, which ApplyStyleConfig fills and RegisterStyleConfig
 * returns). What ObjM's methods do with each word: */
typedef struct Unk50Struct_3bb8c_l {
    s32 unk0;   /* +0x000, SetupSceneStyle: the StageMap's setChildParams `dirs` */
    s32 unk4;   /* +0x004, SetupSceneStyle: setChildParams `colors` */
    s32 unk8;   /* +0x008, SetupSceneStyle: the StageMap's setAmbientColor rgb (a pointer) */
    void *unkC; /* +0x00C, a colour: the viewport's setClearColor (EnterStyleSession); the TimBlockSrc's fadeAllEntries when unk14 is 2 (PollTimBlockLoad); StyleM's gStylePalette entry */
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
 * include/TextEntry.h (FINISHING-PLAN track 4, round 87). */
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
