# StageMap__DispatchLinkCommand

> Renamed from `Class866E8__DispatchLinkCommand` on 2026-09-26 (tools/rename.py). Address 0x8004ab88.

> Renamed from `StageMap__OnCommand` on 2026-09-26 (tools/rename.py). Address 0x8004ab88.

> Renamed from `func_8004AB88` on 2026-09-22 (tools/rename.py). Address 0x8004ab88.

**Unit:** dream_day · **Size:** 18 words · **Status:** MATCHED (18/18 words)

## What it does

`StageMap`'s slot +0x09C method: checks a type tag on another object
(`other`) and, if it matches `0x34`, forwards `other` and a count straight
through to its own slot +0x0D0.

## Derivation

```
lw   $v0, 0x0($a1)        ; v0 = other->methods
lbu  $v1, 0x0($v0)        ; v1 = low byte of other->methods->header (LE)
ori  $v0, $zero, 0x34
bne  $v1, $v0, .L8004ABC0
lw   $v0, 0x0($a0)        ; self->methods
lw   $v0, 0xD0($v0)       ; ->slotD0
jalr $v0                  ; self->methods->slotD0(self, other, count) -- no
                          ; new arg setup: $a0/$a1/$a2 already hold self,
                          ; other and count unchanged from entry
```

Slot +0x0D0 is `StageMap__ForwardAcceptedCommand` (also in this unit, still `INCLUDE_ASM`; 0xCC
words, out of this round's budget). Reading its body confirms slot +0x0D0's
real signature is `(self, void *list, s32 count)` — three genuine register
arguments (it iterates `list` while comparing against `*(void**)other's
methods`, using `count` as a loop bound test against a small constant set).
Since the jalr here sets up no new registers at all, and the callee
definitely needs three real arguments with no register gap, `$a1`/`$a2` at
the call site must be this function's own second and third parameters,
forwarded unchanged — hence `StageMap__DispatchLinkCommand(StageMap *self, GenericObject
*other, s32 count)`.

The type-tag check reads the **low byte of `other->methods->header`** (the
same `s32 header` field convention used across every `*Methods` struct in
this project) — `lbu` on a little-endian 32-bit field is exactly its low
byte, so `(u8)other->methods->header == 0x34` reproduces it without pointer
casts. `GenericObject`/`GenericMethodsHeader` (new, minimal, in
`include/dream_day.h`) model only that one field; `other`'s real class is
unconfirmed. Several *different* class tables share this low byte (0x230,
0x114, 0x34, 0x1F34 all end in `0x34`), so this reads as a family/base-class
membership check, not an exact-class check — noted in the header comment.

## Proposed learning

A `jalr` whose delay slot is a plain `nop`, immediately preceded by no
argument-register writes since function entry, is not evidence of a
zero-argument call — it means the callee's real arguments (established from
its own body or another caller) are whatever this function's own incoming
parameters already left in those registers. Always check the callee's own
body for its true argument count before assuming an untouched register is
just leftover garbage.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AB88` | `StageMap__DispatchLinkCommand` | B | Occupant of vtable slot `+0x09C`. The base occupant of that slot is `SceneNode__DispatchLinkCommand(self, a1, a2)`, which switches on `a2` in `{2,3,4}` -- so the slot's third parameter is a COMMAND CODE, not the `count` this report previously called it. `SceneNode__OnNotify` (SceneNode) dispatches `+0x09C` as `slot9C(self, sender, event)` when the sender's class tag is 4, which fixes the second parameter as the sender. This override accepts only senders whose vtable header low byte is `0x34` and forwards `(sender, command)` to `forwardAcceptedCommand`. Tier B: the filter and the forward are certain, the meaning of tag `0x34` and of the command numbering is not. |

Parameters renamed: `other` -> `sender`, `count` -> `command`. This is the
same parameter that `StageMap__ForwardAcceptedCommand` gates on
`{2,3,5,6,7,8}` -- see that report.

## Track 4 (2026-09-26, round 89)

Renamed `StageMap__OnCommand` -> `StageMap__DispatchLinkCommand` with
tools/rename.py: the function occupies SceneNode's +0x09C
`dispatchLinkCommand` slot (`classtable.py gStageMapMethods --vs
gLightRigMethods`), and its body is the same kind of override as
`GridCell__DispatchLinkCommand`: route a sender whose class-id byte is 0x34
(an Actor) onward and ignore every other sender. Nothing in the body goes
beyond the slot's name. Image byte-identical.

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

`(u8)header == 0x34` -> `ACTOR_CLASS_ID`, new in include/actor.h (tier A: gActorMethods word +0x000 is 0x34, `classtable.py --scan`; the byte compare also passes TodActor, 0x234). Zero bytes.
