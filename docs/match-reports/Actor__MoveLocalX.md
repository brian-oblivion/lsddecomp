# Actor__MoveLocalX -- MATCHED (14/14)

> Renamed from `DreamSys__ApplyOffsetSlot0` on 2026-09-25 (tools/rename.py). Address 0x800574c4.

> Renamed from `func_800574C4` on 2026-09-19 (tools/rename.py). Address 0x800574c4.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0C8`
(base-class-inherited; resolved via `tools/classtable.py gDreamSysMethods`
and confirmed unchanged in the DreamSys-level table too).

## Signature

```c
void Actor__MoveLocalX(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void Actor__MoveLocalX(DreamSys *self, s32 val, void *extra) {
    Actor__MoveAlongLocalAxis(self, &D_8008ABA4[0], val, extra, 7);
}
```

A thin wrapper: forwards to `Actor__MoveAlongLocalAxis` (this unit's own helper, see
its own match report) with a fixed pointer into element 0 of a
newly-identified 2-element `s16` array `D_8008ABA4`, and the constant `7`.

`val` is `s32`, not `s16`, even though `Actor__MoveAlongLocalAxis` only ever uses it
truncated to 16 bits -- see `Actor__MoveAlongLocalAxis`'s report for why: typing it
`s16` here forces a spurious `sll`/`sra` re-sign-extend pair at this call
site that retail does not have.

## `D_8008ABA4` is a 2-element `s16` array, not a lone `s32`

splat's single-word `dlabel D_8008ABA4` (`asm/data/7B008.sdata.s`) is really
`s16 D_8008ABA4[2]`: this function writes element 0
(`%hi/%lo(D_8008ABA4)`), its sibling `Actor__MoveLocalY` writes element 1
(`%hi/%lo(D_8008ABA4 + 0x2)`). Not referenced anywhere else in the repo
(checked with `grep -rn D_8008ABA4 src/ include/` before this round), so
declared locally in `src/class_3bb8c_p.c` rather than added to a shared
header.

## Naming

**`Actor__MoveLocalX` -- tier B.** Mechanics fully confirmed
(forwards to `Actor__MoveAlongLocalAxis` with a fixed pointer
into element 0 of the local `D_8008ABA4[2]` array and the constant `7`);
purpose of "why element 0, why 7" is not established. `Slot0` names the
array element this wrapper owns -- an objective, code-confirmed fact --
rather than guessing which axis or game concept it represents.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__MoveLocalX   # 14/14
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__ApplyOffsetSlot0`. Occupant of +0x0C8 in gActorMethods (the BASE table, so the method is Actor's, not DreamSys's): writes D_8008ABA4[0], the x of the local move vector (see Actor__MoveLocalZ), event 7. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a Class6B5CC subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
