# StageMap__ForEachSlotCell — MATCHED 29/29

> Renamed from `StageMap__ForEachEntryChild` on 2026-09-26 (tools/rename.py). Address 0x8004d1d0.

> Renamed from `Class866E8__ForEachEntryChild` on 2026-09-26 (tools/rename.py). Address 0x8004d1d0.

> Renamed from `func_8004D1D0` on 2026-09-24 (tools/rename.py). Address 0x8004d1d0.

**Status: MATCHED**, whole-image SHA1 green. Closed by the head in round 9
(2026-09-02), same round the runner stalled it.

Walks `item->unk10[]` (an array of `EntryChildObj *`, `0x668` bytes /
`0x19A` elements from the base pointer read at `item->unk10`), calling
`callback(self, element)` for each. Prototype came from a previous round
(traced through `StageMap__AddScaleStepToCell`/`StageMap__ResetCellScale` two hops via
`StageMap__ForEachSlot`); this round supplied the body.

## RESOLUTION — mention the field twice, bound first

```c
void StageMap__ForEachSlotCell(Obj866E8 *self, void (*callback)(Obj866E8 *self, EntryChildObj *item), Elem *item) {
    EntryChildObj **p;
    EntryChildObj **end;

    end = item->unk10 + (0x668 / 4);
    p = item->unk10;
    for (; p < end; p++) {
        callback(self, *p);
    }
}
```

The only difference from the runner's 1-word-short body is the **order and
the double mention**: compute the BOUND from `item->unk10` first, then the
loop pointer from `item->unk10` again — rather than caching the field in `p`
and deriving `end` from `p`. GCC's CSE still emits a single `lw`, but this
write order is what makes it materialise the extra `move $s0, $v0` retail
has, instead of folding the loop variable into the loaded register.

## Why this stalled, and it is the head's fault not the runner's

This exact lever was **derived last round** by runner charlie on
`StageMap__ClearSlotCells`, in this same class block, and written up in that function's
`### Proposed learning` — "try mentioning the SAME source expression TWICE, in
the order [dependent-quantity-first, loop-variable-second]". **The head did
not promote it into `DECOMPILATION_LEARNINGS.md` during round-8
consolidation.** So the runner could not have read it in the place it was
supposed to be, went instead to MATCHING-GUIDE's "best-posed permuter target"
line for the redundant-`move` class, and correctly followed that advice by
declining to burn attempts.

Two things follow, and the second is the more important:

1. The runner's process was right. Its two attempts were the sensible ones,
   its residue reading was exact, and stopping was the documented behaviour.
2. **An unpromoted learning is a learning that does not exist.** Round 8's
   consolidation promoted eight idioms and missed this one; the cost showed up
   one round later as a stall on a function four addresses away from where the
   lever was found. Consolidation is not bookkeeping.

## The residue, for the record

Retail:

```
lw   $v0, 0x10($a2)     ; v0 = item->unk10
nop
move $s0, $v0            ; the move that only the double mention produces
addiu $s1, $s0, 0x668
```

The runner's 1-word-short version loaded straight into `$s0` and skipped the
`move`. Everything else in the function, including the loop and the `jalr`
through the callback, was already byte-exact — which is what made this a
one-instruction question rather than a shape question.

Note the DIRECTION. Round 8's broadcast lever was about removing a redundant
`move` your source restates. This is the constructive opposite: retail
genuinely has the extra `move` and the source has to be written so the
compiler needs it. Both are the same underlying rule — the number of times the
source mentions a value decides whether a copy materialises.

## Attempts

1. `p = item->unk10;` directly — 1 word short (missing the `move`).
2. Introduced an explicit intermediate: `base = item->unk10; p = base;`
   — no change; cc1 still loads `item->unk10` directly into whichever
   register `p` occupies, no intervening `move` materializes.

Per the documented guidance, did not burn further attempts chasing this
— restored to `INCLUDE_ASM`.

## Best-attempt body (inline, `#if 0`)

```c
#if 0
void StageMap__ForEachSlotCell(Obj866E8 *self, void (*callback)(Obj866E8 *self, EntryChildObj *item), Elem *item) {
    EntryChildObj **p;
    EntryChildObj **end;

    p = item->unk10;
    end = p + (0x668 / 4);
    for (; p < end; p++) {
        callback(self, *p);
    }
}
#endif
```

## Attempts

2 (see above; deliberately did not chase further per the documented
"best-posed permuter target, do not spend budget re-deriving it"
guidance).

### Proposed learning

None new — this is a fresh, independent instance of the already-documented
redundant-`move` class (third or later occurrence project-wide), which
strengthens rather than changes that existing entry. Worth noting for
whoever eventually tackles the permuter target: the redundant `move` here
sits immediately after a `lw` from a function ARGUMENT (`item->unk10`,
`item` being `$a2`), which is a slightly different shape from prior
instances (worth checking if the pattern is specific to loads off
incoming arguments vs. `self`-relative loads).

## Naming

**Tier A.** Not a vtable slot -- walks one `Elem`'s `unk10[]`
`EntryChildObj*` array (`0x668` bytes from the base) invoking a callback
per entry. Same reasoning as `StageMap__ForEachSlot`: a generic iterator
whose name is its mechanics.

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

This function: `StageMap__ForEachEntryChild` -> `StageMap__ForEachSlotCell` (`python3 tools/rename.py StageMap__ForEachEntryChild StageMap__ForEachSlotCell`, tier A): an iterator over one slot's 410 cells.

## Round 96 (track 7, delta)

`callback` -> `cellFn`, `item` -> `slot`, `p` -> `cell`; `0x668 / 4` ->
`STAGE_SLOT_CELLS` (StageMap.h, 410: the lattice and its 10 overflow cells).
