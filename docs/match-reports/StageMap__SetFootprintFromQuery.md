# StageMap__SetFootprintFromQuery — MATCHED (49/49 words)

> Renamed from `Class866E8__SetFootprintFromQuery` on 2026-09-26 (tools/rename.py). Address 0x8004cc74.

> Renamed from `func_8004CC74` on 2026-09-24 (tools/rename.py). Address 0x8004cc74.

Fills a stack-local query buffer via a vtable call, seeds
`self->unk88`/`self->slots8C[0]` from it via `StageMap__InitFootprintRect`, then
conditionally adds up to two more `slots8C` entries (via two more
`StageMap__InitFootprintRect` calls) gated on a bounding-box test (`IsPointOutOfBounds`,
the excluded stall) and a range comparison against
`self->unk68->count`.

## The "uninitialized local" register

`StageMap__InitFootprintRect`'s second parameter (established this round as dead/unused
within `StageMap__InitFootprintRect` itself — see its report) is passed a value here
that this function's own body **never assigns**. GCC allocates a
callee-saved register (`$s1`) for it (it must survive across the
intervening calls), saves/restores it in the prologue/epilogue like any
other local, and simply never writes it — because nothing downstream
ever reads it. This reproduces as a genuinely uninitialized C local:

```c
s32 junk;   /* never assigned; StageMap__InitFootprintRect never reads its 2nd arg */
```

Passing an uninitialized value into a parameter the callee provably never
reads is undefined behavior by the letter of the standard, but it is
exactly what retail's compiled code does, and reproducing it byte-exact
requires writing exactly this.

## New struct knowledge (`include/class_3bb8c.h`)

- New vtable slot `Obj866E8Methods::slot10C` (`s32 (*)(Obj866E8*, void*,
  s32)`, +0x10C) — carved out of the existing `pad108[0x110-0x108]` gap
  between `slot104` and `slot110`.
- New type `CC74QueryBuf`: a stack-local buffer (not a field of
  `Obj866E8`) filled by `slot10C`. Only two offsets proven: `+0x2` (`s8
  point[2]`, forwarded to `IsPointOutOfBounds`) and `+0x28` (`s32 count`, read
  directly). Left minimal/opaque elsewhere — this is a local variable's
  layout, not a shared type, so there is no cross-function pressure to
  fill in more than what this function itself proves.

## Final C

```c
/* Forward declaration: defined later in this file (in ROM order, after
 * StageMap__SetFootprintFromQuery), and EXCLUDED from this round's targets (documented
 * STALL, see docs/match-reports/IsPointOutOfBounds.md) -- calling into it
 * while it is still INCLUDE_ASM is fine, per this unit's established
 * convention. Signature per that report. */
extern s32 IsPointOutOfBounds(CellBounds *bounds, s8 *point);

/* Forward declaration: defined later in this file (in ROM order, after
 * IsPointOutOfBounds), but StageMap__SetFootprintFromQuery calls it before its own definition
 * appears. */
extern s32 StageMap__InitFootprintRect(Obj866E8 *self, s32 unused, s32 key, s32 arg3);

void StageMap__SetFootprintFromQuery(Obj866E8 *self) {
    s32 junk;
    CC74QueryBuf buf;

    self->methods->slot10C(self, &buf, 0);
    self->unk88 = 0;
    self->unk88 = StageMap__InitFootprintRect(self, junk, 0, buf.count);
    if (IsPointOutOfBounds(self->unk1DC, buf.point) != 0) {
        if (buf.count + 1 < self->unk68->count) {
            self->unk88 = StageMap__InitFootprintRect(self, junk, self->unk88, buf.count + 1);
        }
    }
    if (buf.count - 1 >= 0) {
        self->unk88 = StageMap__InitFootprintRect(self, junk, self->unk88, buf.count - 1);
    }
}
```

## Attempts

1 (matched on first attempt). Reading `buf.count` fresh at each use site
(rather than caching it into a separate scalar local) mattered: retail
reloads it from the stack after the intervening `IsPointOutOfBounds` call
rather than carrying it in a preserved register, which a direct
struct-field reference reproduces naturally.

### Proposed learning

**An unused parameter in a callee can show up in a caller as a
genuinely-uninitialized local, allocated to a callee-saved register
purely because it must survive across calls.** This is the caller-side
mirror of `StageMap__InitFootprintRect`'s own "dead parameter" finding from earlier
this round — worth checking together whenever one function's parameter
is proven dead: its callers may need an uninitialized local, not a
real value, at that argument position.

## Naming

**Tier B.** Not a vtable slot. The `self->unk68->unk4 != 0` sibling of
`StageMap__ComputeFootprintFromRotation` under `RefreshFootprint`'s
dispatch: fills a query buffer via `slot10C`, seeds `self->gridSlots[0]`
via `StageMap__InitFootprintRect`, then conditionally adds up to two
more slots gated by a bounding-box test (`IsPointOutOfBounds`) and a
range check. Named to parallel dream_day's
`StageMap__SetFootprintFromCell`/`StageMap__SetFootprintRect` pair for
the same subsystem, which the query-buffer + bounding-box shape here most
resembles.

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

## Round 96 (track 7, delta)

`buf` -> `desc`. `junk` keeps its name and gains
`/* MATCHING: never set; InitFootprintRect ignores the argument */` (this
report's "uninitialized local" section). Zero bytes.
