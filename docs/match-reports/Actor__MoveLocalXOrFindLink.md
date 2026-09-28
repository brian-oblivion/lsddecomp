# Actor__MoveLocalXOrFindLink -- MATCHED (12/12)

> Renamed from `DreamSys__DispatchOffsetSlot0` on 2026-09-25 (tools/rename.py). Address 0x800575e0.

> Renamed from `func_800575E0` on 2026-09-19 (tools/rename.py). Address 0x800575e0.

Unit: `src/ObjMStyleActor.c`. Class: `DreamSys`, own vtable slot `+0x0D4`.

## Signature

```c
void Actor__MoveLocalXOrFindLink(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void Actor__MoveLocalXOrFindLink(DreamSys *self, s32 val, void *extra) {
    Actor__MoveOrFindNearbyLink(self, self->vt->Actor__MoveLocalX, val, extra);
}
```

Sibling of `Actor__MoveLocalZOrFindLink`: same shape, but reads the vtable's `+0x0C8`
slot instead of `+0x0C4` -- which is `Actor__MoveLocalX`, THIS unit's own
neighbouring function (a self-referential vtable read: the slot's value is
the address of another function this same unit implements).

## Naming

**`Actor__MoveLocalXOrFindLink` -- tier B.** Sibling of
`Actor__MoveLocalZOrFindLink` (see that report): same shape, but reads
the vtable's `+0x0C8` slot -- `Actor__MoveLocalX`, THIS unit's
own neighbour (a self-referential vtable read). Named consistently with
that function's own `Slot0` naming.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__MoveLocalXOrFindLink   # 12/12
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__DispatchOffsetSlot0`. Occupant of +0x0D4: Actor__MoveOrFindNearbyLink with moveLocalX (+0x0C8). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/ObjMStyleActor.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
