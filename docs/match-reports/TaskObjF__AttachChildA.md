# TaskObjF__AttachChildA -- MATCH

> Renamed from `Class86E00_3bb8c_g__AttachChildA` on 2026-09-23 (tools/rename.py). Address 0x80050340.

> Renamed from `func_80050340` on 2026-09-23 (tools/rename.py). Address 0x80050340.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__AttachChildA`: 52/52 words match.

## Source

```c
void TaskObjF__AttachChildA(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk78 == NULL) {
            self->unk78 = New_Obj86ED0((self->unk48 << 1) + self->unk44, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk78);
        self->unk78->methods->slot44(self->unk78, self->unk68);
        self->unk78->methods->slot4C(self->unk78, self->unk60, self->unk64, self->unk6C);
    }
}
```

## Derivation

Straightforward transcription: two readiness gates (`unk68`, `unk60`), a
lazy-init of `unk78` guarded by a null check, then three calls (one on
`self`'s own vtable, two on `unk78`'s). The combined `&&` guard, unlike
`func_8004E230` last round, compiled to the exact same shape as retail's
two separate `beqz`s here -- both share the identical target label, so
there was no range-check fold at stake and the `&&` form worked first try
for that part.

## One near-miss (51/52), and a wrong external signature

One residue: a redundant `ori $a1, $zero, 0x1` in retail, in the delay
slot of the guard testing `self->unk78 == NULL` -- several instructions
before `New_Obj86ED0` is even called, and dead on the "skip init" path.
My first reading treated this as a scheduling artifact (nothing in my C
mentioned `1` that early) and tried reordering `self->unk74 = 1;` earlier
in the block to coax the compiler into hoisting it there -- that made
things WORSE (13/52, and the function shrank by a word, because the
reorder let the compiler merge/reuse the literal differently instead of
reproducing retail's layout).

**The actual cause: `New_Obj86ED0` takes a second argument, not one.**
Nothing overwrites `$a1` between where it is set (the branch's delay
slot) and the `jal New_Obj86ED0` several instructions later, which is
exactly the established test for "a value surviving to a call is a real
argument" (already documented for `$a0`-`$a3`, here applied to a
helper OUTSIDE this unit's own slice, reached only through this one call
site). Retyping the extern from `New_Obj86ED0(s32 arg0)` to
`New_Obj86ED0(s32 arg0, s32 arg1)` and calling it with `(..., 1)`
matched immediately, with the ORIGINAL statement order (`unk78 = ...;
unk74 = 1;`) restored.

## Struct changes (additive, `include/class_3bb8c.h`)

- `extern void *New_Obj86ED0(s32 arg0, s32 arg1);` **retyped** from a
  single-argument declaration added while surveying the unit
  (`TaskObjF__TickCardIcon`'s report) -- this function is the only call site in
  this unit, so the correction is fully contained.
- `Class86E00SubObjMethods_3bb8c_g::slot44` **retyped** from `(self)` to
  `(self, s32 arg1)` -- the earlier single-argument declaration (also
  from the initial survey) missed that `self->unk68` is loaded into `$a1`
  immediately before this call. Confirmed against `TaskObjF__AttachChildB`'s own
  identical call shape (not yet matched, but its `.s` shows the same
  `lw $a1, 0x68($s0)` pattern), so both callers agree on the corrected
  arity.

### Proposed learning

**Before treating an unexplained delay-slot literal as a scheduling
artifact, check for a call whose argument-register conventions it could
be feeding -- even across a call boundary to a function OUTSIDE the
current unit.** The already-documented test ("a value in an argument
register live at the next call IS an argument") is usually applied within
one function's own body; this instance shows it working identically when
the "next call" is to an external helper this unit does not own the body
of. The reflex to try statement reordering first (rather than checking
call arity) cost one wasted, and materially WORSE, attempt.

## Naming

`TaskObjF__AttachChildA` (was `func_80050340`), tier B: lazily
allocates `self->unk78` via `New_Obj86ED0` (an already-named `New_X`-shaped
factory for the same real class the sibling `class_3bb8c_i`/`class_3bb8c_j`
units call `Obj86ED0`/`Class86ED0`, vtable `D_80086ED0`) on first use, then
attaches and configures it through this class's own vtable. Named "A" to
distinguish it from the identically-shaped `AttachChildB`
(`self->unk7C`, `New_Class86F88`) below -- nothing in either function's own
body says what makes the two children functionally different, so the
suffixes are arbitrary labels, not a claim about purpose.
