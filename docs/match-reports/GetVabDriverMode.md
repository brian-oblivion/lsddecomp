> Renamed from `func_8002C448` on 2026-09-18 (tools/rename.py). Address 0x8002c448.

# GetVabDriverMode -- MATCHED 8/8 (round 43)

Unit `code_179d8_e`. Previously filed as a `gp_rel` stall (round 17); reopened
round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi` resolved that blocker for the
whole project (see `docs/research/gp-relative-blocker.md`, "RESOLVED").

## Derivation

Retail:

```
beqz  $a0, .L8002C45C
nop
lw    $v0, %gp_rel(gVabDriverModeArg)($gp)
nop
sw    $v0, 0x0($a0)
.L8002C45C:
lw    $v0, %gp_rel(gVabDriverMode)($gp)
jr    $ra
nop
```

If the argument pointer is non-NULL, store `gVabDriverModeArg` through it; either
way, return `gVabDriverMode`. Same two globals `SetVabDriverMode` (already matched,
just below in ROM order) writes through plain assignment -- this is the
paired reader. Written as an ordinary conditional store plus return:

```c
extern s32 gVabDriverMode;
extern s32 gVabDriverModeArg;

s32 GetVabDriverMode(s32 *arg0) {
    if (arg0 != NULL) {
        *arg0 = gVabDriverModeArg;
    }
    return gVabDriverMode;
}
```

The `extern` declarations were moved up from just after this function (where
they served only `SetVabDriverMode`) to just before it, since this function now
needs them too and C requires the declaration precede use.

## Result

First build, byte-exact:

```
GetVabDriverMode: 8/8 words match (file 0x1CC48-0x1CC68)
```

Whole-image `./build-and-verify.sh` also green (`OK: build matches retail
SLPS_015.56`).

### Proposed learning

Another confirmation that the round-17 `gp_rel` cluster in this unit needed
no special C idiom at all post round-42 -- ordinary pointer-store-then-return
C reproduces the `%gp_rel` load/store pair once the toolchain flags are in
place.
