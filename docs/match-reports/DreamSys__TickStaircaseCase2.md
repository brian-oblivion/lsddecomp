# DreamSys__TickStaircaseCase2

> Renamed from `func_8005AD68` on 2026-09-22 (tools/rename.py). Address 0x8005ad68.

**Unit:** DreamSys · **Size:** 54 words · **Status:** MATCHED (54/54 words,
whole-image build verified byte-exact).

**Provenance: this report was written by the HEAD, not by the runner that did
the work.** Runner bravo matched this function and then died to an API error
before it could commit or write anything up, leaving the body uncommitted in
its worktree. Recovered under PARALLEL-RUNS §4c. **This is NOT a mid-attempt
snapshot** — the §4c label for a body no author applied a stop rule to. It is
a finished match: 54/54 words with the whole-image SHA1 green, scored in
`main` after the runner was confirmed dead. What is missing is bravo's attempt
history and its reasoning, which died with the session; everything below is
read off the surviving source, so it explains WHAT the body is and not what
else was tried.

## What it does

`(DreamSys *this) -> s32`. A per-frame step function driven by a retry
counter at `this->unk_0x914`, returning `1` when the counter has run past its
band (the caller's cue that the step is finished) and `0` while still
counting.

On the counter being 0 it seeds a position via `DreamSys__ApplyRelativeOffset`. It then
branches on `this->unk_0xAC != 4` into two structurally parallel arms that
differ in their band limits and in what they do inside the band:

| arm | band guard | in-band window | in-band action |
| --- | --- | --- | --- |
| `unk_0xAC != 4` | `< 0x65` | `0x2B..0x39` | `this->unk_0xA4 = 2` |
| `unk_0xAC == 4` | `< 15`   | `8..9`       | `vt->func_8001CEB4(this, 0, &ROTATION_YAW_PLUS45)` |

Past the band guard each arm returns `1` immediately. Otherwise both fall
through to a shared tail: `unk_0xA0 = 1`, `unk_0x914++`, `return 0`.

## Final C

```c
s32 DreamSys__TickStaircaseCase2(DreamSys *this)
{
	if (this->unk_0x914 == 0) {
		DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_2, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 < 0x65) {
			if ((u32)(this->unk_0x914 - 0x2B) < 0xF) {
				this->unk_0xA4 = 2;
			}
		} else {
			return 1;
		}
	} else {
		if (this->unk_0x914 < 15) {
			if ((u32)(this->unk_0x914 - 8) < 2) {
				this->vt->func_8001CEB4(this, 0, &ROTATION_YAW_PLUS45);
			}
		} else {
			return 1;
		}
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}
```

The two-sided window tests are written as the single unsigned comparison
`(u32)(x - LO) < COUNT`, which is what GCC 2.6.3 emits for a range check and
what retail has here (`addiu` of the negated low bound followed by `sltiu`).
Writing them as `x >= LO && x <= HI` produces the two-branch form instead.

## New struct and symbol knowledge (`include/DreamSys.h`)

All of this came in with the salvaged tree and is part of the match:

- **`DreamSys::unk_0x914` (`s32`)** — the retry/attempt counter, read as a
  whole word, compared against several literal bands and incremented by 1 at
  the normal exit. Carved out of what was previously
  `s8 unknown_values_0x914[0x10]`.
- **`DreamSys::unk_0x91C` (`struct RelativePos`)** — address-taken and passed
  as `DreamSys__ApplyRelativeOffset`'s `b` argument. Also carved out of that same 0x10-byte
  raw block, which is why the block's remainder is now
  `unknown_values_0x918[4]` plus `unknown_values_0x922[2]`.
- **`STAIRCASE_OFFSET_2` (`struct RelativePos`)** — a constant, `DreamSys__ApplyRelativeOffset`'s `a`
  argument.
- **`ROTATION_YAW_PLUS45` (`u8[]`)** — address-of only, never dereferenced here;
  forwarded as vtable slot `+0x044`'s (`func_8001CEB4`) second argument. Same
  opaque-generic-pointer shape as that slot's other known call site.

## Attempts

**Unknown.** Bravo's attempt history did not survive. The body was already
byte-exact when the session died, so the count is at least one and the
reasoning that produced the range-check form is lost.

## Proposed learning

None specific to this function. The process finding belongs to the round
rather than the report: bravo produced eleven matches in `code_2c054`, then
seven more across two DreamSys batches, and the last one existed only as
uncommitted working-tree state when the runner died. Round 7 opened with the
same situation inherited from round 6 (three uncommitted matches in a
worktree that was never torn down) and closed with it happening again to a
live runner. **One-commit-per-match is what makes a runner's work survive its
session, and a runner that batches commits to the end of a batch is one API
error away from losing all of it.** See PARALLEL-RUNS §4b/§4c.

## Naming

- **Tier B.** Table index 2 of the same STAIRCASE_TICK_FNS family, against STAIRCASE_OFFSET_2; same evidence and caveat.
