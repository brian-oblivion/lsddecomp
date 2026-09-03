# func_8003DAD4 — STALL (register-identity, 114/118 words on the best body)

**Unit:** code_2cc8c_b · round 12 straggler · slot `+0x10C` (`slot10C`,
per `Obj86B60Methods`).

## What it does

Gated on `self->unk3C == 2` (a twin of the already-matched
`func_8003DCAC`, which is gated on the same value and undoes this
function's `unk3C = 1` at the end — the two form a state-machine pair).
Builds a 2-word local buffer from a target-descriptor record
(`self->unk4C->unk24[idx]`, typed here as the new `Unk24Elem`), walks the
current slot's element array calling two per-element slots with that
buffer, then re-derives the "current" element by `counter` (the target
record's own stashed index) and finishes by writing `counter` back into
the target record's `unk4` field — which is exactly the field
`func_8003DCAC` reads back out as `newVal`, confirming the two share this
record shape.

## New struct knowledge (all confirmed, not part of the stall)

- New type `Unk24Elem` — the pointee of `self->unk4C->unk24[idx]`:
  `+0x004 s32 unk4` (the counter slot `func_8003DCAC` reads as `newVal`,
  written here), `+0x010 s32 unk10`, `+0x014 s32 unk14` (combined with a
  per-slot counter into the 2-word local buffer passed to `slotBC`).
- New `Unk64ElemMethods` slot `+0x0BC slotBC(self, void *buf)`.
- `self->unk68->methods->slot50(self)` — new `Unk68ObjMethods` slot
  `+0x050` (no args beyond self).

All of this is corroborated independently by `func_8003D73C` (the other
stall this round, which touches the SAME `Unk24Elem`/`slotBC`/`unk68.slot50`
surface) and is safe to keep in `include/code_2cc8c.h` regardless of this
function's own stall status.

## Where it stalled

Best body reaches **114/118 words**, with the whole-image build otherwise
green (this body was reverted to `INCLUDE_ASM` before merge — CLAUDE.md is
explicit that no score short of byte-exact stays in `src/`). The remaining
4 words are TWO independent, purely cosmetic mismatches with no semantic
difference:

1. **One `i = 0` loop-counter initialization schedules into a different
   (but equally valid) delay slot than retail chose** — `move sN, zero`
   lands 4 bytes earlier or later depending on unrelated register-pressure
   decisions elsewhere in the function. Every restructuring tried (moving
   the statement, wrapping it in its own block, an explicit `for(i=0;...)`
   vs. a pre-loop `i=0;`, a bare `__asm__("")` scheduling barrier
   immediately before the loop) either left it unchanged or actively
   regressed the rest of the function's byte count. A bare scheduling
   barrier is the CLAUDE.md-sanctioned tool for exactly this (order-only,
   no register pinned) but a barrier only orders MEMORY-touching
   operations relative to itself — a pure register move like `i = 0` has
   no memory side effect for the barrier to anchor, so it can still float
   across.
2. **One temp register choice for the raw `target->unk14` load differs**
   (`a1` in retail vs. `v0` in the best body) at the exact point building
   the local buffer's second word. Every variant tried (separate named
   temp, direct field access, reordering relative to the first field's
   load) either left this unchanged or (twice) triggered a much bigger
   regression elsewhere, confirming the two are linked through the
   compiler's own register allocation in a way this project's permitted
   toolset (source restructuring, bare scheduling barriers) can't
   independently steer.

This is the class of thing CLAUDE.md calls out directly: *"if removing it
changes WHICH REGISTER holds a value, it is banned; if it only changes
instruction ORDER, it is allowed... A register-identity mismatch is a
STALL."* Both remaining diffs are exactly that — a different but
equally-valid register/scheduling choice, not a misunderstanding of the
function's logic (confirmed: every OTHER instruction in the function,
including all four call sites, both loops, and the final field write,
matches byte-for-byte).

### Head note (round 12) — this stall is NOT the same class as `func_8003D73C`

Accepted as written; the classification above is correct for THIS function.
Flagging only because the companion report filed the same round
(`func_8003D73C.md`) described itself as "the same whole-function s-register
renumbering shift documented in `func_8003DAD4`'s report". It is not, and this
report never claimed such a shift:

- Retail here saves **6** callee-saved registers (`$s0..$s5`), leaving two
  s-registers and `$fp` spare — no pressure, and the 114/118 residue is purely
  scheduling and one temp choice.
- Retail in `func_8003D73C` saves **8** (`$s0..$s7`), saturating the file, and
  that body's 40/145 comes from needing a 9th live cross-call value and
  spilling into `$fp`.

```sh
grep -oE 'sw +\$s[0-9]' asm/nonmatchings/code_2cc8c_b/func_8003DAD4.s | sort -u | wc -l   # 6
grep -oE 'sw +\$s[0-9]' asm/nonmatchings/code_2cc8c_b/func_8003D73C.s | sort -u | wc -l   # 8
```

Do not carry this function's fixes to that one expecting them to transfer —
the companion report observed that they did not, and the register census above
is why. See the reclassification in `func_8003D73C.md`.

## What DID work, for the next attempt

Getting from a naive first draft (~10/118, ~180KB of image-wide address
drift from oversized code) to 114/118 took several real fixes, in this
order:

1. **`local[1] = target->unk14 - counter * 10;` as ONE expression silently
   drops a real memory round-trip.** Retail stores the RAW `target->unk14`
   to the stack slot first, then reloads it before subtracting — GCC's
   own optimizer, given the compound form, keeps the value in a register
   and skips the redundant store/reload entirely (verified in isolation
   with the pinned toolchain: `local[1] -= counter*10;` written as either
   one statement or two adjacent statements both compile to a single
   store, no matter how the two are shaped in C). The fix that actually
   reproduces retail's double-store: split the two field reads into named
   temps declared TOGETHER (`s32 t0 = ...; s32 t1 = ...;` both assigned to
   `local[]` before either is used further), THEN a
   `__asm__("" ::: "memory")` barrier, THEN the subtraction as its own
   statement. The memory-clobber barrier forces the two stores to actually
   happen (no register pinned — it's a barrier, not an asm operand), and
   using two named temps for the two loads (rather than the array elements
   directly) makes them land in two DIFFERENT scratch registers matching
   retail's `v1`/`a1` split instead of reusing one register twice.
2. **The element-array loop must NOT cache `*arr` across both per-element
   calls.** First cut: `Unk64Elem *elem = *arr; ...slot60(elem,0);
   ...slotBC(elem,local);`. Retail RELOADS `*arr` independently for the
   SECOND call too (two separate `lw`s from the same not-yet-incremented
   address) rather than keeping the first load's value alive across the
   first call. Rewriting the loop body as `(*arr)->methods->slot60(*arr,
   0); (*arr)->methods->slotBC(*arr, local);` (no named `elem` at all)
   dropped the function from 9 live callee-saved registers to 8 and fixed
   the entire loop body to a byte-for-byte match — this was the single
   biggest jump (43 → 108/118 words in one edit).
3. **The final `arr[counter]` re-derivation needs its OWN fresh variable,
   not the loop's `arr`.** Reusing the loop's `arr` name for
   `self->unk64[idx]` a second time (after the loop) kept it tied to the
   same persistent register the loop needed, forcing the intermediate
   `self->unk64[idx]` value into a saved register too. Naming it `arr2`
   (a genuinely fresh local, scoped to just that one use) let the
   compiler put it in a scratch register instead, closing another 2-word
   gap (108 → 112).
4. **Declaration/statement ORDER inside a single expression (e.g. which
   of `target`/`counter` is computed first, which of `t0`/`t1` is
   declared first) has NO effect on the compiled output** — verified
   repeatedly by swapping and rebuilding; GCC 2.6.3's scheduler here is
   driven by data dependencies, not source order, for anything within one
   basic block. Don't spend attempts reordering independent statements
   hoping to influence register choice; it doesn't.

## Preserved near-miss body

```c
void func_8003DAD4(Obj86B60 *self)
{
    s32 idx;
    s32 counter;
    s32 local[2];
    Unk64Elem **arr;
    s32 count;
    s32 i;

    if (self->unk3C != 2) {
        return;
    }
    idx = self->unk58;
    counter = self->unk60[idx];
    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];
        s32 t0 = target->unk10;
        s32 t1 = target->unk14;

        local[0] = t0;
        local[1] = t1;
    }
    __asm__("" ::: "memory");
    local[1] -= counter * 10;

    arr = (Unk64Elem **)self->unk64[idx];
    count = self->unk5C[idx];
    for (i = 0; i < count; i++) {
        (*arr)->methods->slot60(*arr, 0);
        (*arr)->methods->slotBC(*arr, local);
        local[1] += 10;
        arr++;
    }

    {
        Unk64Elem **arr2 = (Unk64Elem **)self->unk64[idx];
        Unk64Elem *elem = arr2[counter];

        elem->methods->slot60(elem, 1);
        elem->methods->slotB8(elem, self->unk4C->unk10);
    }

    {
        Unk24Elem *target = (Unk24Elem *)self->unk4C->unk24[idx];

        target->unk4 = counter;
    }

    self->unk68->methods->slot50(self->unk68);

    self->unk3C = 1;
    self->methods->slot60(self, 0x10);
}
```

Requires (already committed to `include/code_2cc8c.h`, so this body
compiles as-is against current `main`): `Unk24Elem`, `Unk64ElemMethods`
with `slot60`/`slotBC`/`slotB8`, `Unk68ObjMethods` with `slot50`,
`Obj86B60Methods` with `slot60`, `Obj86B60.unk68`/`unk4C`/`unk58`/`unk3C`,
`Unk4CObj.unk10`/`unk24`/`unk5C`/`unk60`/`unk64`.

### Proposed learning

A CACHED single-load-reused-across-two-calls pattern and a
DOUBLE-RELOAD-per-call pattern are BOTH real, observed shapes in this
codebase (see `func_8003D194`'s report for the tail-merge angle) — which
one retail used is NOT guessable from the C alone; it shows up ONLY as a
register-count difference in the diff (one extra callee-saved register
needed = the value is being kept alive across a call it doesn't need to
survive). When funcdiff/asm-differ shows a uniform s-register renumbering
shift across an ENTIRE function with every individual instruction
otherwise correct, look for exactly this: a local variable whose value is
consumed by TWO OR MORE calls where retail re-derives it via a second
cheap memory read instead of holding it in a register across the first
call.
