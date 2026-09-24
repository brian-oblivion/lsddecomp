# Class86F88__AddChildAndSetState -- MATCHED (27/27 words)

> Renamed from `Class86F88_3bb8c_j__AddChildAndSetState` on 2026-09-24 (tools/rename.py). Address 0x80052110.

> Renamed from `func_80052110` on 2026-09-24 (tools/rename.py). Address 0x80052110.

Unit: `src/class_3bb8c_j.c`. `self` is `Class86F88_3bb8c_j`.

## Body

```c
void Class86F88__AddChildAndSetState(Class86F88_3bb8c_j *self, void *arg1, s32 arg2, s32 arg3)
{
    typedef void (*Slot10NarrowFn)(Class86F88_3bb8c_j *self, s32 arg1);
    void (*fn)(Class86F88_3bb8c_j *self, void *arg1, s32 arg2, s32 arg3);
    s32 zero;

    zero = 0;
    fn = self->methods->slot10;
    do {
        fn(self, arg1, arg2, arg3);
        ((Slot10NarrowFn)self->methods->slot10)(self, arg2);
        self->unk3C = arg3;
        self->unk2C = zero;
    } while (0);
}
```

Calls `Class86F88Methods_3bb8c_j::slot10` (established this round) TWICE at
different arities from the SAME function -- the first call is a straight
passthrough of this function's own `(self, arg1, arg2, arg3)` (nothing
overwrites `$a1`-`$a3` between function entry and the first `jalr`), the
second passes only `(self, arg2)` (confirmed by the disassembly: no `$a2`/
`$a3` setup before the second `jalr`, matching DECOMPILATION_LEARNINGS'
"per-call-site arity" convention). The narrower 2-arg call needs an
explicit function-pointer cast (`Slot10NarrowFn`) since C cannot call a
4-parameter function pointer with 2 arguments.

Also establishes `Class86F88_3bb8c_j::unk2C` (cleared to 0) and `unk3C` (set to
`arg3`).

## Residue and how it closed

Straightforward direct translation (`self->methods->slot10(self, arg1,
arg2, arg3); ((Slot10NarrowFn)self->methods->slot10)(self, arg2);
self->unk3C = arg3; self->unk2C = 0;`) scored 3/27 with a ONE-WORD
size overshoot: retail schedules the preservation of `arg2` (into what
becomes a callee-saved register) into the LOAD-DELAY SLOT between
`self->methods` and `self->methods->slot10`'s own load (the two `lw`s of
the first call), for free; my direct form left that delay slot a plain
`nop` and instead materialized the same preservation as an extra,
separate instruction right at function entry.

Ran the permuter (`tools/setup-permuter.sh`, found in ~5000 iterations).
The zero-score candidate's structural difference from my code: caching
`self->methods->slot10` into a local BEFORE the first call, caching the
literal `0` into a local BEFORE either call, and wrapping the whole body
in a `do { ... } while (0)`. Translating the cache-into-locals part alone
did NOT reproduce the zero; only adding the `do { } while (0)` wrapper
around the body closed it. Removing the wrapper (keeping everything else
identical) reproduces the original 3/27 score exactly -- confirmed by
direct A/B test.

### Proposed learning

**A `do { ... } while (0)` wrapping an otherwise-unconditional function
body can be load-bearing for GCC 2.6.3's instruction scheduling, not just
a stylistic/macro-hygiene artifact.** Here it was the deciding factor
between a delay-slot-filling `move` (retail's shape) and a wasted `nop`
plus a separate early `move` (one word longer) -- with IDENTICAL
statement content inside vs. outside the wrapper. Suspect this lever when
a residue is a single extra/missing instruction tied to delay-slot
filling around a `self->methods->slotN` call and reshaping the statements
alone doesn't move it. Not yet understood WHY the wrapper changes
scheduling (a basic-block boundary artifact of `-O2` in this compiler is
the working guess); flagging as a confirmed-but-unexplained mechanism for
whoever investigates next.

## Naming

- `Class86F88__AddChildAndSetState` -- tier B. The slot4C occupant (classtable.py D_80086F88 +0x04C): dispatches self->methods->slot10 (this class's own addChild override) twice, once at full arity and once narrowed to (self, arg2) via a local function-pointer typedef, then sets unk3C=arg3 and clears unk2C. Mechanics established across rounds 18-19's residue hunt; the double-arity dispatch's in-game reason is not.
