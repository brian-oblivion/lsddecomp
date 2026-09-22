# GetLastSpawnExtra — MATCHED 14/14

> Renamed from `func_8005C118` on 2026-09-22 (tools/rename.py). Address 0x8005c118.

**Unit:** DreamSys · **Size:** 14 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on BOTH `gp_rel` (two references,
`D_8008ACC4`/`D_8008ACC8`) AND `addiu_at` (one runtime-indexed global load
through `D_80088BA4`). Round 21 resolved `addiu_at`; round 42 resolved
`gp_rel` and `nop_mflo_mfhi`. With all three flags now in the Makefile, this
function matched on the first rebuild.

## What it does

A single array-of-array index: `D_80088BA4[D_8008ACC4]` is a `StageSpawn*`
(one of a table of per-stage spawn-point arrays, same table
`Test4StaircaseNodes` already uses a few lines above), indexed a second time
by `D_8008ACC8`, reading that entry's `.extra` byte (`StageSpawn`'s last
field, a signed byte at offset 5 -- matches the retail `lb` at `+0x5`).

## Final body

```c
s32 GetLastSpawnExtra(void)
{
	return D_80088BA4[D_8008ACC4][D_8008ACC8].extra;
}
```

`D_80088BA4`, `D_8008ACC4`, `D_8008ACC8` were all already declared in
`include/DreamSys.h`. Updated the stale header comment on the
`extern s32 GetLastSpawnExtra(void);` prototype (used to type its still-
`INCLUDE_ASM` caller `DreamSys__TryStaircaseLink`, same unit) from "blocked by both
gp-relative and addiu_at" to MATCHED.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py GetLastSpawnExtra` -> `14/14 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys.
