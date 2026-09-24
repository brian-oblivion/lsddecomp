# SetDefaultBMemPMgr — MATCHED (round 45)

> Renamed from `func_80017A9C` on 2026-09-24 (tools/rename.py). Address 0x80017a9c.

**Unit:** `code_8220` · **Size:** 3 words · **Status:** MATCHED, 3/3 words, byte-exact.

## Verdict correction

Filed round 12 (2026-09-03) as `gp_rel`-blocked, reopened round 42
(2026-09-15) as assignable once maspsx's `--gp-symbols` flag was pinned.
This round rebuilt it: byte-exact on the first attempt, no new lever
needed. The prior report's structural read (a one-line setter for the
memory-pool subsystem's global default-pool pointer) was already correct.

## The function

```c
void SetDefaultBMemPMgr(BMemPMgr *pool)
{
    gDefaultBMemPMgr = pool;
}
```

`gDefaultBMemPMgr` is the global "current default pool" pointer, the same global
`SetupBMemPMgrFreeList`, `BMemPMgrAlloc` and `BMemPMgrFree` (this unit) all read.
Declared `extern BMemPMgr *gDefaultBMemPMgr;` in `include/code_8220.h`, next to
`SetupBMemPMgrFreeList`'s own doc comment which already named this global. Not
called from any carved C yet — nothing in this unit or its siblings
invokes it, so whoever establishes the game's one default pool at startup
is still asm elsewhere.

## Provenance

round 45 (2026-09-15), runner alpha. `./build-and-verify.sh` green,
`tools/funcdiff.py SetDefaultBMemPMgr` reports 3/3 words match.

### Proposed learning

None beyond the standing one already on file project-wide: a stale
`gp_rel`-blocked stub, once rebuilt under the round-42 pin, needs zero new
work for a one-line accessor — the old derivation transcribes directly.

## Naming (round 74)

`SetDefaultBMemPMgr`, **tier A** (head, from runner alpha's evidence): the
body is exactly `gDefaultBMemPMgr = pool;`, a pure setter, and its one
caller (`main.c`) calls it immediately after `BMemPMgrInit(pool)` with the
same pool. `D_8008A818` -> `gDefaultBMemPMgr`, tier A: the pointer
`SetupBMemPMgrFreeList`, `BMemPMgrAlloc` and `BMemPMgrFree` read as the
current pool.
