# StageMap__FindSlotIndexByChunk — MATCHED (20/20 words)

> Renamed from `StageMap__FindElemIndexByUnk30` on 2026-09-26 (tools/rename.py). Address 0x8004c5d0.

> Renamed from `Class866E8__FindElemIndexByUnk30` on 2026-09-26 (tools/rename.py). Address 0x8004c5d0.

> Renamed from `func_8004C5D0` on 2026-09-24 (tools/rename.py). Address 0x8004c5d0.

A vtable slot in `gStageMapMethods` (`+0x124`, per `tools/classtable.py`), and
already independently visible from `class_39e08.h`'s own view of the same
table — its own comment types the occupant as `void
*(*slot124)(StageMap *self, void *arg1)`. Reading THIS unit's actual
occupant body shows that typing is likely wrong: the function returns a
plain `s32` (an array index, or `-1`), not a pointer, and its second
argument is compared directly against a signed 16-bit struct field (a
key), not dereferenced as a pointer anywhere. Per parallel-mode rules this
unit does not edit `class_39e08.h` (a different unit's own file) — flagged
here for the head to reconcile; see "Proposed learning" below.

## Disassembly

```
move a2, zero              ; i = 0
li   a3, 0xEC               ; running byte offset
.loop:
addu v0, a0, a3              ; v0 = &self->arr[i]
lw   v1, 0x4(v0)              ; v1 = arr[i].unk4
lh   v0, 0x30(v1)              ; v0 = arr[i].unk4->unk30 (signed)
bne  v0, a1, .next               ; if != key, skip second check
 nop
lh   v0, 0x2c(v1)                 ; v0 = arr[i].unk4->unk2C (signed)
bnez v0, .found                    ; if nonzero, return i
 move v0, a2                        ; delay slot: v0 = i (meaningful only if branch taken)
.next:
addiu a2, a2, 1
slti v0, a2, 7
bnez v0, .loop
 addiu a3, a3, 0x1c
li   v0, -1
.found:
jr ra
 nop
```

Equivalent to `if (unk30 == key && unk2C != 0) return i;` per iteration,
falling through to `return -1;` if the loop exhausts.

## Residue and how it closed (1 residue, 2 attempts)

Same class as `StageMap__FindSlotIndexByNeighbour`'s (own report, matched immediately before
this one in the queue): the plain indexed form
(`self->arr[i].unk4->unk30`/`unk2C`) let GCC repurpose `self` as its own
moving pointer, dropping the separate running-offset register retail
keeps. An explicit `Elem *e = &self->arr[i];`, reused for both field
reads, closed it on the second attempt.

## Final C

```c
s32 StageMap__FindSlotIndexByChunk(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk30 == key && e->unk4->unk2C != 0) {
            return i;
        }
    }
    return -1;
}
```

## New struct knowledge

- `ElemTarget::unk2C` (`s16`, +0x02C) — nonzero-tested, second condition.
- `ElemTarget::unk30` (`s16`, +0x030) — compared against `key`, first
  condition. Sits directly before the already-established `unk32`
  (+0x032) with no gap.

## Attempts

2 (see residue above).

### Proposed learning

**`class_39e08.h`'s independent typing of `gStageMapMethods` slot `+0x124`
(`void *(*slot124)(StageMap *self, void *arg1)`) does not match this
occupant's own body** — the occupant returns `s32` (an index or `-1`) and
never dereferences `arg1`, only compares it against a signed 16-bit field.
This is the same trap the project's "a discarded return value/empty-bodied
occupant is never evidence of a slot's true signature" guidance already
warns about, in the opposite direction: a *guessed* signature from the
caller side, never checked against the occupant's own disassembly. Since
`class_39e08.h` belongs to a different unit under this round's parallel
rules, this is left for the head to reconcile rather than edited directly
— flagged explicitly per the runner brief's shared-vtable-slot caution.

## Naming

**Tier A.** Vtable slot +0x124. Sibling search leaf to
`StageMap__FindSlotIndexByNeighbour`: same loop shape, different field
(`unk4->unk30`) and an extra `unk4->unk2C != 0` gate, returns -1 (not 0)
on a miss. Named the same way and for the same reason -- by the field it
searches, since `unk30`/`unk2C` are cross-unit `ElemTarget` fields this
unit does not own (also read by `StageMap__OnNotifyTag1`/`StageMap__ClearSlotCells` in
class_39e08.c).

## Proposed field names

**Head, round 76: NOT APPLIED.** The set is internally inconsistent: `unk30` and `unk32` cannot both be `key`, and `include/class_3bb8c.h`'s view records `unk30` as a raw rate that StageMap__ComputeFootprintDescriptor sign-extends, so equality-compared-here is not enough for `key`. `unk2C -> enabled` rests on "nonzero enables" alone. Re-propose from the base class's naming pass (class_39e08.c), where all readers are in one unit.

`ElemTarget::unk30` and `ElemTarget::unk2C` (also read by class_39e08.c's
`StageMap__OnNotifyTag1`/`StageMap__ClearSlotCells` -- cross-unit, not renamed here).
Proposed: `unk30` -> `key` (same reasoning as `unk32` above -- compared
for equality against this function's own `key` argument);
`unk2C` -> `enabled` (gates this function's match with a nonzero test,
and class_39e08.c's `StageMap__OnNotifyTag1` gates its own dispatch on the same
field the same way -- a plain "is this target live" flag is the simplest
reading that fits both call sites, though neither establishes it beyond
"nonzero enables").

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

This function: `StageMap__FindElemIndexByUnk30` -> `StageMap__FindSlotIndexByChunk` (`python3 tools/rename.py StageMap__FindElemIndexByUnk30 StageMap__FindSlotIndexByChunk`, tier A): a lookup leaf: the index of the loaded slot whose `loader->chunkIndex` is the key, or -1.

## Round 96 (track 7, delta)

Parameter `key` -> `chunkIndex` (compared with LbdFile::chunkIndex; also
the prototype and slot +0x124 in StageMap.h), `e` -> `slot`; loop bound
`ARRAY_COUNT(self->slots)`. Zero bytes.
