# DreamSys__TickMoveFree — MATCHED

> Renamed from `func_80059AEC` on 2026-09-22 (tools/rename.py). Address 0x80059aec.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 25/25 words, full match.

## Source

```c
s32 DreamSys__TickMoveFree(DreamSys *this)
{
	if (this->unk_0x70 != 0)
		return this->unk_0x70;
	return this->vt->DreamSys__ApplyMoveCommand(this, this->vt->DreamSys__AdvanceMoveCycle(this, 1));
}
```

Vtable slot `0x158`. If `unk_0x70` is set, returns it unchanged (a genuine
early exit returning a DIFFERENT value than the main path — this is exactly
the shape HEAD BROADCAST #1's `goto`-lever example describes). Otherwise
calls `DreamSys__AdvanceMoveCycle(this, 1)` (still `INCLUDE_ASM`, in this runner's range
but not yet reached — vtable slot `0x164`) and feeds its result into
`DreamSys__ApplyMoveCommand(this, ...)` (also still `INCLUDE_ASM`, outside this runner's
assigned range), returning THAT result.

## Residue fixed — and a correction to broadcast #1's lever

Five attempts before the match, all producing the identical 24/25 residue (a
redundant `move $a0,$s0` in the delay slot of the entry guard branch where
retail has a plain `nop` — `this` is still valid in `$a0` at that point,
never having been clobbered, so retail's compiler didn't bother
re-materializing it from the callee-saved copy):

1. `goto`-based early exit exactly matching broadcast #1's `goto fail; ...
   fail: return X;` template (main path first, `goto` past it to the
   value-returning early-exit block at the end).
2. Same `goto` shape plus a bare `__asm__("");` scheduling barrier right
   after the guard (per broadcast #2's permitted-barrier test) — moved
   the `nop`/`move` pair around but did not remove the extra instruction,
   confirming it's not a pure scheduling artifact (a barrier can reorder,
   not delete).
3. Plain `if (cond) { ...; return ...; } return X;` (early exit as the
   nested main-path branch, X falling to the end) with an intermediate
   `result` local.
4. Same as (3) but with the two virtual calls nested into one `return`
   expression instead of using `result`.
5. `if (cond) return X_expr; return X;` — i.e. moved the MAIN path (calls)
   into the `if`, kept the trivial `unk_0x70` return as the function's
   FINAL statement.

None of 1-5 helped. **Fix (6th form): the exact opposite of broadcast #1's
template — a plain, ORDINARY early return for the TRIVIAL case, written
FIRST, with NO `goto` and NO enclosing `if`/`else` for the main path**:

```c
if (this->unk_0x70 != 0)
	return this->unk_0x70;
return this->vt->DreamSys__ApplyMoveCommand(this, this->vt->DreamSys__AdvanceMoveCycle(this, 1));
```

This is precisely the plain "does not need a lever" shape CLAUDE.md's own
guidance would suggest by default — and it turned out to be correct, while
every attempt to apply the `goto` lever (evidently overfit from a different
function's residue) made no difference at all to this specific redundant-move
residue. **Broadcast #3 already warned the `goto` lever is sub-shape-specific
— this is a second, independent confirmation**: the lever's own justification
(GCC 2.6.3 places an early-exit-with-different-value block AFTER the main
path when spelled with `return`, incurring an extra `j`+`nop`) is about
INSTRUCTION COUNT via block placement, not about THIS residue (a redundant
register re-materialization for an argument that's already correctly
placed) — two different residue classes that can both show up around an
early-return-different-value shape, and only one of them responds to the
`goto` rewrite.

## Proposed learning

**The `goto`-vs-`return` lever for early-exit-with-different-value functions
(broadcast #1) fixes a specific residue (extra `j`+`nop` from block
placement) — it does NOT generalize to every residue that happens to occur
near such an early exit.** Here, a redundant `move $aN,$sM` argument
re-materialization (the callee's first argument register already held the
right value, unclobbered, and retail's compiler didn't re-set it) persisted
identically across FIVE structurally different rewrites, including the
`goto` form and a scheduling barrier, and was only resolved by reverting to
the plainest possible early-return shape. When a residue is a single
redundant/missing instruction rather than a block-order or `j`+`nop` pair,
don't reach for the `goto` lever by pattern-matching on "this is an early
exit with a different value" — check whether the plain, unadorned form
(no lever at all) already matches first.
