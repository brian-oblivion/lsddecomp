#ifndef STAGEMAP_H
#define STAGEMAP_H

#include "LightRig.h"
#include "StageGrid.h"

/*
 * StageMap -- the part of a stage's map that is loaded: seven map chunks
 * around a tracked target, each laid out as a lattice of GridCells. Class
 * id 0x114, method table gStageMapMethods; LightRig's subclass (its ctor
 * and finalize chain to LightRig's first; getLight and setAmbientColor are
 * inherited unchanged), no class below it. Methods in src/DayTaskStageMap.c
 * (New_StageMap .. SetConfig), src/DayTaskStageMap.c (SetTargetAndLoadChunks
 * .. FindSlotForPosition) and src/DayTaskStageMap.c
 * (FindSlotIndexByNeighbour .. GetStageMapMethods).
 *
 * Lifecycle. The game makes one, at boot (DayTask__DayTask, via
 * New_StageMap(NULL, 1)). ObjM configures it for its stage: setConfig
 * with the stage's StageGridDimensions (GetStageGridDimensions),
 * setCallback with ObjM__GetGridRecord (a chunk index -> that chunk's
 * file record, GetStageMapChunkRecord), setGridSpan, setAcceptedTags,
 * setChildParams, setBounds, and enables it.
 * DreamSys__SpawnAtLink hands it the target (setTargetAndLoadChunks);
 * Actor keeps it as `grid` (include/Actor.h) when addChild sees a
 * class-0x114 child. Disable unloads every slot; Finalize releases them.
 *
 * The chunk grid. A stage is `columns` x `rows` chunks (StageGridDimensions),
 * a chunk 0xA000 units square, numbered row * columns + column; odd rows
 * sit half a chunk to -x (ComputeCellWorldOffsets), so a chunk has six
 * neighbours. A vertical grid (isVertical) stacks its chunks in y instead.
 *
 * The seven slots (`slots`, ChunkSlot). The ctor gives each an LbdFile
 * `loader`, a PlacementGrid `placements`, a GridCell `cellParent` attached to
 * this object at `origin`, and 410 GridCell `cells` attached to the
 * cellParent 0x800 units apart (20 x 20, row stride 20, then 10 cells for
 * chained placements). Each slot holds a neighbour key, 0..6 (3 is the
 * centre), that says where around the centre chunk it sits:
 *  - loadChunksAround takes a centre chunk index and position and a
 *    ChunkSlotSpec per slot; each slot marked `load` moves its cellParent to
 *    centre + sNeighbourOffsets[key] and gets a ChunkLoadEntry
 *    (ComputeChunkLoadEntry: the neighbour's chunk index from
 *    sChunkNeighbourDeltas, NULL when ComputeNeighbourMask puts it off the
 *    grid, else the chunk's file record from the callback);
 *  - applyChunkLoads starts each entry's LbdFile load (or cancels it);
 *    onNotifyTag1, as the reads finish, links each chunk's placements and
 *    models into its slot's cells (populateSlotCells), and sets
 *    `chunksLoaded` once none is pending.
 *
 * Tracking. Every tick while `enabled`, updateFootprintTracking turns the
 * target's position into a Descriptor10Ext (computeFootprintDescriptor:
 * chunk column/row, cell column/row, offset in the cell), keeps it in
 * `targetCell`, and notifies parents with 5 when the chunk changes. In a
 * flat grid it also re-centres: the slot holding the target picks a spec
 * through sFootprintResultRemap and sFootprintResultPtrTable (NULL, no
 * change, for the centre slot) and loadChunksAround reloads the slots that
 * spec marks around the target's chunk. Then refreshFootprint moves the
 * window of cells that is drawn: it sets GsDOFF (bit 31 of `attribute`) on
 * the cells of the old `rects`, rebuilds `rects`, the up to four cell
 * rectangles around the target, and clears it on theirs
 * (SetFootprintVisible); every other cell stays hidden (the ctor and
 * ClearSlotCells set the bit). forwardAcceptedCommand and
 * applyToSenderFootprint hand a sender's command to every cell under its
 * rectangle (DispatchToRectCells, NotifyGridCell).
 *
 * Scale ramp. startScaleRamp (called by Entity_b/e/g) picks a Ratio16[3]
 * step, y +-1/64 or +-1/4, and a tick count; stepScaleRamp (every tick
 * after tracking) adds it to every cell's scale until the count runs out,
 * and endScaleRamp sets every cell back to 1/1.
 *
 * Descriptor10 has the shape of DreamSys.h's PlayerSpawnPoint (chunk
 * col/row, tile col/row, s16 x/y/z): DreamSys__WallLink copies
 * getCurrentCellKey's result into its linkCoordinates whole. The two are
 * not one type: this class reads the leading bytes signed
 * (ComputeCellWorldOffsets), PlayerSpawnPoint declares them u8.
 *
 * Inherited slots it overrides (`tools/classtable.py gStageMapMethods --vs
 * gLightRigMethods`): +0x008 the ctor, +0x00C Finalize, +0x038 OnNotify,
 * +0x040 Reset, +0x088 notifyWithHull (StageMap__OnSlotEvent, see
 * below), +0x098 update (StageMap__UpdateIfEnabled) and +0x09C
 * DispatchLinkCommand.
 *
 * +0x088: StageMap__OnSlotEvent takes (self, command, slot); the slot
 * keeps SceneNode's (self, event). Its four callers (Finalize,
 * UnloadAllSlots, ApplyChunkLoads, OnNotifyTag1) pass (self, 6 or 7,
 * slot, index) through StageMapOnSlotEventFn below (no code).
 * +0x0D4 getCurrentCellKey and +0x0F8 loadChunksAround keep their CALLERS'
 * shapes: DreamSys__WallLink passes getCurrentCellKey a second word the
 * occupant never reads, and SetTargetAndLoadChunks returns
 * loadChunksAround's value although the occupant returns nothing.
 *
 * The object is 0x1E8 bytes (New_StageMap).
 */

struct LbdFile;
struct PlacementGrid;
struct GridCell;

typedef struct StageMap StageMap;
typedef struct StageMapMethods StageMapMethods;
typedef struct ChunkSlot ChunkSlot;

/* StageMap's class id (gStageMapMethods word +0x000). Three nibbles, so
 * `(header & 0xFFF) == STAGEMAP_CLASS_ID` tests for it or a class below it
 * (ObjM__OnNotify). */
#define STAGEMAP_CLASS_ID 0x114

/* The grid's geometry, in world units. A cell is STAGE_CELL_SIZE square:
 * ComputeFootprintDescriptor shifts a position's offset from its slot's
 * origin right by STAGE_CELL_SHIFT for the cell column/row. A chunk is
 * STAGE_CHUNK_CELLS cells square (the ctor's 20 x 20 lattice, 0x800 apart),
 * so STAGE_CHUNK_SIZE (0xA000, also gDefaultGridSpan) wide; odd rows sit
 * STAGE_CHUNK_SIZE / 2 to -x (ComputeCellWorldOffsets). */
#define STAGE_CELL_SHIFT 11
#define STAGE_CELL_SIZE (1 << STAGE_CELL_SHIFT)
#define STAGE_CHUNK_CELLS 20
#define STAGE_CHUNK_SIZE (STAGE_CHUNK_CELLS * STAGE_CELL_SIZE)

/* Half a chunk, in cells: the odd-row stagger, which is how far a cell
 * column moves when the drawn window crosses into the row above or below
 * (BuildFootprintRects, SplitFootprintRect). */
#define STAGE_CHUNK_HALF_CELLS (STAGE_CHUNK_CELLS / 2)

/* A slot's `cells`: the lattice, row stride STAGE_CHUNK_CELLS, then 10
 * overflow cells PopulateSlotCells hangs chained placements in; 410 in all
 * (the ctor's BMemPMgrAlloc(0x668), ClearSlotCells' walk). */
#define STAGE_SLOT_LATTICE_CELLS (STAGE_CHUNK_CELLS * STAGE_CHUNK_CELLS)
#define STAGE_SLOT_CELLS (STAGE_SLOT_LATTICE_CELLS + 10)

/* What StageMap passes its parents (notifyParents). CHUNK_CHANGED:
 * UpdateFootprintTracking, when the target's chunk column/row changes. The
 * two slot events go through notifyWithHull (StageMap__OnSlotEvent), which
 * records the slot as lastEventSlot before passing the command on:
 * SLOT_RELEASE before a slot is reloaded or its load cancelled
 * (ApplyChunkLoads; UnloadAllSlots and Finalize), releasing its heldObj;
 * SLOT_DATA_READY when the slot's LbdFile has read its data block
 * (OnNotifyTag1, on `dataReady`). */
enum StageMapEvent {
    STAGEMAP_EVENT_CHUNK_CHANGED = 5,
    STAGEMAP_EVENT_SLOT_RELEASE = 6,
    STAGEMAP_EVENT_SLOT_DATA_READY = 7
};

/* A grid-cell descriptor, 10 bytes, alignment 2 (every member s8/s16, so a
 * whole copy is lwl/lwr + swl/swr + sh: SetTargetAndLoadChunks). b0/b1 the
 * chunk column/row (SplitChunkIndex: index % columns, index / columns),
 * b2/b3 the cell column/row inside the chunk, h4/h6/h8 the offset inside
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
    ChunkSlot *slot; /* +0x024, the slot FindSlotForPosition resolved */
    s32 chunkIndex;  /* +0x028, that slot's LbdFile::chunkIndex, sign-extended */
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
 * the seven chunks around it, indexed by ChunkSlotSpec::neighbour
 * (sChunkNeighbourDeltas, ComputeChunkLoadEntry): rowDelta rows, then colDeltaOddRow
 * or colDeltaEvenRow columns by the centre row's parity (odd rows sit half a
 * chunk to -x, ComputeCellWorldOffsets). The table holds the centre (key 3,
 * all 0) and its six staggered neighbours. */
typedef struct ChunkNeighbourDelta {
    s32 rowDelta;        /* +0x0 */
    s32 colDeltaOddRow;  /* +0x4 */
    s32 colDeltaEvenRow; /* +0x8 */
} ChunkNeighbourDelta;

/* The neighbour keys (ChunkSlotSpec::neighbour, ChunkSlot::neighbour,
 * LbdFile::elemKey): the centre chunk and the six around it, as
 * sChunkNeighbourDeltas steps to them (rows, then columns for an odd / even
 * centre row) and sNeighbourOffsets places them (a row is +z, a column +x).
 * Each adjacent row touches two chunks, the lower and the higher column.
 * A vertical grid uses keys 0 .. rows - 1 as its stacked layers instead
 * (ComputeNeighbourMask, FindSlotForPosition). */
enum ChunkNeighbour {
    CHUNK_NEIGHBOUR_PREV_ROW_LO = 0, /* row -1; column -1 (odd row) or 0 (even) */
    CHUNK_NEIGHBOUR_PREV_ROW_HI = 1, /* row -1; column 0 (odd row) or +1 (even) */
    CHUNK_NEIGHBOUR_PREV_COL = 2,    /* column -1 */
    CHUNK_NEIGHBOUR_CENTRE = 3,
    CHUNK_NEIGHBOUR_NEXT_COL = 4,    /* column +1 */
    CHUNK_NEIGHBOUR_NEXT_ROW_LO = 5, /* row +1; column -1 (odd row) or 0 (even) */
    CHUNK_NEIGHBOUR_NEXT_ROW_HI = 6, /* row +1; column 0 (odd row) or +1 (even) */
    CHUNK_NEIGHBOUR_COUNT = 7
};

/* A key's bit, as sNeighbourBits holds it; ComputeNeighbourMask returns a
 * mask of the keys whose chunk lies on the grid. */
#define CHUNK_NEIGHBOUR_BIT(key) (1 << (key))

/* applyChunkLoads' 0xC-byte entries, one per slot to (re)load: the file
 * record chunkFileFn returned for the chunk (NULL: cancel the slot's load), the
 * chunk's index in the stage grid and the neighbour key of the slot that
 * takes it. ComputeChunkLoadEntry writes chunkIndex as a whole word. */
typedef struct ChunkLoadEntry {
    void *file;     /* +0x0 */
    s16 chunkIndex; /* +0x4 */
    u8 pad6[0x8 - 0x6];
    s32 neighbour; /* +0x8 */
} ChunkLoadEntry;

/* The same 0xC stride based at +0x4: ApplyChunkLoads' second walker
 * (strength-reduced from the parameter; its report). */
typedef struct ChunkLoadEntryTail {
    s16 chunkIndex; /* +0x4 in ChunkLoadEntry terms */
    u8 pad2[0x4 - 0x2];
    s32 neighbour; /* +0x8 */
    u8 pad8[0xC - 0x8];
} ChunkLoadEntryTail;

/* loadChunksAround's per-slot pair (sDefaultTargetSpecs and the
 * sFootprintResultPtrTable tables hold seven each): the neighbour key the
 * slot takes (0..6, the index into sNeighbourOffsets and sChunkNeighbourDeltas;
 * 3 is the centre) and whether it is (re)loaded and repositioned. */
typedef struct ChunkSlotSpec {
    u8 neighbour; /* +0x0 */
    u8 load;      /* +0x1 */
} ChunkSlotSpec;

/* One rectangle of cells inside one slot's lattice, 0xC bytes, no padding. */
typedef struct CellRect {
    s32 slotIndex; /* +0x0, index into slots[] */
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

/* One of the seven chunk slots, 0x1C bytes (the ctor, Finalize). */
struct ChunkSlot {
    /* +0x000 */ u16 loadPending; /* 1 while its load is pending (ApplyChunkLoads; OnNotifyTag1 clears it) */
    /* +0x002 */ u16 neighbour; /* the ctor: its index; LoadChunksAround: the spec's neighbour, copied on into loader->elemKey */
    /* +0x004 */ struct LbdFile *loader; /* New_LbdFile(): the slot's chunk file (include/LbdFile.h) */
    /* +0x008 */ struct PlacementGrid *placements; /* New_PlacementGrid(0): its placement records (include/PlacementGrid.h) */
    /* +0x00C */ struct GridCell *cellParent; /* New_GridCell(), attached to the StageMap at `origin`; every cell's parent */
    /* +0x010 */ struct GridCell **cells; /* BMemPMgrAlloc(0x668): 410 New_GridCell() cells, row stride 20 */
    /* +0x014 */ BasicClass *heldObj; /* zeroed by the ctor; OnSlotEvent releases it on event 6 */
    /* +0x018 */ s32 unk18;           /* zeroed by the ctor */
};

/* The ctor's callback pair (setCallback): ComputeChunkLoadEntry calls
 * chunkFileFn(chunkFileCtx, value, 0, 0) and keeps the result as the entry's file record. */
typedef void *(*ChunkFileFn)(void *ctx, s32 value, s32 arg2, s32 arg3);

/* LightRig's slots, then this class's own. */
struct StageMapMethods {
    LIGHTRIG_SLOTS(StageMap, (StageMap * self, LongVec3 *origin, s32 autoLoad));
    /* +0x0C0 */ void (*unloadAllSlots)(StageMap *self); /* StageMap__UnloadAllSlots */
    /* +0x0C4 */ void (*setChildParams)(StageMap *self, s32 count, s32 dirs,
                                        s32 colors); /* StageMap__SetChildParams */
    /* +0x0C8 */ void (*setCallback)(StageMap *self, ChunkFileFn fn, void *ctx); /* StageMap__SetCallback */
    /* +0x0CC */ void (*setAcceptedTags)(StageMap *self, s32 *tags); /* StageMap__SetAcceptedTags */
    /* +0x0D0 */ void (*forwardAcceptedCommand)(StageMap *self, void *sender,
                                                s32 command); /* StageMap__ForwardAcceptedCommand */
    /* +0x0D4 */ Descriptor10 *(*getCurrentCellKey)(StageMap *self, void *arg1); /* StageMap__GetCurrentCellKey (reads only self; see the banner) */
    /* +0x0D8 */ void (*slotD8)(void); /* StageMap__NoOpSlotD8, empty; never called */
    /* +0x0DC */ void (*setGridSpan)(StageMap *self, s32 span); /* StageMap__SetGridSpan */
    /* +0x0E0 */ void (*setConfig)(StageMap *self, StageGridDimensions *config); /* StageMap__SetConfig */
    /* +0x0E4 */ s32 (*setTargetAndLoadChunks)(StageMap *self, void *outPos, SceneNode *target,
                                               Descriptor10 *cell); /* StageMap__SetTargetAndLoadChunks */
    /* +0x0E8 */ s32 (*computeCellOffsets)(StageMap *self, void *outPos,
                                           void *cell);          /* StageMap__ComputeCellOffsets */
    /* +0x0EC */ void (*enable)(StageMap *self);                 /* StageMap__Enable */
    /* +0x0F0 */ void (*disable)(StageMap *self);                /* StageMap__Disable */
    /* +0x0F4 */ s32 (*updateFootprintTracking)(StageMap *self); /* StageMap__UpdateFootprintTracking */
    /* +0x0F8 */ s32 (*loadChunksAround)(StageMap *self, s32 val, LongVec3 *pos,
                                         ChunkSlotSpec *specs); /* StageMap__LoadChunksAround (returns nothing; see the banner) */
    /* +0x0FC */ void (*applyChunkLoads)(StageMap *self, ChunkLoadEntry *entries,
                                         s32 count); /* StageMap__ApplyChunkLoads */
    /* +0x100 */ void (*onNotifyTag1)(StageMap *self, void *sender,
                                      s32 mode); /* StageMap__OnNotifyTag1; OnNotify's class-1 sender case */
    /* +0x104 */ void (*populateSlotCells)(StageMap *self, ChunkSlot *slot); /* StageMap__PopulateSlotCells */
    /* +0x108 */ void (*clearSlotCells)(StageMap *self, ChunkSlot *slot); /* StageMap__ClearSlotCells */
    /* +0x10C */ Descriptor10 *(*getTargetDescriptor)(StageMap *self, Descriptor10Ext *out,
                                                      void **outPos); /* StageMap__GetTargetDescriptor */
    /* +0x110 */ s32 (*computeFootprintDescriptor)(StageMap *self, Descriptor10Ext *out,
                                                   SplitLongVec3 *pos); /* StageMap__ComputeFootprintDescriptor: 0, or 1 when no slot holds pos */
    /* +0x114 */ ChunkSlot *(*getLastEventSlotChunk)(StageMap *self, u8 *out); /* StageMap__GetLastEventSlotChunk */
    /* +0x118 */ ChunkSlot *(*findSlotByNeighbour)(StageMap *self, s32 key); /* StageMap__FindSlotByNeighbour */
    /* +0x11C */ ChunkSlot *(*findSlotForPosition)(StageMap *self, LongVec3 *pos); /* StageMap__FindSlotForPosition */
    /* +0x120 */ s32 (*findSlotIndexByNeighbour)(StageMap *self, s32 key); /* StageMap__FindSlotIndexByNeighbour */
    /* +0x124 */ s32 (*findSlotIndexByChunk)(StageMap *self, s32 chunkIndex); /* StageMap__FindSlotIndexByChunk: an index or -1 */
    /* +0x128 */ void (*refreshFootprint)(StageMap *self); /* StageMap__RefreshFootprint */
    /* +0x12C */ void (*applyToSenderFootprint)(StageMap *self, SceneNode *sender,
                                                s32 command); /* StageMap__ApplyToSenderFootprint */
    /* +0x130 */ void *(*getUnk1CC)(StageMap *self);          /* StageMap__GetUnk1CC */
    /* +0x134 */ void (*setBounds)(StageMap *self, CellBounds *bounds); /* StageMap__SetBounds */
    /* +0x138 */ void (*startScaleRamp)(StageMap *self, s32 rate, s32 fast); /* StageMap__StartScaleRamp */
    /* +0x13C */ void (*stepScaleRamp)(StageMap *self); /* StageMap__StepScaleRamp */
    /* +0x140 */ void (*endScaleRamp)(StageMap *self);  /* StageMap__EndScaleRamp */
}; /* 80 slots */

struct StageMap {
    LIGHTRIG_FIELDS(StageMapMethods);
    /* +0x054 */ LongVec3 origin; /* the ctor: its argument, or gDefaultOrigin; the cellParents attach here */
    /* +0x060 */ ChunkFileFn chunkFileFn; /* setCallback */
    /* +0x064 */ void *chunkFileCtx;      /* setCallback */
    /* +0x068 */ StageGridDimensions *config; /* setConfig (ObjM: GetStageGridDimensions(stage)); NULL after Reset */
    /* +0x06C */ SceneNode *target; /* setTargetAndLoadChunks (DreamSys__SpawnAtLink passes the DreamSys); its coord2 is the tracked position */
    /* +0x070 */ s32 enabled;       /* enable/disable; gates UpdateIfEnabled */
    /* +0x074 */ s32 gridSpan;      /* setGridSpan: gDefaultGridSpan = 0xA000 */
    /* +0x078 */ s16 gridHalfCells; /* gridSpan >> 12 = 10 */
    /* +0x07A */ s16 gridCells;     /* gridSpan >> 11 = 20, the row stride */
    /* +0x07C */ s16 footprintCol; /* BuildFootprintRects' input: a signed column, clamped into [0,20) */
    /* +0x07E */ s16 footprintRow; /* the same, vertical */
    /* +0x080 */ s32 footprintWidth;
    /* +0x084 */ s32 footprintHeight;
    /* +0x088 */ s32 rectCount;     /* how many of rects[] are live */
    /* +0x08C */ CellRectSet rects; /* BuildFootprintRects, SetFootprintRect, InitFootprintRect write; DispatchToRectCells, SetFootprintVisible walk */
    /* +0x0BC */ Descriptor10Ext targetCell; /* UpdateFootprintTracking: the target's last descriptor; SetTargetAndLoadChunks sets .base; getTargetDescriptor returns &.base */
    /* +0x0E8 */ s32 *acceptedTags; /* setAcceptedTags: a 0-terminated list of class ids ForwardAcceptedCommand accepts */
    /* +0x0EC */ ChunkSlot slots[7];
    /* +0x1B0 */ s32 loadsPending; /* 1 while chunk loads are pending (ApplyChunkLoads; OnNotifyTag1 clears it) */
    /* +0x1B4 */ u16 pendingLoadCount; /* CountPendingLoads after ApplyChunkLoads; OnNotifyTag1 counts it down */
    /* +0x1B6 */ u8 pad1B6[0x1B8 - 0x1B6];
    /* +0x1B8 */ s32 chunksLoaded; /* set when that count reaches 0; RefreshFootprint does nothing while it is 0 */
    /* +0x1BC */ ChunkSlot *lastEventSlot; /* OnSlotEvent's slot; GetLastEventSlotChunk reads it */
    /* +0x1C0 */ Descriptor10 curCell; /* DispatchToRectCells: the cell being notified (b0/b1 copied from targetCell as a u16); getCurrentCellKey returns it */
    /* +0x1CA */ u8 pad1CA[0x1CC - 0x1CA];
    /* +0x1CC */ s32 unk1CC;         /* Reset: -1; GetUnk1CC returns its address */
    /* +0x1D0 */ s32 unk1D0;         /* Reset: -1 */
    /* +0x1D4 */ s32 unk1D4;         /* Reset: -1 */
    /* +0x1D8 */ s32 unk1D8;         /* Reset: -1 */
    /* +0x1DC */ CellBounds *bounds; /* setBounds; IsPointOutOfBounds */
    /* +0x1E0 */ s32 scaleRampTicks; /* startScaleRamp: |amount| * the step's y den; stepScaleRamp counts it down (-1 when done), endScaleRamp zeroes it */
    /* +0x1E4 */ Ratio16 *scaleStep; /* startScaleRamp: one of four Ratio16[3] steps (x, y, z) that stepScaleRamp adds to every cell's scale; only y is nonzero, +-1/64 or +-1/4 */
}; /* 0x1E8 bytes: New_StageMap */

/* +0x088's occupant, which takes the slot too (see the banner). */
typedef void (*StageMapOnSlotEventFn)(StageMap *self, s32 command, ChunkSlot *slot, s32 index);

/* ForEachSlot's callbacks. */
typedef void (*StageMapCellFn)(StageMap *self, struct GridCell *cell);
typedef void (*ChunkSlotFn)(StageMap *self, ChunkSlot *slot);

extern StageMapMethods gStageMapMethods;
extern StageMapMethods *GetStageMapMethods(void); /* returns &gStageMapMethods */

/* The class's own functions, in address order: the occupants of
 * gStageMapMethods and their non-slot helpers. */
StageMap *New_StageMap(LongVec3 *origin, s32 autoLoad);
void StageMap__StageMap(StageMap *self, LongVec3 *origin, s32 autoLoad);
void StageMap__Finalize(StageMap *self);
void StageMap__OnNotify(StageMap *self, BasicClass *sender, s32 command);
void StageMap__Reset(StageMap *self);
void StageMap__OnSlotEvent(StageMap *self, s32 command, ChunkSlot *slot);
void StageMap__UpdateIfEnabled(StageMap *self);
void StageMap__DispatchLinkCommand(StageMap *self, BasicClass *sender, s32 command);
void StageMap__UnloadAllSlots(StageMap *self);
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
void StageMap__NoOpSlotD8(void);
void StageMap__SetGridSpan(StageMap *self, s32 span);
void StageMap__SetConfig(StageMap *self, StageGridDimensions *config);
s32 StageMap__SetTargetAndLoadChunks(StageMap *self, void *outPos, SceneNode *target, Descriptor10 *cell);
s32 StageMap__ComputeCellOffsets(StageMap *self, void *outPos, void *cell);
s32 ComputeCellWorldOffsets(s32 *outPos, s32 *outBuf, StageGridDimensions *config, LongVec3 *origin,
                            Descriptor10 *cell);
void StageMap__Enable(StageMap *self);
void StageMap__Disable(StageMap *self);
s32 StageMap__UpdateFootprintTracking(StageMap *self);
void StageMap__LoadChunksAround(StageMap *self, s32 val, LongVec3 *pos, ChunkSlotSpec *specs);
s32 StageMap__ComputeNeighbourMask(StageMap *self, s32 val, s32 flag);
s32 StageMap__ComputeChunkLoadEntry(StageMap *self, ChunkLoadEntry *entry, s32 divisor, s32 flag,
                                    s32 val, s32 savedResult,
                                    s32 key); /* 0 or 1; LoadChunksAround discards it */
void StageMap__ApplyChunkLoads(StageMap *self, ChunkLoadEntry *entries, s32 count);
s32 StageMap__CountPendingLoads(StageMap *self);
void StageMap__OnNotifyTag1(StageMap *self, void *sender, s32 mode);
void StageMap__PopulateSlotCells(StageMap *self, ChunkSlot *slot);
void StageMap__ClearSlotCells(StageMap *self, ChunkSlot *slot);
Descriptor10 *StageMap__GetTargetDescriptor(StageMap *self, Descriptor10Ext *out, void **outPos);
s32 StageMap__ComputeFootprintDescriptor(StageMap *self, Descriptor10Ext *out, SplitLongVec3 *pos);
void StageMap__SplitChunkIndex(StageMap *self, u8 *out, s32 val);
ChunkSlot *StageMap__GetLastEventSlotChunk(StageMap *self, u8 *out);
ChunkSlot *StageMap__FindSlotByNeighbour(StageMap *self, s32 key);
ChunkSlot *StageMap__FindSlotForPosition(StageMap *self, LongVec3 *pos);
s32 StageMap__FindSlotIndexByNeighbour(StageMap *self, s32 key);
s32 StageMap__FindSlotIndexByChunk(StageMap *self, s32 chunkIndex);
void StageMap__RefreshFootprint(StageMap *self);
void StageMap__ComputeFootprintFromRotation(StageMap *self, s32 acrossCells, s32 aheadCells);
void StageMap__BuildFootprintRects(StageMap *self);
s32 StageMap__SplitFootprintRect(StageMap *self, CellRect *rect, s32 count, s32 key, s32 col,
                                 s32 row, s32 width, s32 height);
void StageMap__SetFootprintFromQuery(StageMap *self);
s32 IsPointOutOfBounds(CellBounds *bounds, s8 *point);
s32 StageMap__InitFootprintRect(StageMap *self, s32 unused, s32 index, s32 chunkIndex);
void StageMap__SetFootprintVisible(StageMap *self, s32 visible);
void *StageMap__GetUnk1CC(StageMap *self);
void StageMap__SetBounds(StageMap *self, CellBounds *bounds);
void StageMap__StartScaleRamp(StageMap *self, s32 rate, s32 fast);
void StageMap__StepScaleRamp(StageMap *self);
void StageMap__EndScaleRamp(StageMap *self);
void StageMap__AddScaleStepToCell(StageMap *self, struct GridCell *cell);
void StageMap__ResetCellScale(StageMap *self, struct GridCell *cell);
void StageMap__ForEachSlot(StageMap *self, StageMapCellFn cellFn, ChunkSlotFn slotFn);
void StageMap__ForEachSlotCell(StageMap *self, StageMapCellFn cellFn, ChunkSlot *slot);

#endif
