# GetSsTicksPerSecond -- MATCHED 3/3 (round 43)

> Renamed from `func_8002CC28` on 2026-09-27 (tools/rename.py). Address 0x8002cc28.

Unit `vab_sound`. Previously filed as a `gp_rel` stall (round 17); reopened
round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi` resolved that blocker for the
whole project (see `docs/research/gp-relative-blocker.md`, "RESOLVED").

## Derivation

Same shape as `GetOpenVabCount` right next to it: retail is a single
`lw $v0, %gp_rel(sSsTicksPerSecond)($gp)` then `jr $ra`.

```c
extern s32 sSsTicksPerSecond;

s32 GetSsTicksPerSecond(void) {
    return sSsTicksPerSecond;
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

Kept `GetSsTicksPerSecond` and its global `sSsTicksPerSecond`, tier C. `sSsTicksPerSecond` is
set to the constant `0x3C` once, inside `VabStreamObj__VabStreamObj`'s
second one-time-init guard (alongside an uncarved `SsSetTickMode(1)`
call), and this function is its only reader. That establishes WHEN it's
set and that nothing else in this unit touches it, but not what `0x3C`
configures -- `SsSetTickMode` is still unnamed/uncarved, so there's no
positive evidence to name either the function or the global from.

### Round 98 (charlie, track 7): `func_8002CC28` -> `GetSsTicksPerSecond`, `D_8008A8CC` -> `sSsTicksPerSecond`

Tier A for both. The tier-C note above predates the SDK linking:
`SsSetTickMode` is Sony's (`libsnd`, `<libsnd.h>`), and the call beside the
store is `SsSetTickMode(SS_TICK60)` -- 1 is `SS_TICK60`, sixty sequencer
ticks a second. The store `sSsTicksPerSecond = 60` (was `0x3C`) is that
rate, set in the same one-time guard, and this getter is its only reader.
The one caller, `WBgm__Crescendo` (wbgm.c), passes
`GetSsTicksPerSecond() * scale` as `SsSeqSetCrescendo`'s duration, which
Sony counts in ticks: `scale` is seconds. The value is the tick rate by
its set site and its use; the getter is a leaf (tier A by definition).
