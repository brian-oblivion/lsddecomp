# StageMap__OnNotify — MATCH

> Renamed from `Class866E8__OnNotify` on 2026-09-26 (tools/rename.py). Address 0x8004a984.

> Renamed from `func_8004A984` on 2026-09-22 (tools/rename.py). Address 0x8004a984.

**Unit:** class_39e08 · **Size:** 35 instructions · **Result:** 35/35 words

## What it does

`StageMapMethods` slot `+0x038`. Looks up an external table via
`GetSceneNodeMethods(self, arg1)` (not decompiled anywhere yet, in an uncarved
segment) and calls that table's OWN slot `+0x038` with `(self, arg1,
arg2)`. Then, if `arg1`'s own vtable header's low nibble is `1` (the
family/base-class tag pattern from `docs/research/class-framework.md`,
already used by `StageMap__DispatchLinkCommand` in this unit), also calls
`self->methods->slot100(self, arg1, arg2)`.

Note this does NOT recurse into itself: the table returned by
`GetSceneNodeMethods` is a *different* class's vtable from `self->methods`
(`gStageMapMethods`) — slot `+0x038` there happens to be `StageMap__OnNotify` (this
very function) only in `StageMapMethods`, not necessarily in whatever
`GetSceneNodeMethods` returns.

## GetSceneNodeMethods's inferred signature

Not decompiled (lives in the still-uncarved `asm/SceneNode.s`). Its second
parameter is passed as a plain `s32` from `StageMap__OnSlotEvent` (compared there
against small integer literals 6/7 — not a pointer-shaped use), so it's
declared here as `extern void *GetSceneNodeMethods(StageMap *self, s32
arg1);` and this function casts its own `GenericObject *arg1` to `s32` at
the call site. The cast is a no-op at the machine level (both are 32-bit
register values) and costs nothing.

## Final source

```c
extern void *GetSceneNodeMethods(StageMap *self, s32 arg1);

void StageMap__OnNotify(StageMap *self, GenericObject *arg1, s32 arg2)
{
    void (*fn)(StageMap *self, GenericObject *arg1, s32 arg2);

    fn = *(void (**)(StageMap *, GenericObject *, s32))
        ((u8 *)GetSceneNodeMethods(self, (s32)arg1) + 0x38);
    fn(self, arg1, arg2);

    if ((arg1->methods->header & 0xF) == 1) {
        self->methods->slot100(self, arg1, arg2);
    }
}
```

New struct knowledge: `StageMapMethods::slot100` (called here, dispatched
by `self`), typed `(StageMap *, void *, s32)`.

## Residue

None — matched on the first attempt, no reshaping needed. The raw
pointer-cast call through `GetSceneNodeMethods`'s return is ugly but necessary:
the returned table's class is unknown (only the one slot at `+0x38` this
function reaches is typed), so it cannot reuse `StageMapMethods` even
though the numeric offset happens to coincide.

### Proposed learning

**A helper function's second argument register can carry genuinely
different C types across different call sites in the same unit** (here,
`GetSceneNodeMethods`'s arg1 is a small `s32` tag at one call site and a
`GenericObject *` at another) with no compile-time conflict, as long as
the shared `extern` declaration picks ONE parameter type (word-sized) and
callers cast to it. The cast costs zero instructions on this architecture
since pointers and `s32` are both 32-bit registers.

## Provenance

round 2026-09-02, runner ALPHA, unit class_39e08.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A984` | `StageMap__OnNotify` | A | Occupant of vtable slot `+0x038`, which `include/code_8220.h` establishes as `BasicClassMethods::slot38` / `onNotify` -- the RECEIVING half of `+0x030 notifyParents`, with `(self, sender, event)`. The body is the standard override shape: call the base table's own `+0x038` with the same three arguments, then branch on the SENDER's class tag (`sender->methods->header & 0xF`). `SceneNode__OnNotify` in `SceneNode` is the same shape one class up. |

Parameters renamed from the evidence: `arg1` -> `sender`, `arg2` -> `command`
(`code_8220.h` calls the pair sender/event; this class's own numbering is
described in `StageMap__ForwardAcceptedCommand.md`).

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
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
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

## Track 7 (2026-09-27, round 98, charlie)

Moved here from the `.c` comment above this function, which read: "GetSceneNodeMethods: include/SceneNode.h. Round 59 measured the two arguments these calls used to pass the no-argument getter as zero-cost (the jal's delay slot holds a callee-save spill); track 4 dropped them."

The comment now says what the test does: a sender whose class id's low nibble is 1 is DrawSystem or below it (`classtable.py --scan`: gDrawSystemMethods, 0x1, is the only table with root nibble 1). `0xF` and `1` stay literals in this branch because `CLASS_ID_ROOT_MASK` (BasicClass.h) and `DRAWSYSTEM_CLASS_ID` (DrawSystem.h) landed on main this round after this branch forked; proposed to the head: `(sender->methods->header & CLASS_ID_ROOT_MASK) == DRAWSYSTEM_CLASS_ID`.
