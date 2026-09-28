# GetDataDirectory -- MATCHED 3/3 words, round 42 (2026-09-15)

> Renamed from `func_800270B8` on 2026-09-27 (tools/rename.py). Address 0x800270b8.

> **VERDICT CORRECTED, round 42 (2026-09-15). THIS FUNCTION IS MATCHED.**
> It was blocked by `gp_rel`, which is RESOLVED this round: maspsx gained
> `--gp-symbols` / `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`),
> the whole image is byte-exact with the flags on, and this function was one of
> the live tests -- `return sDataDirectory;`, as this report predicted. The C is in `src/app/game_shell.c`. Everything below is the
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

**Unit:** game_shell · **Size:** 3 instructions · **Status:** STALLED, class TOOLCHAIN

## What it does

A getter: `return sDataDirectory;` — the read side of `SetDataDirectory`'s setter.
See that report for what's known about `sDataDirectory` itself.

## Residue

Same root cause as `LockActiveDataSource` / `SetDataDirectory`: `sDataDirectory` is a real
`.sdata` global (file `0x7b008`) that retail addresses with a single
`lw $v0, %gp_rel(sDataDirectory)($gp)`, which this project's pinned `-G0`
prevents cc1 from ever emitting. See `LockActiveDataSource.md` for the isolated
reproducer.

## Preserved body

```c
extern void *sDataDirectory;

void *GetDataDirectory(void) {
    return sDataDirectory;
}
```

## Proposed learning

See `LockActiveDataSource.md`.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

Not renamed -- see `SetDataDirectory.md` (its setter), which carries the full
evidence and reasoning. Same `sDataDirectory` global, same absence of usage
evidence, same conclusion.

## Naming (round 99, charlie, track 7)

Renamed `func_800270B8` -> `GetDataDirectory`, tier A. This supersedes the
round-52 entry above, whose names the rename tools rewrote. The evidence
(two readers, `BuildCdFilePath` and `CdStream__Open`, which both build
`"\\" + dir + name + ";1"`, and the setter's single caller, which installs
`"CDI\\"`) is in `SetDataDirectory.md`.
