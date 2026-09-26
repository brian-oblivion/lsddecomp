# GetClass6EED8Methods -- MATCHED (4/4 words), round 82

> Renamed from `Get_vtable_D8006EED8` on 2026-09-26 (tools/rename.py). Address 0x800423f0.

> Renamed from `func_800423F0` on 2026-09-25 (tools/rename.py). Address 0x800423f0.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py`).
- **What:** Returns the gClass6EED8Methods method table.
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* Returns the gClass6EED8Methods method table. */
void *GetClass6EED8Methods(void) {
    return gClass6EED8Methods;
}
```

## Naming

- `GetClass6EED8Methods` -- tier A. Table getter ("return gClass6EED8Methods;").
