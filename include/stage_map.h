/**
 * @file stage_map.h
 * @brief StageMap, the loaded part of a stage's map (seven chunk slots laid
 *        out as lattices of GridCells around a tracked target), its method
 *        table, and the chunk, cell and footprint types it works in.
 *
 * The game makes one StageMap, at boot (DayTask__DayTask, via
 * New_StageMap(NULL, 1)), as the init args' light rig. ObjM configures it
 * for each stage and DreamSys__SpawnAtLink hands it the target; Actor keeps
 * it as `grid` (include/actor.h) when addChild sees a class-0x114 child, and
 * Entity calls its startScaleRamp.
 */
#ifndef STAGE_MAP_H
#define STAGE_MAP_H

#include "light_rig.h"
#include "stage_grid.h"

struct LbdFile;
struct PlacementGrid;
struct GridCell;

typedef struct StageMap StageMap;
typedef struct StageMapMethods StageMapMethods;
typedef struct ChunkSlot ChunkSlot;

/** StageMap's class id (gStageMapMethods word +0x000). Three nibbles, so
 * `(header & CLASS_ID_LEVEL3_MASK) == STAGEMAP_CLASS_ID` tests for it or a class below it
 * (ObjM__OnNotify). */
#define STAGEMAP_CLASS_ID 0x114

/** @name The grid's geometry, in world units
 * A cell is STAGE_CELL_SIZE square: ComputeFootprintDescriptor shifts a
 * position's offset from its slot's origin right by STAGE_CELL_SHIFT for the
 * cell column/row. A chunk is STAGE_CHUNK_CELLS cells square (the ctor's
 * 20 x 20 lattice, 0x800 apart), so STAGE_CHUNK_SIZE (0xA000, also
 * sDefaultGridSpan) wide; odd rows sit STAGE_CHUNK_SIZE / 2 to -x
 * (ComputeCellWorldOffsets).
 * @{ */
#define STAGE_CELL_SHIFT 11                                    /**< log2 of STAGE_CELL_SIZE */
#define STAGE_CELL_SIZE (1 << STAGE_CELL_SHIFT)                /**< a cell's side */
#define STAGE_CHUNK_CELLS 20                                   /**< a chunk's side, in cells */
#define STAGE_CHUNK_SIZE (STAGE_CHUNK_CELLS * STAGE_CELL_SIZE) /**< a chunk's side */
/** @} */

/** Half a chunk, in cells: the odd-row stagger, which is how far a cell
 * column moves when the drawn window crosses into the row above or below
 * (BuildFootprintRects, SplitFootprintRect). */
#define STAGE_CHUNK_HALF_CELLS (STAGE_CHUNK_CELLS / 2)

/** A slot's lattice of cells, row stride STAGE_CHUNK_CELLS. */
#define STAGE_SLOT_LATTICE_CELLS (STAGE_CHUNK_CELLS * STAGE_CHUNK_CELLS)

/** A slot's `cells`: the lattice, then 10 overflow cells PopulateSlotCells
 * hangs chained placements in; 410 in all (the ctor's BMemPMgrAlloc(0x668),
 * ClearSlotCells' walk). */
#define STAGE_SLOT_CELLS (STAGE_SLOT_LATTICE_CELLS + 10)

/** What StageMap passes its parents (notifyParents). The two slot events go
 * through notifyWithHull (StageMap__OnSlotEvent), which records the slot as
 * lastEventSlot before passing the command on. */
enum StageMapEvent {
    STAGEMAP_EVENT_CHUNK_CHANGED = 5,  /**< UpdateFootprintTracking: the target's chunk column/row
                                            changed */
    STAGEMAP_EVENT_SLOT_RELEASE = 6,   /**< before a slot is reloaded or its load cancelled
                                            (ApplyChunkLoads; UnloadAllSlots and Finalize);
                                            releases its heldObj */
    STAGEMAP_EVENT_SLOT_DATA_READY = 7 /**< the slot's LbdFile has read its data block
                                            (OnDrawSystemEvent, on `dataReady`) */
};

/**
 * @brief A grid-cell descriptor, 10 bytes, alignment 2: which chunk, which
 *        cell in it, and where in the cell.
 *
 * Every member is s8 or s16, so the struct is 2-byte aligned
 * (SetTargetAndLoadChunks copies it whole). UpdateFootprintTracking and
 * DispatchToRectCells read `chunk` as one u16.
 *
 * It has the shape of dream_sys.h's PlayerSpawnPoint (chunk col/row, tile
 * col/row, s16 x/y/z): DreamSys__WallLink copies getCurrentCellKey's result
 * into its linkCoordinates whole: both are a StageChunk, a StageCell and
 * three s16s.
 */
typedef struct Descriptor10 {
    StageChunk chunk; /**< the chunk (SplitChunkIndex: index % columns, index / columns) */
    StageCell cell;   /**< the cell inside the chunk */

    s16 x; /**< x offset from the cell's centre */
    s16 y; /**< the height: ComputeFootprintDescriptor stores the world y, ComputeCellWorldOffsets adds the grid origin's */
    s16 z; /**< z offset from the cell's centre */
} Descriptor10;

/** @brief A Descriptor10's first half: the chunk's column/row bytes as one
 *         u16, then the cell's. */
typedef struct CellKey {
    u16 chunk; /**< Descriptor10::chunk: column, row */
    u16 cell;  /**< Descriptor10::cell: column, row */
} CellKey;

/** @brief A Descriptor10's second half: the s16 x/y/z offset inside the
 *         cell, with x and y grouped as one struct so a copy takes the pair
 *         whole, then z. */
typedef struct CellOffset {
    /** @brief x and y, copied together. */
    struct {
        s16 x; /**< x offset from the cell's centre */
        s16 y; /**< y */
    } xy;      /**< x and y */

    s16 z; /**< z offset from the cell's centre */
} CellOffset;

/** @brief A Descriptor10 as dream_aux.c and the style layer build it for
 *         computeCellOffsets, from two halves each copied whole. */
typedef struct CellKeyDesc {
    CellKey key;       /**< the chunk and the cell */
    CellOffset offset; /**< the offset inside the cell */
} CellKeyDesc;

/** @brief computeFootprintDescriptor's output, 0x2C bytes: a Descriptor10
 *         and the slot and chunk behind it. `targetCell` holds one. */
typedef struct Descriptor10Ext {
    Descriptor10 base;    /**< +0x000: the cell descriptor */
    LongVec3 chunkCentre; /**< +0x00C: the slot's cellParent position + half a chunk in x and z */
    LongVec3 relPos; /**< +0x018: the queried position less chunkCentre in x and z; y as queried */
    ChunkSlot *slot; /**< +0x024: the slot FindSlotForPosition resolved */
    s32 chunkIndex;  /**< +0x028: that slot's LbdFile::chunkIndex, sign-extended */
} Descriptor10Ext;

/** @brief computeFootprintDescriptor's world position. Each word is read
 *         whole (for the cell) and, later, as its low halfword (for the
 *         offset inside it), so each is a union of the two widths. */
typedef struct SplitLongVec3 {
    /** @brief x, whole or as its low halfword. */
    union {
        s32 w; /**< the whole word */
        u16 h; /**< its low halfword */
    } x;       /**< +0x000 */

    /** @brief y, whole or as its low halfword. */
    union {
        s32 w; /**< the whole word */
        u16 h; /**< its low halfword */
    } y;       /**< +0x004 */

    /** @brief z, whole or as its low halfword. */
    union {
        s32 w; /**< the whole word */
        u16 h; /**< its low halfword */
    } z;       /**< +0x008 */
} SplitLongVec3;

typedef struct SplitCoord2 SplitCoord2;

/**
 * @brief A chunk slot's origin: its cellParent->coord2, a GsCOORDINATE2
 *        (Sony's, include/psyq/libgs.h), with the translation's x and z
 *        readable as the whole word or its low halfword, as SplitLongVec3's
 *        are.
 *
 * StageMap__LoadChunksAround writes all three words;
 * StageMap__ComputeFootprintDescriptor reads tx/tz whole for the cell and as
 * halfwords for the offset inside it.
 */
struct SplitCoord2 {
    u8 pad00[0x018]; /* +0x000, flg and coord.m */

    /** @brief coord.t[0], whole or as its low halfword. */
    union {
        s32 w; /**< the whole word */
        u16 h; /**< its low halfword */
    } tx;      /**< +0x018, coord.t[0] */

    s32 ty; /**< +0x01C, coord.t[1] */

    /** @brief coord.t[2], whole or as its low halfword. */
    union {
        s32 w; /**< the whole word */
        u16 h; /**< its low halfword */
    } tz;      /**< +0x020, coord.t[2] */
};

/** @brief The step from a centre chunk's index (row * columns + column) to
 *         one of the seven chunks around it, indexed by neighbour key
 *         (sChunkNeighbourDeltas, ComputeChunkLoadEntry). The table holds the
 *         centre (key 3, all 0) and its six staggered neighbours. */
typedef struct ChunkNeighbourDelta {
    s32 rowDelta;        /**< +0x0: rows to step */
    s32 colDeltaOddRow;  /**< +0x4: columns to step from an odd centre row (or with no row step) */
    s32 colDeltaEvenRow; /**< +0x8: columns to step from an even centre row */
} ChunkNeighbourDelta;

/** The neighbour keys (ChunkSlotSpec::neighbour, ChunkSlot::neighbour,
 * LbdFile::elemKey): the centre chunk and the six around it, as
 * sChunkNeighbourDeltas steps to them (rows, then columns for an odd / even
 * centre row) and sNeighbourOffsets places them (a row is +z, a column +x).
 * Each adjacent row touches two chunks, the lower and the higher column. A
 * vertical grid uses keys 0 .. rows - 1 as its stacked layers instead
 * (ComputeNeighbourMask, FindSlotForPosition). */
enum ChunkNeighbour {
    CHUNK_NEIGHBOUR_PREV_ROW_LO = 0, /**< row -1; column -1 (odd row) or 0 (even) */
    CHUNK_NEIGHBOUR_PREV_ROW_HI = 1, /**< row -1; column 0 (odd row) or +1 (even) */
    CHUNK_NEIGHBOUR_PREV_COL = 2,    /**< column -1 */
    CHUNK_NEIGHBOUR_CENTRE = 3,      /**< the centre chunk */
    CHUNK_NEIGHBOUR_NEXT_COL = 4,    /**< column +1 */
    CHUNK_NEIGHBOUR_NEXT_ROW_LO = 5, /**< row +1; column -1 (odd row) or 0 (even) */
    CHUNK_NEIGHBOUR_NEXT_ROW_HI = 6, /**< row +1; column 0 (odd row) or +1 (even) */
    CHUNK_NEIGHBOUR_COUNT = 7        /**< the number of keys, and of slots */
};

/** A key's bit, as sNeighbourBits holds it; ComputeNeighbourMask returns a
 * mask of the keys whose chunk lies on the grid. */
#define CHUNK_NEIGHBOUR_BIT(key) (1 << (key))

/** @brief applyChunkLoads' 0xC-byte entries, one per slot to (re)load.
 *         ComputeChunkLoadEntry writes and reads chunkIndex as a whole word;
 *         ApplyChunkLoads reads its low half. */
typedef struct ChunkLoadEntry {
    void *file; /**< +0x0: the chunk's file record from chunkFileFn; NULL cancels the slot's load */

    /** @brief The chunk's index in the stage grid, whole or as its low half. */
    union {
        s32 word;  /**< the whole word */
        s16 index; /**< its low half */
    } chunkIndex;  /**< +0x4 */

    s32 neighbour; /**< +0x8: the neighbour key of the slot that takes it */
} ChunkLoadEntry;

/** @brief A ChunkLoadEntry seen from its +0x4, with the same 0xC stride:
 *         ApplyChunkLoads' second walker over the entries. */
typedef struct ChunkLoadEntryTail {
    s16 chunkIndex; /**< ChunkLoadEntry +0x4: the chunk index's low half */
    u8 pad2[0x4 - 0x2];
    s32 neighbour; /**< ChunkLoadEntry +0x8 */
    u8 pad8[0xC - 0x8];
} ChunkLoadEntryTail;

/** @brief loadChunksAround's per-slot pair; sDefaultTargetSpecs and the
 *         sFootprintResultPtrTable tables hold seven each. */
typedef struct ChunkSlotSpec {
    u8 neighbour; /**< +0x0: the neighbour key the slot takes (0..6; 3 is the centre) */
    u8 load;      /**< +0x1: non-zero to (re)load and reposition the slot */
} ChunkSlotSpec;

/** @brief One rectangle of cells inside one slot's lattice, 0xC bytes, no
 *         padding. */
typedef struct CellRect {
    s32 slotIndex; /**< +0x0: index into slots[] */
    s16 col;       /**< +0x4: starting column */
    s16 row;       /**< +0x6: starting row (row stride 20) */
    s16 width;     /**< +0x8: width in cells */
    s16 height;    /**< +0xA: height in cells */
} CellRect;

/** @brief `rects` as a whole, so ApplyToSenderFootprint can save and restore
 *         it with a plain `=`. */
typedef struct CellRectSet {
    CellRect e[4]; /**< the rectangles; StageMap::rectCount says how many are live */
} CellRectSet;

/** @brief `bounds`' pointee (setBounds): IsPointOutOfBounds' box of cell
 *         columns/rows inside a chunk, inclusive. */
typedef struct CellBounds {
    s16 minCol; /**< +0x000: the lowest column inside */
    s16 minRow; /**< +0x002: the lowest row inside */
    s32 maxCol; /**< +0x004: the highest column inside */
    s32 maxRow; /**< +0x008: the highest row inside */
} CellBounds;

/** @brief One of StageMap's seven chunk slots, 0x1C bytes (the ctor,
 *         Finalize): the chunk loaded there and the cells that draw it. */
struct ChunkSlot {
    /* +0x000 */ u16 loadPending; /**< 1 while its load is pending (ApplyChunkLoads; OnDrawSystemEvent clears it) */
    /* +0x002 */ u16 neighbour; /**< the ctor: its index; LoadChunksAround: the spec's neighbour key, copied on into loader->elemKey */
    /* +0x004 */ struct LbdFile *loader; /**< New_LbdFile(): the slot's chunk file (include/lbd_file.h) */
    /* +0x008 */ struct PlacementGrid *placements; /**< New_PlacementGrid(0): its placement records (include/placement_grid.h) */
    /* +0x00C */ struct GridCell *cellParent; /**< New_GridCell(), attached to the StageMap at `origin`; every cell's parent */
    /* +0x010 */ struct GridCell **cells; /**< STAGE_SLOT_CELLS New_GridCell() cells, row stride 20 */
    /* +0x014 */ BasicClass *heldObj; /**< zeroed by the ctor; OnSlotEvent releases it on SLOT_RELEASE */
    /* +0x018 */ s32 unused18;        /**< zeroed by the ctor */
};

/** The ctor's callback pair (setCallback): ComputeChunkLoadEntry calls
 * chunkFileFn(chunkFileCtx, chunk, 0, 0) and keeps the result as the entry's
 * file record. ObjM__GetGridRecord, the one callback, takes the chunk by x/y
 * when `chunk` is negative. */
typedef void *(*ChunkFileFn)(void *ctx, s32 chunk, s32 x, s32 y);

/**
 * @brief StageMap's method table: LightRig's slots, then StageMap's own.
 *
 * Inherited slots it overrides: +0x008 the ctor, +0x00C Finalize, +0x038
 * OnNotify, +0x040 Reset, +0x088 notifyWithHull (StageMap__OnSlotEvent),
 * +0x098 update (StageMap__UpdateIfEnabled) and +0x09C DispatchLinkCommand.
 * getLight and setAmbientColor are LightRig's, unchanged.
 *
 * +0x088 keeps SceneNode's (self, event); StageMap__OnSlotEvent takes
 * (self, command, slot), and its four callers (Finalize, UnloadAllSlots,
 * ApplyChunkLoads, OnDrawSystemEvent) pass (self, 6 or 7, slot, index)
 * through StageMapOnSlotEventFn. +0x0D4 getCurrentCellKey and +0x0F8
 * loadChunksAround keep their CALLERS' shapes: DreamSys__WallLink passes
 * getCurrentCellKey a second word the occupant never reads, and
 * SetTargetAndLoadChunks returns loadChunksAround's value although the
 * occupant returns nothing. 80 slots.
 */
struct StageMapMethods {
    LIGHTRIG_SLOTS(StageMap, (StageMap * self, LongVec3 *origin, s32 autoLoad));
    /* +0x0C0 */ void (*unloadAllSlots)(StageMap *self); /**< @see StageMap__UnloadAllSlots */
    /* +0x0C4 */ void (*setChildParams)(StageMap *self, s32 count, s32 dirs,
                                        s32 colors); /**< @see StageMap__SetChildParams */
    /* +0x0C8 */ void (*setCallback)(StageMap *self, ChunkFileFn fn, void *ctx); /**< @see StageMap__SetCallback */
    /* +0x0CC */ void (*setAcceptedTags)(StageMap *self, s32 *tags); /**< @see StageMap__SetAcceptedTags */
    /* +0x0D0 */ void (*forwardAcceptedCommand)(StageMap *self, void *sender,
                                                s32 command); /**< @see StageMap__ForwardAcceptedCommand */
    /* +0x0D4 */ Descriptor10 *(*getCurrentCellKey)(StageMap *self, void *sender); /**< @see StageMap__GetCurrentCellKey (reads only self) */
    /* +0x0D8 */ void (*slotD8)(void); /**< @see StageMap__NoOpSlotD8 (empty; never called) */
    /* +0x0DC */ void (*setGridSpan)(StageMap *self, s32 span); /**< @see StageMap__SetGridSpan */
    /* +0x0E0 */ void (*setConfig)(StageMap *self, StageGridDimensions *config); /**< @see StageMap__SetConfig */
    /* +0x0E4 */ s32 (*setTargetAndLoadChunks)(StageMap *self, void *outPos, SceneNode *target,
                                               Descriptor10 *cell); /**< @see StageMap__SetTargetAndLoadChunks */
    /* +0x0E8 */ s32 (*computeCellOffsets)(StageMap *self, void *outPos,
                                           void *cell); /**< @see StageMap__ComputeCellOffsets */
    /* +0x0EC */ void (*enable)(StageMap *self);        /**< @see StageMap__Enable */
    /* +0x0F0 */ void (*disable)(StageMap *self);       /**< @see StageMap__Disable */
    /* +0x0F4 */ s32 (*updateFootprintTracking)(StageMap *self); /**< @see StageMap__UpdateFootprintTracking */
    /* +0x0F8 */ s32 (*loadChunksAround)(StageMap *self, s32 val, LongVec3 *pos,
                                         ChunkSlotSpec *specs); /**< @see StageMap__LoadChunksAround (returns nothing) */
    /* +0x0FC */ void (*applyChunkLoads)(StageMap *self, ChunkLoadEntry *entries,
                                         s32 count); /**< @see StageMap__ApplyChunkLoads */
    /* +0x100 */ void (*onDrawSystemEvent)(StageMap *self, void *sender,
                                           s32 mode); /**< @see StageMap__OnDrawSystemEvent */
    /* +0x104 */ void (*populateSlotCells)(StageMap *self, ChunkSlot *slot); /**< @see StageMap__PopulateSlotCells */
    /* +0x108 */ void (*clearSlotCells)(StageMap *self, ChunkSlot *slot); /**< @see StageMap__ClearSlotCells */
    /* +0x10C */ Descriptor10 *(*getTargetDescriptor)(StageMap *self, Descriptor10Ext *out,
                                                      void **outPos); /**< @see StageMap__GetTargetDescriptor */
    /* +0x110 */ s32 (*computeFootprintDescriptor)(StageMap *self, Descriptor10Ext *out,
                                                   SplitLongVec3 *pos); /**< @see StageMap__ComputeFootprintDescriptor */
    /* +0x114 */ ChunkSlot *(*getLastEventSlotChunk)(StageMap *self, StageChunk *out); /**< @see StageMap__GetLastEventSlotChunk */
    /* +0x118 */ ChunkSlot *(*findSlotByNeighbour)(StageMap *self, s32 key); /**< @see StageMap__FindSlotByNeighbour */
    /* +0x11C */ ChunkSlot *(*findSlotForPosition)(StageMap *self, LongVec3 *pos); /**< @see StageMap__FindSlotForPosition */
    /* +0x120 */ s32 (*findSlotIndexByNeighbour)(StageMap *self, s32 key); /**< @see StageMap__FindSlotIndexByNeighbour */
    /* +0x124 */ s32 (*findSlotIndexByChunk)(StageMap *self, s32 chunkIndex); /**< @see StageMap__FindSlotIndexByChunk */
    /* +0x128 */ void (*refreshFootprint)(StageMap *self); /**< @see StageMap__RefreshFootprint */
    /* +0x12C */ void (*applyToSenderFootprint)(StageMap *self, SceneNode *sender,
                                                s32 command); /**< @see StageMap__ApplyToSenderFootprint */
    /* +0x130 */ void *(*getUnused1CC)(StageMap *self); /**< @see StageMap__GetUnused1CC */
    /* +0x134 */ void (*setBounds)(StageMap *self, CellBounds *bounds); /**< @see StageMap__SetBounds */
    /* +0x138 */ void (*startScaleRamp)(StageMap *self, s32 rate, s32 fast); /**< @see StageMap__StartScaleRamp */
    /* +0x13C */ void (*stepScaleRamp)(StageMap *self); /**< @see StageMap__StepScaleRamp */
    /* +0x140 */ void (*endScaleRamp)(StageMap *self);  /**< @see StageMap__EndScaleRamp */
};

/**
 * @brief StageMap: the part of a stage's map that is loaded, seven map
 *        chunks around a tracked target, each laid out as a lattice of
 *        GridCells.
 *
 * Class id 0x114 (STAGEMAP_CLASS_ID), table gStageMapMethods, parent LightRig
 * (include/light_rig.h; its ctor and finalize chain to LightRig's first); no
 * class below it. Methods in src/world/stage_map.c, New_StageMap through
 * GetStageMapMethods. The object is 0x1E8 bytes (New_StageMap).
 *
 * Lifecycle. ObjM configures it for its stage: setConfig with the stage's
 * StageGridDimensions (GetStageGridDimensions), setCallback with
 * ObjM__GetGridRecord (a chunk index to that chunk's file record,
 * GetStageMapChunkRecord), setGridSpan, setAcceptedTags, setChildParams,
 * setBounds, and enables it. DreamSys__SpawnAtLink hands it the target
 * (setTargetAndLoadChunks). Disable unloads every slot; Finalize releases
 * them.
 *
 * The chunk grid. A stage is `columns` x `rows` chunks
 * (StageGridDimensions), a chunk STAGE_CHUNK_SIZE units square, numbered
 * row * columns + column; odd rows sit half a chunk to -x
 * (ComputeCellWorldOffsets), so a chunk has six neighbours. A vertical grid
 * (isVertical) stacks its chunks in y instead.
 *
 * The seven slots (`slots`, ChunkSlot). The ctor gives each an LbdFile
 * `loader`, a PlacementGrid `placements`, a GridCell `cellParent` attached to
 * this object at `origin`, and STAGE_SLOT_CELLS GridCell `cells` attached to
 * the cellParent a cell apart (20 x 20, then 10 cells for chained
 * placements). Each slot holds a neighbour key (enum ChunkNeighbour) that
 * says where around the centre chunk it sits:
 *  - loadChunksAround takes a centre chunk index and position and a
 *    ChunkSlotSpec per slot; each slot marked `load` moves its cellParent to
 *    centre + sNeighbourOffsets[neighbour] and gets a ChunkLoadEntry
 *    (ComputeChunkLoadEntry: the neighbour's chunk index from
 *    sChunkNeighbourDeltas, NULL when ComputeNeighbourMask puts it off the
 *    grid, else the chunk's file record from the callback);
 *  - applyChunkLoads starts each entry's LbdFile load (or cancels it);
 *    onDrawSystemEvent, as the reads finish, links each chunk's placements
 *    and models into its slot's cells (populateSlotCells), and sets
 *    `chunksLoaded` once none is pending.
 *
 * Tracking. Every tick while `enabled`, updateFootprintTracking turns the
 * target's position into a Descriptor10Ext (computeFootprintDescriptor:
 * chunk column/row, cell column/row, offset in the cell), keeps it in
 * `targetCell`, and notifies parents with STAGEMAP_EVENT_CHUNK_CHANGED when
 * the chunk changes. In a flat grid it also re-centres: the slot holding the
 * target picks a spec through sFootprintResultRemap and
 * sFootprintResultPtrTable (NULL, no change, for the centre slot) and
 * loadChunksAround reloads the slots that spec marks around the target's
 * chunk. Then refreshFootprint moves the window of cells that is drawn: it
 * sets GsDOFF on the cells of the old `rects`, rebuilds `rects`, the up to
 * four cell rectangles around the target, and clears it on theirs
 * (SetFootprintVisible); every other cell stays hidden (the ctor and
 * ClearSlotCells set the bit). forwardAcceptedCommand and
 * applyToSenderFootprint hand a sender's command to every cell under its
 * footprint (DispatchToRectCells, NotifyGridCell).
 *
 * Scale ramp. startScaleRamp (called by Entity's mood cues) picks a
 * Ratio16[3] step, y +-1/64 or +-1/4, and a tick count; stepScaleRamp (every
 * tick after tracking) adds it to every cell's scale until the count runs
 * out, and endScaleRamp sets every cell back to 1/1.
 */
struct StageMap {
    LIGHTRIG_FIELDS(StageMapMethods);
    /* +0x054 */ LongVec3 origin; /**< the ctor: its argument, or sDefaultOrigin; the cellParents attach here */
    /* +0x060 */ ChunkFileFn chunkFileFn; /**< setCallback: chunk index to file record */
    /* +0x064 */ void *chunkFileCtx;      /**< setCallback: chunkFileFn's first argument */
    /* +0x068 */ StageGridDimensions *config; /**< setConfig (ObjM: GetStageGridDimensions(stage)); NULL after Reset */
    /* +0x06C */ SceneNode *target; /**< setTargetAndLoadChunks (DreamSys__SpawnAtLink passes the DreamSys); its coord2 is the tracked position */
    /* +0x070 */ s32 enabled;       /**< enable/disable; gates UpdateIfEnabled */
    /* +0x074 */ s32 gridSpan; /**< setGridSpan: sDefaultGridSpan = 0xA000; the facing vector's length in ComputeFootprintFromRotation */
    /* +0x078 */ s16 gridHalfCells; /**< gridSpan >> 12 = 10 */
    /* +0x07A */ s16 gridCells;     /**< gridSpan >> 11 = 20, the row stride */
    /* +0x07C */ s16 footprintCol; /**< the drawn window's first column, relative to the centre chunk (may be negative) */
    /* +0x07E */ s16 footprintRow;    /**< the drawn window's first row, likewise */
    /* +0x080 */ s32 footprintWidth;  /**< the window's width in cells */
    /* +0x084 */ s32 footprintHeight; /**< the window's height in cells */
    /* +0x088 */ s32 rectCount;       /**< how many of rects[] are live */
    /* +0x08C */ CellRectSet rects; /**< the window, split per slot: BuildFootprintRects, SetFootprintRect, InitFootprintRect write; DispatchToRectCells, SetFootprintVisible walk */
    /* +0x0BC */ Descriptor10Ext targetCell; /**< UpdateFootprintTracking: the target's last descriptor; SetTargetAndLoadChunks sets .base; getTargetDescriptor returns &.base */
    /* +0x0E8 */ s32 *acceptedTags; /**< setAcceptedTags: a 0-terminated list of class ids ForwardAcceptedCommand accepts */
    /* +0x0EC */ ChunkSlot slots[7]; /**< the chunk slots, one per neighbour key */
    /* +0x1B0 */ s32 loadsPending; /**< 1 while chunk loads are pending (ApplyChunkLoads; OnDrawSystemEvent clears it) */
    /* +0x1B4 */ u16 pendingLoadCount; /**< CountPendingLoads after ApplyChunkLoads; OnDrawSystemEvent counts it down */
    /* +0x1B6 */ u8 pad1B6[0x1B8 - 0x1B6];
    /* +0x1B8 */ s32 chunksLoaded; /**< set when that count reaches 0; RefreshFootprint does nothing while it is 0 */
    /* +0x1BC */ ChunkSlot *lastEventSlot; /**< OnSlotEvent's slot; GetLastEventSlotChunk reads it */
    /* +0x1C0 */ Descriptor10 curCell; /**< DispatchToRectCells: the cell being notified; getCurrentCellKey returns it */
    /* +0x1CA */ u8 pad1CA[0x1CC - 0x1CA];
    /* +0x1CC */ s32 unused1CC; /**< Reset: -1; GetUnused1CC (never called) returns its address */
    /* +0x1D0 */ s32 unused1D0; /**< Reset: -1 */
    /* +0x1D4 */ s32 unused1D4; /**< Reset: -1 */
    /* +0x1D8 */ s32 unused1D8; /**< Reset: -1 */
    /* +0x1DC */ CellBounds *bounds; /**< setBounds; IsPointOutOfBounds */
    /* +0x1E0 */ s32 scaleRampTicks; /**< startScaleRamp: |rate| * the step's y den; stepScaleRamp counts it down (-1 when done), endScaleRamp zeroes it */
    /* +0x1E4 */ Ratio16 *scaleStep; /**< startScaleRamp: one of four Ratio16[3] steps (x, y, z) stepScaleRamp adds to every cell's scale; only y is nonzero */
};

/** notifyWithHull's occupant, StageMap__OnSlotEvent, called with the slot
 * (and its index, which it does not read). */
typedef void (*StageMapOnSlotEventFn)(StageMap *self, s32 command, ChunkSlot *slot, s32 index);

/** ForEachSlot's per-cell callback. */
typedef void (*StageMapCellFn)(StageMap *self, struct GridCell *cell);

/** ForEachSlot's per-slot callback. */
typedef void (*ChunkSlotFn)(StageMap *self, ChunkSlot *slot);

/** StageMap's method table (class id 0x114). */
extern StageMapMethods gStageMapMethods;

/**
 * @brief The StageMap method table.
 * @return &gStageMapMethods.
 */
extern StageMapMethods *GetStageMapMethods(void);

/**
 * @brief Allocates a StageMap (0x1E8 bytes) and runs its ctor through the
 *        table.
 * @param origin Where the grid is centred, or NULL for sDefaultOrigin.
 * @param autoLoad Handed to each slot's LbdFile (setAutoLoadData).
 * @return The new StageMap, or NULL when the allocation fails.
 */
StageMap *New_StageMap(LongVec3 *origin, s32 autoLoad);

/**
 * @brief Constructor (slot +0x008): LightRig's ctor, then builds the seven
 *        slots (an LbdFile, a PlacementGrid, a cellParent attached at
 *        `origin`, and STAGE_SLOT_CELLS cells, hidden and fogged, laid out a
 *        cell apart from the cellParent), adds the DrawSystem as a child and
 *        resets. A failed cell-array allocation returns at once.
 * @param self The map.
 * @param origin Where the grid is centred, or NULL for sDefaultOrigin.
 * @param autoLoad Handed to each slot's LbdFile (setAutoLoadData).
 */
void StageMap__StageMap(StageMap *self, LongVec3 *origin, s32 autoLoad);

/**
 * @brief finalize (slot +0x00C): drops the DrawSystem child, sends each slot
 *        a SLOT_RELEASE and releases its loader, placements (and their
 *        LinkResource), cellParent and cells, then LightRig's finalize.
 * @param self The map.
 */
void StageMap__Finalize(StageMap *self);

/**
 * @brief onNotify (slot +0x038): SceneNode's onNotify, then passes a
 *        DrawSystem sender's notification to onDrawSystemEvent.
 * @param self The map.
 * @param sender The notifying object.
 * @param command Its event code.
 */
void StageMap__OnNotify(StageMap *self, BasicClass *sender, s32 command);

/**
 * @brief reset (slot +0x040): forgets the config and accepted tags, empties
 *        the drawn window, restores the default grid span and sets
 *        unused1CC..unused1D8 to -1.
 * @param self The map.
 */
void StageMap__Reset(StageMap *self);

/**
 * @brief notifyWithHull (slot +0x088): SceneNode's notifyWithHull, then for a
 *        slot event records the slot as lastEventSlot and passes the command
 *        to the parents; STAGEMAP_EVENT_SLOT_RELEASE first releases the
 *        slot's heldObj. Other commands stop after SceneNode's.
 * @param self The map.
 * @param command STAGEMAP_EVENT_SLOT_RELEASE or STAGEMAP_EVENT_SLOT_DATA_READY.
 * @param slot The slot the event is about.
 */
void StageMap__OnSlotEvent(StageMap *self, s32 command, ChunkSlot *slot);

/**
 * @brief update (slot +0x098): while enabled, tracks the target
 *        (updateFootprintTracking) and steps the scale ramp.
 * @param self The map.
 */
void StageMap__UpdateIfEnabled(StageMap *self);

/**
 * @brief Link-command slot +0x09C: passes an Actor sender's command to
 *        forwardAcceptedCommand; other senders are ignored.
 * @param self The map.
 * @param sender The object sending the command.
 * @param command The command.
 */
void StageMap__DispatchLinkCommand(StageMap *self, BasicClass *sender, s32 command);

/**
 * @brief unloadAllSlots (slot +0x0C0): cancels every slot's load, clears its
 *        cells, releases its placements' LinkResource, sends it a
 *        SLOT_RELEASE and releases its loader's data block; then zeroes
 *        chunksLoaded and pendingLoadCount and ends the scale ramp.
 * @param self The map.
 */
void StageMap__UnloadAllSlots(StageMap *self);

/**
 * @brief setChildParams (slot +0x0C4): sets the colour and direction of the
 *        first `count` of the inherited lights.
 * @param self The map.
 * @param count How many lights to set.
 * @param dirs The address of `count` directions, three s16 each.
 * @param colors The address of `count` ColorRgb colours.
 */
void StageMap__SetChildParams(StageMap *self, s32 count, s32 dirs, s32 colors);

/**
 * @brief setCallback (slot +0x0C8): the function that turns a chunk index
 *        into its file record, and its context.
 * @param self The map.
 * @param fn The callback (ObjM__GetGridRecord).
 * @param ctx Its first argument.
 */
void StageMap__SetCallback(StageMap *self, ChunkFileFn fn, void *ctx);

/**
 * @brief setAcceptedTags (slot +0x0CC): the class ids whose commands reach the
 *        cells.
 * @param self The map.
 * @param tags A 0-terminated list of class-id words, or NULL for none.
 */
void StageMap__SetAcceptedTags(StageMap *self, s32 *tags);

/**
 * @brief forwardAcceptedCommand (slot +0x0D0): for a hull or Actor-move
 *        command, calls applyToSenderFootprint once for each accepted tag
 *        that equals the sender's class-id word. Other commands, and a map
 *        with no tags, do nothing.
 * @param self The map.
 * @param sender The object sending the command.
 * @param command The command.
 */
void StageMap__ForwardAcceptedCommand(StageMap *self, void *sender, s32 command);

/**
 * @brief applyToSenderFootprint (slot +0x12C): hands a command to every cell
 *        in the 3 x 3 cells around the sender's world position, then restores
 *        the drawn window. Nothing happens when no slot holds the position.
 * @param self The map.
 * @param sender The sender; its world translation is the position.
 * @param command The command to hand on.
 */
void StageMap__ApplyToSenderFootprint(StageMap *self, SceneNode *sender, s32 command);

/**
 * @brief The flat grid's command footprint: a span x span window whose first
 *        cell is one column and one row before the descriptor's, split into
 *        `rects` by BuildFootprintRects.
 * @param self The map.
 * @param desc The centre cell.
 * @param span The window's side in cells.
 */
void StageMap__SetFootprintFromCell(StageMap *self, Descriptor10Ext *desc, s32 span);

/**
 * @brief The vertical grid's command footprint: one rectangle, in the slot
 *        holding the descriptor's chunk, of span x span cells centred on its
 *        cell and clipped at the lattice's edges.
 * @param self The map.
 * @param desc The centre cell.
 * @param span The rectangle's side in cells before clipping.
 */
void StageMap__SetFootprintRect(StageMap *self, Descriptor10Ext *desc, s32 span);

/**
 * @brief Hands a command to every cell of every rectangle in `rects` whose
 *        slot has its header read, and to every cell chained behind each,
 *        with the cell's key in curCell while it is notified.
 * @param self The map.
 * @param sender The command's sender.
 * @param command The command.
 */
void StageMap__DispatchToRectCells(StageMap *self, SceneNode *sender, s32 command);

/**
 * @brief Hands a command to a cell that takes commands
 *        (GRIDCELL_FLAG_TAKES_COMMANDS), through its onNotify.
 * @param cell The cell, or NULL.
 * @param sender The command's sender.
 * @param command The command.
 */
void NotifyGridCell(struct GridCell *cell, SceneNode *sender, s32 command);

/**
 * @brief getCurrentCellKey (slot +0x0D4): the key of the cell being notified.
 * @param self The map.
 * @return &curCell.
 */
Descriptor10 *StageMap__GetCurrentCellKey(StageMap *self);

/**
 * @brief Slot +0x0D8: empty; nothing calls it.
 */
void StageMap__NoOpSlotD8(void);

/**
 * @brief setGridSpan (slot +0x0DC): the grid span, and from it gridCells
 *        (span >> 11) and gridHalfCells (span >> 12).
 * @param self The map.
 * @param span The span in world units.
 */
void StageMap__SetGridSpan(StageMap *self, s32 span);

/**
 * @brief setConfig (slot +0x0E0): resets the map, then takes the stage's grid
 *        dimensions.
 * @param self The map.
 * @param config The stage's StageGridDimensions.
 */
void StageMap__SetConfig(StageMap *self, StageGridDimensions *config);

/**
 * @brief setTargetAndLoadChunks (slot +0x0E4): tracks a new target from a
 *        cell, writes the cell's world position and loads all seven slots
 *        around the cell's chunk (sDefaultTargetSpecs).
 * @param self The map.
 * @param outPos Receives the cell's world position (three s32).
 * @param target The node to track.
 * @param cell Its starting cell; copied into targetCell.
 * @return What the loadChunksAround slot returns; its occupant returns
 *         nothing.
 */
s32 StageMap__SetTargetAndLoadChunks(StageMap *self, void *outPos, SceneNode *target, Descriptor10 *cell);

/**
 * @brief computeCellOffsets (slot +0x0E8): a cell descriptor's world
 *        position, by the map's config and origin (ComputeCellWorldOffsets).
 * @param self The map.
 * @param outPos Receives the world position (three s32).
 * @param cell The cell descriptor (a Descriptor10 or CellKeyDesc).
 * @return The cell's chunk index (0 in a vertical grid).
 */
s32 StageMap__ComputeCellOffsets(StageMap *self, void *outPos, void *cell);

/**
 * @brief A cell descriptor to world positions: the chunk's centre, and the
 *        cell point (the cell's centre plus the descriptor's offset). The
 *        grid is centred on `origin`; odd rows sit half a chunk to -x.
 * @param outPos Receives the cell point (three s32).
 * @param chunkPos Receives the chunk's centre (three s32).
 * @param dims The stage's grid dimensions.
 * @param origin The grid's centre.
 * @param desc The cell descriptor.
 * @return The chunk's index, row * columns + column (0 in a vertical grid).
 */
s32 ComputeCellWorldOffsets(s32 *outPos, s32 *chunkPos, StageGridDimensions *dims, LongVec3 *origin,
                            Descriptor10 *desc);

/**
 * @brief enable (slot +0x0EC): starts the per-tick tracking.
 * @param self The map.
 */
void StageMap__Enable(StageMap *self);

/**
 * @brief disable (slot +0x0F0): unloads every slot and stops the tracking.
 * @param self The map.
 */
void StageMap__Disable(StageMap *self);

/**
 * @brief updateFootprintTracking (slot +0x0F4): re-reads the target's
 *        descriptor; in a flat grid reloads the slots the target's slot
 *        selects (sFootprintResultRemap) around its chunk; moves the drawn
 *        window; and notifies the parents with STAGEMAP_EVENT_CHUNK_CHANGED
 *        when the chunk changed.
 * @param self The map.
 * @return The selected spec's index: 0 for the centre slot (nothing
 *         reloads), or when no slot holds the target.
 */
s32 StageMap__UpdateFootprintTracking(StageMap *self);

/**
 * @brief loadChunksAround (slot +0x0F8): gives every slot the neighbour key
 *        its spec names; moves each slot the spec marks for loading to the
 *        centre position plus that key's offset (in a vertical grid, to its
 *        layer) and builds its ChunkLoadEntry; then starts the loads
 *        (applyChunkLoads). NULL specs do nothing.
 * @param self The map.
 * @param centreChunk The centre chunk's index.
 * @param centrePos The centre chunk's centre.
 * @param specs Seven ChunkSlotSpecs, one per slot, or NULL.
 */
void StageMap__LoadChunksAround(StageMap *self, s32 centreChunk, LongVec3 *centrePos,
                                ChunkSlotSpec *specs);

/**
 * @brief Which neighbour keys of a chunk lie on the grid.
 * @param self The map.
 * @param chunk The centre chunk's index.
 * @param oddRow Non-zero when the chunk's row is odd.
 * @return A mask of CHUNK_NEIGHBOUR_BIT(key) for each key on the grid; in a
 *         vertical grid, the bits of the keys 0 .. rows - 1.
 */
s32 StageMap__ComputeNeighbourMask(StageMap *self, s32 chunk, s32 oddRow);

/**
 * @brief Fills the load entry for the slot taking one neighbour of a centre
 *        chunk: the neighbour's chunk index and its file record from the
 *        callback, or a NULL file when the neighbour is off the grid.
 * @param self The map.
 * @param out The entry to fill.
 * @param columns The grid's columns.
 * @param oddRow Non-zero when the centre chunk's row is odd.
 * @param centreChunk The centre chunk's index.
 * @param onGridMask ComputeNeighbourMask's result.
 * @param neighbour The neighbour key.
 * @return 1 for a file, 0 for none; LoadChunksAround discards it.
 */
s32 StageMap__ComputeChunkLoadEntry(StageMap *self, ChunkLoadEntry *out, s32 columns, s32 oddRow,
                                    s32 centreChunk, s32 onGridMask, s32 neighbour);

/**
 * @brief applyChunkLoads (slot +0x0FC): for each entry, sends its slot a
 *        SLOT_RELEASE, clears a chunk already linked there, then starts the
 *        entry's LbdFile header load (marking the slot and the map pending)
 *        or, for a NULL file, cancels the slot's load; then counts the slots
 *        left pending.
 * @param self The map.
 * @param entry The entries.
 * @param count How many.
 */
void StageMap__ApplyChunkLoads(StageMap *self, ChunkLoadEntry *entry, s32 count);

/**
 * @brief How many slots have a load pending.
 * @param self The map.
 * @return The count, 0 to 7.
 */
s32 StageMap__CountPendingLoads(StageMap *self);

/**
 * @brief onDrawSystemEvent (slot +0x100): on a VSync, sends
 *        SLOT_DATA_READY for each slot whose data block has been read, and
 *        finishes each pending load whose header has been read (links it
 *        into the slot's cells, marks the header consumed); a slot whose
 *        loader went idle stops pending. Sets chunksLoaded once the last
 *        pending load finishes.
 * @param self The map.
 * @param sender The DrawSystem.
 * @param command Its event code; only DRAWSYSTEM_EVENT_VSYNC acts.
 */
void StageMap__OnDrawSystemEvent(StageMap *self, void *sender, s32 command);

/**
 * @brief populateSlotCells (slot +0x104): links a loaded chunk into its slot.
 *
 * Points the slot's PlacementGrid at the header's placement records,
 * replaces its LinkResource with one over the header's model block, then
 * resolves record after record until the grid returns 0. A record with no
 * model (-1) hides its lattice cell; one with a model links it into the next
 * lattice cell (a chained record: the next overflow cell), sets the cell's
 * position, y rotation and flags, and hides it until the drawn window shows
 * it. While a record has `next` set, the cell's nextInCell is the overflow
 * cell the following record takes.
 *
 * @param self The map.
 * @param slot The slot whose loader has read its header.
 */
void StageMap__PopulateSlotCells(StageMap *self, ChunkSlot *slot);

/**
 * @brief clearSlotCells (slot +0x108): when the slot has a chunk, releases
 *        the loader's header and hides every cell, dropping its model.
 * @param self The map.
 * @param slot The slot.
 */
void StageMap__ClearSlotCells(StageMap *self, ChunkSlot *slot);

/**
 * @brief getTargetDescriptor (slot +0x10C): the target's position and, when
 *        asked, its fresh descriptor.
 * @param self The map.
 * @param desc Receives computeFootprintDescriptor's result for the target's
 *        position, or NULL.
 * @param outPos Receives the address of the target's translation, or NULL.
 * @return &targetCell.base (the descriptor last tracked, not `desc`), or
 *         NULL when `desc` was asked for and no slot holds the target.
 */
Descriptor10 *StageMap__GetTargetDescriptor(StageMap *self, Descriptor10Ext *desc, void **outPos);

/**
 * @brief computeFootprintDescriptor (slot +0x110): the descriptor of a world
 *        position: the slot holding it, that slot's chunk index and
 *        column/row, the chunk's centre, the position relative to it, and
 *        the cell column/row and the offset from the cell's centre.
 * @param self The map.
 * @param out Receives the descriptor.
 * @param pos The world position.
 * @return 0, or 1 when no slot holds the position (`out` untouched).
 */
s32 StageMap__ComputeFootprintDescriptor(StageMap *self, Descriptor10Ext *out, SplitLongVec3 *pos);

/**
 * @brief A chunk index to its column and row.
 * @param self The map (for its config's columns).
 * @param out Receives the column and the row.
 * @param chunkIndex The chunk's index.
 */
void StageMap__SplitChunkIndex(StageMap *self, StageChunk *out, s32 chunkIndex);

/**
 * @brief getLastEventSlotChunk (slot +0x114): the slot the last slot event
 *        was about, and its chunk's column and row.
 * @param self The map.
 * @param out Receives the column and the row.
 * @return lastEventSlot.
 */
ChunkSlot *StageMap__GetLastEventSlotChunk(StageMap *self, StageChunk *out);

/**
 * @brief findSlotByNeighbour (slot +0x118): the slot whose loader holds a
 *        neighbour key. For a key no slot holds, the function reaches its end
 *        without a return value.
 * @param self The map.
 * @param neighbour The neighbour key.
 * @return The slot.
 */
ChunkSlot *StageMap__FindSlotByNeighbour(StageMap *self, s32 neighbour);

/**
 * @brief findSlotForPosition (slot +0x11C): the slot whose chunk holds a
 *        position in x and z; in a vertical grid, also the layer holding y
 *        (key i spans y in (-(i + 1) * 2048, -i * 2048]).
 * @param self The map.
 * @param pos The world position.
 * @return The slot, or NULL.
 */
ChunkSlot *StageMap__FindSlotForPosition(StageMap *self, LongVec3 *pos);

/**
 * @brief findSlotIndexByNeighbour (slot +0x120): the index of the slot at a
 *        neighbour key.
 * @param self The map.
 * @param key The neighbour key.
 * @return The index into slots[]; 0 when no slot holds the key.
 */
s32 StageMap__FindSlotIndexByNeighbour(StageMap *self, s32 key);

/**
 * @brief findSlotIndexByChunk (slot +0x124): the index of the slot holding a
 *        chunk with its header read.
 * @param self The map.
 * @param chunkIndex The chunk's index.
 * @return The index into slots[], or -1.
 */
s32 StageMap__FindSlotIndexByChunk(StageMap *self, s32 chunkIndex);

/**
 * @brief refreshFootprint (slot +0x128): once every chunk is loaded, hides
 *        the drawn window's cells, recomputes the window (from the target's
 *        facing in a flat grid, from its chunk in a vertical one) and shows
 *        the new window's cells.
 * @param self The map.
 */
void StageMap__RefreshFootprint(StageMap *self);

/**
 * @brief The flat grid's drawn window, from the target's cell and y
 *        rotation: aheadCells deep along whichever of x and z the target
 *        faces (within 45 degrees), starting at the target's cell and running
 *        the way it faces, and acrossCells wide, centred on the target and
 *        shifted toward where it looks (the off-axis part of a gridSpan-long
 *        facing vector, in cells, kept inside half the grid). Ends with
 *        BuildFootprintRects.
 * @param self The map.
 * @param acrossCells The window's width across the facing axis.
 * @param aheadCells The window's depth along it.
 */
void StageMap__ComputeFootprintFromRotation(StageMap *self, s32 acrossCells, s32 aheadCells);

/**
 * @brief Splits the window (footprintCol/Row, footprintWidth/Height) into
 *        `rects` and sets rectCount. A window that starts left of the centre
 *        chunk or above it starts in that neighbour's slot, its column and
 *        row moved into that chunk (rows above are staggered by half a
 *        chunk); a part past the right edge goes to the slot of key + 1, and
 *        SplitFootprintRect splits off the part past the bottom edge.
 * @param self The map.
 */
void StageMap__BuildFootprintRects(StageMap *self);

/**
 * @brief Clips a rectangle at the chunk's bottom edge, opening one in the
 *        slot below for the rest (and, when that part also runs past the
 *        right edge, one more in the slot to its right).
 * @param self The map.
 * @param rect The rectangle, already given its slot, column, row and width.
 * @param count rect's index in `rects`.
 * @param key The neighbour key of rect's slot.
 * @param col The window's column in that slot.
 * @param row The window's row in that slot.
 * @param width The window's width.
 * @param height The window's height.
 * @return The index of the last rectangle written.
 */
s32 StageMap__SplitFootprintRect(StageMap *self, CellRect *rect, s32 count, s32 key, s32 col,
                                 s32 row, s32 width, s32 height);

/**
 * @brief The vertical grid's drawn window: the whole of the target's chunk,
 *        the next chunk when the target's cell lies outside `bounds` and that
 *        chunk exists, and the previous chunk when it exists.
 * @param self The map.
 */
void StageMap__SetFootprintFromQuery(StageMap *self);

/**
 * @brief Whether a cell lies outside a box of cells.
 * @param bounds The box, inclusive, or NULL.
 * @param cell The cell.
 * @return 1 when there is no box or the cell lies outside it, else 0.
 */
s32 IsPointOutOfBounds(CellBounds *bounds, StageCell *cell);

/**
 * @brief Makes rects[index] the whole lattice (sFullSlotRect) of the slot
 *        holding a chunk.
 * @param self The map.
 * @param unused Not read.
 * @param index The rectangle to set.
 * @param chunkIndex The chunk.
 * @return index + 1.
 */
s32 StageMap__InitFootprintRect(StageMap *self, s32 unused, s32 index, s32 chunkIndex);

/**
 * @brief Shows or hides (GsDOFF) every cell of the drawn window whose slot
 *        has its header read, and every cell chained behind each.
 * @param self The map.
 * @param visible Non-zero to show, 0 to hide.
 */
void StageMap__SetFootprintVisible(StageMap *self, s32 visible);

/**
 * @brief getUnused1CC (slot +0x130); nothing calls it.
 * @param self The map.
 * @return &unused1CC, the first of the four words Reset sets to -1.
 */
void *StageMap__GetUnused1CC(StageMap *self);

/**
 * @brief setBounds (slot +0x134): the box SetFootprintFromQuery tests the
 *        target's cell against.
 * @param self The map.
 * @param bounds The box, or NULL.
 */
void StageMap__SetBounds(StageMap *self, CellBounds *bounds);

/**
 * @brief startScaleRamp (slot +0x138): picks the scale step by the sign of
 *        `rate` (y grows for a positive rate, shrinks otherwise) and by
 *        `fast` (1/4 per tick, else 1/64), and runs the ramp for |rate| times
 *        the step's y denominator ticks.
 * @param self The map.
 * @param rate The ramp's direction and length.
 * @param fast Non-zero for the large step.
 */
void StageMap__StartScaleRamp(StageMap *self, s32 rate, s32 fast);

/**
 * @brief stepScaleRamp (slot +0x13C): while the ramp runs, adds the step to
 *        every cell's scale and counts a tick; the count ends at -1.
 * @param self The map.
 */
void StageMap__StepScaleRamp(StageMap *self);

/**
 * @brief endScaleRamp (slot +0x140): after a ramp, sets every cell's scale
 *        back to 1/1 and clears the count.
 * @param self The map.
 */
void StageMap__EndScaleRamp(StageMap *self);

/**
 * @brief StepScaleRamp's per-cell callback: adds scaleStep to the cell's
 *        scale.
 * @param self The map.
 * @param cell The cell.
 */
void StageMap__AddScaleStepToCell(StageMap *self, struct GridCell *cell);

/**
 * @brief EndScaleRamp's per-cell callback: sets the cell's scale to 1/1.
 * @param self The map.
 * @param cell The cell.
 */
void StageMap__ResetCellScale(StageMap *self, struct GridCell *cell);

/**
 * @brief Runs a callback on every slot and on every cell of every slot.
 * @param self The map.
 * @param cellFn Called for each cell.
 * @param slotFn Called for each slot before its cells, or NULL.
 */
void StageMap__ForEachSlot(StageMap *self, StageMapCellFn cellFn, ChunkSlotFn slotFn);

/**
 * @brief Runs a callback on every cell of one slot (all STAGE_SLOT_CELLS).
 * @param self The map.
 * @param cellFn Called for each cell.
 * @param slot The slot.
 */
void StageMap__ForEachSlotCell(StageMap *self, StageMapCellFn cellFn, ChunkSlot *slot);

#endif
