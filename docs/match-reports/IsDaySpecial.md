# IsDaySpecial

**Unit:** DreamSys · **Size:** 52 words · **Status:** STALL — best 18/52
(**not a real 18/52**: the residue is 2 extra `nop` instructions mid-function
shifting everything after them; only the first ~9 words and the last ~14
words of the function are genuinely settled, see below)

## What it does

Already forward-declared (`MoodGraphPoint *IsDaySpecial(CinematicCall
*cinematic, int day);`). Linear-searches `SPECIAL_DAYS[0..0x2A)` for `day`;
on a match, fills `cinematic->entry` with `rand() % 6` and `cinematic->bank`
with `(match index) % 12`, and returns the fixed special-day mood pointer
`&D_8008ABF4`. Returns `NULL` if no match is found after all 42 entries.

## The C reached (preserved here, `#if 0`-style per the reporting convention)

This is genuinely correct C — every field, every divisor, every branch
target verified against the disassembly by hand (see "Derivation" below) —
and it compiles, links, and is semantically right. The ONLY problem is two
extra `nop` instructions GCC's scheduler inserts that retail's build does
not, which shifts every following instruction by 2 words and makes the
per-function window unreliable past that point (confirmed real — not a
misread — by checking the raw `.o` disassembly directly, not just
`funcdiff`'s reported score).

```c
#if 0
MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day)
{
	s32 i;

	for (i = 0; (u32)i < 0x2A; i++) {
		if (day == SPECIAL_DAYS[i]) {
			cinematic->entry = rand() % 6;
			cinematic->bank = i % 12;
			return &D_8008ABF4;
		}
	}
	return NULL;
}
#endif
```

Needs (already added to `include/DreamSys.h`, kept regardless of this
stall since they're correct and cost nothing unused):

```c
extern MoodGraphPoint D_8008ABF4;
extern s32 rand(void);
```

## Derivation (all confirmed correct independent of the residue)

- `SPECIAL_DAYS[i]` compared against `day` via `bne $a1,$v0`, `$a1` being
  the untouched `day` parameter — matches directly.
- The `for` loop's bound check compiles to `sltiu $v0,$s0,0x2a` in retail —
  an UNSIGNED comparison despite `i` being a plain loop counter starting at
  0. Reproducing this needed an explicit `(u32)i < 0x2A` cast in the loop
  condition; a bare `i < 0x2A` (signed `slti`) does NOT reproduce it. This
  is now a settled, matched part of the function (confirmed at the tail:
  `sltiu`/`bnez`/array-pointer-increment/`move v0,zero`/epilogue all match
  byte-for-byte).
- Both divisors solved arithmetically from the magic-multiply constant
  (`0x2AAAAAAB ≈ 2^32/6`), per `DECOMPILATION_LEARNINGS.md`'s "solve a
  magic-multiply divisor arithmetically" guidance, not guessed:
  - `cinematic->entry`: `mfhi` read directly, no extra shift before the
    sign-correction `subu` → divisor `6`.
  - `cinematic->bank`: `mfhi` read, THEN an extra `sra ..,1` before the
    sign correction → divisor `6 * 2 = 12`. Both confirmed against the
    field's own later multiply-back-out (`*3` then `<<1` for the first,
    `*3` then `<<2` for the second — `6` and `12` respectively).
- `cinematic->entry` (offset `+0x2`, `sh`) and `cinematic->bank` (offset
  `+0x0`, `sh`) match `CinematicCall`'s existing `{s16 bank; s16 entry;}`
  layout exactly.
- The whole function's tail (loop increment, bound check, array-pointer
  increment, not-found return, epilogue) matches byte-for-byte once the
  `(u32)` cast was in place — 14 words there are genuinely settled, not
  coincidentally passing.

## The residue itself

Retail: `mult $v0,$s1` (rand-based modulo) / `mfhi $a0` / `mult $s0,$s1`
(index-based modulo) — the second `mult` issued IMMEDIATELY after the first
`mfhi`, zero gap, then ~10 unrelated instructions (finishing the first
modulo's correction and storing it) run before the second `mfhi` reads back
`$s0`'s product. This is legitimate MIPS I latency hiding: two independent
multiplies overlapped, with enough real work between the second `mult` and
its own `mfhi` to cover the hardware latency for free.

Every attempt here reproduces the SAME two `mult`/`mfhi` pairs in the SAME
relative order, but GCC's scheduler inserts two literal `nop` instructions
between the first `mfhi` and the second `mult` that retail's build does not
have — confirmed directly in the built `.o`'s disassembly (`0x00000000` ×2
at exactly that position), not an artifact of misreading `funcdiff`'s
positional diff.

## Attempts (13, all preserving semantics; every one rebuilt and re-scored)

1. Baseline (`for`, `i%12`, `rand()%6`, no cast on the loop bound): loop tail
   wrong (`slti` not `sltiu`) — 3/52, unrelated to the mult residue.
2. Added `(u32)i < 0x2A` cast: fixed the tail. **18/52, best result,
   residue isolated to the two `nop`s.** All following attempts compared
   against this baseline.
3. `u32 i;` (whole variable unsigned, not just the comparison): broke the
   `i % 12` divisor entirely (GCC switched to unsigned-modulo codegen,
   structurally different from retail's signed `mult`) — 4/52.
4. Named local for `rand()`'s return (`s32 r = rand(); ... r % 6;`): 18/52,
   no change.
5. Named locals for BOTH remainders, assigned to the struct fields as a
   separate final step: 16/52, worse.
6. Swapped statement order (`bank` before `entry`): 1/52 — breaks the
   `rand()` call's own position relative to the store, much worse.
7. `__asm__("")` immediately after `rand()`: 17/52, worse.
8. `__asm__("")` between the two remainder statements: 17/52, worse.
9. `do`/`while` instead of `for` (same logical bound): 8/52, worse.
10. `s16 *p` array-pointer walk instead of index (mirroring retail's `$v1`
    register more literally) with a separate `s32 i` counter: 12/52, worse.
11. `s16 entry;`/`s16 bank;` locals matching the field width exactly,
    assigned then copied to the struct: 18/52, no change (neutral).
12. Combination of #4 and #11: no change from baseline.
13. `__asm__("")` at the very top of the loop body (before `rand()`): 17/52,
    worse.

None closed the gap; several made it worse by disturbing the (already
correct) surrounding instructions instead. Per CLAUDE.md rule 6, a bare
`__asm__("")` is the only barrier form tried (attempts 7, 8, 13) — no
`register T v asm("$N")` or operand-constraint fix was used or considered,
since removing the barrier only ever changed instruction order in these
attempts, never which register held a value.

## Why this looks like the project's known open residue class

`docs/DECOMPILATION_LEARNINGS.md` already has an open, unresolved entry for
this exact shape: *"Magic-multiply constant load POSITION... unmoved by six
reshapes... Flagged as a candidate 'not a source-shape question' case"*
(`func_80066340`, a different unit, different round). This function's
residue is the same family — a compiler SCHEDULING choice around a
magic-multiply sequence, not a source-shape or field-typing error — and 13
reshapes across variable decomposition, loop form, barrier placement, and
type width did not move it, mirroring that entry's own experience. Filed
here as a second data point for that class rather than reopened as a fresh
mystery.

## Proposed learning

**A residue that is EXACTLY two extra `nop`s inserted between an `mfhi` and
a subsequent independent `mult`, with the surrounding computation otherwise
byte-identical, is worth checking against
`docs/DECOMPILATION_LEARNINGS.md`'s existing magic-multiply-scheduling class
before spending a full 30-attempt budget on reshapes** — it did not close
here across 13 varied attempts (declaration shape, loop form, barrier
placement, explicit width), consistent with that class's existing
description. A permuter run (once set up per project's plan) is a better
next step than further manual reshaping.

## Status

Restored to `INCLUDE_ASM`. No C left in `src/DreamSys.c` for this function.
`include/DreamSys.h` keeps the two new externs (`D_8008ABF4`, `rand`) since
both are independently correct and will save the next attempt's setup work.
