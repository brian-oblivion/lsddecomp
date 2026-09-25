# D8006EED8__Finalize -- MATCHED (15/15 words), round 82

> Renamed from `func_800423A8` on 2026-09-25 (tools/rename.py). Address 0x800423a8.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EED8 slot +0x00C (finalize) (`tools/classtable.py`).
- **What:** Clears +0x02C (the flag `D8006EED8__SetFlag2C` sets), then calls slot +0x00C of `GetActiveDataSourceMethods()` (code_171e0). The store lands in the `jal` delay slot. `GetActiveDataSourceMethods` is declared locally with a local `Slot0CMethods_322b4` return type, as `code_179d8_e.c` does with its own view.
- **Result:** byte-exact; 15/15 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* D_8006EED8 slot +0x00C (finalize): clear +0x2C, then the base finalize. */
void D8006EED8__Finalize(D_8006EED8Obj *self) {
    self->flag2C = 0;
    GetActiveDataSourceMethods()->slot0C(self);
}
```

## Naming

- `D8006EED8__Finalize` -- tier A. Finalize (slot +0x00C): clears flag2C then calls the base (GetActiveDataSourceMethods) finalize.
