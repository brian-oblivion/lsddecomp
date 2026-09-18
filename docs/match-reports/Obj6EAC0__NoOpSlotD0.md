## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040FA0` | `Obj6EAC0__NoOpSlotD0` | A |

**Evidence.** An empty function body (`{ }`), splat-generated (`jr $ra;
nop`). `slotD0` is derived-only -- the base table has no occupant for it
at all (see `include/code_2cc8c.h`'s `Obj6EAC0Methods` comment) -- so this
is a reserved/unused slot filled with a do-nothing stub rather than an
override of a real base behaviour, unlike `Obj6EAC0__NoOpSetter`
(`slotC8`, which DOES override a real base setter). Kept the slot number
in the name rather than inventing a guessed purpose for an unused slot.
