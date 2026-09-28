# StageMap__OnDrawSystemEvent

> Renamed from `StageMap__OnNotifyTag1` on 2026-09-28 (tools/rename.py). Address 0x8004bd14.

> Renamed from `Class866E8__OnNotifyTag1` on 2026-09-26 (tools/rename.py). Address 0x8004bd14.

> Renamed from `func_8004BD14` on 2026-09-24 (tools/rename.py). Address 0x8004bd14.

**Unit:** DayTaskStageMap · **Size:** 80 words · **Status:** MATCHED (first attempt).

## Result

```c
void StageMap__OnDrawSystemEvent(Obj866E8 *self, void *arg1, s32 mode) {
    s32 i;
    Elem *e;
    s32 curMode;

    if (mode != 2) {
        return;
    }
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk2E != 0) {
            e->unk4->unk2E = 0;
            self->methods->slot88(self, 7, e, i);
        }
        curMode = self->unk1B0;
        if (curMode == 1 && e->flag != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot104(self, e);
                e->unk4->unk2C = 2;
                e->flag = 0;
                if (--self->unk1B4 == 0) {
                    self->unk1B4 = 0;
                    self->unk1B0 = 0;
                    self->unk1B8 = curMode;
                }
            } else if (e->unk4->unk2A == 0) {
                e->flag = 0;
            }
        }
    }
}
```

`self->arr[7]` walk with stride `0x1C` (matching `Elem`'s already-established
size). `arg1` (the DayTaskStageMap independent view of this same slot,
`slot100`, is `void (*)(StageMap *self, void *arg1, s32 arg2)`) is truly
unused here -- confirmed by register tracing, `$a1` is never read.

`curMode = self->unk1B0;` is the interesting piece: `self->unk1B0` is read
ONCE per outer-loop iteration into a local, checked against `1`, and later --
AFTER `self->unk1B0` has ALREADY been zeroed a few lines down in the same
branch -- written back into `self->unk1B8`. Reproducing this needs the
explicit local (`curMode`), not a second read of `self->unk1B0`, because by
the time of the final store the field no longer holds `1`.

This function is also where `ElemTarget`'s `+0x02A`/`+0x02C`/`+0x02E` fields
(all read/written through `e->unk4`) were established, and where
`Obj866E8Methods::slot88`/`slot104` got their signatures (both cross-checked
against DayTaskStageMap's independent view of the same vtable, which names them
identically in arity if not in exact parameter types).

### Proposed learning

**A field read once into a local, tested, and later WRITTEN BACK to a
different field after the ORIGINAL field has itself been overwritten in the
interim, needs the explicit local -- re-reading the original field at the
write-back site would read the now-stale (already-zeroed) value.** A second
instance of the "value reused after an intervening write needs an explicit
local" family already documented in DECOMPILATION_LEARNINGS.md, but the
INTERVENING operation here is a plain field STORE (`self->unk1B0 = 0;`), not
a call -- worth generalizing that entry's discriminator ("what sits BETWEEN
the reads") to include a direct sibling-field write, not just a call or an
aliasing-suspect memory op.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004BD14` | `StageMap__OnDrawSystemEvent` | B | Occupant of `gStageMapMethods` +0x100 (`slot100`). `DayTaskStageMap`'s `StageMap__OnNotify` (already matched) dispatches this slot, unconditionally, only when `(sender->methods->header & 0xF) == 1` -- i.e. only for one sender class tag. Follows the SAME "OnNotifyTagN" naming convention already established elsewhere in this codebase for identical per-tag dispatch targets (`Viewport__OnFrameClockEvent`/`Viewport__OnDrawSystemEvent`, `src/app/Task.c`). `mode` here is the caller's own `command`, but the body itself early-returns unless `mode == 2` -- narrower than "every tag-1 notification", which the name does not claim (only that this IS the tag-1 dispatch target). |

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

## Track 7 (2026-09-27, round 95, charlie)

Parameters and locals: `arg1` -> `sender`, `mode` -> `command` (OnNotify's command, tier A), `e` -> `slot`, `curMode` -> `pending` (`loadsPending`'s value).

Constants: 2 -> `DRAWSYSTEM_EVENT_VSYNC` (include/DrawSystem.h, new): OnNotify calls this method for a sender whose class id's low nibble is 1, DrawSystem's family (class id 0x1), and DrawSystem__RunLoop (src/code_10ee0.c) calls `notifyParents(self, 2)` once per VSync pass; DrawSystem.h's banner says StageMap adds it as a child. 7 -> `STAGEMAP_EVENT_SLOT_DATA_READY`, `headerReady = 2` -> `LBDFILE_HEADER_CONSUMED` (include/LbdFile.h, new; its banner already said "marks the header consumed (headerReady 2)"), loop bound -> `ARRAY_COUNT(self->slots)`.

Not renamed: the method name. Viewport has the same `OnNotifyTag1` for its DrawSystem case (src/app/Task.c), so it is a convention across two classes; proposed to the head as one rename of both (e.g. `OnDrawSystemNotify`) rather than breaking the pair here.
