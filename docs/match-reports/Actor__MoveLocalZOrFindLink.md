# Actor__MoveLocalZOrFindLink -- MATCHED (12/12)

> Renamed from `DreamSys__DispatchOffsetSlotC4` on 2026-09-25 (tools/rename.py). Address 0x800575b0.

> Renamed from `func_800575B0` on 2026-09-19 (tools/rename.py). Address 0x800575b0.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0D0`.

## Signature

```c
void Actor__MoveLocalZOrFindLink(DreamSys *self, s32 val, void *extra);
```

## Body

```c
void Actor__MoveLocalZOrFindLink(DreamSys *self, s32 val, void *extra) {
    Actor__MoveOrFindNearbyLink(self, self->vt->Actor__MoveLocalZ, val, extra);
}
```

Reads (does NOT call) the vtable's `+0x0C4` slot -- `Actor__MoveLocalZ`, out
of this unit/runner's range -- as a raw function-pointer VALUE, and
forwards it plus its own two arguments to `Actor__MoveOrFindNearbyLink` (this unit's own
helper, see its report), which is what actually invokes it.

## Naming

**`Actor__MoveLocalZOrFindLink` -- tier B.** Confirmed mechanics: reads
(does not call) the vtable's `+0x0C4` slot (`Actor__MoveLocalZ`, a
sibling of this unit's own offset-slot family defined in a DIFFERENT
unit, `class_3bb8c_o.c`, and out of this runner's scope to rename) as a
raw function pointer, and forwards it to
`Actor__MoveOrFindNearbyLink`. Named by the SLOT it reads (`C4`,
objective) rather than by inventing a name for `Actor__MoveLocalZ`'s
own purpose, which belongs to another unit.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__MoveLocalZOrFindLink   # 12/12
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__DispatchOffsetSlotC4`. Occupant of +0x0D0: Actor__MoveOrFindNearbyLink with moveLocalZ (+0x0C4). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
