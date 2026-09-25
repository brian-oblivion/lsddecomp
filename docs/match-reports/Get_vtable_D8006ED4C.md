# Get_vtable_D8006ED4C -- MATCHED (4/4 words), round 82

> Renamed from `func_80041ED8` on 2026-09-25 (tools/rename.py). Address 0x80041ed8.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py`).
- **What:** Returns the D_8006ED4C method table.
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* Returns the D_8006ED4C method table. */
void *Get_vtable_D8006ED4C(void) {
    return D_8006ED4C;
}
```
