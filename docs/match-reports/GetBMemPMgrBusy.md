# GetBMemPMgrBusy — MATCHED (round 45)

> Renamed from `func_80018458` on 2026-09-17 (tools/rename.py). Address 0x80018458.

**Unit:** `TmdRenderer` · **Size:** 3 words · **Status:** MATCHED, 3/3 words, byte-exact.

## Verdict correction

Filed round 12 (2026-09-03) as `gp_rel`-blocked, reopened round 42
(2026-09-15). Rebuilt this round: byte-exact on the first attempt. The
prior report's read (a getter, paired with `SetBMemPMgrBusy`'s setter) was
already correct.

## The function

```c
s32 GetBMemPMgrBusy(void)
{
    return sBMemPMgrBusy;
}
```

Same global as `SetBMemPMgrBusy` (this unit, matched alongside this one this
round) — the pool allocator/free critical-section flag. Declaration shared
via `include/BMemPMgr.h`.

## Provenance

round 45 (2026-09-15), runner alpha. `./build-and-verify.sh` green,
`tools/funcdiff.py GetBMemPMgrBusy` reports 3/3 words match.

### Proposed learning

None beyond the standing one: a stale `gp_rel`-blocked one-line accessor
needs zero new work once rebuilt under the round-42 `--gp-symbols` pin.

## Naming (round 51, bravo)

`func_80018458` -> `GetBMemPMgrBusy`. **Tier A** -- a one-line getter,
mechanics are its purpose. The evidence for the "Busy" reading of the
global it returns is written up once, in
`docs/match-reports/SetBMemPMgrBusy.md`.

The one thing specific to this half: **this function is what pins the
semantics**, and it does so from outside this unit. `func_800280EC`
(`src/cd/CdDriver.c`) calls it and returns early when it is non-zero,
before touching `VSyncCallback` -- interrupt-time code declining to run
while the pool allocator is walking its free list. The setter's two call
sites alone would only have shown a flag being raised and lowered.
