#ifndef CLASS_3BB8C_H
#define CLASS_3BB8C_H

#include "common.h"
#include "BasicClass.h"
#include "TaskCore.h"
#include "StageMap.h"
#include "DrawSystem.h"

/*
 * class_3bb8c.h -- the data and shared helper declarations of the
 * class_3bb8c* units. The classes those units hold each have their own
 * header, and nothing here redefines them:
 *
 *   class_3bb8c, _b    StageMap (include/StageMap.h), the chunk grid manager
 *   class_3bb8c_c      NodeGuardedViewport, GridCell, TitleMenu's ctor
 *   class_3bb8c_d      TitleMenu (include/TitleMenu.h, a TaskCore)
 *   class_3bb8c_d..g   TaskObjF (include/TaskObjF.h), the memory-card task
 *   class_3bb8c_i, _j  TextEntry (include/TextEntry.h)
 *   class_3bb8c_j, _k  ItemList (include/ItemList.h)
 *   class_3bb8c_k..m   ObjM (include/ObjM.h)
 *
 * What is here, in that order: StageMap's lookup tables and SplitCoord2,
 * the view of a slot's origin its methods read; TitleMenu's data (menu
 * description, paths, the save title's buffers, the colour cycle) and
 * DreamSaveBlock, the save-block view CheckSaveScoreFlag reads; TaskObjF's
 * event table, the memory-card device names (McDevicePath) and helpers;
 * ObjM's StyleConfig record and the helpers its methods call.
 *
 * A prototype for a Sony library function, or for a function one unit
 * alone calls under its own reading, lives in that unit instead: every
 * unit that includes this header would otherwise inherit it, and a second
 * reading of the same name in any of them would collide.
 */
typedef struct SplitCoord2 SplitCoord2;

/* ---- StageMap ---------------------------------------------------- */

/* The rectangle StageMap__InitFootprintRect copies into rects[index] before
 * setting its slotIndex: no slot (-1), the whole 20 x 20 cells from (0, 0). */
extern CellRect gFullSlotRect;

/* The default "enable every element" spec table SetTargetAndLoadChunks
 * passes to buildRateEntries: seven entries, every `flag` nonzero. */
extern ChunkSlotSpec sDefaultTargetSpecs[7];

/* Indexed by ChunkSlotSpec::key in StageMap__LoadChunksAround: the world
 * offset of that neighbour's cellParent from the centre position. Unsized:
 * `key` is the caller's byte. */
extern LongVec3 sNeighbourOffsets[];

/* `key`-indexed bitmask table (`1 << key`) StageMap__ComputeChunkLoadEntry
 * tests against ComputeNeighbourMask's result. */
extern const s32 sNeighbourBits[7];

/* `key`-indexed chunk-index steps to the seven chunks around a centre chunk
 * (ChunkNeighbourDelta, include/StageMap.h). */
extern const ChunkNeighbourDelta sChunkNeighbourDeltas[7];

/* LbdFile::ownerKey-indexed remap, read signed by
 * StageMap__UpdateFootprintTracking (01 02 03 00 04 05 06 00). The byte
 * (0..6) is UpdateFootprintTracking's return value and the index into
 * sFootprintResultPtrTable. */
extern const s8 sFootprintResultRemap[8];

/* 7 pointers, the first NULL, the rest to 4-word tables of ChunkSlotSpecs
 * (seven 2-byte entries, padded): UpdateFootprintTracking passes the
 * selected one to buildRateEntries as its spec table, as
 * SetTargetAndLoadChunks passes sDefaultTargetSpecs. */
extern ChunkSlotSpec *sFootprintResultPtrTable[7];

/*
 * A chunk slot's origin: its cellParent->coord2, a GsCOORDINATE2
 * (SceneNodeSub14, include/SceneNode.h), with the translation's x and z
 * readable as the whole word or its low halfword, as SplitLongVec3's are.
 * StageMap__LoadChunksAround writes all three words;
 * StageMap__ComputeFootprintDescriptor reads tx/tz whole for the cell and
 * as halfwords for the offset inside it.
 * MATCHING: the unions; retail reloads tx/tz at the narrower width.
 */
struct SplitCoord2 {
    u8 pad00[0x018]; /* +0x000, flg and coord.m */

    union {
        s32 w;
        u16 h;
    } tx; /* +0x018, coord.t[0] */

    s32 ty; /* +0x01C, coord.t[1] */

    union {
        s32 w;
        u16 h;
    } tz; /* +0x020, coord.t[2] */
};

/* ---- the pool allocator ------------------------------------------ */

extern void *BMemPMgrAlloc(s32 size);

/* ---- TitleMenu --------------------------------------------------- */

/* TitleMenu's menu description, a TaskCoreTarget: TitleMenu__TitleMenu
 * passes &D_80086D44 as TaskCore's ctor's `target` and again to setTarget. */
extern TaskCoreTarget D_80086D44;

/* "ETC\ETCSE", TitleMenu__TitleMenu's soundBankPath for TaskCore's ctor
 * (the ctor casts away the const for its `char *`). */
extern const char D_800114DC[];

/* "ETC\TITLE.TIM", TitleMenu__Reset's path for setSubHandle. */
extern const char sTitleTimPath[];

/* "CARD\FILEICN1.TIM", TitleMenu__BeginCardAccess's path for New_TimImage.
 * A string splat already emitted as a symbol: a literal would emit a
 * second copy. */
extern const char sSaveIconTimPath[];

/* The two 320 x 240 display buffers, stacked in VRAM at y 0 and y 240:
 * TitleMenu__OnDeinit clears each with the DrawSystem's clearImage. */
extern DrawRect sDisplayBufferRects[2];

/* TitleMenu__AttachSaveTitle's offset for the name field's attachToParent
 * (TextRow +0x04C), passed as a LongVec3 (the words are -4, -23, ...). */
extern s32 sSaveTitleOffset;

/*
 * The save file's name and title buffers, read by value (%gp_rel) and
 * forwarded: TitleMenu__SaveToCard and TitleMenu__LoadFromCard pass both to
 * TaskObjF's beginSave/beginLoad as `fileName` and `title`. The ROM image
 * points each into the rodata block at D_80011434 (0x80011464, 0x8001149C;
 * no symbol of their own there), so they are opaque `void *`.
 * TitleMenu__CreateSaveTitle strcpy's into gSaveTitle + 0x18 and strlen's
 * it, so at run time gSaveTitle holds a writable buffer; nothing in these
 * units sets it.
 */
extern void *sSaveFileName;
extern void *gSaveTitle;

/* The buffer FormatNumberIntoBuffer formats the day into
 * (FormatFullWidthNumber) before copying it into gSaveTitle's title. The
 * ROM image points it at the "7654321" string D_8008AA1C. */
extern void *D_8008AA24;

/* TitleMenu__CreateSaveTitle's strcpy source for the save title. The ROM
 * image holds 0x80011474, just past sSaveFileName's "BISLPS-01556xxx" string. */
extern void *sSaveTitleBlanks;

/* TaskObjF's init `namePrefix` (TitleMenu__BeginCardAccess): the ROM image
 * points it at the "BISLPS-01556" string in D_80011434. */
extern void *sCardFilePrefix;

/* TaskObjF's init `nameSuffixes` (TitleMenu__BeginCardAccess): 16 string
 * pointers, D_8008AA0C down to D_8008A9D4, then NULL. Address only. */
extern s32 sSaveFileSuffixes;

/* TitleMenu__CycleSaveTitleColor's index (0, 1, 2) into its 3-byte colour.
 * The storage is a word; every access is a byte (lbu/sb). */
extern u8 sSaveTitleColorChannel;

/* TitleMenu__CycleSaveTitleColor's frame counter (wraps to 0 at 0x101). */
extern s32 D_8008AA2C;

/* TaskObjF__TaskObjF's construction count: InitCARD/StartCARD/_bu_init run
 * only on the first construction, when it was 0 before the increment. */
extern s32 D_8008AA30;

/* Formats the current day into the save title (src/class_3bb8c_c.c);
 * TitleMenu__TitleMenu calls it with DreamSys's getCurrentDayAndYear. */
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

/* ---- TaskObjF ---------------------------------------------------- */

/* The kernel event calls TaskObjF's event methods make (OpenEvent,
 * EnableEvent, DisableEvent, TestEvent; Sony's libapi) are declared in the
 * units that call them. */

/* Per TaskObjF::events slot: the event spec TaskObjF__OpenEvents passes to
 * OpenEvent, and the value WaitForReadyEvent returns for that slot.
 * Unsized: only the four slots are read. */
extern s32 gCardEventSpecs[];

/* Loops until one of `count` events tests ready (TestEvent) and returns its
 * gCardEventSpecs entry. TaskObjF__WaitForReadyEvent passes TaskObjF::events. */
extern s32 WaitForReadyEvent(s32 *arr, s32 count);

/* Returns a pointer, which TaskObjF__FreeUnusedBuffers stores back into the
 * freed slot. */
extern void *BMemPMgrFree(void *ptr);

/* A 6-byte memory-card device name, "bu00:" or "bu10:" (the BIOS names of
 * the two card slots). BuildMemcardPath copies one as a whole struct.
 * MATCHING: all-s8 members (alignment 1) make that copy retail's unaligned
 * lwl/lwr plus byte stores. */
typedef struct McDevicePath {
    s8 b0, b1, b2, b3, b4, b5;
} McDevicePath;

extern McDevicePath gMcDevicePath1; /* "bu10:" */
extern McDevicePath gMcDevicePath0; /* "bu00:" */

/* The game's own strcat (src/code_171e0.c). */
extern char *strcat(char *dest, char *src);

/* Game code (src/class_3bb8c_g.c). TaskObjF__WriteMemcardSaveFile calls it
 * around its retry loop, and with (arg, 0) when the loop gives up. The BIOS
 * file calls (open, read, lseek, close, delete; Sony's libapi) are declared
 * in the units that call them. */
extern s32 StampSaveTitleFileLetter(char *titleText, char *fileName);

/* ---- ObjM -------------------------------------------------------- */

/* ObjM::styleConfig's pointee (include/ObjM.h): the day's scene style, a
 * plain record. RegisterStyleConfig returns D_80087424 after
 * FillStyleFromConfig fills its last four words from the stage's config
 * bytes (class_3bb8c_m, whose local StyleM views the same words), or
 * InitStyleAndWorld's caller supplies one. ObjM__SetupSceneStyle hands the
 * first three to the StageMap's lights, ObjM__EnterStyleSession the rest to
 * the viewport, ObjM__PollTimBlockLoad a colour to the TimBlockSrc. */
typedef struct StyleConfig {
    s32 lightDirs;    /* +0x000, SetupSceneStyle: the StageMap's setChildParams `dirs` */
    s32 lightColors;  /* +0x004, SetupSceneStyle: setChildParams `colors` */
    s32 ambientColor; /* +0x008, SetupSceneStyle: setAmbientColor's rgb (a pointer) */
    void *clearColor; /* +0x00C, EnterStyleSession: the viewport's setClearColor; a gStylePalette entry */
    u8 pad10[0x014 - 0x010];
    s32 colorMode; /* +0x014, EnterStyleSession: 1 makes the far colour clearColor; PollTimBlockLoad: 2 fades to clearColor, else farColor */
    void *farColor; /* +0x018, EnterStyleSession: setFarColor unless colorMode is 1; a gStylePalette entry */
    s32 fogNear; /* +0x01C, EnterStyleSession: the viewport's setFogNear; a D_8008730C value */
} StyleConfig;

/* ObjM__GetGridRecord's grid lookups (src/code_39094.c): a
 * non-negative code is a linear cell index (GetGridRecordAt(index, code)),
 * a negative one sends x/y to GetGridRecordXY. */
extern s32 GetGridRecordAt(s32 index, s32 sub);
extern void GetGridRecordXY(s32 index, s32 x, s32 y);

/* ObjM__TickStyle's helper (src/class_3bb8c_n.c), typed from that call. */
extern void TickStyle(void *arg0, void *arg1, s32 arg2);

/* ObjM__TeardownStyle's helpers (src/code_4cd08.c, src/class_3bb8c_n.c). */
extern void TickDreamAuxSlots2(void);
extern void StyleTeardown(void);

#endif
