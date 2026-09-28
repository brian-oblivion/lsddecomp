# TmdModel__SetQuad -- MATCHED (10/10 words), round 82

> Renamed from `func_8001F314` on 2026-09-25 (tools/rename.py). Address 0x8001f314.

Round 82, runner charlie (matching slot). Unit `src/graphics/TmdModel.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x040 of gTmdModelMethods (`tools/classtable.py gTmdModelMethods`).
- **What:** copies four words from `src` into object +0x14..+0x20
- **Result:** byte-exact; 10/10 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.
- **Lever:** `lw` x4 then `sw` x4 (all loads before all stores) is a whole-struct assignment of a 4-`s32` struct, `self->quad = *src;`. Four separate field statements would interleave loads and stores.

## Source

```c
void TmdModel__SetQuad(TmdModel *self, Quad_fa50 *src) {
    self->quad = *src;
}
```

## Naming

`TmdModel__SetQuad` -- tier A. Slot +0x040: `self->quad = *src;`, a plain
4-word struct setter. A setter's mechanics are its purpose (FINISHING-PLAN
track 3's tier-A rule for a pure leaf).
