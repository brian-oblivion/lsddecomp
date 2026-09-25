# IsTmdModelConstructed -- MATCHED (3/3 words), round 82

> Renamed from `func_8001F3A4` on 2026-09-25 (tools/rename.py). Address 0x8001f3a4.

Round 82, runner charlie (matching slot). Unit `src/code_fa50.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called from Class6B5CC (code_d294_b.c) and class_3bb8c_o.c as `IsTmdModelConstructed(model)` (`tools/classtable.py D_8006BEA0`).
- **What:** returns the sbss flag `gTmdModelConstructed` (reached `%gp_rel`, via `--gp-symbols`); its argument is ignored. Callers use the result both as a truth value and as a count.
- **Result:** byte-exact; 3/3 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.


## Source

```c
s32 IsTmdModelConstructed(void *self) {
    return gTmdModelConstructed;
}
```

## Naming

`IsTmdModelConstructed` -- KEPT (not renamed this round). Tier C: mechanics fully
known (returns `gTmdModelConstructed`, ignoring its argument), but renaming
would touch `src/class_3bb8c_o.c`, a live types-runner unit this round
(FINISHING-PLAN track 4); `tools/rename.py` rewrites every caller
tree-wide, so this rename is deferred to avoid the collision.

## Proposed name

`IsTmdModelConstructed` (or `TmdModel__IsConstructed` if a caller confirms
it always takes an actual `TmdModel*`) -- tier B. The head should apply this
with `tools/rename.py` once `class_3bb8c_o.c` is not live, then re-verify.
Posted to the broadcast.
