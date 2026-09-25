# D8006EFAC__D8006EFAC -- MATCHED (43/43 words), round 82

> Renamed from `func_800426E4` on 2026-09-25 (tools/rename.py). Address 0x800426e4.

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** D_8006EFAC slot +0x008 (ctor) (`tools/classtable.py`).
- **What:** Class6B5CC ctor via `GetClass6B5CCMethods()`, installs `func_800428E4()`'s table, creates `New_FlatLightObj(0..2)` into +0x044..+0x04C and adds each as a child (slot +0x010), then calls reset (+0x040).
- **Result:** byte-exact; 43/43 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK).
- **Types:** new unit-local view `D_8006EFACObj` (CLASS6B5CC_FIELDS/SLOTS + `lights[3]` +0x044, `SpriteRgb ambient` +0x050, slot +0x0B8 `getChild`); `New_FlatLightObj` and `GsSetAmbient` declared locally. No shared header touched.

## Lever

About ten builds on the loop's source shape, all equal length. `self->lights[i]` index form: store came before `i++`. Pointer with `light++` as a statement: pointer inc in the wrong slot. `New(i++)`: the loop label moved and `move a0,s0` went into the `bnez` slot. The byte-exact shape is `for (i = 0, light = self->lights; i < 3; i++, light++) { *light = New_FlatLightObj(i); self->methods->addChild(self, *light); }`. Initialising `light` before the `for` (or writing `light = ..., i = 0`) swaps `move s0,zero`/`addiu s2,s1,0x44` around the ctor's `jalr`; writing the increment as `light++, i++` moves `addiu s0` after the method load.

## Source

```c
/* D_8006EFAC slot +0x008 (ctor): the Class6B5CC ctor, install the table,
 * create and add the three flat lights, then reset. */
void D8006EFAC__D8006EFAC(D_8006EFACObj *self) {
    s32 i;
    BasicClass **light;

    GetClass6B5CCMethods()->ctor((Class6B5CC *)self);
    self->methods = func_800428E4();
    for (i = 0, light = self->lights; i < 3; i++, light++) {
        *light = New_FlatLightObj(i);
        self->methods->addChild(self, *light);
    }
    self->methods->reset(self);
}
```

### Proposed learning

With a counter and a pointer walking together, the order of the comma operands in BOTH the for-init and the for-increment is visible in the schedule: init order decides which of the two setup instructions lands in the preceding call's delay slot, and increment order decides whether the counter bump is scheduled before the store or after the method-pointer load.
