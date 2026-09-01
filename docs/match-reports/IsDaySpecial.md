# IsDaySpecial -- STALL (best 18/52 words, restored to INCLUDE_ASM)

Unit: `DreamSys`. Round 2026-09-01-e (runner echo). ~14 attempts.

## Best body reached

```c
/* Not #included by this unit (only include/psyq/RAND.H and Entity.h
   declare it); forward-declared locally, matching Entity.h's own
   `s32 rand(void)` shape. */
extern s32 rand(void);
extern MoodGraphPoint D_8008ABF4;

MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day)
{
	s32 i;

	for (i = 0; i < 42; i++) {
		if (day == SPECIAL_DAYS[i]) {
			cinematic->entry = rand() % 6;
			cinematic->bank = i % 12;
			return &D_8008ABF4;
		}
	}
	return NULL;
}
```

This reaches 18/52 with `build exit=0` (whole-file SHA1 fails, as expected
short of byte-exact) and is NOT left in `src/DreamSys.c` -- restored to
`INCLUDE_ASM("asm/nonmatchings/DreamSys", IsDaySpecial);`.

## Derivation that IS solid (do not re-derive)

- Loop scans `SPECIAL_DAYS[0..41]` (42 = `0x2A`, hardcoded literal, matches
  retail's `sltiu $v0, $s0, 0x2A`) for a `s16` equal to `day`.
- On a match: `cinematic->entry = rand() % 6;` then `cinematic->bank = i %
  12;`, then returns `&D_8008ABF4` (a `MoodGraphPoint`, matching this
  function's already-declared return type in `include/DreamSys.h`).
- **Both `% 6` and `% 12` divisor identifications are confirmed
  ARITHMETICALLY**, not guessed, per CLAUDE.md's "solve a magic-multiply
  divisor arithmetically" rule: magic constant is `0x2AAAAAAB` =
  715827883. `715827883 * 6 = 4294967298 = 2^32 + 2` (small positive
  remainder, shift 0) confirms the first division is by 6 -- matching the
  reconstruction `sll 1 / addu / sll 1` (`q*2+q, then *2` = `q*6`). The
  second division has an EXTRA `sra $a0,$a0,0x1` folded in before the sign
  fix, and its reconstruction is `sll 1 / addu / sll 2` (`q*3, then *4` =
  `q*12`), confirming divisor 12. On the first pass this was momentarily
  misread as `/3` and `/6` respectively (naively pattern-matching against
  the CLAUDE.md example) -- the extra `sra ...,1` on the SECOND division
  only is the tell that distinguishes them; do not re-derive, this part
  reproduced byte-for-byte on the first correctly-typed attempt.
- All of this reproduces PERFECTLY, instruction-for-instruction, once the
  divisors are right -- confirmed by the diff below, which shows every
  instruction from `mult v0,s1` (rand's `%6`) through the final `sh
  v1,0(s2)` (bank's `%12` store) lining up 1:1 with retail, just offset by a
  constant +8 bytes (two words). This is not a structural/CFG problem.

## The residue (unclosed)

Two independent-looking symptoms, always co-occurring across every reshape
tried:

1. **Two extra `nop`s inserted between the first `mfhi` and the second
   `mult`.** Retail:
   ```
   mult v0,s1      # rand_val * magic
   mfhi  a0        # entry's raw quotient
   mult  s0,s1     # i * magic -- STARTS IMMEDIATELY, filling the mfhi hazard
   lui   v0,%hi(D_8008ABF4)
   ...
   ```
   Every reshape tried here produces instead:
   ```
   mult v0,s1
   mfhi  a0
   nop
   nop
   mult  s0,s1
   lui   v0,%hi(D_8008ABF4)
   ...
   ```
   Retail's scheduler pulls the SECOND division's independent `mult`
   forward to fill the R3000's mult-to-mfhi latency gap of the FIRST
   division. This is GCC 2.6.3's list scheduler interleaving two
   independent instruction chains from what are, in source, two separate
   statements (`cinematic->entry = ...;` then `cinematic->bank = ...;`).
   The +2-word growth from the un-filled hazard is what drives the
   "differs outside this range" cascade seen on every attempt.
2. **`sltiu` vs `slti` on the loop-closing bound check** (`i < 42`, `0x2A`).
   Retail uses the UNSIGNED form even though `i` is a plain incrementing
   counter starting at 0 with no unsigned semantics anywhere else in the
   function. Every C-level reshape tried here (signed `s32`, `s16`, `u32`,
   `i != 42`, a `do/while` with a pre-incremented condition) either left
   this unchanged or made things substantially worse -- `u32 i` in
   particular flips the modulo codegen to UNSIGNED division entirely
   (wrong instructions, not just wrong signedness of one compare), because
   `%` on an unsigned type changes which magic-multiply variant GCC emits
   for `i % 12`. This one is a genuine GCC-internal choice this session
   found no C-level lever for.

## Attempts tried (all rejected, best score noted)

- Baseline two-statement form (`entry` then `bank`, array indexing): **18/52**, both residues present. (kept as the reported best.)
- Reversed comparison operand (`day == SPECIAL_DAYS[i]` vs
  `SPECIAL_DAYS[i] == day`): fixed a THIRD, now-resolved residue (`bne`
  operand order) but did not touch either remaining one. Kept in the final
  body above.
- Hoisting `rand()`'s result into a named local before the modulo: 18/52,
  no change.
- Swapping statement order (`bank` computed/stored first, `entry` second):
  **5/52**, much worse -- store order in the source clearly does drive
  instruction order here, just not in a way that reaches retail's
  interleaving.
- `u32 i` (chasing the `sltiu`): **4/52**, worse -- changes `i % 12` to an
  unsigned-division codegen shape entirely.
- `s16 i`: **0/52**, worse -- forces sign/zero-extension traffic around
  every use of `i`.
- Pointer-walk form (`s16 *p = SPECIAL_DAYS; ... *p ...; p++`) alongside
  the index `i`, matching the "inner loops want incrementing pointers"
  idiom from `func_80066340`: **12/52**, worse than the plain-indexing
  baseline for this particular function -- retail's `v1` pointer walk is
  used ONLY as the array pointer (never read back), while its `s0` index is
  used only for the bound check and the `%12`; splitting into two C
  variables did not reproduce that division of labor.
- `__asm__("")` barrier between the two statements: **17/52**, worse (grew
  the function further and perturbed unrelated register allocation --
  consistent with CLAUDE.md's warning that the barrier's blast radius
  scales with function size and is not a free lever).
- `for (i = 0; i != 42; ...)`: **16/52**, worse -- changes the comparison
  instruction class entirely (no longer a `slt`-family compare).
- Comma-expression single statement
  (`cinematic->entry = rand() % 6, cinematic->bank = i % 12;`): **18/52**,
  identical to the baseline -- GCC 2.6.3 treats it the same at `-O2`.
- `do { ... } while (++i < 42);` with pre-incremented condition: **8/52**,
  substantially worse -- the loop-closing branch and the `%2A` bound check
  both change shape, confirming the `for`-with-hoisted-first-iteration
  shape (already matching retail's actual entry/exit topology) is right
  and the `do/while` is not.

## Proposed learning

- **A near-miss where every content instruction lines up 1:1 but the
  function is padded by 1-2 extra `nop`s from an unfilled `mult`->`mfhi`
  hazard, where the filler retail uses comes from a LATER, independent
  statement's own division, is a genuine open residue class** -- add to
  DECOMPILATION_LEARNINGS.md's "New residue classes opened this round" list
  alongside `DreamSys__LogMood`'s pure-scheduling stall. Statement order,
  temporaries, and barriers were all tried and none reproduced the
  interleave; this looks like a `-O2` scheduler decision this project's
  toolchain does not currently have a source-level lever for. Flagging as
  a second data point for the "instruction scheduling, no branch/call to
  reason about" bucket -- except this one DOES have a call (`rand()`)
  and a branch (the `if`), so the bucket may be broader than "branch-free,
  call-free."
- **Confirms (does not newly discover) the arithmetic-divisor-solving
  rule**: two divisors 6 and 12, both derived from magic constant
  `0x2AAAAAAB`, distinguished only by an extra `sra ...,1` on the second
  one. Worth keeping as a second worked example next to `func_8002658C`'s
  "assumed /9, actually /15" in case a future function needs the exact
  reconstruction-instruction-count table (shift 0 -> `sll1/addu/sll1`
  rebuilds `N=6`; shift 1 -> `sll1/addu/sll2` rebuilds `N=12`).
