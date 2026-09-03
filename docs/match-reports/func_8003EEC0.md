# func_8003EEC0 — MATCHED

Unit: `code_2cc8c_d`. Round 14, runner delta. 99/99 words, full match (4
real attempts).

## Signature

```c
void func_8003EEC0(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x09C` slot occupant (`slot9C`, dispatched by
`func_8003EE40`, this round).

## What it does

Per-frame update, guarded by `self->unk70` (only runs once
`func_8003ECD0`'s init has succeeded — that function stalled this round,
see its report, but this one is unaffected). Notifies `slotA0` if
`self->unk10->unkC` is set (passing `self->unk10` itself as the 2nd arg —
see below), updates three sub-objects (`unk40`/`unk4C`/`unk54`),
conditionally re-notifies a PsyQ helper when `unk54` is 1 or 3, resets
`self->unk30`'s pointee, recomputes `self->unk98` from `(unk50-unk4C) /
(1<<unk3C)`, forwards the current `unk74`-indexed slot to two more
helpers, dispatches `slotA0` again with `self->unkAC`, and finally — if
`self->unk10` is set — walks it to its list tail (`func_8003F25C`,
already matched this round) and dispatches `slotA0` a third time with that
tail.

```c
void func_8003EEC0(Unk18Obj *self) {
    s32 idx;
    Unk18Obj *tail;

    if (self->unk70 == 0) {
        return;
    }

    if (self->unk10->unkC != NULL) {
        ((void (*)(Unk18Obj *, GenericObj *))self->methods->slotA0)(self, self->unk10);
    }

    func_8003F28C((Unk18Obj *)self->unk40);
    func_8003FB0C(self->unk4C);
    func_8003FC70(self->unk54);

    if (self->unk54 == 1 || self->unk54 == 3) {
        u8 *rawBytes = (u8 *)&self->unk5B;
        func_80024AE4(rawBytes[0], rawBytes[1], rawBytes[2]);
        func_8003FD4C(self->unk60, self->unk40);
    }

    func_8003F2AC(&self->unk14);
    *(s32 *)self->unk30 = 0;

    self->unk98 = (u32)(self->unk50 - self->unk4C) / (u32)(1 << self->unk3C) + 1;

    idx = self->unk74;
    func_8003FBE4(*(s32 *)((u8 *)self + 0x88 + idx * 4));

    idx = self->unk74;
    func_8003FC18(0, 0, *(s32 *)((u8 *)self + 0x78 + idx * 4));

    ((void (*)(Unk18Obj *, void *))self->methods->slotA0)(self, self->unkAC);

    if (self->unk10 != NULL) {
        tail = func_8003F25C((Unk18Obj *)self->unk10);
        ((void (*)(Unk18Obj *, Unk18Obj *))self->methods->slotA0)(self, tail);
    }
}
```

## What each real attempt fixed

1. **`slotA0`'s first dispatch needs a 2nd argument, `self->unk10` itself
   (not just `self`).** Retail loads `self->unk10` into `$a1` for the
   `->unkC` truthy check, and that register is STILL `self->unk10` (never
   overwritten) when the `jalr` fires — an implicit "leftover register"
   2nd argument, same family as this project's existing "unused parameter
   shows up in the caller as genuinely uninitialized" idiom, except here
   the leftover value is meaningful, not garbage. Writing the call
   explicitly as `slotA0(self, self->unk10)` reproduces it.
2. **The division must be unsigned.** `self->unk50 - self->unk4C`, divided
   by `1 << self->unk3C`, is a plain C `/` on signed `s32` fields by
   default — which maspsx expands with an extra `INT_MIN`/`-1` overflow
   check (an `li $at,-1` guard) that retail's own `divu` (no such check)
   doesn't have. Casting both operands to `u32` before dividing drops the
   guard and matches.
3. **The division's operands must be computed inline in ONE expression,
   not through an intermediate `s32 divisor = 1 << self->unk3C;`
   variable.** Splitting it out moved the `1 << unk3C` computation (an
   `li $v0,1`) to BEFORE the preceding `*(s32*)self->unk30 = 0;` store,
   where retail computes it only after the subtraction, interleaved with
   it. One inline expression `(self->unk50 - self->unk4C) / (1 <<
   self->unk3C)` reproduces retail's own instruction order.
4. **`self->unk5B`'s three bytes are read UNSIGNED here (`lbu`), despite
   being SIGNED (`lb`) at their own writer, `func_8003EAA4`.** The same
   memory, read from two different call sites, disagrees on signedness —
   not a contradiction, just two independent readings of an opaque field.
   Cast to `u8 *` at this call site rather than retyping `SByte3_d294`
   (which would break the writer's own already-matched codegen).

## Self-caught bug: another forgotten pad, this time from THIS round's own
## earlier work

Adding `unkC` to `GenericObj` (needed for the `self->unk10->unkC` check
above) left a 4-byte gap unaccounted: `unkC` (`void *`, 4 bytes at `+0x00C`)
ends at `+0x010`, but the already-existing `unk14` (round 13) needs
`+0x014` — a forgotten `u8 pad010[0x014-0x010];`. Same failure mode as
`func_8003E8B8`'s report earlier this round (compiles clean, `build exit`
non-zero, whole-image SHA1 off by one byte, `cmp -l` + `lsdde.map`
localizes it — this time inside `func_8003E770`, round 13). Third live
instance of this exact bug class within two rounds; see the Proposed
learning below.

## Header changes

`include/code_2cc8c.h`:
- `GenericObj` gains `unkC` (`+0x00C`, `void *`) plus the corrective
  `pad010` gap.
- `Unk18Obj` gains `unk4C`/`unk50` (both `s32`, splitting the old
  `pad04C[0x054-0x04C]`) and `unk98` (`s32`, splitting `pad094`).
- `Unk18ObjMethods` gains `slotA0` — kept as an untyped `void *` and cast
  per call site, since this function alone dispatches it with three
  different arities (`self` alone is never actually used — see fix #1
  above — but `self`+`GenericObj*`, `self`+`Unk18AcObj*`, and
  `self`+`Unk18Obj*` all appear).
- Corrected two earlier comments that conflated "which function OBSERVED
  this slot" with "which function OCCUPIES it" — `slot9C`'s real occupant
  is this function (not `func_8003EE40`, which only dispatches it), and
  `slotA4`'s real occupant is `func_8003F04C` (not `func_8003EE88`, ditto).
- New externs: `func_8003FB0C`/`func_8003FC70`/`func_8003FD4C`/
  `func_8003FBE4` (all `asm/code_2cc8c_e.s`, next slice, uncarved) and
  `func_80024AE4` (PsyQ, `asm/psyq_GsLinkObject4.s`).

## Proposed learning

**Third instance in two rounds of "insert a new field, forget the leading
pad, break a DIFFERENT already-matched function silently."** (First:
`func_8001D204`/round 13's delta, a different unit entirely. Second:
`func_8003E6CC`/this round's `func_8003E8B8`. Third: this report.) All
three were caught the same way — `build exit` non-zero with no compile
error, localized via `cmp -l` (1-based!) + `lsdde.map`. This is now
clearly not a one-off: **any struct field insertion in this project should
be followed by an immediate arithmetic gap-check** (does the new field's
end offset equal the very next field's start offset, by inspection, not
just by trusting the `[hi - lo]` macro syntax to be self-correcting — it
computes the SPAN correctly but does nothing to catch a missing span
between two named fields).
