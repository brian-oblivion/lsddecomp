# IsDaySpecial -- MATCHED 52/52 words, round 42 (2026-09-15)

> **VERDICT CORRECTED, round 42 (2026-09-15). THIS FUNCTION IS MATCHED.**
> It was blocked by `nop_mflo_mfhi`, which is RESOLVED this round: maspsx gained
> `--gp-symbols` / `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`),
> the whole image is byte-exact with the flags on, and this function was one of
> the live tests -- the body preserved below, unchanged except for dropping a redundant `extern` that DreamSys.h already declares. The C is in `src/world/DreamSys.c`. Everything below is the
> pre-fix record and is kept as evidence.

> **REOPENED -- WAS ASSIGNABLE, SINCE MATCHED (marker spent), round 42 (2026-09-15).** This function was
> screened as blocked by `nop_mflo_mfhi`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# IsDaySpecial

**Unit:** DreamSys · **Size:** 52 instructions · **Status:** STALL — but
**solved at the C level**; blocked only by `nop_mflo_mfhi`. Restored to
`INCLUDE_ASM`.

## The body. It is correct — all 52 instructions.

```c
/* Not #included by this unit (only include/psyq/rand.h and entity.h declare
   it); forward-declared locally, matching entity.h's own s32 rand(void). */
extern s32 rand(void);
extern MoodGraphPoint sSpecialDayMood;

MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day)
{
	s32 i;

	for (i = 0; (u32)i < 42; i++) {
		if (day == sSpecialDays[i]) {
			cinematic->entry = rand() % 6;
			cinematic->bank = i % 12;
			return &sSpecialDayMood;
		}
	}
	return NULL;
}
```

Compiled standalone through the pinned pipeline with **only
`--aspsx-version` varied**, this produces **52 instructions that match
retail's 52 one-for-one, register for register**, from the `addiu $sp` to the
trailing `nop` — the only textual differences being unresolved relocations
(`lui $v1, 0x0` for `%hi(sSpecialDays)`) and branch targets printed as local
offsets, both expected in a relocatable object.

At the project's pin it is 2 instructions longer, and that is the whole stall.

| `--aspsx-version` | `.text` size | shape |
| --- | --- | --- |
| **2.34** (the pin) | 0xD8 | `mult` / `mfhi` / **`nop`** / **`nop`** / `mult` |
| 2.29 | 0xD0 | `mult` / `mfhi` / `mult` — **retail** |

## Two residues were reported. One was real, one was a missing cast.

The first pass reached 18/52 and named two symptoms. Both are now resolved,
and neither is what it looked like.

### Residue 2 — `sltiu` vs `slti` — was a C-level problem with a C-level fix

Retail closes the loop with `sltiu $v0, $s0, 0x2A`, unsigned, on a plainly
signed counter. The first pass tried `u32 i` and correctly rejected it: making
the *variable* unsigned changes `i % 12` to an unsigned-division codegen shape
entirely, which is a much larger wrong than the one it fixes (4/52).

The fix is to cast only the **comparison** and leave the variable signed:

```c
	for (i = 0; (u32)i < 42; i++)
```

That emits `sltiu` while `i % 12` keeps its signed magic-multiply sequence
(`sra $v1, $s0, 31` and the `subu` sign fix are still there, exactly as
retail has them). Verified in isolation: `.text` size is unchanged, and the
only instruction that changes anywhere in the function is the one intended.

**This is a reusable lever, not a trick for this function.** A cast at the
comparison and a cast on the variable are very different edits, and only the
first is surgical.

### Residue 1 — the `mfhi` → `mult` gap — is the documented `nop_mflo_mfhi` blocker

Retail runs the second division's `mult` immediately after the first's
`mfhi`, filling the R3000's mult-to-mfhi latency with independent work. The
first pass read this as *retail's scheduler interleaving two chains that our C
never reproduced*, and searched for a source shape that would provoke the
interleave — ~14 reshapes, none successful.

**The C was already right, and cc1 already emits retail's order.** Reading
cc1's own output, before maspsx:

```
	mult	$2,$17
	mfhi	$4
	#nop
	#nop
	mult	$16,$17
```

Those are **commented-out** `#nop`s — Psy-Q-patched cc1 emits them as *hints*
marking where a hazard nop may be needed, and puts the `mult` immediately
after the `mfhi` exactly as retail has it. What turns the hints into real
instructions is the assembler: maspsx's `nop_mflo_mfhi`, which is `True` at
2.34 and `False` below 2.30. Its own debug output shows it doing the
substitution and displacing the `mult` behind the pair.

So this is not a scheduling residue and there is no source shape that closes
it. See `docs/research/addiu-at-blocker.md`, addendum 2026-09-02b.

## What is preserved for the day the flag question is settled

The body above is complete and needs no further work. If `nop_mflo_mfhi` is
ever resolved, paste it in and it should match with no reshaping.

## Derivation that is solid (do not re-derive)

- Loop scans `sSpecialDays[0..41]` (42 = `0x2A`) for an `s16` equal to `day`.
- Both divisors confirmed **arithmetically**, not guessed: magic constant
  `0x2AAAAAAB` = 715827883; `715827883 * 6 = 2^32 + 2`, so the first division
  is by 6 (`sll 1 / addu / sll 1` = `q*6`). The second has an extra
  `sra $a0, $a0, 1` folded in before the sign fix and reconstructs as
  `sll 1 / addu / sll 2` = `q*12`. The extra `sra` on the second division only
  is the tell that distinguishes 6/12 from 3/6.
- `day == sSpecialDays[i]` and not the reverse — operand order drives the
  `bne`.
- Plain array indexing, not a pointer walk. Retail's `$v1` pointer walk and
  its `$s0` index are one C variable each doing separate jobs; splitting them
  in the source scores worse (12/52).

## Provenance

Stalled at 18/52 by runner echo, round 2026-09-02, DreamSys tail range.
Re-derived by the head in the same round: the `sltiu` residue closed with the
comparison cast, and the remaining gap identified as `nop_mflo_mfhi` and
confirmed against cc1's pre-maspsx output. echo's ~14 attempts and its
divisor arithmetic are preserved above because they are sound.

### Proposed learning

**When retail compares a signed loop counter with an unsigned `sltiu`, cast at
the comparison, not at the declaration.** `(u32)i < N` changes one
instruction; `u32 i` changes every arithmetic operation the variable
participates in, and on this project that includes swapping signed for
unsigned magic-multiply division.

**And before spending reshapes on an apparent scheduling residue, read cc1's
own output.** cc1 emits `#nop` as a commented hint, not an instruction, so the
raw `cpp | cc1` stream shows what the compiler actually decided — separately
from what the assembler then does to it. A residue that is present after
maspsx and absent in cc1's output is a toolchain question and no amount of
source reshaping will close it.

## A second, independent account exists

This function was worked twice, on two machines, from the same base commit
(`0a544ff`) — the two rounds overlapped without knowing it. The other
machine's runner reached the same C, including the same `(u32)i` cast on the
comparison, and stopped at 18/52 with the two `nop`s unexplained.

That account is preserved in git history at `cc4208e`
(`docs/match-reports/IsDaySpecial.md` on that commit). It has its own attempt
log, which differs from echo's. Nothing in it contradicts this report; it
simply stops one step earlier, before the `nop`s were traced to
`nop_mflo_mfhi` rather than to GCC's scheduler.

Worth noting for its own sake: **two independent runners converged on the
`(u32)i` comparison cast.** That is the strongest evidence available that the
cast is the right reading and not an artifact of one session's search order.

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* The fixed "special day" mood, returned by IsDaySpecial on a match
   (round 2026-09-02); only ever address-taken there, never dereferenced by
   this unit's queued functions. */
```

## History: track 10, debt-world

sSpecialDays is declared with its 42 entries and the loop runs to `(u32)i < ARRAY_COUNT(sSpecialDays)`. The (u32) stays: ARRAY_COUNT is s32 in common.h, and without the cast the compare is signed (51/52).

## Track 10 (2026-09-28, round 104, echo)

`rand() % 6` is `rand() % SPECIAL_DAY_RECORD_COUNT`: the entry it picks is the index GetSpecialDayOrEventRecord takes into the special day's six records (`&rec[pick.entry]`), the count GetSpecialDayRecords strides by. The define (with SPECIAL_DAY_MOVIE_COUNT beside it) moved from GameFiles.c to include/GameFiles.h. The `% 12` stays literal: no GameFiles constant counts the special days. Byte-identical.
