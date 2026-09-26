#ifndef CLASS866E8_H
#define CLASS866E8_H

#include "LightRig.h"

/*
 * Class866E8 -- the grid manager (class id 0x114, method table
 * gClass866E8Methods at 0x800866E8): LightRig's subclass, no class below it.
 * Its ctor and finalize chain to LightRig's first, and it inherits getLight
 * and setAmbientColor unchanged. Methods in src/class_3ac78.c
 * (New_Class866E8 .. SetConfig), src/class_3bb8c.c (SetTargetAndBuildRates
 * .. FindElementForPosition) and src/class_3bb8c_b.c (FindElemIndexByUnk32
 * .. GetClass866E8Methods). The game makes one, at boot
 * (Class865C8__Class865C8 via New_Class866E8(NULL, 1)); Actor keeps it as
 * `grid` (include/Actor.h) when addChild sees a class-0x114 child.
 *
 * What its own methods do:
 *  - the ctor makes seven elements (`elems`), each a Class81940 `loader`, a
 *    Class6D940 `placements`, a Class86AA0 `cellParent` attached to this
 *    object at `origin`, and a 0x668-byte block of Class86AA0 `cells`
 *    attached to the cellParent on a 0x800-unit lattice. Every grid index
 *    uses a row stride of 20 cells: gDefaultGridSpan (0xA000) >> 11, stored
 *    by setGridSpan in `gridCells`.
 *  - applyRateEntries loads an element's file through its loader
 *    (Class81940 loadHeader); onNotifyTag1 consumes the header when it is
 *    read and loadElementResources links each cell to its placement's model.
 *  - updateFootprintTracking (every tick while `enabled`) turns `target`'s
 *    world position into a Descriptor10Ext (computeFootprintDescriptor) and
 *    keeps it in `targetCell`, notifying parents with 5 when its leading
 *    halfword (the element row/column pair) changes; refreshFootprint then
 *    rebuilds `rects`, the up to four element-local rectangles the target
 *    covers, and flags their cells (Class866E8__SetFootprintCellFlag).
 *  - forwardAcceptedCommand/applyToSenderFootprint re-notify every cell of
 *    a sender's rectangle (DispatchToRectCells, NotifyGridCell).
 * Descriptor10 has the shape of DreamSys.h's PlayerSpawnPoint (chunk
 * col/row, tile col/row, s16 x/y/z): DreamSys__WallLink copies
 * getCurrentCellKey's result into its linkCoordinates whole. The two are
 * not one type: this class reads the leading bytes signed
 * (ComputeCellWorldOffsets), PlayerSpawnPoint declares them u8.
 *
 * Inherited slots it overrides (`tools/classtable.py gClass866E8Methods --vs
 * gLightRigMethods`): +0x008 the ctor, +0x00C Finalize, +0x038 OnNotify,
 * +0x040 Reset, +0x088 notifyIfUnk20Active (Class866E8__OnElementEvent,
 * see below), +0x098 update (Class866E8__UpdateIfEnabled) and +0x09C
 * DispatchLinkCommand.
 *
 * +0x088: Class866E8__OnElementEvent takes (self, command, elem); the slot
 * keeps Class6B5CC's (self, event). Its four callers (Finalize,
 * ResetAllElements, ApplyRateEntries, OnNotifyTag1) pass (self, 6 or 7,
 * elem, index) through Class866E8OnElementEventFn below (no code).
 * +0x0D4 getCurrentCellKey and +0x0F8 buildRateEntries keep their CALLERS'
 * shapes: DreamSys__WallLink passes getCurrentCellKey a second word the
 * occupant never reads, and SetTargetAndBuildRates returns buildRateEntries'
 * value although the occupant returns nothing.
 *
 * The object is 0x1E8 bytes (New_Class866E8).
 */

struct Class81940;
struct Class6D940;
struct Class86AA0;

typedef struct Class866E8 Class866E8;
typedef struct Class866E8Methods Class866E8Methods;
typedef struct Class866E8Elem Class866E8Elem;

/* A 3-word world position: `origin`, the ctor's argument and
 * gDefaultOrigin, FindElementForPosition's query, and the rate tables. */
typedef struct Unk54Struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk54Struct;

/* `config`'s pointee, set by setConfig. The elements are laid out `divisor`
 * to a row and `count` deep; `unk4` nonzero switches every footprint query
 * to the other layout (RefreshFootprint, ComputeRateFlags,
 * FindElementForPosition, ApplyToSenderFootprint). */
typedef struct Unk68Struct {
    s16 divisor;   /* +0x000 */
    s16 count;     /* +0x002 */
    s32 unk4;      /* +0x004 */
} Unk68Struct;

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

/* computeFootprintDescriptor's output, 0x2C bytes: the Descriptor10, the
 * element's centre (its origin + 0x5000 in x and z, +0x00C..+0x014), the
 * position relative to that (+0x018..+0x020), the element and its rate. `targetCell` holds one. */
typedef struct Descriptor10Ext {
    Descriptor10 base;         /* +0x000 */
    s32 unkC;                  /* +0x00C */
    s32 unk10;                 /* +0x010 */
    s32 unk14;                 /* +0x014 */
    s32 unk18;                 /* +0x018 */
    s32 unk1C;                 /* +0x01C */
    s32 unk20;                 /* +0x020 */
    Class866E8Elem *unk24;     /* +0x024, the element findElementForPosition resolved */
    s32 unk28;                 /* +0x028, that element's Class81940::ownerRate, sign-extended */
} Descriptor10Ext;

/* computeFootprintDescriptor's world position: each word is read whole (the
 * cell) and, later, as its low halfword (the offset), which is why each is
 * a union: retail reloads at the narrower width. */
typedef struct QueryPos866E8 {
    union { s32 w; u16 h; } unk0;   /* +0x000 */
    union { s32 w; u16 h; } unk4;   /* +0x004 */
    union { s32 w; u16 h; } unk8;   /* +0x008 */
} QueryPos866E8;

/* applyRateEntries' 0xC-byte entries: a file name for the element's
 * loader (NULL: cancel), its rate and the element key. */
typedef struct SetupEntry866E8 {
    void *ptr0;     /* +0x0 */
    s16 rate;       /* +0x4 */
    u8 pad6[0x8 - 0x6];
    s32 id;         /* +0x8 */
} SetupEntry866E8;

/* The same 0xC stride based at +0x4: ApplyRateEntries' second walker
 * (strength-reduced from the parameter; its report). */
typedef struct SetupSub866E8 {
    s16 rate;       /* +0x4 in SetupEntry866E8 terms */
    u8 pad2[0x4 - 0x2];
    s32 id;         /* +0x8 */
    u8 pad8[0xC - 0x8];
} SetupSub866E8;

/* buildRateEntries' per-element key/enable pair (sDefaultTargetSpecs and
 * the sFootprintResultPtrTable tables hold seven each). */
typedef struct TargetSpec866E8 {
    u8 key;   /* +0x0 */
    u8 flag;  /* +0x1 */
} TargetSpec866E8;

/* One element-local rectangle of cells, 0xC bytes, no padding. */
typedef struct GridSlot866E8 {
    s32 elemIdx;    /* +0x0, index into elems[] */
    s16 col;        /* +0x4, starting column */
    s16 row;        /* +0x6, starting row (row stride 20) */
    s16 width;      /* +0x8 */
    s16 height;     /* +0xA */
} GridSlot866E8;

/* `rects` as a whole, so ApplyToSenderFootprint can save and restore it
 * with a plain `=` (a batched 4-word block move; an indexed loop does not
 * compile to it). */
typedef struct GridSlotList866E8 {
    GridSlot866E8 e[4];
} GridSlotList866E8;

/* One of the four static 0xC-byte entries configureRateEntry picks for
 * `rateEntry` (D_8008699C..D_800869C0); it is handed to every cell's
 * updateScale (ApplyRateToChild), and +0x006 scales the countdown. */
typedef struct EntryDesc866E8 {
    u8 pad0[0x6];
    s16 scale;          /* +0x006 */
    u8 pad8[0xC - 0x8];
} EntryDesc866E8;

/* `bounds`' pointee (setBounds): IsPointOutOfBounds' min/max box. */
typedef struct Bounds866E8_3bb8c_b {
    s16 minX;                      /* +0x000 */
    s16 minY;                      /* +0x002 */
    s32 maxX;                      /* +0x004 */
    s32 maxY;                      /* +0x008 */
} Bounds866E8_3bb8c_b;

/* One of the seven elements, 0x1C bytes (the ctor, Finalize). */
struct Class866E8Elem {
    /* +0x000 */ u16 flag;                        /* 1 while its load is pending (ApplyRateEntries; OnNotifyTag1 clears it) */
    /* +0x002 */ u16 key;                         /* the ctor: its index; BuildRateEntries: the spec's key, copied on into loader->ownerKey */
    /* +0x004 */ struct Class81940 *loader;       /* New_Class81940(): the element's file (include/Class81940.h) */
    /* +0x008 */ struct Class6D940 *placements;   /* New_Class6D940(0): its placement records (include/Class6D940.h) */
    /* +0x00C */ struct Class86AA0 *cellParent;   /* New_Class86AA0(), attached to the Class866E8 at `origin`; every cell's parent */
    /* +0x010 */ struct Class86AA0 **cells;       /* BMemPMgrAlloc(0x668): 410 New_Class86AA0() cells, row stride 20 */
    /* +0x014 */ BasicClass *heldObj;             /* zeroed by the ctor; OnElementEvent releases it on event 6 */
    /* +0x018 */ s32 unk18;                       /* zeroed by the ctor */
};

/* The ctor's callback pair (setCallback): ComputeRateEntry calls
 * valueFn(valueFnCtx, value, 0, 0) and keeps the result as an entry's name. */
typedef void *(*Class866E8ValueFn)(void *ctx, s32 value, s32 arg2, s32 arg3);

/* LightRig's slots, then this class's own. */
struct Class866E8Methods {
    LIGHTRIG_SLOTS(Class866E8, (Class866E8 *self, Unk54Struct *origin, s32 autoLoad));
    /* +0x0C0 */ void (*resetAllElements)(Class866E8 *self);                              /* Class866E8__ResetAllElements */
    /* +0x0C4 */ void (*setChildParams)(Class866E8 *self, s32 count, s32 dirs, s32 colors); /* Class866E8__SetChildParams */
    /* +0x0C8 */ void (*setCallback)(Class866E8 *self, Class866E8ValueFn fn, void *ctx);  /* Class866E8__SetCallback */
    /* +0x0CC */ void (*setAcceptedTags)(Class866E8 *self, s32 *tags);                     /* Class866E8__SetAcceptedTags */
    /* +0x0D0 */ void (*forwardAcceptedCommand)(Class866E8 *self, void *sender, s32 command); /* Class866E8__ForwardAcceptedCommand */
    /* +0x0D4 */ Descriptor10 *(*getCurrentCellKey)(Class866E8 *self, void *arg1);       /* Class866E8__GetCurrentCellKey (reads only self; see the banner) */
    /* +0x0D8 */ void (*slotD8)(void);                                                     /* func_8004B324, empty; never called */
    /* +0x0DC */ void (*setGridSpan)(Class866E8 *self, s32 span);                          /* Class866E8__SetGridSpan */
    /* +0x0E0 */ void (*setConfig)(Class866E8 *self, Unk68Struct *config);                 /* Class866E8__SetConfig */
    /* +0x0E4 */ s32 (*setTargetAndBuildRates)(Class866E8 *self, void *outPos, Class6B5CC *target, Descriptor10 *cell); /* Class866E8__SetTargetAndBuildRates */
    /* +0x0E8 */ s32 (*computeCellOffsets)(Class866E8 *self, void *outPos, void *cell);   /* Class866E8__ComputeCellOffsets */
    /* +0x0EC */ void (*enable)(Class866E8 *self);                                         /* Class866E8__Enable */
    /* +0x0F0 */ void (*disable)(Class866E8 *self);                                        /* Class866E8__Disable */
    /* +0x0F4 */ s32 (*updateFootprintTracking)(Class866E8 *self);                         /* Class866E8__UpdateFootprintTracking */
    /* +0x0F8 */ s32 (*buildRateEntries)(Class866E8 *self, s32 val, Unk54Struct *pos, TargetSpec866E8 *specs); /* Class866E8__BuildRateEntries (returns nothing; see the banner) */
    /* +0x0FC */ void (*applyRateEntries)(Class866E8 *self, SetupEntry866E8 *entries, s32 count); /* Class866E8__ApplyRateEntries */
    /* +0x100 */ void (*onNotifyTag1)(Class866E8 *self, void *sender, s32 mode);          /* Class866E8__OnNotifyTag1; OnNotify's class-1 sender case */
    /* +0x104 */ void (*loadElementResources)(Class866E8 *self, Class866E8Elem *elem);    /* Class866E8__LoadElementResources */
    /* +0x108 */ void (*resetElementCells)(Class866E8 *self, Class866E8Elem *elem);       /* Class866E8__ResetElementCells */
    /* +0x10C */ Descriptor10 *(*getTargetDescriptor)(Class866E8 *self, Descriptor10Ext *out, void **outPos); /* Class866E8__GetTargetDescriptor */
    /* +0x110 */ s32 (*computeFootprintDescriptor)(Class866E8 *self, Descriptor10Ext *out, QueryPos866E8 *pos); /* Class866E8__ComputeFootprintDescriptor: 0, or 1 when no element holds pos */
    /* +0x114 */ Class866E8Elem *(*getLastTargetRateSplit)(Class866E8 *self, u8 *out);   /* Class866E8__GetLastTargetRateSplit */
    /* +0x118 */ Class866E8Elem *(*findElemByUnk32)(Class866E8 *self, s32 key);          /* Class866E8__FindElemByUnk32 */
    /* +0x11C */ Class866E8Elem *(*findElementForPosition)(Class866E8 *self, Unk54Struct *pos); /* Class866E8__FindElementForPosition */
    /* +0x120 */ s32 (*findElemIndexByUnk32)(Class866E8 *self, s32 key);                  /* Class866E8__FindElemIndexByUnk32 */
    /* +0x124 */ s32 (*findElemIndexByUnk30)(Class866E8 *self, s32 key);                  /* Class866E8__FindElemIndexByUnk30: an index or -1 */
    /* +0x128 */ void (*refreshFootprint)(Class866E8 *self);                               /* Class866E8__RefreshFootprint */
    /* +0x12C */ void (*applyToSenderFootprint)(Class866E8 *self, Class6B5CC *sender, s32 command); /* Class866E8__ApplyToSenderFootprint */
    /* +0x130 */ void *(*getUnk1CC)(Class866E8 *self);                                     /* Class866E8__GetUnk1CC */
    /* +0x134 */ void (*setBounds)(Class866E8 *self, Bounds866E8_3bb8c_b *bounds);        /* Class866E8__SetBounds */
    /* +0x138 */ void (*configureRateEntry)(Class866E8 *self, s32 rate, s32 flag);        /* Class866E8__ConfigureRateEntry */
    /* +0x13C */ void (*advanceRateCountdown)(Class866E8 *self);                           /* Class866E8__AdvanceRateCountdown */
    /* +0x140 */ void (*flushRateLatch)(Class866E8 *self);                                 /* Class866E8__FlushRateLatch */
};                                                                                         /* 80 slots */

struct Class866E8 {
    LIGHTRIG_FIELDS(Class866E8Methods);
    /* +0x054 */ Unk54Struct origin;           /* the ctor: its argument, or gDefaultOrigin; the cellParents attach here */
    /* +0x060 */ Class866E8ValueFn valueFn;     /* setCallback */
    /* +0x064 */ void *valueFnCtx;              /* setCallback */
    /* +0x068 */ Unk68Struct *config;           /* setConfig; NULL after Reset */
    /* +0x06C */ Class6B5CC *target;            /* setTargetAndBuildRates (DreamSys__SpawnAtLink passes the DreamSys); its coord2 is the tracked position */
    /* +0x070 */ s32 enabled;                   /* enable/disable; gates UpdateIfEnabled */
    /* +0x074 */ s32 gridSpan;                  /* setGridSpan: gDefaultGridSpan = 0xA000 */
    /* +0x078 */ s16 gridHalfCells;             /* gridSpan >> 12 = 10 */
    /* +0x07A */ s16 gridCells;                 /* gridSpan >> 11 = 20, the row stride */
    /* +0x07C */ s16 footprintCol;              /* BuildFootprintSlots' input: a signed column, clamped into [0,20) */
    /* +0x07E */ s16 footprintRow;              /* the same, vertical */
    /* +0x080 */ s32 footprintWidth;
    /* +0x084 */ s32 footprintHeight;
    /* +0x088 */ s32 rectCount;                 /* how many of rects[] are live */
    /* +0x08C */ GridSlotList866E8 rects;       /* BuildFootprintSlots, SetFootprintRect, InitFootprintSlot write; DispatchToRectCells, SetFootprintCellFlag walk */
    /* +0x0BC */ Descriptor10Ext targetCell;    /* UpdateFootprintTracking: the target's last descriptor; SetTargetAndBuildRates sets .base; getTargetDescriptor returns &.base */
    /* +0x0E8 */ s32 *acceptedTags;             /* setAcceptedTags: a 0-terminated list of class ids ForwardAcceptedCommand accepts */
    /* +0x0EC */ Class866E8Elem elems[7];
    /* +0x1B0 */ s32 unk1B0;                    /* 1 while element loads are pending (ApplyRateEntries; OnNotifyTag1 clears it) */
    /* +0x1B4 */ u16 unk1B4;                    /* CountFlaggedElements after ApplyRateEntries; OnNotifyTag1 counts it down */
    /* +0x1B6 */ u8 pad1B6[0x1B8 - 0x1B6];
    /* +0x1B8 */ s32 unk1B8;                    /* set when that count reaches 0; RefreshFootprint does nothing while it is 0 */
    /* +0x1BC */ Class866E8Elem *lastEventElem; /* OnElementEvent's elem; GetLastTargetRateSplit reads it */
    /* +0x1C0 */ Descriptor10 curCell;          /* DispatchToRectCells: the cell being notified (b0/b1 copied from targetCell as a u16); getCurrentCellKey returns it */
    /* +0x1CA */ u8 pad1CA[0x1CC - 0x1CA];
    /* +0x1CC */ s32 unk1CC;                    /* Reset: -1; GetUnk1CC returns its address */
    /* +0x1D0 */ s32 unk1D0;                    /* Reset: -1 */
    /* +0x1D4 */ s32 unk1D4;                    /* Reset: -1 */
    /* +0x1D8 */ s32 unk1D8;                    /* Reset: -1 */
    /* +0x1DC */ Bounds866E8_3bb8c_b *bounds;   /* setBounds; IsPointOutOfBounds */
    /* +0x1E0 */ s32 rateCountdown;             /* configureRateEntry; advanceRateCountdown/flushRateLatch */
    /* +0x1E4 */ EntryDesc866E8 *rateEntry;     /* configureRateEntry */
};                                              /* 0x1E8 bytes: New_Class866E8 */

/* +0x088's occupant, which takes the element too (see the banner). */
typedef void (*Class866E8OnElementEventFn)(Class866E8 *self, s32 command, Class866E8Elem *elem, s32 index);

/* ForEachElem's callbacks. */
typedef void (*Class866E8CellFn)(Class866E8 *self, struct Class86AA0 *cell);
typedef void (*Class866E8ElemFn)(Class866E8 *self, Class866E8Elem *elem);

extern Class866E8Methods gClass866E8Methods;
extern Class866E8Methods *GetClass866E8Methods(void); /* returns &gClass866E8Methods */

/* The class's own functions, in address order: the occupants of
 * gClass866E8Methods and their non-slot helpers. */
Class866E8 *New_Class866E8(Unk54Struct *origin, s32 autoLoad);
void Class866E8__Class866E8(Class866E8 *self, Unk54Struct *origin, s32 autoLoad);
void Class866E8__Finalize(Class866E8 *self);
void Class866E8__OnNotify(Class866E8 *self, BasicClass *sender, s32 command);
void Class866E8__Reset(Class866E8 *self);
void Class866E8__OnElementEvent(Class866E8 *self, s32 command, Class866E8Elem *elem);
void Class866E8__UpdateIfEnabled(Class866E8 *self);
void Class866E8__DispatchLinkCommand(Class866E8 *self, BasicClass *sender, s32 command);
void Class866E8__ResetAllElements(Class866E8 *self);
void Class866E8__SetChildParams(Class866E8 *self, s32 count, s32 dirs, s32 colors);
void Class866E8__SetCallback(Class866E8 *self, Class866E8ValueFn fn, void *ctx);
void Class866E8__SetAcceptedTags(Class866E8 *self, s32 *tags);
void Class866E8__ForwardAcceptedCommand(Class866E8 *self, void *sender, s32 command);
void Class866E8__ApplyToSenderFootprint(Class866E8 *self, Class6B5CC *sender, s32 command);
void Class866E8__SetFootprintFromCell(Class866E8 *self, Descriptor10Ext *desc, s32 span);
void Class866E8__SetFootprintRect(Class866E8 *self, Descriptor10Ext *desc, s32 span);
void Class866E8__DispatchToRectCells(Class866E8 *self, Class6B5CC *sender, s32 command);
void NotifyGridCell(struct Class86AA0 *cell, Class6B5CC *sender, s32 command);
Descriptor10 *Class866E8__GetCurrentCellKey(Class866E8 *self);
void func_8004B324(void);
void Class866E8__SetGridSpan(Class866E8 *self, s32 span);
void Class866E8__SetConfig(Class866E8 *self, Unk68Struct *config);
s32 Class866E8__SetTargetAndBuildRates(Class866E8 *self, void *outPos, Class6B5CC *target, Descriptor10 *cell);
s32 Class866E8__ComputeCellOffsets(Class866E8 *self, void *outPos, void *cell);
s32 ComputeCellWorldOffsets(s32 *outPos, s32 *outBuf, Unk68Struct *config, Unk54Struct *origin, Descriptor10 *cell);
void Class866E8__Enable(Class866E8 *self);
void Class866E8__Disable(Class866E8 *self);
s32 Class866E8__UpdateFootprintTracking(Class866E8 *self);
void Class866E8__BuildRateEntries(Class866E8 *self, s32 val, Unk54Struct *pos, TargetSpec866E8 *specs);
s32 Class866E8__ComputeRateFlags(Class866E8 *self, s32 val, s32 flag);
s32 Class866E8__ComputeRateEntry(Class866E8 *self, SetupEntry866E8 *entry, s32 divisor, s32 flag, s32 val, s32 savedResult, s32 key); /* 0 or 1; BuildRateEntries discards it */
void Class866E8__ApplyRateEntries(Class866E8 *self, SetupEntry866E8 *entries, s32 count);
s32 Class866E8__CountFlaggedElements(Class866E8 *self);
void Class866E8__OnNotifyTag1(Class866E8 *self, void *sender, s32 mode);
void Class866E8__LoadElementResources(Class866E8 *self, Class866E8Elem *elem);
void Class866E8__ResetElementCells(Class866E8 *self, Class866E8Elem *elem);
Descriptor10 *Class866E8__GetTargetDescriptor(Class866E8 *self, Descriptor10Ext *out, void **outPos);
s32 Class866E8__ComputeFootprintDescriptor(Class866E8 *self, Descriptor10Ext *out, QueryPos866E8 *pos);
void Class866E8__ComputeDivisorSplit(Class866E8 *self, u8 *out, s32 val);
Class866E8Elem *Class866E8__GetLastTargetRateSplit(Class866E8 *self, u8 *out);
Class866E8Elem *Class866E8__FindElemByUnk32(Class866E8 *self, s32 key);
Class866E8Elem *Class866E8__FindElementForPosition(Class866E8 *self, Unk54Struct *pos);
s32 Class866E8__FindElemIndexByUnk32(Class866E8 *self, s32 key);
s32 Class866E8__FindElemIndexByUnk30(Class866E8 *self, s32 key);
void Class866E8__RefreshFootprint(Class866E8 *self);
void Class866E8__ComputeFootprintFromRotation(Class866E8 *self, s32 width, s32 height);
void Class866E8__BuildFootprintSlots(Class866E8 *self);
s32 Class866E8__SplitFootprintSlot(Class866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 col, s32 row, s32 width, s32 height);
void Class866E8__SetFootprintFromQuery(Class866E8 *self);
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point);
s32 Class866E8__InitFootprintSlot(Class866E8 *self, s32 unused, s32 key, s32 arg3);
void Class866E8__SetFootprintCellFlag(Class866E8 *self, s32 setBit);
void *Class866E8__GetUnk1CC(Class866E8 *self);
void Class866E8__SetBounds(Class866E8 *self, Bounds866E8_3bb8c_b *bounds);
void Class866E8__ConfigureRateEntry(Class866E8 *self, s32 rate, s32 flag);
void Class866E8__AdvanceRateCountdown(Class866E8 *self);
void Class866E8__FlushRateLatch(Class866E8 *self);
void Class866E8__ApplyRateToChild(Class866E8 *self, struct Class86AA0 *cell);
void Class866E8__ResetChildRate(Class866E8 *self, struct Class86AA0 *cell);
void Class866E8__ForEachElem(Class866E8 *self, Class866E8CellFn cellFn, Class866E8ElemFn elemFn);
void Class866E8__ForEachEntryChild(Class866E8 *self, Class866E8CellFn cellFn, Class866E8Elem *elem);

#endif
