> Renamed from `func_800405D0` on 2026-09-20 (tools/rename.py). Address 0x800405d0.

# ClassEAC0__FinishConstruct -- MATCH (37/37 words, first attempt)

Unit `code_2cc8c_e`, carved round 14. `ClassEAC0Methods::slot40` (`+0x040`),
dispatched by the class's own ctor (`ClassEAC0__ClassEAC0`).

```c
void ClassEAC0__FinishConstruct(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    ClassEAC0Methods *methods;

    self->unk44 = a3;
    self->unk48 = 1;
    self->unk4C = 0;
    self->unk58 = 0;
    self->unk5C = 0;
    self->unk5E = 0;
    self->unk60 = a1->x;
    self->unk62 = a1->y;
    methods = self->methods;
    if (a2 == NULL) {
        a2 = D_8008A924;
    }
    methods->slotB8(self, 1, a2);
    self->methods->slotCC(self, 0xD);
}
```

**Correction: `self->methods` needs to be hoisted into a local BEFORE the
`a2` null-check**, not read fresh at the `slotB8` call site. An earlier
version of this report read `self->methods` inline at the call and claimed
37/37 -- taken during the stale-build window described in
`New_Class6E99C.md`. Genuinely rebuilt, that inline form regressed
(16/37): after `Class6E99CObj::unk60`/`unk62` were retyped `s16` -> `u16`
(see `Class6E99C__PushPosition.md`), GCC stopped loading `self->methods` early and
deferred it past the `a2` check instead, where retail loads it right after
storing `unk62`. Hoisting it into a named local restores retail's early
load. Worth noting as a SHARED-FIELD side effect: a field TYPE change in
`include/code_2cc8c.h` regressed an unrelated, already-matched function's
SCHEDULING (not its correctness) purely by changing surrounding register
pressure -- a milder cousin of the project's documented "any struct edit
is potentially non-local" caution, this time via codegen shape rather than
a byte-offset shift.

## Misread caught before committing

First reading treated `a2` as a MODE flag (0 vs nonzero) selecting between
two fixed tables (`D_8008A924`/`D_8006EAA8`). Re-reading the raw
instructions closely: retail's `bnez $a2, .L80040628` branches on `a2`
ITSELF being nonzero, and when `a2 == 0` the fallthrough OVERWRITES `a2`
with `&D_8008A924` before the shared call -- i.e. `a2` isn't a flag at
all, it IS the `tableEntry` pointer, with `D_8008A924` only as the
default when the caller passes `NULL`. Same family as the project's
established "a discarded/defaulted value is not evidence of the wrong
type" caution, just for a parameter rather than a return.
