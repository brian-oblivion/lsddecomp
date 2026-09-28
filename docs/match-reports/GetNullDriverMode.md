# GetNullDriverMode -- MATCHED 8/8 (round 43)

> Renamed from `GetVabDriverMode` on 2026-09-28 (tools/rename.py). Address 0x8002c448.

> Renamed from `func_8002C448` on 2026-09-18 (tools/rename.py). Address 0x8002c448.

Unit `PlacementGridVabSound`. Previously filed as a `gp_rel` stall (round 17); reopened
round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi` resolved that blocker for the
whole project (see `docs/research/gp-relative-blocker.md`, "RESOLVED").

## Derivation

Retail:

```
beqz  $a0, .L8002C45C
nop
lw    $v0, %gp_rel(sNullDriverModeArg)($gp)
nop
sw    $v0, 0x0($a0)
.L8002C45C:
lw    $v0, %gp_rel(sNullDriverMode)($gp)
jr    $ra
nop
```

If the argument pointer is non-NULL, store `sNullDriverModeArg` through it; either
way, return `sNullDriverMode`. Same two globals `SetNullDriverMode` (already matched,
just below in ROM order) writes through plain assignment -- this is the
paired reader. Written as an ordinary conditional store plus return:

```c
extern s32 sNullDriverMode;
extern s32 sNullDriverModeArg;

s32 GetNullDriverMode(s32 *arg0) {
    if (arg0 != NULL) {
        *arg0 = sNullDriverModeArg;
    }
    return sNullDriverMode;
}
```

The `extern` declarations were moved up from just after this function (where
they served only `SetNullDriverMode`) to just before it, since this function now
needs them too and C requires the declaration precede use.

## Result

First build, byte-exact:

```
GetNullDriverMode: 8/8 words match (file 0x1CC48-0x1CC68)
```

Whole-image `./build-and-verify.sh` also green (`OK: build matches retail
SLPS_015.56`).

### Proposed learning

Another confirmation that the round-17 `gp_rel` cluster in this unit needed
no special C idiom at all post round-42 -- ordinary pointer-store-then-return
C reproduces the `%gp_rel` load/store pair once the toolchain flags are in
place.

## Naming

Renamed `func_8002C448` -> `GetNullDriverMode`, tier B. Evidence:
`game_shell.c`'s own `func_80026FAC` calls `GetCdDriverMode()` when
`sActiveDataSource == 0x13`, else calls this function -- a direct,
call-site-level substitution for a named Sony "get driver mode" accessor,
confirming this backend's own `sNullDriverMode`/`sNullDriverModeArg` pair
serves the same role for the SPU/VAB data source. Not tier A: the exact
in-game reason `sNullDriverModeArg` exists (a second word alongside the mode
itself) is not established, only that it's read/written alongside the mode.

## Extern arity (round 59)

**Verdict: arity-ok idiom.** `src/app/game_shell.c`'s `(void)` declaration stays.
Identical in shape to `GetCdDriverMode`, its sibling on the previous line.

**Callee evidence** (`0x8002C448`): the first instruction is `beqz a0,...`, so
`$a0` is read before it is written. The definition in `src/sound/PlacementGridVabSound.c`
(`s32 GetNullDriverMode(s32 *arg0)`) is right: one real argument, an optional
out-pointer written only when non-NULL.

**Why the `(void)` extern is right anyway.** Its only carved caller,
`GetActiveDataSourceDriverMode` (this unit, matched), takes no arguments of its
own and sets none:

```
80026fc0:  jal   8002c448 <GetNullDriverMode>
80026fc4:  nop                            <- no $a0 setup, in retail
```

`$a0` is whatever that function's own caller left, and the callee's NULL test
consumes it. A real one-parameter prototype would force an argument retail does
not have.

**Declaration sites changed:** none (arity unchanged). `/* arity-ok: ... */`
added to `src/app/game_shell.c:231`. Oracle green.

## Round 98 (charlie, track 7): parameter

`arg0` -> `outMode2`, after the CD driver's `GetCdDriverMode(s32 *outMode2)`
(cd_driver.c): game_shell.c's `GetActiveDataSourceDriverMode` forwards to
one or the other, so they answer the same query.
