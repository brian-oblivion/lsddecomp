# DreamSys__AddFlashback -- MATCHED (52/52 words)

Unit: `DreamSys`. Round 2026-09-01-e (runner echo). ~7 attempts.

## Final C

```c
struct DreamSysAngles3 {
	struct Angle pitch;
	struct Angle heading;
	struct Angle roll;
};

void DreamSys__AddFlashback(DreamSys *this, s32 stage, PlayerSpawnPoint *pos, s32 *angles, s32 unknown, s32 time, s32 day)
{
	s32 count;
	s32 wordIndex;
	FlashbackEntry *entry;

	entry = this->storedFlasbacks;
	if (this->amountFlashbacksAvailable < 10) {
		count = this->amountFlashbacksAvailable;
		this->amountFlashbacksAvailable = count + 1;
		wordIndex = count * 9;
	} else {
		wordIndex = ((u32)this->dreamTimer % 9) * 9;
	}
	entry = (FlashbackEntry *)((s32 *)entry + wordIndex);
	entry->stageID = stage;
	entry->position = *pos;
	*(struct DreamSysAngles3 *)&entry->pitch = *(struct DreamSysAngles3 *)angles;
	entry->unknown_value_0x1c = unknown;
	entry->timeLimit = time;
	entry->day = day;
}
```

Retail is a full LEAF function -- no stack frame, no saved registers, `a0`-`a3`
and `t0`-`t4` only -- which made this the strictest-shaped function tried
this round: any extra local that outlived its natural register lifetime
showed up immediately as a spilled-register prologue.

## Derivation

`amountFlashbacksAvailable` gates a fixed-size (10-slot) ring: while there is
room, append at the current count and increment it; once full, overwrite a
slot chosen by `dreamTimer % 9` (an 9-wide sub-cycle inside the 10-slot
array -- unconfirmed WHY 9 rather than 10, but the magic constant leaves no
ambiguity about WHAT it computes). `FlashbackEntry` is `0x24` (36) bytes =
9 words, and retail's addressing is genuinely `wordIndex * 4`, not
`entryIndex * 36` collapsed into one multiply -- see the residue class below.

- `this->storedFlasbacks[0]`'s address is hoisted into its own register
  (`t0 = this + 0x470`) UNCONDITIONALLY, before the `if`, and used
  identically by both branches. `this` (`a0`) is then free to be clobbered
  in the `else` branch to hold `dreamTimer`.
- Both divisor identifications are solved arithmetically, not guessed: `9`
  from the `0x38E38E39` magic constant with the standard "one extra `sra`"
  reduction described for `IsDaySpecial`'s `%12`.
- `FlashbackEntry::unknown_value_0x1c` was `s32`; retyped to two `s16`
  halves (`unknown_value_0x1c` / `unknown_value_0x1e`) in
  `include/DreamSys.h` -- retail stores this function's 5th argument there
  with `sh`, not `sw`, so only the first half is confirmed written by this
  function.

## The residue chain (three distinct issues, each independently confirmed)

1. **Sharing the `wordIndex * 9` multiply across both branches, instead of
   duplicating it inside each, drops the multiply entirely from the `true`
   branch and costs the whole shared merge its shape.** First attempt wrote
   `entry = &this->storedFlasbacks[index];` (clean array indexing) placed
   AFTER the `if`/`else`, so GCC computed ONE `index * 36` for both paths --
   collapsing retail's `(candidate * 9)` computed separately per branch,
   THEN a shared `* 4`. Retail's shape requires literally writing `wordIndex
   = candidate * 9;` inside EACH branch and doing the final `* 4` via
   pointer arithmetic (`(s32 *)entry + wordIndex`) after the merge -- the
   two-step decomposition (`* 9` then `* 4`, not one `* 36`) is not
   idiom-visible from `storedFlasbacks[index]` and has to be written
   explicitly.
2. **The `else` branch's modulo needs an explicit `(u32)` cast.**
   `this->dreamTimer % 9` (plain `s32 %`) compiles to `mult` (signed) with a
   sign-correction `sra`; retail uses `multu` (unsigned) with none. Nothing
   else in this function treats `dreamTimer` as unsigned, so this cast is
   local to this one expression, not a struct-wide retype.
3. **The 12-byte `pitch`/`heading`/`roll` triple wants ONE struct
   assignment, not three separate per-field ones.** Three separate
   `entry->pitch = ang[0]; entry->heading = ang[1]; entry->roll = ang[2];`
   statements compile to an interleaved LOAD-STORE-LOAD-STORE-LOAD-STORE
   sequence (reusing one register). Retail's actual sequence is
   LOAD-LOAD-LOAD-STORE-STORE-STORE across three DIFFERENT registers (`v0`,
   `v1`, `a0` held live simultaneously) -- the signature of a single 12-byte
   block-move codegen, matching CLAUDE.md's "whole-struct assignment, not
   an indexed loop, for a block copy" idiom exactly, just for THREE
   contiguous struct fields treated as one unit rather than an array. Fixed
   by declaring a local 3-`Angle` container type and doing ONE cast-and-copy
   assignment (`*(struct DreamSysAngles3 *)&entry->pitch = *(struct
   DreamSysAngles3 *)angles;`) instead of three field assignments.

Each of the three was isolated and fixed independently, in that order,
confirmed by re-running `funcdiff` after each change (18/52 baseline segfault
-> 24/52 after hoisting the base pointer -> 38/52 after fixing #1+#2
together -> 52/52 after #3).

## Proposed learning

- **When retail's per-field struct-member writes come as three (or more)
  LOADS followed by three (or more) STORES using DIFFERENT scratch
  registers, that is one block-copy assignment of a MULTI-FIELD span, not
  N separate field assignments** -- even when the fields involved
  (`pitch`/`heading`/`roll`) are individually named and semantically
  distinct. The existing "whole-struct assignment for a block copy" idiom
  in DECOMPILATION_LEARNINGS.md is stated for a single array/struct; this
  extends it to "or several adjacent struct fields you'd otherwise assign
  one at a time." Declaring a small local struct wrapping just those fields
  and doing one cast-assignment is the mechanical fix.
- **A shared post-branch index computation is not always what retail did,
  even when the same multiply appears in both branches of an if/else.**
  When retail computes the SAME multiply (`candidate * N`) separately
  inside EACH branch rather than hoisting the candidate value out and
  multiplying once after the merge, only the FOLLOWING step (here, `* 4` to
  convert word offset to byte offset) is actually shared. Telling the two
  apart from the .s: if the multiply-by-N reconstruction sequence
  (`sll`/`addu`) appears twice, once per branch, before the branches merge,
  it is NOT hoistable in the source either -- write it once per branch.
