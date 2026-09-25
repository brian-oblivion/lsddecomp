# D8006EF50__Finalize -- MATCHED (14/14 words), round 82

> Renamed from `func_800424A8` on 2026-09-25 (tools/rename.py). Address 0x800424a8.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EF50 slot +0x00C (finalize) (`tools/classtable.py`).
- **What:** `Get_vtable_BasicClass()->finalize(self)`, typed through the UNIFIED `include/BasicClass.h` (reached via `Class6B5CC.h`).
- **Result:** byte-exact; 14/14 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* D_8006EF50 slot +0x00C (finalize): the BasicClass finalize. */
void D8006EF50__Finalize(BasicClass *self) {
    Get_vtable_BasicClass()->finalize(self);
}
```
