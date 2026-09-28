# StageMap__OnSlotEvent — MATCH

> Renamed from `StageMap__OnElementEvent` on 2026-09-26 (tools/rename.py). Address 0x8004aa6c.

> Renamed from `Class866E8__OnElementEvent` on 2026-09-26 (tools/rename.py). Address 0x8004aa6c.

> Renamed from `func_8004AA6C` on 2026-09-22 (tools/rename.py). Address 0x8004aa6c.

**Unit:** DayTaskStageMap · **Size:** 46 instructions · **Result:** 46/46 words

## What it does

`StageMapMethods` slot `+0x088`. Calls the same external, not-decompiled
table lookup as `StageMap__OnNotify` (`GetSceneNodeMethods(self, arg1)`), but through
that table's slot `+0x088` with only `(self, arg1)` (a genuinely 2-arg
call — `$a2` is left live/unset). Then:

- if `arg1 == 6`: possibly refresh `arg2->unk14` through its own base-class
  slot `unk04` (`GenericMethodsHeader::unk04`, the same `BasicClass` slot
  StageMapMethods documents as `+0x004`/`BasicClass__Release`), then
  fall into the shared tail;
- if `arg1 == 7`: skip straight to the shared tail;
- any other `arg1`: return immediately, skipping the tail entirely.
- shared tail: `self->unk1BC = arg2;` then `self->methods->slot30(self,
  arg1)` — **note this passes `arg1` (the tag), not `arg2`, to slot30.**

## Final source

```c
void StageMap__OnSlotEvent(StageMap *self, s32 arg1, UnkListObj_3ac78 *arg2)
{
    void (*fn)(StageMap *self, s32 arg1);

    fn = *(void (**)(StageMap *, s32))((u8 *)GetSceneNodeMethods(self, arg1) + 0x88);
    fn(self, arg1);

    if (arg1 == 6)
        goto handle6;
    if (arg1 == 7)
        goto merge;
    return;

handle6:
    if (arg2->unk14 != NULL) {
        arg2->unk14 = arg2->unk14->methods->unk04(arg2->unk14);
    }

merge:
    self->unk1BC = arg2;
    self->methods->slot30(self, arg1);
}
```

New struct knowledge: `UnkListObj_3ac78` (opaque, only field `+0x14`
known — a `GenericObject *`), `self->unk1BC` (`UnkListObj_3ac78 *`),
`StageMapMethods::slot30` (`self, s32`), and `GenericMethodsHeader::unk04`
(a `void *(*)(void *self)` base-class slot, generalized from
`StageMapMethods`'s own `+0x004` comment since the same offset/shape
shows up on an unrelated class here).

## Residue and the fix that closed it

Two independent mistakes on the first attempt (33/46):

1. **Wrong argument to the tail call.** First draft passed `arg2` to
   `self->methods->slot30(self, arg2)`; retail's `move a1,s0` at the very
   end is `s0` (== `arg1`, the tag), not `s2` (== `arg2`). Caught by
   diffing the LAST instruction pair, which otherwise looked identical.
2. **Branch structure.** Writing the three-way dispatch as
   `if (arg1==6) {...} else if (arg1!=7) return;` compiled to an inverted
   `bne`/fallthrough shape (retail: `beq 6,label; beq 7,label2; j end` —
   three independent forward branches, none of them an else-arm of
   another). Rewriting as three literal `if`+`goto` statements matching
   retail's own branch-per-comparison shape fixed it outright.

### Proposed learning

**A three-way dispatch on independent equality checks (`beq`, `beq`,
unconditional `j`) is not the same control-flow graph as an
`if`/`else-if` chain**, even when the taken branches end up at the same
places. Write it as literal sequential `if (x==A) goto L1; if (x==B) goto
L2; return;` when retail's branches are three independent tests rather
than a cascading `bne`-chain — this is the same "write the literal jump
graph" lesson as the DECOMPILATION_LEARNINGS cross-jump/tail-merge note,
just for forward dispatch instead of a shared tail.

## Provenance

round 2026-09-02, runner ALPHA, unit DayTaskStageMap. First attempt 33/46 (two
distinct residues); second attempt closed both together, 46/46.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AA6C` | `StageMap__OnSlotEvent` | B | Occupant of vtable slot `+0x088`. Three call sites agree on the argument shape `(self, command, elem, index)`: `StageMap__Finalize` and `StageMap__UnloadAllSlots` here (command 6), and `DayTaskStageMap`'s `StageMap__OnDrawSystemEvent` (command 7). The body forwards to the BASE table's `+0x088` -- `SceneNode__NotifyWithHull`, which itself gates on a command code in `{2,3}` -- then, for command 6, releases the element's own held object, and for 6 and 7 records the element in `lastEventElem` and re-emits with `notifyParents(self, command)`. So: an element-scoped event arrives, is handled, and is passed up. Tier B: the mechanics are complete, what commands 6 and 7 MEAN in the game is not established. |

Type correction made this round, byte-neutral and oracle-verified: the third
parameter was typed `UnkListObj_3ac78 *` (this unit's SENDER type). All three
callers pass an element, and the field this function reaches is `+0x014` --
`UnkSlotEntry_3ac78`'s own, zeroed by the ctor. Retyped to
`UnkSlotEntry_3ac78 *elem`, and `UnkSlotEntry_3ac78::unk14` from `s32` to
`GenericObject *heldObj`, since the body dereferences it as an object with a
vtable at `+0x000`. Callers checked: `StageMap__Finalize`,
`StageMap__UnloadAllSlots` (both in this unit, both pass `&self->elems[i]`),
and `DayTaskStageMap`'s `StageMap__OnDrawSystemEvent` (passes `Elem *entry`). Whole-image SHA1
green after the change.

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `UnkSlotEntry+0x014` | `heldObj` | B | Released here through the shared BasicClass `release` slot with the store-back shape `x = x->methods->release(x)`, and zeroed by the ctor. The name claims only that the entry holds an object it owns. |
| `StageMap+0x1BC` | `lastEventElem` | B | Written only here, with the element the event concerned. `DayTaskStageMap`'s `StageMap__GetLastEventSlotChunk` reads `unk1BC->unk4` and treats it as an `ElemTarget *`, which is exactly `Elem::unk4` -- independent confirmation that the pointer is an element. |

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

This function: `StageMap__OnElementEvent` -> `StageMap__OnSlotEvent` (`python3 tools/rename.py StageMap__OnElementEvent StageMap__OnSlotEvent`, tier B): +0x088's occupant: SceneNode's notifyWithHull, then event 6 (a slot unloading) releases the slot's heldObj, and 6 or 7 (its data block read) records it in `lastEventSlot` and notifies parents.

## Track 7 (2026-09-27, round 98, charlie)

Events `6`/`7` -> `STAGEMAP_EVENT_SLOT_RELEASE`/`STAGEMAP_EVENT_SLOT_DATA_READY` (include/StageMap.h); labels `handle6`/`merge` -> `release`/`record`; parameter `elem` -> `slot` (the header prototype's name). The `if`+`goto` shape carries a `MATCHING:` line (Residue 2 above). Zero bytes.
