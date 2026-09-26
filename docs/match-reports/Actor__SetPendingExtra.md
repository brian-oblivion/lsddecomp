# Actor__SetPendingExtra -- MATCHED (2/2)

> Renamed from `DreamSys__SetPendingExtra` on 2026-09-25 (tools/rename.py). Address 0x80057c7c.

> Renamed from `func_80057C7C` on 2026-09-19 (tools/rename.py). Address 0x80057c7c.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0EC`
(base-class-inherited; `include/DreamSys.h` already named this slot in an
earlier commit this round).

## Body

```c
void Actor__SetPendingExtra(DreamSys *self, void *extra) {
    self->pendingExtra = extra;
}
```

A single `sw` store. Newly names `DreamSys::pendingExtra` (splitting the
existing `unknown_values_0x50[8]` array, additive/size-preserving, into
`unknown_values_0x50[4]` + this new `void *pendingExtra`).

## Naming

**`Actor__SetPendingExtra` -- tier A.** A pure setter (`self->pendingExtra = extra`) --
mechanics ARE the purpose. Named after the field it sets
(`DreamSys::pendingExtra`, this round's naming pass); no reader is
confirmed yet, so the field itself stays tier B/C in intent even though
this setter's own mechanics are unambiguous.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__SetPendingExtra   # 2/2
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__SetPendingExtra`. Occupant of +0x0EC: pendingExtra (+0x054) = extra. The parameter is s32 now: Actor__NotifyMove adds the field to an s16 (StyleEffect overrides the slot with StyleEffect__Update). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
