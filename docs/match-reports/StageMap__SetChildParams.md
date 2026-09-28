# StageMap__SetChildParams — MATCH

> Renamed from `Class866E8__SetChildParams` on 2026-09-26 (tools/rename.py). Address 0x8004acf8.

> Renamed from `func_8004ACF8` on 2026-09-22 (tools/rename.py). Address 0x8004acf8.

**Unit:** dream_day · **Size:** 51 instructions · **Result:** 51/51 words

## What it does

A simple counted loop: for `i` in `[0, count)`, fetch a child object via
`self->methods->slotB8(self, i)` and call two of ITS OWN vtable slots
(`+0x44`, `+0x48`) on it, each with a literal `1` and a running
accumulator that steps by a different amount per call (`arg3 += 3` for
`slot44`, `arg2 += 6` for `slot48`).

`slotB8` (`LightRig__GetLight`) and the child's own class are not decompiled;
only the two slots this function reaches on the child are typed, as
`UnkChildObj_3ac78`/`UnkChildMethods_3ac78` in `include/dream_day.h`.

## Final source

```c
void StageMap__SetChildParams(StageMap *self, s32 count, s32 arg2, s32 arg3)
{
    s32 i;
    UnkChildObj_3ac78 *child;

    for (i = 0; i < count; i++) {
        child = self->methods->slotB8(self, i);
        child->methods->slot44(child, 1, arg3);
        arg3 += 3;
        child->methods->slot48(child, 1, arg2);
        arg2 += 6;
    }
}
```

## Residue

None — matched on the first attempt. A plain `for` loop with the two calls
and their accumulator updates written in straight source order was enough;
no reordering or barrier was needed.

## Provenance

round 2026-09-02, runner ALPHA, unit dream_day.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004ACF8` | `StageMap__SetChildParams` | B | Occupant of vtable slot `+0x0C4`. `getChild` (`+0x0B8`) resolves to `LightRig__GetLight`, whose whole body is `return ((void **)((u8 *)self + 0x44))[index]` -- a plain indexed fetch from a small pointer array in the object, so it really is "get child i". This function fetches `count` of them and feeds each a pair of values through the child's own `+0x44` and `+0x48` slots, with the two values advancing by 3 and by 6 per child. Tier B, deliberately: the two accumulators' meaning is unknown (they step at different rates, which rules out a single shared index), so the name claims only "sets per-child parameters". |

Slot named this round: `StageMapMethods::slotB8` -> `getChild`, tier A
(the occupant's body is the whole evidence).

NOT named, and why: the child class itself. `LightRig__GetLight` returns an
untyped pointer out of an array this unit never populates, and the two slots
dispatched on it (`+0x44`, `+0x48`) have no decompiled occupant. The local
view stays `UnkChildObj_3ac78`/`UnkChildMethods_3ac78`.

## Track 4

2026-09-26, round 86 (delta): class 0x14 (was D_8006EFAC) unified as LightRig in `include/LightRig.h`; the +0x0B8 slot it calls is LightRig's `getLight` (LightRig__GetLight, `lights[index]`), and the two slots it drives on each result, +0x044 and +0x048, are FlatLightObj's setColor and setDirection (src/code_3311c.c), consistent with the 3- and 6-byte strides. StageMap's own view (include/dream_day.h) still names the slot `getChild` and its result `UnkChildObj_3ac78`; the body is untouched.

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

## Track 7 (2026-09-27, round 98, charlie)

`colors += 3` -> `colors += sizeof(FlatLightColor)`, `dirs += 6` -> `dirs += 3 * sizeof(s16)` (an r,g,b and an s16 vx,vy,vz per light, include/flat_light_obj.h). Zero bytes. The `s32` source parameters stay (the slot's type in StageMap.h; proposed to the head as `FlatLightColor *` / `s16 *`, which would drop the casts and let the steps be `++` / `+= 3`).

## Track 10 (2026-09-28, round 104, echo)

The six per-class aliases of `ColorRgb` (include/draw_system.h) -- BgLayerRgb, BoxFillRgb, FlatLightColor, LightRigRgb, ViewportRgb, TimBlockSrcColor -- are deleted and every use is spelled `ColorRgb`. Byte-identical. Measured for the MATCHING line in TaskCore__SetColors: the whole-struct copy is three `lb` then three `sb`, and rewriting one of the copies byte by byte loads each byte with `lbu` and interleaves the stores (asm-differ on the experiment), so the struct copy stays; the old line's "signed bytes" was wrong (ColorRgb's channels are u8; the lb comes from the block copy, not the type), and the same claim in GraphRoom__BuildGraphPoints' colour comment is corrected.
