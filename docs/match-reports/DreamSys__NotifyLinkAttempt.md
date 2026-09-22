# DreamSys__NotifyLinkAttempt — MATCHED

> Renamed from `func_80058B08` on 2026-09-22 (tools/rename.py). Address 0x80058b08.

**Unit:** DreamSys · **Size:** 84 words (97 instructions incl. delay
slots/nops, `0x49308`-`0x49458`) · **Status:** MATCHED (84/84 words), whole-
image SHA1 verified green by `build-and-verify.sh`. Round 2026-09-02,
runner BRAVO.

## What it does

Vtable slot `+0x088`. Dispatches on `arg1`:

1. Unconditionally calls the shared base table's own `+0x088` slot as
   `(this, arg1)`, discarding its return value.
2. If `arg1 == -2`: chases `this->unk_0x4C->methods->slot0x11C(this->unk_0x4C,
   (u8 *)this->unk_0x14 + 0x18)`, and if the result's `->unk_0x4->unk_0x2C`
   is NOT `2`, calls `this->vt->func_8005B990(this)` (vtable slot `+0x224`,
   already matched elsewhere in this unit — the `unk_0x14` snapshot
   restorer) and returns; otherwise falls into the shared tail below.
3. Else if `arg1 == -1`: recomputes `this->unk_0xB8` from
   `this->unk_0x28->unk_0x36 & 0x7F` (clamped to 0 if `>= 0x18`, forced to 2
   if `this->unknwon_int_0x44 == 15` and it's still 0), then returns unless
   `this->currentStage == 9`, in which case it falls into the shared tail.
4. Else (any other `arg1`): returns immediately.
5. Shared tail: calls `this->vt->func_8005A7A0(this,
   this->unk_0x4C->methods->slot0x10C(this->unk_0x4C, 0, 0))` — same
   `slot0x10C(obj, 0, 0)` call shape already established by
   `DreamSys__FlashbackSaving`, here feeding `func_8005A7A0`'s `currentPos`
   argument instead.

Both blocker screens are clean: no `gp_rel` hit and no
`addiu $at, $at, %lo` hit anywhere in `DreamSys__NotifyLinkAttempt.s`.

## New struct/vtable knowledge

- `DreamSys::unk_0x28` (was `unknown_values_0x28[28]`) is a POINTER, not
  inline data — confirmed by this function reading it as a `lw` (whole
  word) and then dereferencing the result at `+0x36` for a `u16`. New type
  `DreamSysUnk28Target` holds only that one confirmed field; the rest of
  the 28-byte region beyond the pointer's own 4 bytes is still opaque
  padding (`unknown_values_0x2C[24]`).
- `DreamSys::unk_0xB8` (was folded into `unknown_values_0xB8[4]`, itself
  carved this round for `func_80059BE0`'s `unk_0xB4`) is a plain `s32`: the
  masked `unk_0x28->unk_0x36` value, described above.
- `DreamSys::currentStage` at `+0x164` was ALREADY a named field (derived
  from `MoodGraphContributor areaMoods/entityMoods` sizing) — this function
  is simply the first one in this unit's queue to actually read it; no
  header change needed there, just confirms the offset arithmetic.
- `vtable_DreamSys::DreamSys__NotifyLinkAttempt` (was inside `unknown_functions_0x64[13]`)
  — this function's own slot, `+0x088`, split out of that array.
- `DreamSysBaseMethods::slot0x88` (the SHARED base table returned by
  `DreamSys__GetBaseMethods()`, a DIFFERENT table from DreamSys's own vtable despite
  the coincidentally-identical offset) — called unconditionally as
  `(this, arg1)`, return discarded.
- `DreamSysUnk4CMethods::slot0x11C` — new slot, `DreamSysUnk11CResult
  *(*)(void *self, void *arg1)`. Two new minimal local-view types support
  the two-level chase its result undergoes:
  `DreamSysUnk11CResult { unknown[4]; DreamSysUnk11CInner *unk_0x4; }` and
  `DreamSysUnk11CInner { unknown[0x2C]; s16 unk_0x2C; }`. Only the one path
  this function actually walks is confirmed; everything else in both
  structs is unconfirmed padding.
- `DreamSysUnk4CMethods::slot0x10C` was already typed (by
  `DreamSys__FlashbackSaving`, an earlier round) with the exact matching
  signature; this function is a second, independent caller with the
  identical `(obj, 0, 0)` argument shape, which is corroborating evidence
  for that existing type, not a new one.

## Source

```c
void DreamSys__NotifyLinkAttempt(DreamSys *this, s32 arg1)
{
	s32 v;

	DreamSys__GetBaseMethods()->slot0x88(this, arg1);
	if (arg1 == -2)
		goto handle_neg2;
	if (arg1 != -1)
		return;

	v = this->unk_0x28->unk_0x36 & 0x7F;
	this->unk_0xB8 = v;
	if (v >= 0x18)
		this->unk_0xB8 = 0;

	if (this->unknwon_int_0x44 == 15 && this->unk_0xB8 == 0)
		this->unk_0xB8 = 2;

	if (this->currentStage != 9)
		return;
	goto shared_tail;

handle_neg2:
	if (this->unk_0x4C->methods->slot0x11C(this->unk_0x4C, (u8 *)this->unk_0x14 + 0x18)->unk_0x4->unk_0x2C != 2)
		goto neg2_mismatch;

shared_tail:
	this->vt->func_8005A7A0(this, this->unk_0x4C->methods->slot0x10C(this->unk_0x4C, 0, 0));
	return;

neg2_mismatch:
	this->vt->func_8005B990(this);
}
```

## Derivation

First pass (plain `if (arg1 == -2) {A} else if (arg1 == -1) {B} else
return;`, each block's body written directly rather than merged into a
shared tail) compiled cleanly and drift-free at 62/84 — everything BEFORE
the three-way dispatch and the `-1`/`unk_0x28`/`unk_0x44`/`currentStage`
body matched immediately. The remaining 22-word gap was entirely
CONTROL-FLOW LAYOUT, not value logic: retail places the `arg1 == -1` body
as the immediate fallthrough of the two comparisons (with the `-2` body
positioned physically LATER, reached by a forward branch), and — more
subtly — places the `-2` path's MISMATCH handling (the `func_8005B990`
call) physically AFTER the shared tail block, reached by its own forward
branch from a `bne`, while the shared tail itself is what naturally falls
out of both the `-1` success path and the `-2` match path. A plain nested
`if`/`else if`/`else` cannot express "two independent forward jumps into
a shared block, with one arm's failure case relocated to the very end of
the function" — converting to an explicit `goto`-labelled version that
mirrors retail's block order (`-1` body, `-2` body, shared tail, `-2`
mismatch) closed the remaining 22 words in one step, with zero further
residue.

## Proposed learning

**A three-or-more-way dispatch into a SHARED continuation, where one arm's
failure case is relocated to the very end of the function (past the shared
block), is a real retail shape here — not a compiler artifact — and needs
explicit `goto`/labels laid out in the SAME physical order as the target
disassembly's blocks.** Nested `if`/`else if` reliably gets the VALUE logic
right but silently picks its own (different) block order/branch polarity
for anything beyond a simple two-way split; when a residue is pure
control-flow-layout (branch targets/polarity differ, but every value and
instruction WITHIN each block already matches), stop tuning the nesting and
read the disassembly's block order directly into `goto` labels instead.
This is a superset of the already-documented "goto vs return" lever
(DECOMPILATION_LEARNINGS.md) — it generalizes past a single early-exit to
an arbitrary number of blocks that converge on one shared tail.
