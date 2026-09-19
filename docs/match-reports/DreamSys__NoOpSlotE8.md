# DreamSys__NoOpSlotE8 -- MATCHED (2/2)

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family. No match report
existed before this round (same situation as `DreamSys__NoOpSlotD8`,
this unit): a splat-matched, length-exact two-word leaf (`jr $ra; nop`),
never a `STALL`, just never documented.

## Which slot this overrides

`include/DreamSys.h`'s `vtable_DreamSys` names the DEFAULT `+0x0E8` value
`func_800590E0` ("this unit's own no-op stub... Called by DreamSys__WallLink
as (this)"), a DIFFERENT function belonging to a different unit. This
function (`0x80057C74`) is a SIBLING override of that SAME conceptual
slot, used by different subclass instance tables:

```
asm/data/76DC8.data.s:1661:  /* 77FE4 800877E4 */ .word DreamSys__NoOpSlotE8
asm/data/76DC8.data.s:1842:  /* 781BC 800879BC */ .word DreamSys__NoOpSlotE8
asm/data/79528.data.s:1274:  /* 7A3BC 80089BBC */ .word DreamSys__NoOpSlotE8
asm/data/79528.data.s:2323:  /* 7AFAC 8008A7AC */ .word DreamSys__NoOpSlotE8
```

Each of those table words sits exactly `+0x20` past that same instance
table's own `+0x0C8` slot (confirmed against the `DreamSys__ApplyOffsetSlot0`
occurrences at the same table bases), i.e. `+0x0C8 + 0x20 = +0x0E8` --
the same slot `DreamSys__WallLink` calls unconditionally on `this`
(per `func_800590E0`'s own comment), just a different subclass's
override of it. Not a vtable slot type change: `vtable_DreamSys` keeps
one field name per slot (its DEFAULT occupant, `func_800590E0`); this is
a per-INSTANCE override value, invisible to the struct definition itself.

## Body

```c
void DreamSys__NoOpSlotE8(void) {
}
```

Same "callee ignores `this`" shape as `func_800590E0` and
`DreamSys__NoOpSlotD8` (this unit) -- present only to match the caller's
calling convention, per the project's established "per-call-site
signature" precedent.

## Naming

**`DreamSys__NoOpSlotE8` -- tier A.** A pure no-op leaf, same reasoning
as `DreamSys__NoOpSlotD8`'s report. The `E8` in the name refers to the
SLOT offset this overrides, not this function's own address, since the
offset is the mechanically-confirmed fact and the purpose of the slot
itself (beyond "`DreamSys__WallLink` calls it and ignores the result")
is unconfirmed.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py DreamSys__NoOpSlotE8   # 2/2
```
