#ifndef STAGEMAP_H
#define STAGEMAP_H

#include "LightRig.h"
#include "StageGrid.h"

/*
 * StageMap -- the grid manager (class id 0x114, method table
 * gStageMapMethods at 0x800866E8): LightRig's subclass, no class below it.
 * Its ctor and finalize chain to LightRig's first, and it inherits getLight
 * and setAmbientColor unchanged. Methods in src/class_3ac78.c
 * (New_StageMap .. SetConfig), src/class_3bb8c.c (SetTargetAndBuildRates
 * .. FindElementForPosition) and src/class_3bb8c_b.c (FindElemIndexByUnk32
 * .. GetStageMapMethods). The game makes one, at boot
 * (Class865C8__Class865C8 via New_StageMap(NULL, 1)); Actor keeps it as
 * `grid` (include/Actor.h) when addChild sees a class-0x114 child.
 *
 * What its own methods do:
 *  - the ctor makes seven elements (`elems`), each a LbdFile `loader`, a
 *    Class6D940 `placements`, a GridCell `cellParent` attached to this
 *    object at `origin`, and a 0x668-byte block of GridCell `cells`
 *    attached to the cellParent on a 0x800-unit lattice. Every grid index
 *    uses a row stride of 20 cells: gDefaultGridSpan (0xA000) >> 11, stored
 *    by setGridSpan in `gridCells`.
 *  - applyRateEntries loads an element's file through its loader
 *    (LbdFile loadHeader); onNotifyTag1 consumes the header when it is
 *    read and loadElementResources links each cell to its placement's model.
 *  - updateFootprintTracking (every tick while `enabled`) turns `target`'s
 *    world position into a Descriptor10Ext (computeFootprintDescriptor) and
 *    keeps it in `targetCell`, notifying parents with 5 when its leading
 *    halfword (the element row/column pair) changes; refreshFootprint then
 *    rebuilds `rects`, the up to four element-local rectangles the target
 *    covers, and flags their cells (StageMap__SetFootprintCellFlag).
 *  - forwardAcceptedCommand/applyToSenderFootprint re-notify every cell of
 *    a sender's rectangle (DispatchToRectCells, NotifyGridCell).
 * Descriptor10 has the shape of DreamSys.h's PlayerSpawnPoint (chunk
 * col/row, tile col/row, s16 x/y/z): DreamSys__WallLink copies
 * getCurrentCellKey's result into its linkCoordinates whole. The two are
 * not one type: this class reads the leading bytes signed
 * (ComputeCellWorldOffsets), PlayerSpawnPoint declares them u8.
 *
 * Inherited slots it overrides (`tools/classtable.py gStageMapMethods --vs
 * gLightRigMethods`): +0x008 the ctor, +0x00C Finalize, +0x038 OnNotify,
 * +0x040 Reset, +0x088 notifyIfUnk20Active (StageMap__OnElementEvent,
 * see below), +0x098 update (StageMap__UpdateIfEnabled) and +0x09C
 * DispatchLinkCommand.
 *
 * +0x088: StageMap__OnElementEvent takes (self, command, elem); the slot
 * keeps SceneNode's (self, event). Its four callers (Finalize,
 * ResetAllElements, ApplyRateEntries, OnNotifyTag1) pass (self, 6 or 7,
 * elem, index) through StageMapOnSlotEventFn below (no code).
 * +0x0D4 getCurrentCellKey and +0x0F8 buildRateEntries keep their CALLERS'
 * shapes: DreamSys__WallLink passes getCurrentCellKey a second word the
 * occupant never reads, and SetTargetAndBuildRates returns buildRateEntries'
 * value although the occupant returns nothing.
 *
 * The object is 0x1E8 bytes (New_StageMap).
 */

struct LbdFile;
struct Class6D940;
struct GridCell;

typedef struct StageMap StageMap;
typedef struct StageMapMethods StageMapMethods;
typedef struct ChunkSlot ChunkSlot;

/* A grid-cell descriptor, 10 bytes, alignment 2 (every member s8/s16, so a
 * whole copy is lwl/lwr + swl/swr + sh: SetTargetAndBuildRates). b0/b1 the
 * element column/row (ComputeDivisorSplit: rate % divisor, rate / divisor),
 * b2/b3 the cell column/row inside the element, h4/h6/h8 the offset inside
 * the cell (ComputeFootprintDescriptor). UpdateFootprintTracking and
 * DispatchToRectCells read b0/b1 as one u16. */
typedef struct Descriptor10 {
    s8 b0;
    s8 b1;
    s8 b2;
    s8 b3;
    s16 h4;
    s16 h6;
    s16 h8;
} Descriptor10;

/* computeFootprintDescriptor's output, 0x2C bytes. `targetCell` holds one. */
typedef struct Descriptor10Ext {
    Descriptor10 base; /* +0x000 */
    LongVec3 chunkCentre; /* +0x00C, the slot's cellParent position + 0x5000 in x and z (half a chunk) */
    LongVec3 relPos; /* +0x018, the queried position less chunkCentre in x and z; y as queried */
    ChunkSlot *slot; /* +0x024, the slot findElementForPosition resolved */
    s32 chunkIndex;       /* +0x028, that slot's LbdFile::chunkIndex, sign-extended */
} Descriptor10Ext;

/* computeFootprintDescriptor's world position: each word is read whole (the
 * cell) and, later, as its low halfword (the offset), which is why each is
 * a union: retail reloads at the narrower width. */
typedef struct SplitLongVec3 {
    union {
        s32 w;
        u16 h;
    } x; /* +0x000 */

    union {
        s32 w;
        u16 h;
    } y; /* +0x004 */

    union {
        s32 w;
        u16 h;
    } z; /* +0x008 */
} SplitLongVec3;

/* The step from a centre chunk's index (row * columns + column) to one of
 * the seven chunks around it, indexed by ChunkSlotSpec::key
 * (sRateEntryTable, ComputeRateEntry): rowDelta rows, then colDeltaOddRow
 * or colDeltaEvenRow columns by the centre row's parity (odd rows sit half a
 * chunk to -x, ComputeCellWorldOffsets). The table holds the centre (key 3,
 * all 0) and its six staggered neighbours. */
typedef struct ChunkNeighbourDelta {
    s32 rowDelta;        /* +0x0 */
    s32 colDeltaOddRow;  /* +0x4 */
    s32 colDeltaEvenRow; /* +0x8 */
} ChunkNeighbourDelta;

/* applyRateEntries' 0xC-byte entries, one per slot to (re)load: the file
 * record valueFn returned for the chunk (NULL: cancel the slot's load), the
 * chunk's index in the stage grid and the neighbour key of the slot that
 * takes it. ComputeRateEntry writes chunkIndex as a whole word. */
typedef struct ChunkLoadEntry {
    void *file;     /* +0x0 */
    s16 chunkIndex; /* +0x4 */
    u8 pad6[0x8 - 0x6];
    s32 neighbour; /* +0x8 */
} ChunkLoadEntry;

/* The same 0xC stride based at +0x4: ApplyRateEntries' second walker
 * (strength-reduced from the parameter; its report). */
typedef struct ChunkLoadEntryTail {
    s16 chunkIndex; /* +0x4 in ChunkLoadEntry terms */
    u8 pad2[0x4 - 0x2];
    s32 neighbour; /* +0x8 */
    u8 pad8[0xC - 0x8];
} ChunkLoadEntryTail;

/* buildRateEntries' per-slot pair (sDefaultTargetSpecs and the
 * sFootprintResultPtrTable tables hold seven each): the neighbour key the
 * slot takes (0..6, the index into sRateOffsetTable and sRateEntryTable;
 * 3 is the centre) and whether it is (re)loaded and repositioned. */
typedef struct ChunkSlotSpec {
    u8 neighbour; /* +0x0 */
    u8 load;      /* +0x1 */
} ChunkSlotSpec;

/* One element-local rectangle of cells, 0xC bytes, no padding. */
typedef struct CellRect {
    s32 slotIndex; /* +0x0, index into elems[] */
    s16 col;       /* +0x4, starting column */
    s16 row;       /* +0x6, starting row (row stride 20) */
    s16 width;     /* +0x8 */
    s16 height;    /* +0xA */
} CellRect;

/* `rects` as a whole, so ApplyToSenderFootprint can save and restore it
 * with a plain `=` (a batched 4-word block move; an indexed loop does not
 * compile to it). */
typedef struct CellRectSet {
    CellRect e[4];
} CellRectSet;

/* `bounds`' pointee (setBounds): IsPointOutOfBounds' box of cell columns/rows inside a chunk. */
typedef struct CellBounds {
    s16 minCol; /* +0x000 */
    s16 minRow; /* +0x002 */
    s32 maxCol; /* +0x004 */
    s32 maxRow; /* +0x008 */
} CellBounds;

/* One of the seven elements, 0x1C bytes (the ctor, Finalize). */
struct ChunkSlot {
    /* +0x000 */ u16 loadPending; /* 1 while its load is pending (ApplyRateEntries; OnNotifyTag1 clears it) */
    /* +0x002 */ u16 neighbour; /* the ctor: its index; BuildRateEntries: the spec's neighbour, copied on into loader->elemKey */
    /* +0x004 */ struct LbdFile *loader; /* New_LbdFile(): the element's file (include/LbdFile.h) */
    /* +0x008 */ struct Class6D940 *placements; /* New_Class6D940(0): its placement records (include/Class6D940.h) */
    /* +0x00C */ struct GridCell *cellParent; /* New_GridCell(), attached to the StageMap at `origin`; every cell's parent */
    /* +0x010 */ struct GridCell **cells; /* BMemPMgrAlloc(0x668): 410 New_GridCell() cells, row stride 20 */
    /* +0x014 */ BasicClass *heldObj; /* zeroed by the ctor; OnElementEvent releases it on event 6 */
    /* +0x018 */ s32 unk18;           /* zeroed by the ctor */
};

/* The ctor's callback pair (setCallback): ComputeRateEntry calls
 * valueFn(valueFnCtx, value, 0, 0) and keeps the result as an entry's name. */
typedef void *(*ChunkFileFn)(void *ctx, s32 value, s32 arg2, s32 arg3);

/* LightRig's slots, then this class's own. */
struct StageMapMethods {
    LIGHTRIG_SLOTS(StageMap, (StageMap * self, LongVec3 *origin, s32 autoLoad));
    /* +0x0C0 */ void (*resetAllElements)(StageMap *self); /* StageMap__ResetAllElements */
    /* +0x0C4 */ void (*setChildParams)(StageMap *self, s32 count, s32 dirs,
                                        s32 colors); /* StageMap__SetChildParams */
    /* +0x0C8 */ void (*setCallback)(StageMap *self, ChunkFileFn fn, void *ctx); /* StageMap__SetCallback */
    /* +0x0CC */ void (*setAcceptedTags)(StageMap *self, s32 *tags); /* StageMap__SetAcceptedTags */
    /* +0x0D0 */ void (*forwardAcceptedCommand)(StageMap *self, void *sender,
                                                s32 command); /* StageMap__ForwardAcceptedCommand */
    /* +0x0D4 */ Descriptor10 *(*getCurrentCellKey)(StageMap *self, void *arg1); /* StageMap__GetCurrentCellKey (reads only self; see the banner) */
    /* +0x0D8 */ void (*slotD8)(void); /* func_8004B324, empty; never called */
    /* +0x0DC */ void (*setGridSpan)(StageMap *self, s32 span); /* StageMap__SetGridSpan */
    /* +0x0E0 */ void (*setConfig)(StageMap *self, StageGridDimensions *config); /* StageMap__SetConfig */
    /* +0x0E4 */ s32 (*setTargetAndBuildRates)(StageMap *self, void *outPos, SceneNode *target,
                                               Descriptor10 *cell); /* StageMap__SetTargetAndBuildRates */
    /* +0x0E8 */ s32 (*computeCellOffsets)(StageMap *self, void *outPos,
                                           void *cell); /* StageMap__ComputeCellOffsets */
    /* +0x0EC */ void (*enable)(StageMap *self);      /* StageMap__Enable */
    /* +0x0F0 */ void (*disable)(StageMap *self);     /* StageMap__Disable */
    /* +0x0F4 */ s32 (*updateFootprintTracking)(StageMap *self); /* StageMap__UpdateFootprintTracking */
    /* +0x0F8 */ s32 (*buildRateEntries)(StageMap *self, s32 val, LongVec3 *pos,
                                         ChunkSlotSpec *specs); /* StageMap__BuildRateEntries (returns nothing; see the banner) */
    /* +0x0FC */ void (*applyRateEntries)(StageMap *self, ChunkLoadEntry *entries,
                                          s32 count); /* StageMap__ApplyRateEntries */
    /* +0x100 */ void (*onNotifyTag1)(StageMap *self, void *sender,
                                      s32 mode); /* StageMap__OnNotifyTag1; OnNotify's class-1 sender case */
    /* +0x104 */ void (*loadElementResources)(StageMap *self, ChunkSlot *elem); /* StageMap__LoadElementResources */
    /* +0x108 */ void (*resetElementCells)(StageMap *self, ChunkSlot *elem); /* StageMap__ResetElementCells */
    /* +0x10C */ Descriptor10 *(*getTargetDescriptor)(StageMap *self, Descriptor10Ext *out,
                                                      void **outPos); /* StageMap__GetTargetDescriptor */
    /* +0x110 */ s32 (*computeFootprintDescriptor)(StageMap *self, Descriptor10Ext *out,
                                                   SplitLongVec3 *pos); /* StageMap__ComputeFootprintDescriptor: 0, or 1 when no element holds pos */
    /* +0x114 */ ChunkSlot *(*getLastTargetRateSplit)(StageMap *self, u8 *out); /* StageMap__GetLastTargetRateSplit */
    /* +0x118 */ ChunkSlot *(*findElemByUnk32)(StageMap *self, s32 key); /* StageMap__FindElemByUnk32 */
    /* +0x11C */ ChunkSlot *(*findElementForPosition)(StageMap *self, LongVec3 *pos); /* StageMap__FindElementForPosition */
    /* +0x120 */ s32 (*findElemIndexByUnk32)(StageMap *self, s32 key); /* StageMap__FindElemIndexByUnk32 */
    /* +0x124 */ s32 (*findElemIndexByUnk30)(StageMap *self, s32 key); /* StageMap__FindElemIndexByUnk30: an index or -1 */
    /* +0x128 */ void (*refreshFootprint)(StageMap *self); /* StageMap__RefreshFootprint */
    /* +0x12C */ void (*applyToSenderFootprint)(StageMap *self, SceneNode *sender,
                                                s32 command); /* StageMap__ApplyToSenderFootprint */
    /* +0x130 */ void *(*getUnk1CC)(StageMap *self);        /* StageMap__GetUnk1CC */
    /* +0x134 */ void (*setBounds)(StageMap *self, CellBounds *bounds); /* StageMap__SetBounds */
    /* +0x138 */ void (*configureRateEntry)(StageMap *self, s32 rate,
                                            s32 flag); /* StageMap__ConfigureRateEntry */
    /* +0x13C */ void (*advanceRateCountdown)(StageMap *self); /* StageMap__AdvanceRateCountdown */
    /* +0x140 */ void (*flushRateLatch)(StageMap *self);       /* StageMap__FlushRateLatch */
}; /* 80 slots */

struct StageMap {
    LIGHTRIG_FIELDS(StageMapMethods);
    /* +0x054 */ LongVec3 origin; /* the ctor: its argument, or gDefaultOrigin; the cellParents attach here */
    /* +0x060 */ ChunkFileFn valueFn; /* setCallback */
    /* +0x064 */ void *valueFnCtx;          /* setCallback */
    /* +0x068 */ StageGridDimensions *config; /* setConfig (ObjM: GetStageGridDimensions(stage)); NULL after Reset */
    /* +0x06C */ SceneNode *target; /* setTargetAndBuildRates (DreamSys__SpawnAtLink passes the DreamSys); its coord2 is the tracked position */
    /* +0x070 */ s32 enabled;       /* enable/disable; gates UpdateIfEnabled */
    /* +0x074 */ s32 gridSpan;      /* setGridSpan: gDefaultGridSpan = 0xA000 */
    /* +0x078 */ s16 gridHalfCells; /* gridSpan >> 12 = 10 */
    /* +0x07A */ s16 gridCells;     /* gridSpan >> 11 = 20, the row stride */
    /* +0x07C */ s16 footprintCol; /* BuildFootprintSlots' input: a signed column, clamped into [0,20) */
    /* +0x07E */ s16 footprintRow; /* the same, vertical */
    /* +0x080 */ s32 footprintWidth;
    /* +0x084 */ s32 footprintHeight;
    /* +0x088 */ s32 rectCount;           /* how many of rects[] are live */
    /* +0x08C */ CellRectSet rects; /* BuildFootprintSlots, SetFootprintRect, InitFootprintSlot write; DispatchToRectCells, SetFootprintCellFlag walk */
    /* +0x0BC */ Descriptor10Ext targetCell; /* UpdateFootprintTracking: the target's last descriptor; SetTargetAndBuildRates sets .base; getTargetDescriptor returns &.base */
    /* +0x0E8 */ s32 *acceptedTags; /* setAcceptedTags: a 0-terminated list of class ids ForwardAcceptedCommand accepts */
    /* +0x0EC */ ChunkSlot elems[7];
    /* +0x1B0 */ s32 loadsPending; /* 1 while element loads are pending (ApplyRateEntries; OnNotifyTag1 clears it) */
    /* +0x1B4 */ u16 unk1B4; /* CountFlaggedElements after ApplyRateEntries; OnNotifyTag1 counts it down */
    /* +0x1B6 */ u8 pad1B6[0x1B8 - 0x1B6];
    /* +0x1B8 */ s32 chunksLoaded; /* set when that count reaches 0; RefreshFootprint does nothing while it is 0 */
    /* +0x1BC */ ChunkSlot *lastEventElem; /* OnElementEvent's elem; GetLastTargetRateSplit reads it */
    /* +0x1C0 */ Descriptor10 curCell; /* DispatchToRectCells: the cell being notified (b0/b1 copied from targetCell as a u16); getCurrentCellKey returns it */
    /* +0x1CA */ u8 pad1CA[0x1CC - 0x1CA];
    /* +0x1CC */ s32 unk1CC;                  /* Reset: -1; GetUnk1CC returns its address */
    /* +0x1D0 */ s32 unk1D0;                  /* Reset: -1 */
    /* +0x1D4 */ s32 unk1D4;                  /* Reset: -1 */
    /* +0x1D8 */ s32 unk1D8;                  /* Reset: -1 */
    /* +0x1DC */ CellBounds *bounds; /* setBounds; IsPointOutOfBounds */
    /* +0x1E0 */ s32 rateCountdown;  /* configureRateEntry; advanceRateCountdown/flushRateLatch */
    /* +0x1E4 */ Ratio16 *scaleStep; /* configureRateEntry: one of four Ratio16[3] steps (x, y, z) that advanceRateCountdown adds to every cell's scale; only y is nonzero, +-1/64 or +-1/4 */
}; /* 0x1E8 bytes: New_StageMap */

/* +0x088's occupant, which takes the element too (see the banner). */
typedef void (*StageMapOnSlotEventFn)(StageMap *self, s32 command, ChunkSlot *elem, s32 index);

/* ForEachElem's callbacks. */
typedef void (*StageMapCellFn)(StageMap *self, struct GridCell *cell);
typedef void (*ChunkSlotFn)(StageMap *self, ChunkSlot *elem);

extern StageMapMethods gStageMapMethods;
extern StageMapMethods *GetStageMapMethods(void); /* returns &gStageMapMethods */

/* The class's own functions, in address order: the occupants of
 * gStageMapMethods and their non-slot helpers. */
StageMap *New_StageMap(LongVec3 *origin, s32 autoLoad);
void StageMap__StageMap(StageMap *self, LongVec3 *origin, s32 autoLoad);
void StageMap__Finalize(StageMap *self);
void StageMap__OnNotify(StageMap *self, BasicClass *sender, s32 command);
void StageMap__Reset(StageMap *self);
void StageMap__OnElementEvent(StageMap *self, s32 command, ChunkSlot *elem);
void StageMap__UpdateIfEnabled(StageMap *self);
void StageMap__DispatchLinkCommand(StageMap *self, BasicClass *sender, s32 command);
void StageMap__ResetAllElements(StageMap *self);
void StageMap__SetChildParams(StageMap *self, s32 count, s32 dirs, s32 colors);
void StageMap__SetCallback(StageMap *self, ChunkFileFn fn, void *ctx);
void StageMap__SetAcceptedTags(StageMap *self, s32 *tags);
void StageMap__ForwardAcceptedCommand(StageMap *self, void *sender, s32 command);
void StageMap__ApplyToSenderFootprint(StageMap *self, SceneNode *sender, s32 command);
void StageMap__SetFootprintFromCell(StageMap *self, Descriptor10Ext *desc, s32 span);
void StageMap__SetFootprintRect(StageMap *self, Descriptor10Ext *desc, s32 span);
void StageMap__DispatchToRectCells(StageMap *self, SceneNode *sender, s32 command);
void NotifyGridCell(struct GridCell *cell, SceneNode *sender, s32 command);
Descriptor10 *StageMap__GetCurrentCellKey(StageMap *self);
void func_8004B324(void);
void StageMap__SetGridSpan(StageMap *self, s32 span);
void StageMap__SetConfig(StageMap *self, StageGridDimensions *config);
s32 StageMap__SetTargetAndBuildRates(StageMap *self, void *outPos, SceneNode *target,
                                       Descriptor10 *cell);
s32 StageMap__ComputeCellOffsets(StageMap *self, void *outPos, void *cell);
s32 ComputeCellWorldOffsets(s32 *outPos, s32 *outBuf, StageGridDimensions *config, LongVec3 *origin,
                            Descriptor10 *cell);
void StageMap__Enable(StageMap *self);
void StageMap__Disable(StageMap *self);
s32 StageMap__UpdateFootprintTracking(StageMap *self);
void StageMap__BuildRateEntries(StageMap *self, s32 val, LongVec3 *pos, ChunkSlotSpec *specs);
s32 StageMap__ComputeRateFlags(StageMap *self, s32 val, s32 flag);
s32 StageMap__ComputeRateEntry(StageMap *self, ChunkLoadEntry *entry, s32 divisor, s32 flag,
                                 s32 val, s32 savedResult, s32 key); /* 0 or 1; BuildRateEntries discards it */
void StageMap__ApplyRateEntries(StageMap *self, ChunkLoadEntry *entries, s32 count);
s32 StageMap__CountFlaggedElements(StageMap *self);
void StageMap__OnNotifyTag1(StageMap *self, void *sender, s32 mode);
void StageMap__LoadElementResources(StageMap *self, ChunkSlot *elem);
void StageMap__ResetElementCells(StageMap *self, ChunkSlot *elem);
Descriptor10 *StageMap__GetTargetDescriptor(StageMap *self, Descriptor10Ext *out, void **outPos);
s32 StageMap__ComputeFootprintDescriptor(StageMap *self, Descriptor10Ext *out, SplitLongVec3 *pos);
void StageMap__ComputeDivisorSplit(StageMap *self, u8 *out, s32 val);
ChunkSlot *StageMap__GetLastTargetRateSplit(StageMap *self, u8 *out);
ChunkSlot *StageMap__FindElemByUnk32(StageMap *self, s32 key);
ChunkSlot *StageMap__FindElementForPosition(StageMap *self, LongVec3 *pos);
s32 StageMap__FindElemIndexByUnk32(StageMap *self, s32 key);
s32 StageMap__FindElemIndexByUnk30(StageMap *self, s32 key);
void StageMap__RefreshFootprint(StageMap *self);
void StageMap__ComputeFootprintFromRotation(StageMap *self, s32 width, s32 height);
void StageMap__BuildFootprintSlots(StageMap *self);
s32 StageMap__SplitFootprintSlot(StageMap *self, CellRect *slot, s32 count, s32 baseIdx,
                                   s32 col, s32 row, s32 width, s32 height);
void StageMap__SetFootprintFromQuery(StageMap *self);
s32 IsPointOutOfBounds(CellBounds *bounds, s8 *point);
s32 StageMap__InitFootprintSlot(StageMap *self, s32 unused, s32 key, s32 arg3);
void StageMap__SetFootprintCellFlag(StageMap *self, s32 setBit);
void *StageMap__GetUnk1CC(StageMap *self);
void StageMap__SetBounds(StageMap *self, CellBounds *bounds);
void StageMap__ConfigureRateEntry(StageMap *self, s32 rate, s32 flag);
void StageMap__AdvanceRateCountdown(StageMap *self);
void StageMap__FlushRateLatch(StageMap *self);
void StageMap__ApplyRateToChild(StageMap *self, struct GridCell *cell);
void StageMap__ResetChildRate(StageMap *self, struct GridCell *cell);
void StageMap__ForEachElem(StageMap *self, StageMapCellFn cellFn, ChunkSlotFn elemFn);
void StageMap__ForEachEntryChild(StageMap *self, StageMapCellFn cellFn, ChunkSlot *elem);

#endif
