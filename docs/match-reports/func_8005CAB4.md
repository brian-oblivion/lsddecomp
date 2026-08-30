# func_8005CAB4 -- MATCHED

Unit `code_4cd08` ("DreamAux"). 69/69 words, `0x4D2B4`-`0x4D3C8`. Whole-image
`build-and-verify.sh` green.

```c
bool func_8005CAB4(s32 value, void *ctx, TriggerRecord *record, TriggerWorld *world)
{
    s8 *p;
    s8 *end;
    void *callResult;
    s32 scratch[4];

    if (!func_8005CBC8(value, record)) {
        goto fail;
    }

    func_8005C714(record->kind);

    p = record->entries;
    end = record->entries + 4;
    callResult = ((TriggerWorldFn)world->vtable[0x22])(world, record->parity);
    scratch[3] = (s32)callResult;

    if (callResult == NULL) {
        goto skip;
    }

    while (p < end) {
        if (*p == -1) {
            break;
        }
        if (func_8005CDF8(record->kind, scratch, ctx, (u8)*p)) {
            return true;
        }
        p++;
    }

skip:
    if (record->kind == 2) {
        return func_8005CAB4(value, ctx, record + 1, world);
    }

fail:
    return false;
}
```

## What it does

Recursive walk over an array of `TriggerRecord` (stride `0x38`, confirmed by
the recursive call passing `record + 1`). For the current record: validate
it against `func_8005CBC8(value, record)` (bail out `false` if rejected);
run `func_8005C714` on `record->kind` (a trigger-type side-effect, already
matched -- calls `SetTeleportsEnabled` for specific kind values); call
through `world`'s vtable slot `0x88` with `record->parity`; if that returned
non-NULL, try each of up to 4 sentinel(`-1`)-terminated entry bytes against
`func_8005CDF8`, returning `true` on the first one that accepts; if none did
(or the vtable call returned NULL) and `record->kind == 2`, recurse into the
next record in the array; otherwise return `false`.

## Types derived, and what's still unknown

New in `include/code_4cd08.h`: `TriggerRecord` (fields at `0x1`-used-by-
`func_8005CBC8`... actually not yet named there, `0x2` `parity`, `0x3`
`kind`, `0x4..0x7` `entries[4]`, stride `0x38` total), `TriggerWorld` (a
method-table object per CLAUDE.md's offset-0 convention, only slot `0x88`
known), `TriggerWorldFn`. `func_8005CBC8`'s and `func_8005CDF8`'s own bodies
are NOT derived here -- `func_8005CBC8` is next in this unit's queue (own
report), `func_8005CDF8` is gp-relative-blocked and stays `INCLUDE_ASM`
(off-limits per the round's assignment) -- only their call-site signatures
were reverse-engineered from register usage, per CLAUDE.md's "calling into
a function that is still INCLUDE_ASM in another unit is fine."

`value` (param 0, `$a0`/`$s6`) is a plain `s32`, not a pointer -- confirmed
because it flows straight into `func_8005CBC8`'s own `a0`, which that
function's body (read for context, not yet converted) uses in `mult`/mod
arithmetic, never as an address.

## The residue chase, and what actually mattered

First attempt (structured as `bool result = false; if (cbc8check) { ...
result = ...; } return result;`, `do { } while` for the entries loop) built
green but only 10/69, with an EXTRA callee-saved register (`$s7`) holding
`result` and a structurally different loop (retail shares one test point
between loop-entry and the back-edge; mine had two separate reload+compare
sites). Per the head's broadcast on branch-target checking
(`func_8005CAB4` was in fact the case this round that needed it): lining up
branch targets in `asm-differ` showed every target disagreed, not just
delay-slot content -- a real CFG difference, not a scheduling stall.

Fixes, in order:

1. **Drop the `result` local entirely; use direct/tail `return` and `goto`
   for early exits instead of assign-then-fall-through-to-one-`return`.**
   `return true;` inside the loop, `return func_8005CAB4(...);` for the
   recursive tail call, plain `goto fail; ... fail: return false;` for the
   initial reject. This removed `$s7` and got every later register
   (`s0`-`s6`) to match retail's allocation. Consistent with the
   `func_80025B34` broadcast's "goto vs return" lever, generalized: the
   lesson there was about a *single* early exit; here it applied to
   *multiple* exit points at once, and avoiding a persistent "result"
   variable across a function call was the bigger win.
2. **`while (p < end) { if (*p==sentinel) break; ...; p++; }`, not
   `do { ...; p++; } while (p < end)` with the bound-and-sentinel check
   combined up front.** The `do-while` compiled to two separate
   reload-and-compare sites (loop-top plus a second one after the
   increment); the plain pre-test `while` let GCC's own loop rotation
   produce retail's single shared test point reused by both the entry and
   the back-edge. This is the branch-target-driven fix the broadcast
   described -- once the CFG shape was right, EVERY branch target in the
   `asm-differ` output lined up, taking the score to 19/69 with only one
   residue left.
3. **The last residue (19/69 -> 69/69): `scratch[3] = (s32)callResult;`.**
   Retail spills the vtable-call's return value to `0x1c($sp)` -- the LAST
   word of the same 4-word (`0x10`-byte) stack buffer passed to
   `func_8005CDF8` as its `out` parameter -- immediately after the call,
   before the `NULL` check. This isn't a dead spill: it is the caller
   pre-populating part of the buffer `func_8005CDF8` reads, which is why no
   amount of reshaping the *control flow* around the `NULL` check moved it
   (tried: wrapping the loop in `if (callResult != NULL) { while ... }` vs.
   `if (callResult == NULL) goto skip;` -- identical either way, still
   19/69, confirming it was never a control-flow residue at all).

## Register/goto levers checked (per head broadcasts)

- **Early exit returning a different value (Lever, `func_80025B34`):**
  applies, generalized to *multiple* exit points converging on one epilogue
  via `goto`/direct `return` rather than a shared `result` variable -- see
  fix (1) above. This is the main lesson of this function.
- **Branch-target check before classifying a residue (broadcast #3):** this
  is the function that motivated it. The first attempt's residue looked
  like it could be dismissed as "extra register, different loop shape,
  probably unreachable" -- checking branch targets showed it was a genuine
  CFG difference from a `do-while` vs. pre-test `while` choice, fixable from
  C. Filing that as a stall without the branch-target check would have been
  wrong.
- **"Let GCC hoist its own loop invariants" (`func_80025D10`):** `p`/`end`
  are named pointers set once before the loop, not hand-maintained byte
  offsets -- already the natural shape here, no separate experiment needed.

## Proposed learning

- **A caller-populated "output" buffer can have a field set by the CALLER
  before the callee is ever invoked, and that field can be entirely
  unreferenced again by the caller.** `scratch[3] = (s32)callResult;` here
  looks exactly like dead code from the caller's own perspective (nothing in
  `func_8005CAB4` reads `scratch[3]` again) and very nearly got written off
  as an unreachable register-allocator spill. It is real, load-bearing
  caller-side argument setup for a still-`INCLUDE_ASM` callee. Before
  calling a "spilled but seemingly-dead value" a compiler artifact, check
  whether it lines up with a buffer/struct passed to another call in the
  same function -- it may be initializing a field the callee actually reads.
