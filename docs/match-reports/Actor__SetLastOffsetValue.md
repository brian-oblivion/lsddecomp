# Actor__SetLastOffsetValue -- MATCHED (2/2)

> Renamed from `DreamSys__SetLastOffsetValue` on 2026-09-25 (tools/rename.py). Address 0x80057c6c.

> Renamed from `func_80057C6C` on 2026-09-19 (tools/rename.py). Address 0x80057c6c.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys`, own vtable slot `+0x0E4`
(base-class-inherited; `include/DreamSys.h` already named this slot in an
earlier commit this round).

## Body

```c
void Actor__SetLastOffsetValue(DreamSys *self, s16 val) {
    self->lastOffsetValue = val;
}
```

A single `sh` store, splat-matched-length two-word leaf.

## Naming

**`Actor__SetLastOffsetValue` -- tier A.** A pure setter (`self->lastOffsetValue = val`) --
mechanics ARE the purpose. Named after the field it sets
(`DreamSys::lastOffsetValue`, this round's naming pass -- see that
field's own comment in `include/DreamSys.h`).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__SetLastOffsetValue   # 2/2
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__SetLastOffsetValue`. Occupant of +0x0E4 in gActorMethods: lastOffsetValue (+0x048) = val. Class65650__Reset calls it with 300, the value Actor__Reset stores. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a Class6B5CC subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_p.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
