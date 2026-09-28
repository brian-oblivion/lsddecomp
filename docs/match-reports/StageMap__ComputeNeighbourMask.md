# StageMap__ComputeNeighbourMask

> Renamed from `StageMap__ComputeRateFlags` on 2026-09-26 (tools/rename.py). Address 0x8004b930.

> Renamed from `Class866E8__ComputeRateFlags` on 2026-09-26 (tools/rename.py). Address 0x8004b930.

> Renamed from `func_8004B930` on 2026-09-24 (tools/rename.py). Address 0x8004b930.

**Unit:** DayTaskStageMap · **Size:** 68 words · **Status:** MATCHED (~6 attempts).

## Result

```c
s32 StageMap__ComputeNeighbourMask(Obj866E8 *self, s32 val, s32 flag) {
    Unk68Struct *u;
    s32 divisor;
    s32 unk4;
    s32 count;
    s32 flags;
    s32 i;

    u = self->unk68;
    divisor = u->divisor;
    unk4 = u->unk4;
    count = u->count;
    if (unk4 == 0) {
        flags = (val < divisor) ? 3 : 0;
        if (val >= divisor * (count - 1)) {
            flags |= 0x60;
        }
        if (val % divisor == 0) {
            flags |= flag ? 0x25 : 4;
        }
        if ((val + 1) % divisor != 0) {
            return ~flags;
        }
        flags |= flag ? 0x10 : 0x52;
        return ~flags;
    } else {
        flags = -1;
        for (i = 0; i < count; i++) {
            flags <<= 1;
        }
        return ~flags;
    }
}
```

## Derivation

`self->unk68` (`Unk68Struct`) gets all three of its fields
(`divisor`/`count`/`unk4`) read UNCONDITIONALLY at the top, before the branch
on `unk4` -- even `divisor`, which only the `unk4 == 0` path uses. This
confirmed `Unk68Struct`'s full layout: `s16 divisor @0`, `s16 count @2`, `s32
unk4 @4` (corroborated independently by `StageMap__SplitChunkIndex` and by
`ComputeCellWorldOffsets`'s own `arg2` parameter, fed this exact pointer at its one call
site).

The `unk4 != 0` branch is a small closed form: `~(-1 << count)`, written out
as an explicit shift loop because that's what retail's own trip-count-gated
`do`/`while`-shaped loop compiles from -- `for (i = 0; i < count; i++) flags
<<= 1;` reproduced it directly, no manual unrolling needed.

## Two residues, both closed

**1. Branch polarity/layout inverted.** First attempt wrote the natural
`if (unk4 != 0) { ALT; } MAIN;` (early-return) form. Retail's actual layout
places the `unk4 == 0` case (MAIN) as the branch's FALLTHROUGH and the
`unk4 != 0` case (ALT) at a distant label reached only by a taken branch --
i.e. the source tests `unk4 == 0` (not `!= 0`) with `if (cond) A else B`,
which GCC lays out as "if `!cond` goto B; A; goto end; B: ...". Confirmed by
literally reading the branch instruction (`bnez`, not `beqz`) and target
address direction; fixed by writing the `if`/`else` with the EQUALITY
(`unk4 == 0`) as the guarding condition and MAIN as the `if`-arm, matching the
`if(cond) A else B` compilation shape rather than the intuitively-cleaner
early-return form.

**2. `s16` locals for `divisor`/`count` produced a spurious
`lhu`+`sll`/`sra` sign-extension pair that retail does not have.** Retail
loads both fields with plain sign-extending `lh` and reuses the sign-extended
register value everywhere with no further extension. Caching them into `s16`
NAMED LOCALS (rather than `s32`) caused GCC, in this case, to defer the sign
extension: load the raw 16-bit pattern via `lhu`, then manually `sll`/`sra` by
16 wherever the signed value is actually needed (once per branch that uses
it) -- extra instructions with no retail counterpart. **Fix: declare the
locals `s32`, not `s16`, when caching a signed-halfword struct field that is
subsequently used in ordinary signed arithmetic.** The struct field itself
stays `s16` (that's real, confirmed layout); only the LOCAL COPY needs the
wider type to avoid the deferred-extension codegen.

### Proposed learning

**Caching a signed 16-bit struct field into an `s16` local (rather than
`s32`) can make GCC 2.6.3 defer the field's sign extension to point-of-use
(`lhu` at load + `sll`/`sra` per use) instead of extending once at load
(`lh`) -- especially when the same local's value is live into a branch used
by more than one path.** If a residue is an unexplained `sll`/`sra`-by-16
pair immediately preceding an arithmetic op on a value you cached from a
signed-halfword field, widen the LOCAL's declared type to `s32` (leaving the
struct field itself `s16`) before looking anywhere else.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B930` | `StageMap__ComputeNeighbourMask` | B | Takes `self` as its first parameter (a method, not a free function). Computes a bitmask from `val`/`self->unk68->divisor`/`flag` (boundary tests against the divisor, remainder tests, complemented at every return) with no field write -- a pure computation, its result forwarded by `StageMap__LoadChunksAround` to `StageMap__ComputeChunkLoadEntry` as `savedResult`, tested there against `sNeighbourBits[key]`. "Compute...Flags" names the mechanic; the individual bit meanings (0x25/0x60/0x10/0x52/...) are not established. |

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

This function: `StageMap__ComputeRateFlags` -> `StageMap__ComputeNeighbourMask` (`python3 tools/rename.py StageMap__ComputeRateFlags StageMap__ComputeNeighbourMask`, tier B): a bit per neighbour key (`sNeighbourBits`) whose chunk is inside the grid: the edge tests clear keys 0/1 (top row), 5/6 (bottom), and the side neighbours of the first/last column by row parity; a vertical grid keeps the low `rows` bits.

## Track 7 (2026-09-27, round 95, charlie)

Parameters and locals, tier A: `val` -> `chunk`, `flag` -> `oddRow`, `u` -> `dims`, `divisor` -> `columns`, `unk4` -> `isVertical`, `count` -> `rows`, `flags` -> `offGrid` (the neighbours off the grid; the return value is its complement).

Constants: each mask is spelled as `CHUNK_NEIGHBOUR_BIT()`s of the keys it covers (include/StageMap.h): 3 = PREV_ROW_LO | PREV_ROW_HI (first row), 0x60 = NEXT_ROW_LO | NEXT_ROW_HI (last row), 0x25 = PREV_ROW_LO | PREV_COL | NEXT_ROW_LO and 4 = PREV_COL (first column, odd / even row), 0x10 = NEXT_COL and 0x52 = PREV_ROW_HI | NEXT_COL | NEXT_ROW_HI (last column, odd / even row). The key names come from sChunkNeighbourDeltas' data, (row, col odd, col even): 0 (-1,-1,0), 1 (-1,0,1), 2 (0,-1,-1), 3 (0,0,0), 4 (0,1,1), 5 (1,-1,0), 6 (1,0,1); and sNeighbourOffsets' z (row) and x (column) agree. The masks are exactly the keys each edge case puts off the grid, which is the cross-check.
