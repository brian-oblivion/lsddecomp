# SetBMemPMgrBusy — MATCHED (round 45)

> Renamed from `func_8001844C` on 2026-09-17 (tools/rename.py). Address 0x8001844c.

**Unit:** `TmdRenderer` · **Size:** 3 words · **Status:** MATCHED, 3/3 words, byte-exact.

## Verdict correction

Filed round 12 (2026-09-03) as `gp_rel`-blocked, reopened round 42
(2026-09-15). Rebuilt this round: byte-exact on the first attempt. The
prior report's read (a setter for a global, paired with `GetBMemPMgrBusy`
as the getter) was already correct.

## The function

```c
void SetBMemPMgrBusy(s32 val)
{
    sBMemPMgrBusy = val;
}
```

`sBMemPMgrBusy` is a pool allocator/free critical-section flag: this unit's
`BMemPMgrAlloc`/`BMemPMgrFree` (in `BMemPMgr.c`, still `INCLUDE_ASM` this
round) bracket their free-list walk with `SetBMemPMgrBusy(1)` on entry and
`SetBMemPMgrBusy(0)` on exit, per those functions' own (stale-verdict, still
undecoded) match reports. Declared `extern s32 sBMemPMgrBusy;` in
`include/BMemPMgr.h` since both `BMemPMgr.c` and `TmdRenderer.c` read/
write it.

## Provenance

round 45 (2026-09-15), runner alpha. `./build-and-verify.sh` green,
`tools/funcdiff.py SetBMemPMgrBusy` reports 3/3 words match.

### Proposed learning

None beyond the standing one: a stale `gp_rel`-blocked one-line accessor
needs zero new work once rebuilt under the round-42 `--gp-symbols` pin.

## Naming (round 51, bravo)

`func_8001844C` -> `SetBMemPMgrBusy`, and its global `D_8008A820` ->
`sBMemPMgrBusy`.

**The function: tier A** -- a one-line setter, mechanics are its purpose.

**The global's name: tier B** -- "Busy" describes a protocol that was
measured, but nothing establishes what the flag is called in the original
source. Three independent pieces of evidence, two of them new this round:

1. `BMemPMgrAlloc` and `BMemPMgrFree` (the pool allocator's alloc/free
   pair, `BMemPMgr.c`, still `INCLUDE_ASM`) bracket their free-list walk
   with `SetBMemPMgrBusy(1)` on entry and `SetBMemPMgrBusy(0)` on exit --
   visible as two `jal func_8001844C` in
   `asm/nonmatchings/BMemPMgr/BMemPMgrAlloc.s`.
2. **New this round:** `func_800280EC` in `src/cd/CdDriver.c` reads it
   through the getter and BAILS OUT -- `if (GetBMemPMgrBusy() != 0) return
   0;` -- before doing `VSyncCallback` work. So the flag is read by
   interrupt-time code specifically to avoid running while the allocator is
   mid-walk.
3. There is no busy-wait and no test-and-set anywhere: nothing spins on it.

(1) plus (2) is a re-entrancy guard; (3) is why the name is not `Lock`.
`BMemPMgr` is the pool manager's already-established type name.

Renaming the global also made `config/gp-symbols.txt` stale --
`tools/rename.py` rewrites an sdata symbol's name in place, while
`tools/gpsyms.py` regenerates the list in its own sort order, so the content
was already right and only the line's position differed.
`python3 tools/gpsyms.py` fixes it and the build stayed byte-exact either
way; any naming round touching an sdata/sbss symbol needs the same step.

## Round 91 polish (delta, track 7)

Parameter `val` -> `busy`.
