# StageMap__UpdateFootprintTracking -- MATCHED (81/81 words, byte-exact)

> Renamed from `Class866E8__UpdateFootprintTracking` on 2026-09-26 (tools/rename.py). Address 0x8004b5bc.

> Renamed from `func_8004B5BC` on 2026-09-24 (tools/rename.py). Address 0x8004b5bc.

Unit `DayTaskStageMap`. Slot `Obj866E8Methods::slotF4` (verified against
`tools/classtable.py 0x800866E8`, which resolves `gStageMapMethods`'s own
`+0x0F4` entry directly to `StageMap__UpdateFootprintTracking`).

Round 26 (runner delta) picked this up as the only FRESH function on the
work list; round 24 had left a hand-read-only structural note in the `.c`
file (not a report, per convention) with several confidence-graded
observations. This round re-derived it from the raw `.s` from scratch,
confirmed every one of round 24's claims, resolved the two items it left
open, and matched on the first build attempt.

## What it does

```c
s32 StageMap__UpdateFootprintTracking(Obj866E8 *self) {
    Descriptor10Ext buf;
    Elem *e;
    s32 key;
    s32 result;
    u16 oldRaw;

    if (self->methods->slot10C(self, &buf, 0) == 0) {
        return 0;
    }

    e = buf.unk24;
    key = e->unk4->unk32;
    result = sFootprintResultRemap[key];

    if (self->unk68->unk4 == 0) {
        self->methods->slotF8(self, buf.unk28, &buf.unkC, sFootprintResultPtrTable[result]);
    }

    self->methods->slot128(self);

    oldRaw = *(u16 *)((u8 *)self + 0xBC);
    *(Descriptor10Ext *)((u8 *)self + 0xBC) = buf;

    if ((s16)oldRaw != *(s16 *)&buf) {
        self->methods->slot30(self, 5);
    }

    return result;
}
```

1. Fills a stack-local `Descriptor10Ext` via `self->methods->slot10C(self,
   &buf, 0)`. On failure (`0` returned) bails out early returning `0`.
2. Resolves `e = buf.unk24` (an `Elem*`, already the established meaning of
   `Descriptor10Ext::unk24` from `StageMap__ComputeFootprintDescriptor`'s independent derivation)
   and reads `key = e->unk4->unk32` (`ElemTarget::unk32`, already typed).
3. `result = sFootprintResultRemap[key]` -- an 8-entry signed-byte remap table
   (values 0..6). This is both the eventual RETURN VALUE and (scaled by 4
   by the compiler, not by this C) the index used next.
4. If `self->unk68->unk4 == 0`: calls `self->methods->slotF8(self,
   buf.unk28, &buf.unkC, sFootprintResultPtrTable[result])` -- forwarding the raw rate
   (`buf.unk28`), a pointer to `buf`'s own next THREE consecutive `s32`
   fields (`unkC`/`unk10`/`unk14`, an in-place 3-word out-parameter, same
   shape as `slotF8`'s other established call site in `StageMap__SetTargetAndLoadChunks`) and
   one of `sFootprintResultPtrTable`'s 7 pointers (first NULL) selected by `result`.
5. Unconditionally calls `self->methods->slot128(self)`.
6. Reads the OLD raw first 16 bits of `self->unkBC` (`oldRaw`, BEFORE
   overwriting it -- this is why the read has to come before step 7, not
   after, even though it looks unrelated to the copy that follows).
7. Overwrites `self->unkBC` with the entire 44-byte `buf` via a whole-
   struct assignment through a `(u8 *)self + 0xBC` cast (see below for why
   this is a cast rather than a named-field assignment).
8. If the just-overwritten region's leading 16 bits (read fresh from `buf`,
   as a genuine signed `s16`) differ from `oldRaw` (reinterpreted signed),
   calls `self->methods->slot30(self, 5)` -- i.e. "notify if this
   descriptor's leading value changed".
9. Returns `result`.

## Two derivation points worth recording

**Why the `self->unkBC` write is a raw pointer-cast whole-struct
assignment, not `self->unkBC = buf` through the named field.** `unkBC` is
already declared `Descriptor10 unkBC` (10 bytes) in `Obj866E8`, proven
byte-exact by the ALREADY-MATCHED `StageMap__SetTargetAndLoadChunks`'s own whole-struct copy
(`self->unkBC = *arg3;`, a `Descriptor10*`). Retail's copy loop here writes
44 bytes starting at the SAME address (`self+0xBC`), which is 34 bytes MORE
than the declared field -- it runs into what is currently `padC6` and stops
4 bytes short of `arr[7]` at `+0xEC`, so it is in-bounds, just bigger than
the named field. Retyping `unkBC` itself to a 44-byte struct so both
functions could use a named-field assignment was considered and rejected:
`StageMap__SetTargetAndLoadChunks`'s `self->unkBC = *arg3` would need rewriting to
`self->unkBC.base = *arg3`, and whether GCC 2.6.3 still compiles THAT to
the unaligned `lwl`/`lwr`+`swl`/`swr` sequence `StageMap__SetTargetAndLoadChunks` depends on
(alignment-2, per this header's own "no s32 member forces alignment 2"
rule) or instead treats the embedded field as inheriting the OUTER
struct's alignment-4 (since `Descriptor10Ext` has plain `s32` members after
`base`) was not something to gamble an already-matched, byte-verified
function on to save one derivation step. The pointer-cast form
(`*(Descriptor10Ext *)((u8 *)self + 0xBC) = buf;`) reaches the identical
44-byte aligned-word copy without touching `unkBC`'s declared type or
`StageMap__SetTargetAndLoadChunks` at all -- confirmed: the two-iteration 4-word loop cycling
`v0,v1,a0,a1` plus a 3-word tail is exactly the `Pad__LoadButtonTable` whole-
struct-assignment idiom documented in `docs/DECOMPILATION_LEARNINGS.md`
("A whole-struct assignment, not an indexed `for`, for a block copy"), not
a hand-written copy loop.

**Why `oldRaw` is `u16` compared via `(s16)` cast against a raw `*(s16 *)`
read, rather than named `Descriptor10` fields.** Retail loads the OLD value
via `lhu` (zero-extend) then manually sign-extends it (`sll 16`/`sra 16`)
for the compare, while the NEW value (read directly from `buf`) loads via a
plain `lh` (already sign-extended, no shift). A `u16` local compared
against a `s16`-typed read reproduces exactly this asymmetry through C's
ordinary integer promotion rules (unsigned short promotes by zero-extension,
signed short by sign-extension) without needing any cast in the C source
for the promotion itself -- the explicit `(s16)oldRaw` only matters because
`*(s16 *)&buf` on the other side is already a genuine signed value at`s16`
rank, so both sides need to reach the same signed 32-bit value for the
comparison to type-check the way retail computed it. This reads the SAME
two bytes that end up as `Descriptor10::b0`/`b1` once copied, but as a
raw 16-bit reinterpretation rather than through those byte-typed fields --
a pointer-cast read, deliberately, to avoid introducing a union into
`Descriptor10` (used byte-wise elsewhere by the already-matched
`ComputeCellWorldOffsets`) just for this one call site.

## Header changes made this round

- **`sFootprintResultRemap`** (new): `extern const s8 sFootprintResultRemap[8];` -- 8-entry
  signed-byte remap table. Bound PROVEN from `asm/data/76DC8.data.s` (next
  symbol `sDefaultTargetSpecs` starts immediately after byte 8).
- **`sFootprintResultPtrTable`** (new): `extern s32 *sFootprintResultPtrTable[7];` -- 7-entry pointer
  table (first entry NULL), bound PROVEN from the same data file. Element
  type `s32 *` matches the ALREADY-established `Obj866E8Methods::slotF8`
  `arg3` type.
- **`Obj866E8Methods::slot30`** (new field, additive pad split): was part
  of `pad000[0x88]`; split into `pad000[0x30]` + `slot30` +
  `pad034[0x88-0x34]` (0x30 + 4 + 0x54 = 0x88, total preserved). Resolved
  via `tools/classtable.py 0x800866E8`'s own `+0x030` entry to
  `BasicClass__NotifyParents` (`BasicClass::notifyParents`, already typed in
  `include/code_8220.h` as `void (*notifyParents)(BasicClass *self, s32
  arg1)`) -- this class's low vtable slots are inherited straight from
  `BasicClassMethods` (classtable confirms slots +0x004 through +0x038 all
  resolve to `BasicClass__func_*` symbols). Typed here as
  `void (*slot30)(Obj866E8 *self, s32 arg1);` for consistency with this
  header's existing per-class local-view convention (three OTHER classes
  in this same header already have their own `slot30` fields with the
  identical `(self, s32 arg1)` shape at the same offset, for the same
  reason -- `Class86E00_3bb8c_g`, `ObjM`, `Obj87034_3bb8c_l`, `ItemList`).
- **`Obj866E8Methods::slot128`** (new field, appended -- no padding
  needed, `slot124` ends exactly at `+0x128`): resolves via classtable to
  `StageMap__RefreshFootprint`, already matched in the sibling unit `DayTaskStageMap`
  with signature `void StageMap__RefreshFootprint(Obj866E8 *self)`. Typed
  `void (*slot128)(Obj866E8 *self);` to match.

No existing declaration was retyped; `unkBC`'s type in `Obj866E8` is
UNCHANGED (see derivation note above for why).

## Proposed learnings

- **`tools/classtable.py` can resolve a vtable slot to an ALREADY-MATCHED
  sibling function, and when it does, that function's real signature (not
  a fresh guess from the new call site alone) is the one to trust.** Both
  new slots here (`slot30` -> `BasicClass::notifyParents`, `slot128` ->
  `StageMap__RefreshFootprint`) had their true signatures already nailed down elsewhere
  in the codebase; classtable is what connects a raw `self->methods->slotNN`
  offset to that existing knowledge instead of re-deriving a shape from
  scratch.
- **A field write that goes PAST a named struct field's declared size, into
  its own trailing padding, does not require retyping that field.** A
  pointer-cast whole-struct assignment (`*(WiderType *)((u8*)self + same
  offset) = value;`) reaches the correct wider write without disturbing any
  OTHER function's already-matched, narrower, named-field access to the
  same base address -- useful whenever two different producers fill
  overlapping-but-different-sized views of the same struct field and
  retyping the field would force one of them to change its own (already
  verified) source.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B5BC` | `StageMap__UpdateFootprintTracking` | B | Occupant of `gStageMapMethods` +0x0F4 (`slotF4`). `DayTaskStageMap`'s own `StageMap__UpdateIfEnabled` (round 67 report) dispatches this slot then `slot13C` (`StageMap__StepScaleRamp`) back-to-back, guarded by `enabled` -- i.e. this runs every "enabled" tick alongside the rate countdown. Body reads the current footprint query (`slot10C`), resolves an element and its remap byte, conditionally forwards through `slotF8` (`StageMap__LoadChunksAround`), calls `slot128` (`StageMap__RefreshFootprint`), then updates `self->unkBC` (the current descriptor) and notifies (`slot30`) only if the descriptor's leading value changed. "Update...Tracking" names the mechanic (per-tick refresh-and-notify-on-change of the tracked footprint state), not an unproven in-game purpose. |

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

## Track 7 (2026-09-27, round 95, charlie)

Locals, tier A: `buf` -> `desc` (the Descriptor10Ext getTargetDescriptor fills), `e` -> `slot`, `key` -> `neighbour` (the slot loader's `elemKey`, a neighbour key), `result` -> `specIndex` (sFootprintResultRemap's byte, the index into sFootprintResultPtrTable and the return value), `oldRaw` -> `oldChunk` (the old descriptor's b0/b1, chunk column/row, read as one u16). Constant: 5 -> `STAGEMAP_EVENT_CHUNK_CHANGED` (enum StageMapEvent, include/StageMap.h). The one-line pointer comment (`/* StageMap__UpdateFootprintTracking -- see docs/match-reports/... */`) became a comment saying what the function does.
