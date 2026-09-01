# New_DreamSys — MATCHED (round 2026-08-30-c)

**Unit:** DreamSys · **Size:** 31 instructions · **Status:** MATCHED (31/31 words)

> **RESOLVED, round 2026-08-30-c.** Head-adjudicated re-check after
> `func_80059E3C` turned out to be a wrong-parameter-list bug, not a true
> register-identity stall (see that report). New_DreamSys does NOT have the
> same root cause -- the residue register was `$v0` (the return-value
> register), not an argument register `$a0`-`$a3`, and it was never live
> into a subsequent call in either branch, so the "loaded-into-an-argument-
> register-and-not-overwritten" test does not apply here. It WAS still
> reachable, though, by a sixth reshape distinct from every prior attempt
> (see below) -- two explicit `return` statements at the tail, rather than
> one trailing `return this;` after an early exit. Kept as attempt 6 of the
> history below rather than replacing it, per the instruction to preserve
> the attempt record.

## What it does

The `New_X` allocator wrapper for `DreamSys`: `malloc(sizeof(DreamSys))`
(literal `0x928` in the asm -- see below), and if non-NULL, calls
`Get_vtable_DreamSys()->Constructor(this, arg0, arg1, arg2)` (still
`INCLUDE_ASM`: `DreamSys__DreamSys`) before returning the allocation
regardless of the constructor's own return value. This is allocator
sub-shape (1) from `DECOMPILATION_LEARNINGS.md`'s "at least three `New_X`
sub-shapes" list -- ignores the constructor's return, one early exit -- but
unlike `func_80025B34` (the shape's worked example, which needed `goto`
because its early exit returns a DIFFERENT value), here BOTH paths return
the same logical value (the allocation, or NULL). That similarity turned out
to be exactly the trap; see the residue below.

## Key finding used by this round even though the function itself stalled

`New_DreamSys` allocates via a literal `ori $a0, $zero, 0x928`. That is
`sizeof(DreamSys)` from the allocator's own mouth, and it does NOT match
what the header modeled at the time (`0x890`, ending at `storedDay`). This
was the trigger for extending the struct by the missing `0x98` bytes (see
`DreamSys__func_588ec.md`, which needed four of those newly-uncovered
fields). The finding stands regardless of this function's own match status
-- it comes from retail's own INCLUDE_ASM bytes, not from anything this
round wrote.

## The matching C

```c
DreamSys *New_DreamSys(void *arg0, s32 arg1, s32 arg2)
{
	DreamSys *this;

	this = func_80017B34(sizeof(DreamSys));
	if (this != NULL) {
		Get_vtable_DreamSys()->Constructor(this, arg0, arg1, arg2);
		return this;
	}
	return NULL;
}
```

The only difference from reshape 1 below: `return this;` moved INSIDE the
`if` block (immediately after the constructor call), with a second, explicit
`return NULL;` as the function's final statement, instead of one trailing
`return this;` reached by fallthrough from both branches. Same single
variable, same overall shape as reshape 3 (`goto`) -- but this one moved the
residue and the `goto` version didn't. Not fully explained; recorded as
"reshape 6 works, reshape 3 doesn't" without a deeper theory, since the two
looked equivalent going in.

## History: the residue that took 6 reshapes to close (kept for the record)

retail materializes a literal 0 for the null-allocation return path where
five of the six attempted bodies reused the already-zero register instead

Single-word diff, offset `0x800587A8`:

```
retail: 21100000   addu $v0, $zero, $zero
built:  21100002   addu $v0, $s0, $zero
```

Both are semantically identical -- on the branch-taken (allocation-failed)
path, `$s0` (== `this`) is ALREADY zero, since that's exactly what the
`beqz $s0` tested. Retail's delay slot for that branch materializes the
return value as the literal `$zero` register instead of reusing `$s0`; this
body's natural codegen reuses `$s0`.

Reshapes tried:

1. Plain `if (this != NULL) { ctor(); } return this;` -- 30/31, this exact
   residue.
2. `if (this == NULL) return NULL; ctor(); return this;` (early return with a
   literal `NULL`) -- changed the function's OWN SIZE (GCC 2.6.3 did not
   share the epilogue between the two return points), which shifted every
   later function in the unit by dozens of bytes. Reverted immediately;
   flagged in `DECOMPILATION_LEARNINGS.md`'s "three ways a score lies" sense
   -- a good-looking single-function diff after this change would have been
   reading a mis-sized build.
3. `goto`-based early exit (`if (this == NULL) goto done; ctor(); done: return
   this;`) -- identical 30/31, same residue, no size change. Doesn't move it
   either way.
4. A SECOND local variable (`alloc` for the malloc result, `this` initialized
   to `NULL` and conditionally overwritten -- the "default value in the delay
   slot" idiom from `DECOMPILATION_LEARNINGS.md`) -- this looked like the
   right shape on paper (matches retail's "precompute the false case,
   overwrite on the true path" pattern exactly), but it forced GCC to
   allocate a FIFTH callee-saved register (retail only spends four: this,
   arg0, arg1, arg2), corrupting the whole prologue/epilogue and blowing the
   diff out completely (1/31, ~151KB image-wide drift). Reverted immediately.
5. A bare `__asm__("")` scheduling barrier as the function's first statement
   -- changed prologue INSTRUCTION ORDER (new mismatches appeared elsewhere,
   27/31) without touching this residue at all. Confirms it isn't an
   ordering issue.
6. **(round 2026-08-30-c, the one that worked)** Single variable again, like
   1-3, but `return this;` moved INSIDE the `if` block right after the
   constructor call, with an explicit trailing `return NULL;` as the
   function's own last statement -- see "The matching C" above. 31/31.

This was NOT the `new_class_6d3c8` / "redundant move" residue class after
all, despite looking identical to it through attempts 1-5 -- it was an
ordinary reshape that just hadn't been tried yet. Retracting the earlier
"second permuter candidate" suggestion below; no permuter target here.

### Proposed learning

Two explicit `return` statements (one inside the success branch, one as the
function's trailing statement) are NOT interchangeable with `goto`-to-a-
shared-return OR with one trailing `return this;` reached by fallthrough
from both branches, even when all three read as "the same control flow" and
even when the early-return-changes-size trap (attempt 2) doesn't apply
because both paths return the same logical value. All four single-variable
shapes (1, 2, 3, 6) look equivalent on paper; only 2 and 6 differ from 1 and
3 in an observable way (2 changes size, 6 changes register selection), and
that difference isn't explained by anything else this round found. Try the
early-return-inside-the-if variant (6) as a distinct, cheap reshape before
concluding a residue is a genuine stall -- it is easy to skip over because
it looks redundant with the `goto` version.

## Provenance

round 2026-08-30-b, runner ALPHA, address range `0x80058774`-`0x8005A1EC`
(reshapes 1-5, stalled). Resolved round 2026-08-30-c, same runner, address
range widened to the whole unit (reshape 6, matched).
