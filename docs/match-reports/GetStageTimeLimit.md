# GetStageTimeLimit

> Renamed from `func_8005BB14` on 2026-09-22 (tools/rename.py). Address 0x8005bb14.

**Unit:** DreamSys · **Size:** 7 words · **Status:** MATCHED, 7/7.

## What it does

A one-line accessor: index into the `s16 sStageTimeLimits[]` table (already
declared in `include/dream_sys.h:1059`) and return the time limit for a given
stage.

```c
s32 GetStageTimeLimit(s32 stage)
{
	return sStageTimeLimits[stage];
}
```

The disassembly shows the classic indexed-load shape (`sll $a0,$a0,1` to scale
the index for a 2-byte element, `lui`/`addiu %lo` to form the table base,
`addu`, then `lh`). This is the runtime-indexed-global-load construct that used
to be the `addiu_at` blocker; it is resolved as of round 21 and this function
was screened clean by the head against both remaining blockers
(`gp_rel`, `nop_mflo_mfhi`) before assignment.

## Attempts

First attempt matched byte-exact. No iteration needed — `lh` sign-extends
into `$v0` directly, so the return type (`s16` vs `s32`) is not observable at
this call shape; `s32` was chosen since no caller/prototype fixes it yet.

### Proposed learning

None beyond what CLAUDE.md already documents about `addiu_at` being resolved —
this function is a clean confirmation that a single-instruction-scaled table
index (`sll $a0,$a0,1` then `addiu_at` form) now matches on the first try with
plain array indexing, no reshaping required.

## Naming

- **Tier A.** One-line table lookup, sStageTimeLimits[stage]. Free function, no `this`.

## Head naming review, round 65: TIER corrected A -> B

The round-65 naming runner recorded this name at tier A on the evidence
"table lookup". The body is `return sStageTimeLimits[stage];` and the whole
name is read off that array's symbol — which is an INHERITED name
(`config/symbols.slps01556.lsdde.txt`, `sStageTimeLimits = 0x80087F14`,
predating this round; the runner did not rename it).

Track 3: "Every inherited name is a tier-B hypothesis. Confirm it with
evidence or rename it; either way, record it." A tier-A name whose only
support is an unconfirmed inherited name inherits that name's uncertainty
rather than escaping it. The name itself is probably right — two other call
sites in this unit assign the lookup straight into a variable already called
`timeLimit` — but that is the same inherited reading again, not independent
evidence. **Tier B** until a consumer is found that shows the value used as a
time limit.
