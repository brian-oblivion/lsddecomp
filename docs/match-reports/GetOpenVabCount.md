> Renamed from `func_8002CC1C` on 2026-09-18 (tools/rename.py). Address 0x8002cc1c.

# GetOpenVabCount -- MATCHED 3/3 (round 43)

Unit `code_179d8_e`. Previously filed as a `gp_rel` stall (round 17); reopened
round 42 once `--gp-symbols`/`--no-nop-mflo-mfhi` resolved that blocker for the
whole project (see `docs/research/gp-relative-blocker.md`, "RESOLVED").

## Derivation

The prior stall's classification was correct on mechanism: retail is a single
`lw $v0, %gp_rel(gOpenVabCount)($gp)` then `jr $ra`. With the gp-relative flags
now passed by the Makefile, an ordinary C accessor compiles straight to that
same form -- no special handling needed.

```c
extern s32 gOpenVabCount;

s32 GetOpenVabCount(void) {
    return gOpenVabCount;
}
```

## Result

First build, byte-exact:

```
GetOpenVabCount: 3/3 words match (file 0x1D41C-0x1D428)
```

Whole-image `./build-and-verify.sh` also green (`OK: build matches retail
SLPS_015.56`).

### Proposed learning

The round-17 `gp_rel` stalls in this unit were correctly diagnosed on
mechanism and needed zero C changes once the toolchain flags landed in round
42 -- an ordinary declared `extern` plus `return` was sufficient. Nothing
about the C shape needed to change for `--gp-symbols` to kick in; it is purely
a maspsx-side fix.

## Naming

Renamed `func_8002CC1C` -> `GetOpenVabCount`, tier A. The global it reads
(`gOpenVabCount`, `D_8008A8C4` before round 52) is incremented once per
object in `VabStreamObj__VabStreamObj` and decremented/clamped-at-zero in
`VabStreamObj__Close`, with a zero-count check gating the shared subsystem
teardown -- an unambiguous open-object refcount, so this is a plain getter
for it.
