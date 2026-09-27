# GetDataDirectory -- MATCHED 3/3 words, round 42 (2026-09-15)

> Renamed from `func_800270B8` on 2026-09-27 (tools/rename.py). Address 0x800270b8.

> **VERDICT CORRECTED, round 42 (2026-09-15). THIS FUNCTION IS MATCHED.**
> It was blocked by `gp_rel`, which is RESOLVED this round: maspsx gained
> `--gp-symbols` / `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`),
> the whole image is byte-exact with the flags on, and this function was one of
> the live tests -- `return D_8008A854;`, as this report predicted. The C is in `src/code_171e0.c`. Everything below is the
> pre-fix record and is kept as evidence.

> **REOPENED -- WAS ASSIGNABLE, SINCE MATCHED (marker spent), round 42 (2026-09-15).** This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# GetDataDirectory

**Unit:** code_171e0 · **Size:** 3 instructions · **Status:** STALLED, class TOOLCHAIN

## What it does

A getter: `return D_8008A854;` — the read side of `SetDataDirectory`'s setter.
See that report for what's known about `D_8008A854` itself.

## Residue

Same root cause as `LockActiveDataSource` / `SetDataDirectory`: `D_8008A854` is a real
`.sdata` global (file `0x7b008`) that retail addresses with a single
`lw $v0, %gp_rel(D_8008A854)($gp)`, which this project's pinned `-G0`
prevents cc1 from ever emitting. See `LockActiveDataSource.md` for the isolated
reproducer.

## Preserved body

```c
extern void *D_8008A854;

void *GetDataDirectory(void) {
    return D_8008A854;
}
```

## Proposed learning

See `LockActiveDataSource.md`.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

Not renamed -- see `SetDataDirectory.md` (its setter), which carries the full
evidence and reasoning. Same `D_8008A854` global, same absence of usage
evidence, same conclusion.
