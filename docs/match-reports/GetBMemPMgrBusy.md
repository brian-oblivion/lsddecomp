> Renamed from `func_80018458` on 2026-09-17 (tools/rename.py). Address 0x80018458.

# GetBMemPMgrBusy — MATCHED (round 45)

**Unit:** `code_8220_b` · **Size:** 3 words · **Status:** MATCHED, 3/3 words, byte-exact.

## Verdict correction

Filed round 12 (2026-09-03) as `gp_rel`-blocked, reopened round 42
(2026-09-15). Rebuilt this round: byte-exact on the first attempt. The
prior report's read (a getter, paired with `SetBMemPMgrBusy`'s setter) was
already correct.

## The function

```c
s32 GetBMemPMgrBusy(void)
{
    return gBMemPMgrBusy;
}
```

Same global as `SetBMemPMgrBusy` (this unit, matched alongside this one this
round) — the pool allocator/free critical-section flag. Declaration shared
via `include/code_8220.h`.

## Provenance

round 45 (2026-09-15), runner alpha. `./build-and-verify.sh` green,
`tools/funcdiff.py GetBMemPMgrBusy` reports 3/3 words match.

### Proposed learning

None beyond the standing one: a stale `gp_rel`-blocked one-line accessor
needs zero new work once rebuilt under the round-42 `--gp-symbols` pin.
