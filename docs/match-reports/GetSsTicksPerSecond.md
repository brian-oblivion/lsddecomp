# GetSsTicksPerSecond -- MATCHED 3/3 (round 43)

> Renamed from `func_8002CC28` on 2026-09-27 (tools/rename.py). Address 0x8002cc28.

Unit `code_179d8_e`. Previously filed as a `gp_rel` stall (round 17); reopened
round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi` resolved that blocker for the
whole project (see `docs/research/gp-relative-blocker.md`, "RESOLVED").

## Derivation

Same shape as `GetOpenVabCount` right next to it: retail is a single
`lw $v0, %gp_rel(gSsTicksPerSecond)($gp)` then `jr $ra`.

```c
extern s32 gSsTicksPerSecond;

s32 GetSsTicksPerSecond(void) {
    return gSsTicksPerSecond;
}
```

## Result

First build, byte-exact:

```
GetSsTicksPerSecond: 3/3 words match (file 0x1D428-0x1D434)
```

Whole-image `./build-and-verify.sh` also green (`OK: build matches retail
SLPS_015.56`).

### Proposed learning

See `GetOpenVabCount.md` -- same cluster, same result: a plain `extern`
accessor is all `--gp-symbols` needed to reproduce the `%gp_rel` load.

## Naming

Kept `GetSsTicksPerSecond` and its global `gSsTicksPerSecond`, tier C. `gSsTicksPerSecond` is
set to the constant `0x3C` once, inside `VabStreamObj__VabStreamObj`'s
second one-time-init guard (alongside an uncarved `SsSetTickMode(1)`
call), and this function is its only reader. That establishes WHEN it's
set and that nothing else in this unit touches it, but not what `0x3C`
configures -- `SsSetTickMode` is still unnamed/uncarved, so there's no
positive evidence to name either the function or the global from.
