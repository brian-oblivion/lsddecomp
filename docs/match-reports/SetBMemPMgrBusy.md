> Renamed from `func_8001844C` on 2026-09-17 (tools/rename.py). Address 0x8001844c.

# SetBMemPMgrBusy — MATCHED (round 45)

**Unit:** `code_8220_b` · **Size:** 3 words · **Status:** MATCHED, 3/3 words, byte-exact.

## Verdict correction

Filed round 12 (2026-09-03) as `gp_rel`-blocked, reopened round 42
(2026-09-15). Rebuilt this round: byte-exact on the first attempt. The
prior report's read (a setter for a global, paired with `GetBMemPMgrBusy`
as the getter) was already correct.

## The function

```c
void SetBMemPMgrBusy(s32 val)
{
    gBMemPMgrBusy = val;
}
```

`gBMemPMgrBusy` is a pool allocator/free critical-section flag: this unit's
`func_80017B34`/`func_80017CFC` (in `code_8220.c`, still `INCLUDE_ASM` this
round) bracket their free-list walk with `SetBMemPMgrBusy(1)` on entry and
`SetBMemPMgrBusy(0)` on exit, per those functions' own (stale-verdict, still
undecoded) match reports. Declared `extern s32 gBMemPMgrBusy;` in
`include/code_8220.h` since both `code_8220.c` and `code_8220_b.c` read/
write it.

## Provenance

round 45 (2026-09-15), runner alpha. `./build-and-verify.sh` green,
`tools/funcdiff.py SetBMemPMgrBusy` reports 3/3 words match.

### Proposed learning

None beyond the standing one: a stale `gp_rel`-blocked one-line accessor
needs zero new work once rebuilt under the round-42 `--gp-symbols` pin.
