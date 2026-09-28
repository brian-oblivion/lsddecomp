# TmdModel__NoOpSlot4C -- MATCHED (2/2 words), round 82

> Renamed from `TmdModel__func_8001F37C` on 2026-09-28 (tools/rename.py). Address 0x8001f37c.

> Renamed from `func_8001F37C` on 2026-09-25 (tools/rename.py). Address 0x8001f37c.

Round 82, runner charlie (matching slot). Unit `src/graphics/tmd_model.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x04C of gTmdModelMethods (`tools/classtable.py gTmdModelMethods`).
- **What:** empty method (`jr $ra; nop`)
- **Result:** byte-exact; 2/2 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.
- **Lever:** An empty body is `void f(void) {}` (broadcast lever).

## Source

```c
void TmdModel__NoOpSlot4C(void) {
}
void *GetTmdModelMethods(void) {
    return gTmdModelMethods;
}
```

## Naming

`TmdModel__NoOpSlot4C` -- tier C, class known. Slot +0x04C is an empty
override (`jr $ra; nop`) with no callers found in this unit and no other
information about what the slot means. Per FINISHING-PLAN track 3's tier-C
form for a method whose class is known (`Class__func_xxxxx`), kept as such
rather than guessing a purpose from an empty body.
