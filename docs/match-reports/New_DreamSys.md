# New_DreamSys — STALL

**Unit:** DreamSys · **Size:** 31 instructions · **Best reached:** 30/31 words

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

## Best-reached body (does NOT compile to retail bytes)

```c
#if 0
DreamSys *New_DreamSys(void *arg0, s32 arg1, s32 arg2)
{
	DreamSys *this;

	this = func_80017B34(sizeof(DreamSys));
	if (this != NULL)
		Get_vtable_DreamSys()->Constructor(this, arg0, arg1, arg2);
	return this;
}
#endif
```

## Residue (NOT blocking, but not reachable by reshaping in ~6 tries):
retail materializes a literal 0 for the null-allocation return path where
this body reuses the already-zero register

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

Reshapes tried, all either reproducing the exact same single-word diff or
making it worse:

1. Plain `if (this != NULL) { ctor(); } return this;` (shown above) -- 30/31,
   this exact residue.
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

This looks like the same class `DECOMPILATION_LEARNINGS.md` already
describes for `new_class_6d3c8` and the "redundant `move`" entries: a value
GCC materializes that plain source reshaping doesn't reach, not a stall
caused by wrong control flow or wrong types. Flagging as a second permuter
candidate alongside `new_class_6d3c8` given the `New_X` shape overlap --
worth checking whether the same source form that closes one closes both.

### Proposed learning

A second `New_X` sub-shape stalls on the SAME kind of residue as
`new_class_6d3c8` (a materialized value reshaping can't reach), not a new
one: when both branches of a null-checked allocator return the same logical
value, don't assume the "default value in a delay slot" idiom applies just
because retail's disassembly LOOKS like that idiom (a literal in one delay
slot) -- introducing the second variable that idiom implies can cost an
EXTRA callee-saved register if the allocator result and the return value
were already unified in the working register, making the diff much worse,
not better. Check the register budget (how many `s`-regs retail's own
prologue saves) before restructuring into two variables.

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`. Restored to `INCLUDE_ASM`.
