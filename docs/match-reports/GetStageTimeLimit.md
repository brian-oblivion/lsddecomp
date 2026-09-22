# GetStageTimeLimit

> Renamed from `func_8005BB14` on 2026-09-22 (tools/rename.py). Address 0x8005bb14.

**Unit:** DreamSys · **Size:** 7 words · **Status:** MATCHED, 7/7.

## What it does

A one-line accessor: index into the `s16 STAGE_TIME_LIMITS[]` table (already
declared in `include/DreamSys.h:1059`) and return the time limit for a given
stage.

```c
s32 GetStageTimeLimit(s32 stage)
{
	return STAGE_TIME_LIMITS[stage];
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

- **Tier A.** One-line table lookup, STAGE_TIME_LIMITS[stage]. Free function, no `this`.
