# StageMap__StageMap -- MATCH (163/163 words, ~7 real rebuild attempts)

> Renamed from `Class866E8__Class866E8` on 2026-09-26 (tools/rename.py). Address 0x8004a534.

> Renamed from `func_8004A534` on 2026-09-22 (tools/rename.py). Address 0x8004a534.

Unit `DayTaskStageMap`, 177-line body, `StageMapMethods::ctor` (the occupant
of the ctor slot, dispatched by `New_StageMap`'s `New_StageMap`
allocator). 8 distinct callee-saved registers (`$s0`-`$s7`, fully
saturated) -- flagged by the head as being in the band that was 0
matched / 4 stalled across 81 pooled samples from rounds 13-14, sent
anyway on the reasoning that the screen deprioritizes rather than forbids
and no report existed on an instance in this unit. **It matched.** See
"On the register-count screen" below.

```c
typedef struct BaseCtorTable_3ac78 BaseCtorTable_3ac78;
struct BaseCtorTable_3ac78 {
    u8 pad0[0x8];
    void (*ctor)(void *self); /* +0x008, standard "further-base ctor first" slot */
};

extern BaseCtorTable_3ac78 *GetLightRigMethods(void);
extern UnkSlotChildObj_3ac78 *New_LbdFile(void);
extern UnkSlotListObj_3ac78 *New_PlacementGrid(s32 arg1);
extern GenericObject *New_GridCell(void);
extern s32 GetDrawSystem(void);
extern Vec3_3ac78 sDefaultOrigin;

void StageMap__StageMap(StageMap *self, Vec3_3ac78 *arg1, s32 arg2)
{
    s32 i;
    UnkSlotEntry_3ac78 *entry;
    GenericObject *obj;
    StageMap **cellp;
    u8 *p;
    u8 *end;
    s32 buf[3];

    GetLightRigMethods()->ctor(self);
    self->methods = GetStageMapMethods();

    if (arg1 != NULL) {
        self->unk54 = *arg1;
    } else {
        self->unk54 = sDefaultOrigin;
    }

    self->unk1B0 = 0;
    self->unk1B4 = 0;
    self->unk1B8 = 0;
    self->unk70 = 0;
    self->unk6C = 0;
    self->unkE8 = 0;
    self->unk1E0 = 0;

    for (i = 0; i < 7; i++) {
        entry = &self->unkEC[i];

        entry->unk4 = New_LbdFile();
        entry->unk4->unk20 = (entry->unk4->unk10 != 0);
        entry->unk4->unk32 = i;
        entry->unk4->methods->slot88(entry->unk4, arg2);

        entry->unk14 = 0;
        entry->unk18 = 0;
        entry->unk2 = i;
        entry->unk0 = 0;

        entry->unk8 = New_PlacementGrid(0);
        entry->unkC = New_GridCell();
        entry->unkC->methods->slot4C(entry->unkC, self, &self->unk54);

        entry->unk10 = (StageMap **)BMemPMgrAlloc(0x668);
        if (entry->unk10 == NULL) {
            return;
        }

        buf[0] = 0x400;
        buf[1] = 0;
        buf[2] = 0x400;

        cellp = entry->unk10;
        end = (u8 *)cellp + 0x668;
        p = (u8 *)cellp;
        while (p < end) {
            obj = New_GridCell();
            *(GenericObject **)p = obj;
            obj->methods->slot4C(obj, entry->unkC, buf);

            buf[0] += 0x800;
            if (buf[0] > 0xA400) {
                buf[0] = 0x400;
                buf[2] += 0x800;
            }

            obj = *(GenericObject **)p;
            obj->methods->slot70(obj, 1);
            obj = *(GenericObject **)p;
            p += 4;
            obj->unk10 |= 0x80000000;
        }
    }

    self->methods->slot10(self, GetDrawSystem());
    self->methods->slot40(self);
}
```

## What it does

StageMap's own ctor. Calls a further-base ctor (`GetLightRigMethods()->ctor(self)`,
the standard "base ctor first, then set own vtable pointer" idiom already
established elsewhere in this project), sets `self->methods`, copies a
3-word block into `self->unk54` (from `arg1` if given, else a default
global), zeroes several scalar fields, then fills `self->unkEC[0..6]`
(7 slot entries): each gets a child object (`New_LbdFile`), a list object
(`New_PlacementGrid`), a generic object (`New_GridCell`) dispatched with the
just-copied `self->unk54` block, and a freshly-allocated 0x668-byte buffer
of pointers -- each pointer itself a `New_GridCell()`-created object,
initialized via a rect-packing-style budget (`buf[0]`/`buf[2]`, wrapping
at `0xA400` back to `0x400`, stepping `0x800` per cell) and flagged with a
high bit on `unk10` after a `slot70(obj, 1)` dispatch. Finishes with two
more dispatches on `self->methods` (`slot10` with a fresh `GetDrawSystem()`
value, then `slot40`, already known gp_rel-blocked as a CALLEE -- irrelevant
here since this function only DISPATCHES to it, never inlines its body).

## New struct ground opened (all additive; DayTaskStageMap.h is unique to this unit)

- `StageMapMethods::slot10` (+0x010, replacing a 4-byte pad) -- the ctor's
  own dispatch, `(self, s32 arg1)`.
- `StageMap::unk54` (`Vec3_3ac78`, a new 3-word type, +0x054..+0x05F,
  carved out of the `pad03C` range) and `unk6C`/`unk1B0`/`unk1E0` (each a
  zeroed `s32`, carved out of existing single-word pad ranges).
- `UnkSlotEntry_3ac78::unk2` (u16, was `pad2`) and `unk14`/`unk18` (two new
  `s32`s, replacing the trailing `pad14[0x1C-0x14]`).
- `UnkSlotChildMethods_3ac78::slot88` (+0x088, appended directly after the
  existing `slot84` with no gap) and `UnkSlotChildObj_3ac78::unk10`/`unk20`/
  `unk32` (new fields carved out of existing opaque padding).
- `GenericMethodsHeader::slot4C`/`slot70` and `GenericObject::unk10` --
  additive extensions of a struct already used by two OTHER already-matched
  functions in this unit (`StageMap__OnNotify`, `StageMap__DispatchLinkCommand`). Verified safe:
  neither touches the new slots/fields, and the whole-image SHA1 stayed
  green immediately after this specific edit, before any of this function's
  own body was written.

## Two real defects found, one MISREAD structural detail corrected along the way

1. **The `self->unk54` copy is a WHOLE-STRUCT assignment, not three
   separate field copies.** First read of the disassembly mis-transcribed
   the shape as interleaved load/store/load/store/load/store; the actual
   instructions are three loads THEN three stores (batched) -- the same
   "whole-struct `=` compiles to a batched block move" idiom as
   `HistoryBlock_3ac78` (documented elsewhere in this header). Writing
   `self->unk54 = *arg1;` / `self->unk54 = sDefaultOrigin;` instead of
   field-by-field fixed a register-swap-and-shift residue immediately
   (23/163 -> 70/163 in one change).
2. **`New_PlacementGrid` takes an argument, not zero.** Retail sets
   `$a0 = 0` right after the `slot88` dispatch and never touches it again
   before the `jal` -- the "leftover register is a forwarded/explicit
   argument" tell, same family as this round's `TextEntryItemList` slot-arity
   fixes, except here the argument is a plain literal `0` rather than
   forwarded. Declaring it `(s32 arg1)` and calling `New_PlacementGrid(0)`,
   plus reordering the four field-zeroing statements to match retail's
   actual order (`unk14, unk18, unk2, unk0`, not declaration order),
   fixed a second residue (70/163 -> 86/163).
3. **The very last residue was a single missing `move`.** After both fixes
   above, the function matched exactly except for a uniform 1-word address
   shift starting at `entry->unk10`'s reload into the loop cursor: retail
   emits `lw v0,0x10(s1); move s0,v0` (load into `$v0`, THEN copy to the
   persistent `$s0`) where the direct `p = (u8 *)entry->unk10;` phrasing
   compiled to a single `lw s0,0x10(s1)` (load straight into `$s0`).
   Introducing an explicit intermediate step did NOT fix it by itself
   (`StageMap **cellp = entry->unk10; p = (u8 *)cellp;` still compiled
   to one instruction) -- what fixed it was additionally computing `end`
   from `cellp` BEFORE `p`, i.e. `end = (u8 *)cellp + 0x668;` written
   textually before `p = (u8 *)cellp;`. That extra use of `cellp` between
   its definition and `p`'s assignment was enough to make GCC stage the
   value through `$v0` rather than target `$s0` directly. 86/163 -> 163/163.

## On the register-count screen

This is the first matched instance in the "8 distinct callee-saved
registers" band (previously 0/4 pooled). It does not overturn the
screen -- CLAUDE.md is explicit that it deprioritizes, not forbids, and
one instance does not out-weigh four stalls -- but it is worth recording
as a counter-example for calibration: a fully-saturated register file is
not automatically fatal when the body is long, straight-line, and has few
genuinely independent live values fighting for the SAME registers at the
SAME time (here, most of the register pressure comes from long-lived
values -- `self`, `arg2`, the loop index, the entry pointer, three loop
bounds/step constants -- that are each read/written in disjoint stretches
of the function rather than all contending across one tight expression).
The residues that showed up were exactly the two structural-shape
misreads above plus one single-instruction register-staging quirk, not an
unrecoverable "one value too many, no register to hold it" wall.

### Proposed learning

An explicit intermediate variable does not by itself force a compiler to
stage a value through a scratch register before committing it to a
persistent one -- GCC 2.6.3 will still fuse `local = expr; named = local;`
into one instruction if `local` has no OTHER use in between. What worked
here was giving the intermediate variable a second, real use (computing
`end` from it) BEFORE the assignment that needed the extra `move` --
i.e. the lever is "does this value get used more than once between its
definition and the point where retail shows a staged copy," not merely
"is there a named temp in the C."

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A534` | `StageMap__StageMap` | A | Occupant of vtable slot `+0x008`, which `include/code_8220.h` establishes as `BasicClassMethods::ctor`, and which `New_StageMap` dispatches right after allocating. `Class__Class` is the constructor convention in FINISHING-PLAN.md track 3. |
| `D_8008682C` | `sDefaultOrigin` | A | Its only use is this ctor's fallback when `arg1 == NULL`: `self->origin = sDefaultOrigin`. `asm/data/76DC8.data.s` shows the three words are all zero, so it is literally the default origin. |

Field names this function established (all unit-local -- the compiler listed
no accessor outside `src/world/DayTaskStageMap.c`):

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x054` | `origin` | B | A 3-word block, copied here from `arg1` or `sDefaultOrigin`, then passed as the THIRD argument of `cellParent->methods->slot4C(cellParent, self, &self->origin)`. The same parameter position in the sibling call one loop deeper receives `buf = {x, 0, z}`, a literal world position on the 0x800 lattice -- so the slot takes a position and this field is one. |
| `StageMap+0x0EC` | `elems[7]` | A | Seven 0x1C-byte records, walked 0..6 here, in `StageMap__Finalize` and in `StageMap__UnloadAllSlots`; `DayTaskStageMap` reaches the same array from four more functions and calls it `arr[7]`. |
| `UnkSlotEntry+0x000` | `flag` | B | Zeroed here and in `StageMap__UnloadAllSlots`; `DayTaskStageMap`'s independent view names the same halfword `Elem::flag`. |
| `UnkSlotEntry+0x002` | `key` | B | Set to the loop index here and copied on into `target->key`; `DayTaskStageMap`'s `StageMap__LoadChunksAround` copies a caller-supplied key byte into the same field. |
| `UnkSlotEntry+0x004` | `target` | B | `DayTaskStageMap` types the same pointer `ElemTarget *` from six functions. |
| `UnkSlotEntry+0x008` | `list` | C-ish/B | Built here by `New_PlacementGrid(0)`; the name records only that it is the list object the entry owns. |
| `UnkSlotEntry+0x00C` | `cellParent` | B | Initialized here with `slot4C(cellParent, self, &self->origin)` and then passed as the PARENT argument of every grid cell's own `slot4C(cell, cellParent, buf)`. Its role in this function is exactly "the node the cells hang off". |
| `UnkSlotEntry+0x010` | `cells` | A | 0x668 raw bytes allocated here and filled with freshly built cell objects, one per 4 bytes; `StageMap__Finalize` walks the same span tearing them down; `DayTaskStageMap`'s byte-matched `StageMap__SetFootprintVisible` indexes the same block as a 2D grid. |

**The 21-vs-20 discrepancy, recorded not resolved.** This ctor's placement
loop wraps X after 21 columns (`x = 0x400 + k * 0x800`, reset when
`x > 0xA400`), and 0x668 bytes is 410 cell pointers -- neither `20 * 20` nor a
whole number of 21-cell rows. The grid's INDEX stride is 20, byte-verified
twice over (`DayTaskStageMap`'s matched `StageMap__SetFootprintVisible`, and
`sDefaultGridSpan >> 11`). This function is byte-exact, so both constants are
certainly right; what the extra column and the 10 spare pointers are for is
unknown. Do not "correct" the stride to 21 on this function's evidence alone.

## Track 4

2026-09-26, round 86 (delta): class 0x14 (was D_8006EFAC) unified as LightRig in `include/LightRig.h`; the first call is `GetLightRigMethods()->ctor((LightRig *)self)` through include/LightRig.h (was a unit-local `BaseCtorTable_3ac78 *` view of the same getter). A pointer cast emits no code; image byte-identical. StageMap's own view is unchanged.

## Track 4 (2026-09-26, round 87, bravo)

The local GetDrawSystem/New_DrawSystem extern this unit carried is gone; it comes from `include/draw_system.h` (gDrawSystemMethods unified), with a pointer cast where this unit's own slot type asks for one. Byte-identical.

## Track 4 (2026-09-26, round 88, alpha: GridCell's unification)

The objects this ctor builds with `New_GridCell()` are typed as what they
are: `UnkSlotEntry_3ac78::cellParent` is `struct GridCell *` and the
cell-building local `obj` is `GridCell *` (include/GridCell.h), and the
local `extern GenericObject *New_GridCell(void)` is gone. The generic
slots resolve to SceneNode's: `slot4C` is `attachToParent` (the cellParent
attached to the StageMap at `origin`, each cell attached to the
cellParent at `buf`), `slot70` is `setLightMode(obj, 1)`, and `unk10` is
`attribute`. Casts at the two attachToParent calls (`(SceneNode *)`,
`(LongVec3 *)`) emit no code; image byte-identical. `cells` is still
declared `StageMap **` although it holds these same GridCell objects:
that field, NotifyGridCell's parameter and StageMap__DispatchToRectCells
are StageMap's own track 4 job (gStageMapMethods) and were left alone.

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/scene_node.h),
`EntryDesc866E8` is `Ratio16[3]` (include/scene_node.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

### History moved from the unit banners

The three units' banners were rewritten as documentation (track 6). What they carried that was history, verbatim:

`src/class_3ac78.c`:

```
/*
 * class_3ac78 -- TimedTask's last two functions (TimedTask__PlaySound and
 * GetTimedTaskMethods, include/TimedTask.h; the rest are in class_39e08),
 * then the front half of StageMap, the class whose method table is
 * gStageMapMethods (80 slots, header 0x114; tools/classtable.py gStageMapMethods). It
 * derives from SceneNode (code_d294) through LightRig (include/LightRig.h,
 * gLightRigMethods: the three flat lights and the ambient colour), whose ctor
 * and finalize its own chain to, and the game builds exactly one, at boot, in class_39e08's DayTask__DayTask via New_StageMap(0, 1).
 *
 * What it manages is a GRID. The object owns seven elements (elems[7]), each
 * pairing a loader, a placement list, a parent node, and a 0x668-byte heap
 * block holding that element's grid of GridCell cells; the constructor seeds every
 * cell with a world position on a 0x800 lattice. Indexing the grid uses a row
 * stride of 20 cells -- the same 20 that gDefaultGridSpan >> 11 produces
 * (0xA000 / 0x800, see StageMap__SetGridSpan) and the same stride
 * class_3bb8c_b's byte-matched StageMap__SetFootprintVisible walks.
 *
 * Work reaches the cells through a rectangle list (rects[4]/rectCount): a
 * notification arrives at StageMap__OnNotify or StageMap__DispatchLinkCommand,
 * StageMap__ForwardAcceptedCommand filters the sender against acceptedTags,
 * StageMap__ApplyToSenderFootprint turns the sender's position into one
 * rectangle, and StageMap__DispatchToRectCells re-notifies every cell in it
 * and every cell chained behind it. The queries that build those rectangles,
 * and an element's resource and GPU sides, live in class_3bb8c*. The class is
 * declared once, in include/StageMap.h (track 4, round 89).
 *
 * Every function in the unit is matched C; the last three stalls
 * (StageMap__UnloadAllSlots, StageMap__SetFootprintRect and
 * StageMap__DispatchToRectCells) were matched in round 71. StageMap__NoOpSlotD8 keeps its placeholder name
 * deliberately -- it is an empty vtable stub with no established purpose, the
 * same case as SceneNode__NoOpSlot5C in code_d294_b.
 */
```

`src/class_3bb8c.c`:

```
/* First slice of the 365-function class_3bb8c block -- 20 functions,
 * 0x3BB8C..0x3CD88, all matched C, occupants of `gStageMapMethods` +0x0E4..+0x11C
 * (`tools/classtable.py 0x800866E8`). The remainder is `class_3bb8c_b` and
 * is still a monolithic asm segment.
 *
 * This slice is StageMap's FOOTPRINT/RATE engine: the position-to-grid-cell
 * math and the per-element resource/GPU work that `class_3ac78`'s own unit
 * header (src/class_3ac78.c) describes as living in "class_3bb8c*" --
 * StageMap__ComputeFootprintDescriptor converts a world position
 * (SplitLongVec3) into a grid-cell descriptor (Descriptor10, byte row/column
 * plus sub-cell halfword offsets); StageMap__UpdateFootprintTracking runs
 * every enabled tick (paired with class_3ac78's StageMap__StepScaleRamp)
 * to refresh that descriptor and notify on change; StageMap__LoadChunksAround
 * / StageMap__ComputeNeighbourMask / StageMap__ComputeChunkLoadEntry /
 * StageMap__ApplyChunkLoads build and apply a per-element rate table from
 * a ChunkSlotSpec key/flag array (sDefaultTargetSpecs); and
 * StageMap__PopulateSlotCells / StageMap__ClearSlotCells own an
 * element's resource-load and GPU-link cell array (the same 0x668-byte grid
 * class_3ac78 calls out) and its teardown. StageMap__Enable/Disable set
 * the `enabled` flag class_3ac78 gates all of this on (cross-confirmed
 * there independently, see docs/match-reports/StageMap__Enable.md).
 *
 * The class is declared once, in include/StageMap.h (track 4, round 89);
 * the element's origin is read through SplitCoord2 (include/class_3bb8c.h), a
 * GsCOORDINATE2 view with halfword reads.
 *
 * Carve notes for whoever takes the NEXT slice: this block holds all 13 of
 * the game's PSX BIOS trampolines (`jr $t2` with the vector in $t2 and the
 * call number in $t1) and 38 switch jump tables. None of either landed in
 * THIS slice -- verified, not assumed -- which is why it needs no attached
 * rodata slot and no `hasm` segment. The next slice will hit both, and both
 * have to be dispositioned at carve time rather than discovered by a runner
 * that has already spent its attempt budget. See Gate 2 in
 * docs/PARALLEL-RUNS.md.
 */
```

`src/class_3bb8c_b.c`:

```
/* Second slice of the 365-function class_3bb8c block, 0x3CD88..0x3DA54 --
 * the same class as class_3bb8c.c's first slice (StageMap,
 * gStageMapMethods, declared in include/StageMap.h), split only for
 * parallel runners, so this unit reuses that unit's header (same convention
 * as Entity.c/Entity_b.c).
 *
 * Functionally this slice is the class's SPATIAL GRID / FOOTPRINT
 * subsystem: the seven elements (self->slots), each mapped onto up to four
 * CellRect rectangles (self->rects), and a per-cell bit (bit 31 of a
 * GridCell cell's `attribute`) that RefreshFootprint clears, recomputes
 * (via either ComputeFootprintFromRotation or SetFootprintFromQuery, gated
 * on self->config->isVertical) and sets again through SetFootprintCellFlag. A
 * second, unrelated mechanism lives at the tail of the unit: a rate/
 * countdown pair (self->scaleRampTicks/self->scaleStep) that
 * StepScaleRamp/EndScaleRamp apply to every cell of every element
 * (their updateScale) via the generic ForEachSlot/ForEachSlotCell
 * iterators.
 *
 * "Footprint" is not this unit's own coinage: class_3ac78 already named the
 * analogous mechanism there (StageMap__ApplyToSenderFootprint,
 * SetFootprintRect, SetFootprintFromCell) before this unit's naming pass,
 * and this unit's names were chosen to agree with that vocabulary. */
```

## Track 7 (2026-09-27, round 98, charlie)

Zero-byte polish, each step verified whole-image:
- the cell walk indexes `GridCell **` (`cells`, `cursor`, `end = cells + STAGE_SLOT_CELLS`) instead of a `u8 *` stepped by 4; the round-67 staging fix above (`end` computed from `cells` before `cursor = cells`) still applies and now carries a `MATCHING:` line;
- the lattice position is a `LongVec3 pos` instead of `s32 buf[3]` cast to `LongVec3 *`;
- measured this round: dropping the two reloads `cell = *cursor` (reusing the `New_GridCell()` result) breaks the image, so they carry a `MATCHING:` line;
- constants: `0x668` -> `STAGE_SLOT_CELLS * sizeof(GridCell *)`, `0x400` -> `STAGE_CELL_SIZE / 2`, `0x800` -> `STAGE_CELL_SIZE`, `0xA400` -> `STAGE_CHUNK_SIZE + STAGE_CELL_SIZE / 2` (all include/StageMap.h), `0x80000000` -> Sony's `GsDOFF`, the `setLightMode` 1 annotated `GsFOG` (SceneNode__SetLightMode's 3-bit field at bit 3, as in entity.c and ObjMStyleActor.c), `i < 7` -> `ARRAY_COUNT(self->slots)`, `acceptedTags = 0` -> `NULL`;
- `BMemPMgrAlloc`/`BMemPMgrFree` come from include/bmem_pmgr.h; the unit's local externs and their comment ("all still-uncarved elsewhere, typed purely from this call site's own register usage", stale since bmem_pmgr.c was carved) are gone.
- the in-code comment on the wrap test records the 21-positions-per-row fact from "The 21-vs-20 discrepancy" above, without a reading of it.

## History (moved from include/class_3bb8c.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * class_3bb8c.h -- the data and shared helper declarations of the units
 * carved from the old class_3bb8c segment. The classes those units hold
```
