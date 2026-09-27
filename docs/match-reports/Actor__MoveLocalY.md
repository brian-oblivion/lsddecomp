# Actor__MoveLocalY -- MATCHED (14/14)

> Renamed from `DreamSys__ApplyOffsetSlot1` on 2026-09-25 (tools/rename.py). Address 0x800574fc.

> Renamed from `func_800574FC` on 2026-09-19 (tools/rename.py). Address 0x800574fc.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0CC`
(base-class-inherited; resolved via `tools/classtable.py gDreamSysMethods`).

## Signature

```c
void Actor__MoveLocalY(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void Actor__MoveLocalY(DreamSys *self, s32 val, void *extra) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMove[1], val, extra, 8);
}
```

Sibling of `Actor__MoveLocalX` (see that report for the shared helper and the
`gActorLocalMove[2]` array discovery): same shape, writes element 1 instead of
element 0, and passes `8` instead of `7` as the trailing constant.

## Naming

**`Actor__MoveLocalY` -- tier B.** Sibling of
`Actor__MoveLocalX` (see that report's Naming section): same
reasoning, element 1 instead of 0.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__MoveLocalY   # 14/14
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__ApplyOffsetSlot1`. Occupant of +0x0CC: gActorLocalMove[1], the y component, event 8. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
