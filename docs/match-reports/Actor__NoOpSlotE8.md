# Actor__NoOpSlotE8 -- MATCHED (2/2)

> Renamed from `DreamSys__NoOpSlotE8` on 2026-09-25 (tools/rename.py). Address 0x80057c74.

Unit: `src/world/ObjMStyleActor.c`. Class: `DreamSys` family. No match report
existed before this round (same situation as `Actor__NoOpSlotD8`,
this unit): a splat-matched, length-exact two-word leaf (`jr $ra; nop`),
never a `STALL`, just never documented.

## Which slot this overrides

`include/DreamSys.h`'s `vtable_DreamSys` names the DEFAULT `+0x0E8` value
`DreamSys__NoOpSlotE8Default` ("this unit's own no-op stub... Called by DreamSys__WallLink
as (this)"), a DIFFERENT function belonging to a different unit. This
function (`0x80057C74`) is a SIBLING override of that SAME conceptual
slot, used by different subclass instance tables:

```
asm/data/76DC8.data.s:1661:  /* 77FE4 800877E4 */ .word Actor__NoOpSlotE8
asm/data/76DC8.data.s:1842:  /* 781BC 800879BC */ .word Actor__NoOpSlotE8
asm/data/79528.data.s:1274:  /* 7A3BC 80089BBC */ .word Actor__NoOpSlotE8
asm/data/79528.data.s:2323:  /* 7AFAC 8008A7AC */ .word Actor__NoOpSlotE8
```

Each of those table words sits exactly `+0x20` past that same instance
table's own `+0x0C8` slot (confirmed against the `Actor__MoveLocalX`
occurrences at the same table bases), i.e. `+0x0C8 + 0x20 = +0x0E8` --
the same slot `DreamSys__WallLink` calls unconditionally on `this`
(per `DreamSys__NoOpSlotE8Default`'s own comment), just a different subclass's
override of it. Not a vtable slot type change: `vtable_DreamSys` keeps
one field name per slot (its DEFAULT occupant, `DreamSys__NoOpSlotE8Default`); this is
a per-INSTANCE override value, invisible to the struct definition itself.

## Body

```c
void Actor__NoOpSlotE8(void) {
}
```

Same "callee ignores `this`" shape as `DreamSys__NoOpSlotE8Default` and
`Actor__NoOpSlotD8` (this unit) -- present only to match the caller's
calling convention, per the project's established "per-call-site
signature" precedent.

## Naming

**`Actor__NoOpSlotE8` -- tier A.** A pure no-op leaf, same reasoning
as `Actor__NoOpSlotD8`'s report. The `E8` in the name refers to the
SLOT offset this overrides, not this function's own address, since the
offset is the mechanically-confirmed fact and the purpose of the slot
itself (beyond "`DreamSys__WallLink` calls it and ignores the result")
is unconfirmed.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py Actor__NoOpSlotE8   # 2/2
```

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DreamSys__NoOpSlotE8`. Occupant of +0x0E8, empty; Actor__NotifyMove calls it on an Actor linkTarget, DreamSys overrides it (DreamSys__NoOpSlotE8Default). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/world/ObjMStyleActor.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
