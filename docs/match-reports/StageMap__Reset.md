# StageMap__Reset -- MATCHED, round 45 (2026-09-15)

> Renamed from `Class866E8__Reset` on 2026-09-26 (tools/rename.py). Address 0x8004aa10.

> Renamed from `func_8004AA10` on 2026-09-22 (tools/rename.py). Address 0x8004aa10.

**Unit:** `DayTaskStageMap` · **Size:** 23 words · **Status:** MATCHED, 23/23 exact.

Previously filed as blocked by `gp_rel` (round 42 reopen note); that blocker
was RESOLVED in round 42 by `maspsx --gp-symbols`, pinned in the Makefile.
Rebuilt clean on the first attempt this round -- the earlier report's only
finding (the `lw $a1, %gp_rel(sDefaultGridSpan)($gp)` instruction) was correct, it
just isn't a blocker any more.

## What it does

`StageMap`'s vtable slot `+0x040` (constructor-adjacent init, called by
`StageMap__SetConfig`): zeroes `unk68`/`unkE8`/`unk88`, dispatches slot `+0x0DC`
with `self` and the loaded value of a lone global word `sDefaultGridSpan`, then
sets four new fields (`unk1CC`/`unk1D0`/`unk1D4`/`unk1D8`) to `-1`.

## Body

```c
extern s32 sDefaultGridSpan;

void StageMap__Reset(StageMap *self)
{
    self->unk68 = NULL;
    self->unkE8 = 0;
    self->unk88 = 0;
    self->methods->slotDC(self, sDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}
```

## Derivation notes

- `sDefaultGridSpan` (`asm/data/7B12C.sdata.s`) is a single `.word 0x0000A000`
  with no other reference anywhere in the image (checked with
  `grep -rl sDefaultGridSpan asm/`). It sits in an unnamed top-level `sdata`
  segment, not owned by any carved unit, so it is declared `extern s32`
  directly in `src/world/DayTaskStageMap.c` -- same pattern already used for
  `sDefaultOrigin` in this same file. `%gp_rel(sDefaultGridSpan)($gp)` loads its
  *value*, not its address, so the call argument is a plain `s32`, not a
  pointer.
- `self->unk0` (the vtable pointer) is read early in retail's own
  instruction order, and slot `+0x0DC` had no prior occupant or
  declaration -- added to `StageMapMethods` as `slotDC(StageMap*, s32)`,
  splitting the padding that used to run `0xD4..0xF4` at `0xDC`.
  `include/DayTaskStageMap.h` updated (was already shared only within this
  runner's assignment this round; no cross-runner contention).
- Offsets `0x1CC`/`0x1D0`/`0x1D4`/`0x1D8` were inside the `StageMap`
  struct's `pad1C4[0x1E0-0x1C4]` catch-all padding; split into four new
  `s32` fields (`unk1CC..unk1D8`), leaving a smaller `pad1DC[0x1E0-0x1DC]`
  behind them. No other function in this unit currently reads that byte
  range (checked via the header's own existing comments), so this is a
  pure padding narrowing, not a retype of anything already read elsewhere.
- Instruction ORDER in the C source (fields zeroed, then the dispatch,
  then the four `-1` stores) matches retail's own statement order; the
  `$a1`/`self->methods` loads appearing early in the raw asm is ordinary
  GCC 2.6.3 scheduling of independent loads ahead of the stores, not a
  sign the source order is wrong -- confirmed by the byte-exact result on
  the first attempt with straight-line, in-order C.

### Proposed learning

None new -- this is exactly the round-42 `gp_rel` resolution playing out:
the prior report's technical observation (the `%gp_rel` load) was correct
and complete, only its blocked-verdict framing needed rebuilding.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AA10` | `StageMap__Reset` | B | Occupant of vtable slot `+0x040`. Body does nothing but put the object back to a known state: clears `config`, `acceptedTags` and `rectCount`, re-applies the default grid span, and writes the four `-1` sentinels at `+0x1CC..+0x1D8`. The sibling table `gDayTaskMethods` names its own `+0x040` occupant `resetUnk3C` (`include/DayTaskStageMap.h`), so `+0x040` is a reset slot in this family. Tier B and not A because nothing establishes WHEN a reset is wanted -- its one known caller is `StageMap__SetConfig`. |
| `D_8008A980` | `sDefaultGridSpan` | B | Value `0x0000A000`, and this is its only reader in the whole image. It is handed straight to `setGridSpan`, which derives `span >> 11 == 20` -- the byte-verified grid row stride -- and `span >> 12 == 10`. See `StageMap__SetGridSpan.md` for the full arithmetic. |

Field names established here:

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x068` | `config` | B | Cleared here, set by `StageMap__SetConfig`, read by `StageMap__ApplyToSenderFootprint` as `config->unk4`; `DayTaskStageMap` reads the same pointee as `{s16 divisor, s16 count, s32 unk4}` from four functions -- a small parameter block, not an object. |
| `StageMap+0x0E8` | `acceptedTags` | A | See `StageMap__ForwardAcceptedCommand.md`: its only reader walks it as a NUL-terminated list of vtable header words and uses it to accept or reject a sender. |
| `StageMap+0x088` | `rectCount` | A | Its only writers set it to 1 (`StageMap__SetFootprintRect`) or save/restore it around a walk (`StageMap__ApplyToSenderFootprint`), and its only readers bound a loop over `rects[]` (here, and `DayTaskStageMap`'s matched `StageMap__SetFootprintVisible`). |

`unk1CC`/`unk1D0`/`unk1D4`/`unk1D8` deliberately keep placeholder names: all
that is known is that they are four consecutive words set to `-1` here and
never read by any decompiled function. Writing a name for them would be a
guess.

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
