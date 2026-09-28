# StageMap__SetFootprintVisible — MATCHED (97/97 words)

> Renamed from `StageMap__SetFootprintCellFlag` on 2026-09-27 (tools/rename.py). Address 0x8004ce24.

> Renamed from `Class866E8__SetFootprintCellFlag` on 2026-09-26 (tools/rename.py). Address 0x8004ce24.

> Renamed from `func_8004CE24` on 2026-09-24 (tools/rename.py). Address 0x8004ce24.

The largest function attempted this round. Iterates `self->slots8C[0..
self->unk88-1]`; for each slot, resolves `self->arr[slot->elemIdx]`, skips
it if `e->unk4->unk2C == 0`, then walks a sub-rectangle of `e->unk10`'s
pointer grid (row stride 20 cells, `slot->h4`/`h6` the starting column/row,
`slot->h8`/`hA` the sub-rectangle's width/height) setting or clearing bit
31 of `EntryChildObj::unk10` for every cell in the rectangle AND every
node in that cell's `unk38` singly-linked chain.

`self->slots8C` and `self->unk88` are the SAME memory `StageMap__InitFootprintRect`
(matched earlier this round, a different unit's function) writes via a
coarser `Unk54Struct` whole-block-copy view — this function establishes
the finer-grained field layout from the READ side.

## New struct knowledge (`include/class_3bb8c.h`)

- **`Obj866E8::unk88`** (`s32`, +0x088) and **`Obj866E8::slots8C`**
  (`CellRect[4]`, +0x08C) carved out of the previous
  `pad7C[0xBC-0x7C]`. The array's capacity (4) is not a guess: `0x8C +
  4*sizeof(CellRect)` (`0x8C + 4*0xC = 0xBC`) lands EXACTLY on the
  already-established `Descriptor10 unkBC` field with zero slack, so 4 is
  a hard ceiling, not an inferred one.
- New type **`CellRect`** (0xC bytes): `elemIdx` (`s32`, +0x0),
  `h4`/`h6`/`h8`/`hA` (`s16` each, +0x4/+0x6/+0x8/+0xA). This is a
  DIFFERENT, more granular view of the same memory `StageMap__InitFootprintRect`
  addresses as a flat `Unk54Struct` (3x `s32`) — kept as two independent
  views per the project's established convention (a whole-struct copy
  doesn't care about the internal layout it copies, so the coarser
  write-side view is left alone rather than retrofitted).
- **`EntryChildObj::unk38`** (`EntryChildObj *`, +0x038) — a
  singly-linked "next" pointer, walked while non-NULL. Extended
  `EntryChildObj`'s existing `pad1C[0x20-0x1C]`/`unk20` tail with a new
  `pad24[0x38-0x24]` before it.
- `EntryChildObj::unk10`'s existing comment (already `u32`, bit-31 OR'd by
  `StageMap__ClearSlotCells`) extended to note this function also touches it
  (set/cleared per its own `setBit` argument).

## Final C

```c
void StageMap__SetFootprintVisible(Obj866E8 *self, s32 setBit) {
    s32 i;
    s32 j;
    s32 k;
    CellRect *slot;
    Elem *e;
    EntryChildObj **cell;
    EntryChildObj *next;

    slot = self->slots8C;
    for (i = 0; i < self->unk88; slot++, i++) {
        e = &self->arr[slot->elemIdx];
        if (e->unk4->unk2C == 0) {
            continue;
        }
        cell = e->unk10 + slot->h4 + slot->h6 * 20;
        for (j = 0; j < slot->hA; j++) {
            for (k = 0; k < slot->h8; k++) {
                if (setBit != 0) {
                    (*cell)->unk10 &= 0x7FFFFFFF;
                } else {
                    (*cell)->unk10 |= 0x80000000;
                }
                next = (*cell)->unk38;
                while (next != 0) {
                    if (setBit != 0) {
                        next->unk10 &= 0x7FFFFFFF;
                    } else {
                        next->unk10 |= 0x80000000;
                    }
                    next = next->unk38;
                }
                cell++;
            }
            cell += 20 - slot->h8;
        }
    }
}
```

## Attempts (7)

1. First full-body attempt used array indexing (`slot = &self->slots8C
   [i]`) inside the loop and a cached `EntryChildObj *obj = *cell;` local
   for both the direct hit and the chain walk — 14 words too long (whole
   image shifted). Two separate residues, addressed one at a time below.
2. Switched from array indexing to an explicit incrementing pointer
   (`slot = self->slots8C; for (...; ...; slot++)`), matching how
   `StageMap__ForEachSlot`'s `&self->arr[i]` idiom generalizes to "increment a
   pointer" rather than "recompute `base + i*stride`" when the loop is
   this register-heavy — fixed most of the size gap (14 words -> 1 word).
3. Replaced the cached `obj` local with direct `(*cell)->...` dereferences
   for both the first hit and the chain-walk start (retail re-reads
   `*cell` for the chain rather than reusing the register already holding
   it) — this, combined with #2, got the loop BODY itself byte-exact,
   leaving only the `setBit`/mask branch's polarity and the outer loop's
   increment order.
4. The `if (setBit == 0) {AND} else {OR}` vs `{OR} else {AND}` mapping
   needed BOTH the correct value-per-branch mapping AND the correct
   branch-polarity/fallthrough-layout, and these are two independent
   axes (confirmed by trying 3 of the 4 combinations and getting either
   the right values with the wrong `beqz`/`bnez` instruction, or the
   right instruction with values swapped). The combination that matched
   both: `if (setBit != 0) { AND-mask } else { OR-mask }` — i.e. retail's
   `beqz $a1` target is the `setBit == 0` (OR) case, NOT the `setBit != 0`
   case as the naming might suggest; and the `!=` spelling (not `==`) was
   needed to land the OR case in the branch target rather than the
   fallthrough.
5. Same fix applied to the second (chain-loop) copy of the same
   if/else — needed independently since it's a separate statement, not
   shared code.
6. With the branch fixed, one word remained: the loop-continue path
   (`continue;` when `e->unk4->unk2C == 0`, and the two `h8`/`hA` early
   skips) advanced the "slot" pointer register (`a3` in retail, an
   internal alias `slot+0xA` GCC introduced for the `h4..hA` field reads)
   BEFORE incrementing the loop counter `i`, while my `for (i = 0; i <
   self->unk88; i++, slot++)` produced the opposite order.
7. Swapping the increment-clause order to `for (i = 0; i < self->unk88;
   slot++, i++)` — byte-exact, 97/97.

### Proposed learning

**A `setBit != 0`/`setBit == 0` mask branch can need BOTH the branch
polarity AND the value-per-branch mapping fixed independently — they are
two separate, independently-toggleable axes, not one.** Confirmed by
exhaustively trying multiple of the four `{==,!=} x {A-first,B-first}`
combinations for the same two-armed bitmask if/else and observing that
different combinations fixed the branch instruction (`beqz` vs `bnez`)
without fixing which value landed in which arm, and vice versa — only one
of the four combinations matched both. When a residue shows a branch
instruction AND its arm contents both differing from retail, try the
full 2x2 rather than assuming fixing one will fix the other.

**A `for` loop's multi-variable increment clause has an order that
survives to codegen, and it can matter even when the two updated values
are logically independent (a pointer and a counter with no data
dependency between them).** `for (i = 0; i < N; i++, ptr++)` and `for (i
= 0; i < N; ptr++, i++)` produced different instruction ORDER at a
`continue`-style early-exit landing pad (one extra word's worth of
reordering), even though nothing downstream depends on which happens
first. Worth checking when a residue is confined to a loop's
continue/back-edge and is a pure ordering difference.

## Naming

**Tier B.** Not a vtable slot. Walks every `self->gridSlots[]` entry's
covered grid cells (and each cell's `unk38` overflow chain), setting or
clearing bit 31 of `EntryChildObj::unk10` per the `setBit` argument (a
name already established before this naming pass, not introduced by it).
What bit 31 represents in-game is not established from this unit alone,
so the name describes the mechanics (marks/clears a per-cell flag over
the object's footprint) rather than asserting a meaning for the bit.

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

## Round 96 (track 7, delta)

### Naming

Renamed from `StageMap__SetFootprintCellFlag` (tools/rename.py), tier A:
the flag is libgs's GsDOFF (bit 31 of GsDOBJ2 attribute, display off);
nonzero clears it on every cell under `rects` and every cell chained behind
it (`nextInCell`), zero sets it. RefreshFootprint calls it with 0 before
rebuilding `rects` and with 1 after. Parameter `setBit` -> `visible`;
locals `slot` -> `rect` (CellRect), `e` -> `slot` (ChunkSlot).

### Constants

`0x7FFFFFFF`/`0x80000000` -> `~GsDOFF`/`GsDOFF` (libgs.h); `20` ->
`STAGE_CHUNK_CELLS` (the row stride); `next != 0` -> `!= NULL`.
