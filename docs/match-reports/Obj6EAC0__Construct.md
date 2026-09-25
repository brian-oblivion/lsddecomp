# Obj6EAC0__Construct — MATCHED (58/58 words)

> Renamed from `func_80040948` on 2026-09-18 (tools/rename.py). Address 0x80040948.

Unit: `src/code_2cc8c_f.c`. 12 attempts. **Re-attempted after a
coordinator correction**: this function had originally been triaged
out as "predicted-hard" purely on its 6-callee-saved-register count,
using a threshold derived from round 13's 22-sample ceiling of 4. A
pooled census across rounds 13+14 (81 matched, 10 stalled) showed the
5-6 register band is 83% MATCHED — the earlier triage was wrong, and
this function proves it.

```c
void Obj6EAC0__Construct(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3) {
    s32 i;
    Obj6EAC0 **cursor;

    GetCharSpriteMethods()->ctor((CharSprite *)self, (void *)a1, 0x20);
    self->methods = Obj6EAC0__GetDerivedMethods();
    self->unkA9 = a2;
    self->unkAB = a2;
    self->unkAC = 0;
    self->unkAA = 0;
    cursor = BMemPMgrAlloc(a2 * 4);
    if (cursor != NULL) {
        self->unkB4 = cursor;
        i = 0;
        if (i < a2) {
            do {
                *cursor = (Obj6EAC0 *)New_CharSprite((void *)a1, 0x20);
                i++;
                cursor++;
            } while (i < a2);
        }
        self->methods->slot40(self, a3);
    }
}
```

The derived table's constructor (`Obj6EAC0Methods::slot08`), called by
this unit's own `New_Obj6EAC0` via
`Obj6EAC0__GetDerivedMethods()->slot08(self, ctx, len, name)`. Chains to the THIRD
sibling table's own `slot08` (a base-class-style constructor call
through `GetCharSpriteMethods()`, same shape as `New_Obj6EAC0` calling
`Obj6EAC0__GetDerivedMethods()->slot08`), then sets up `self->methods` to the
DERIVED table, zeroes/initialises the slice-index fields, allocates an
`a2`-element child array via `BMemPMgrAlloc`, fills each slot by
calling the external New_X-shaped allocator `New_CharSprite(a1, 0x20)`
(itself a "return-regardless" New_X variant, not this unit's function
to attempt), and — only if the array allocation succeeded — dispatches
`self->methods->slot40(self, a3)`.

## The decisive lever: a genuinely 3-argument call site

The residue, once the loop and allocation shape were right, was ONE
extra instruction: `move $a3, $s5` immediately before the
`GetCharSpriteMethods()->slot08` call, which retail does not have. Every
straightforward call written as `slot08(self, a1, 0x20, a3)` (passing
`a3` explicitly, matching the struct's WIDEST declared arity) forces
GCC to reload `a3` from its callee-saved cache back into the argument
register — because by that point in the function `a3` has already been
promoted to a callee-saved pseudo (it must survive to the LATER
`slot40` call regardless).

Retail does NOT reload it. The resolving insight: **this first call is
genuinely a 3-argument call** — `self`, `a1`, and the literal `0x20`
— and whatever sits in the hardware `$a3` register at that moment
(still the function's own untouched incoming 4th parameter, since
nothing has called anything yet) is LEFTOVER, not a real 4th argument,
exactly the "leftover register implicit argument" shape already
documented for other functions in this project. Casting the slot
pointer down to a narrower, 3-parameter function-pointer type for
THIS call site only —
`((void (*)(Obj6EAC0 *, s32, s32))GetCharSpriteMethods()->slot08)(self, a1, 0x20)`
— removes the compiler's obligation to materialise `a3` into the
argument register at all, and the leftover value in `$a3` (never
touched since function entry) does the rest for free. Zero-attempt
verification that this doesn't disturb the OTHER call through
`slot08` (`New_Obj6EAC0`, which genuinely does use 4 args): that call
still goes through the struct's own canonical 4-arg field, untouched.

## Attempts (12)

1. Direct translation with `self->unkB4[i]` array indexing instead of
   an incrementing cursor pointer, `a3` passed directly to both
   `slot08` and `slot40` — near-total register mismatch (retail uses 6
   callee-saved registers; this compiled to only 5, since the array-
   indexing loop let the allocator coalesce `i`'s register with `a3`'s
   after `a3`'s own first use).
2. Same, but split `a3`'s later use into a separate `arg4` local
   assigned right after the first call — no change; GCC coalesces the
   copy back into the same pseudo.
3. Replaced array indexing with an explicit incrementing
   `Obj6EAC0 **cursor` (the established "walk with an incrementing
   pointer" idiom) — jumped to 41/58, loop body byte-identical to
   retail, only two residues left: (a) the allocation result tested
   via the raw return value instead of a cached local, and (b) the
   `a3` redundant reload.
4. Fixed (a): cache the allocation result into `cursor` FIRST, test
   `cursor`, and only THEN store it into `self->unkB4` and use it as
   the loop's own starting point (matching the "cache a value before
   testing it" idiom, and reusing register identity between the cache
   and the loop cursor, as retail does with `$s1`) — closed that
   residue; only the `a3` move remained.
5. Bare `__asm__("");` immediately before the first call — WORSE (grew
   the diff and the frame), confirming (per CLAUDE.md rule 6's own
   test) this residue is not an ordering question and the barrier does
   not belong here. Reverted.
6. `a3 + 0` in place of `a3` at the call site — no change (GCC folds
   the no-op arithmetic away).
7-11. Minor variations on temp placement and declaration order for the
   `arg4`/`a3` split (before vs. after the first call, combined with
   attempt 3's cursor fix) — none moved the single-instruction residue.
12. **Cast the call to a narrower 3-argument function-pointer type**
   (dropping the explicit `a3` argument entirely, relying on the
   leftover register) — 58/58, byte-exact.

### Proposed learning

- **The register-saturation screen deprioritises; it does not predict
  failure**, and this round's pooled census (81 matched / 10 stalled)
  shows the 5-6 register band is majority-MATCHED (5 of 6) — a
  correction to round 13's original, too-narrow reading of its own
  22-sample ceiling. Treat 5-6 registers as ordinary difficulty; only
  7+ has zero confirmed matches across both rounds so far.
- **The "leftover register implicit argument" idiom generalises to
  chained/inherited constructor calls, not just ordinary method
  calls.** When a constructor forwards to a "base class" constructor
  through a SIBLING vtable (here, `GetCharSpriteMethods()`'s third table)
  using fewer arguments than the slot's own widest declared arity, the
  fix is a per-call-site cast to a NARROWER function-pointer type
  (leaving the struct's canonical field type at its widest use) —
  exactly the same shape as this project's existing per-call-site
  arity convention for `slotC4`/`slot4C`'s narrower dispatches, but
  achieved here via an explicit CAST (since `slot08` stays prototyped,
  unlike those two which were made unprototyped). This is the
  preferable fix over widening a slot to unprototyped when only ONE
  narrower call site exists and the widest call site is otherwise
  clean — prefer a cast at the odd site over weakening the whole
  slot's type checking for everyone.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040948` | `Obj6EAC0__Construct` | B |

**Evidence.** Derived occupant of `slot08`: chains to the third sibling
table's own `slot08` first (base-class-style construction), sets
`self->methods` to the derived table, initialises the child-slice fields
(`totalChildCount`, `childCount`, `childStart`, `gapIndex`), allocates an
`a2`-element `children` array and fills each slot via the external
New_X-shaped `New_CharSprite`, then dispatches `self->methods->slot40`.
This is unambiguously a constructor by mechanics (allocates + wires up
the object this class needs); named `Construct` per the project's
`Class__Class`-shaped constructor convention rather than `Class__Class`
literally, since `Obj6EAC0` is a placeholder table-address name, not a
real class identity -- tier B (the CTOR role is certain, the class's own
purpose is the unit's working hypothesis only).

## Track 4

2026-09-26, round 86 (bravo): the parent class 0x1144 is unified as CharSprite (`include/CharSprite.h`, formerly D_8006EC74; this class, 0x11144, is still its own job). The chain to the parent ctor is now `GetCharSpriteMethods()->ctor((CharSprite *)self, (void *)a1, 0x20)`, still a genuine 3-argument prototyped call, which this report's lever needs; the old spelling cast `slot08` to a local function type. New_CharSprite's result is cast back to the view's `Obj6EAC0 *`: the children are CharSprites, not objects of this class, which the view's `children` comment does not say. Both casts emit no code. Image byte-identical.
