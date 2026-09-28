# StageMap__FindSlotByNeighbour — MATCHED (15/15 words)

> Renamed from `StageMap__FindElemByUnk32` on 2026-09-26 (tools/rename.py). Address 0x8004c434.

> Renamed from `Class866E8__FindElemByUnk32` on 2026-09-26 (tools/rename.py). Address 0x8004c434.

> Renamed from `func_8004C434` on 2026-09-24 (tools/rename.py). Address 0x8004c434.

Not a vtable slot (not in `gStageMapMethods`), a plain non-virtual helper —
companion to `StageMap__CountPendingLoads`, walking the same `self->arr` array but
searching a different field.

## Disassembly

```
addu  $a3, $zero, $zero      ; i = 0
li    $a2, 0xEC              ; running byte offset, starts at arr's own offset
.loop:
addu  $v1, $a0, $a2          ; v1 = &self->arr[i]
lw    $v0, 0x4($v1)          ; self->arr[i].unk4
nop
lh    $v0, 0x32($v0)         ; self->arr[i].unk4->unk32 (SIGNED halfword)
nop
beq   $v0, $a1, .found
 addu $v0, $v1, $zero        ; delay slot: v0 = &self->arr[i] (only meaningful if branch taken)
addiu $a3, $a3, 0x1          ; i++
slti  $v0, $a3, 0x7
bnez  $v0, .loop
 addiu $a2, $a2, 0x1C        ; offset += 0x1C
.found:
jr $ra
 nop
```

On the "not found" (loop exhausted) path, `$v0` is whatever the last
comparison's `lh` produced — not a pointer. Retail's own source has no
explicit statement for that path either; see below.

## Final C

```c
Elem *StageMap__FindSlotByNeighbour(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            return e;
        }
    }
}
```

No `return` after the loop — C89 permits falling off the end of a
non-`void` function (the caller's use of the result is then undefined
behavior, but the function itself compiles exactly as retail's own body
does: whatever's left in `$v0` from the last executed statement). This is
the natural, direct reproduction; anything else (an explicit `return NULL;`
or `return e;` after the loop) would add a instruction retail doesn't have.

**Head confirmation (round 7): that prediction was tested, not just
reasoned.** Rewriting the loop as `break;` plus a single terminal
`return e;` — the form that removes the undefined fall-through — drops the
score to 6/15 and adds exactly one instruction, `addu $v0, $a3, $zero`, at
`0x8004C46C` where retail has `nop`. So the UB-shaped body IS the faithful
reconstruction and must stay.

Worth recording what retail's not-found path actually does, since it is an
artifact rather than an intent: `addu $v0, $v1, $zero` sits in the `beq`'s
delay slot, so it executes on *every* iteration, and on loop exhaustion
`$v0` is left holding `&self->arr[6]` — the last element, not null. Any
caller relying on a null return would be relying on something retail does
not do. The original source almost certainly had no terminal return at all,
with the loop presumed always to find its key.

## Residue and how it closed (2 attempts)

First attempt typed `ElemTarget::unk32` as `u16`, producing `lhu` where
retail has `lh` (signed halfword load) — 14/15. Retyped to `s16`, matching
the SAME residue class already seen this round (`StageMap__FindSlotByNeighbour`'s own
comparison against a plain `s32 key`, decoded as signed). 15/15.

## New struct knowledge (`include/class_3bb8c.h`)

- `Elem::unk4` (`ElemTarget *`, +0x004) — a pointer to another object.
- New opaque type `ElemTarget`, only field known: `unk32` (`s16`, +0x032).

## Attempts

2 (see residue above).

### Proposed learning

None new — same signed/unsigned halfword lesson already documented
elsewhere this project (`TimedTask__CheckTimeout`, `dream_day`).

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C434` | `StageMap__FindSlotByNeighbour` | A | Occupant of `gStageMapMethods` +0x118 (`slot118`). Pure linear search: loops `self->arr[7]`, returns the first `Elem *` whose `unk4->unk32 == key`. Named to parallel the already-matched sibling `StageMap__FindSlotIndexByNeighbour` (+0x120), which searches the SAME field (`ElemTarget::unk32`) but returns an index rather than the element pointer -- consistent family naming for two functions doing the identical field comparison with a different return shape. A pure search-and-return is tier A by the "getter" clause. |

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
the per-stage config. Header now `include/stage_map.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/stage_grid.h), `Unk54Struct` is `LongVec3` (include/scene_node.h),
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

This function: `StageMap__FindElemByUnk32` -> `StageMap__FindSlotByNeighbour` (`python3 tools/rename.py StageMap__FindElemByUnk32 StageMap__FindSlotByNeighbour`, tier B): returns the slot whose `loader->elemKey` is the key; LoadChunksAround sets elemKey to the slot's neighbour key (0..6).

## Track 7 (2026-09-27, round 95, charlie)

Parameter `key` -> `neighbour` (a neighbour key, compared with `loader->elemKey`), `e` -> `slot`; loop bound 7 -> `ARRAY_COUNT(self->slots)`.
