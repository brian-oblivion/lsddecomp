# SetNullDriverMode -- MATCHED 4/4 words, round 42 (2026-09-15)

> Renamed from `SetVabDriverMode` on 2026-09-28 (tools/rename.py). Address 0x8002c468.

> Renamed from `func_8002C468` on 2026-09-18 (tools/rename.py). Address 0x8002c468.

> **VERDICT CORRECTED, round 42 (2026-09-15). THIS FUNCTION IS MATCHED.**
> It was blocked by `gp_rel`, which is RESOLVED this round: maspsx gained
> `--gp-symbols` / `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`),
> the whole image is byte-exact with the flags on, and this function was one of
> the live tests -- two stores to `sNullDriverMode`/`sNullDriverModeArg` and `return 1`. The C is in `src/sound/vab_sound.c`. Everything below is the
> pre-fix record and is kept as evidence.

> **REOPENED -- WAS ASSIGNABLE, SINCE MATCHED (marker spent), round 42 (2026-09-15).** This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# SetNullDriverMode -- STALL (gp-relative blocker, not attempted)

Unit `vab_sound`, carved round 17 (2026-09-04). **Not attempted.**

## Classification

```sh
grep -n 'gp_rel' asm/nonmatchings/vab_sound/SetNullDriverMode.s
```

Hit:

```
sw $a0, %gp_rel(sNullDriverMode)($gp)
```

Retail reaches this small-data global in ONE instruction off `$gp`. The
pinned pipeline (`-G0` at both cc1 and `as`) cannot emit that form from C --
it emits the two-instruction absolute `lui`/`lw` pair instead, so the
mismatch is not contained: every function after it in the same translation
unit shifts by a word.

`docs/research/gp-relative-blocker.md` records the reproducer and the
measured `-G` matrix: the gp-relative form needs a non-zero `-G` at BOTH
stages, and the `-G` experiment was run in 2026-08-29 WITH operator
authorisation and REJECTED. The pin stands. This is the operator's call, not
something to experiment with mid-round.

No C was written and no score was measured. The screen ran at carve time,
before the unit was offered to a runner, so no attempt budget was spent
discovering this.

## Naming

Renamed `func_8002C468` -> `SetNullDriverMode`, tier B. Same evidence as
`GetNullDriverMode` (its report): `game_shell.c`'s `func_80026F34` assigns
this function to a `DataSourceSetDriverModeFn` variable used exactly where it assigns
Sony's `SetCdDriverMode` on the other branch of `sActiveDataSource == 0x13`
-- a genuine drop-in substitute for a named "set driver mode" call, for
this backend's own state pair.

## Round 98 (charlie, track 7): parameters

`(a, b)` -> `(async, mode2)`, after the CD driver's
`SetCdDriverMode(s32 async, s32 mode2, s32 useVSyncCallback)`: game_shell.c's
`SetActiveDataSourceDriverMode` calls one or the other with the same three
words, and this one ignores the third.
