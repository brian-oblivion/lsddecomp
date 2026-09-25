# D8006EFAC__Finalize -- MATCHED (33/33 words), round 82

> Renamed from `func_80042790` on 2026-09-25 (tools/rename.py). Address 0x80042790.

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** D_8006EFAC slot +0x00C (finalize) (`tools/classtable.py`).
- **What:** For i = 0..2, fetches child i through its own slot +0x0B8 (D8006EFAC__GetChild) and calls that child's release (+0x004); then Class6B5CC's finalize via `GetClass6B5CCMethods()`.
- **Result:** byte-exact; 33/33 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** the unit-local `D_8006EFACObj` view (see D8006EFAC__D8006EFAC).

## Source

```c
/* D_8006EFAC slot +0x00C (finalize): release the three lights, then the
 * Class6B5CC finalize. */
void D8006EFAC__Finalize(D_8006EFACObj *self) {
    s32 i;
    BasicClass *light;

    for (i = 0; i < 3; i++) {
        light = self->methods->getChild(self, i);
        light->methods->release(light);
    }
    GetClass6B5CCMethods()->finalize((Class6B5CC *)self);
}
```

## Naming

- `D8006EFAC__Finalize` -- tier A. Finalize (slot +0x00C): releases the three light children then calls the Class6B5CC finalize.
