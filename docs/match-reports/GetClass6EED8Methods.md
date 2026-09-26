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

## Track 4 (2026-09-26, round 87, delta)

Class id 0xB03 is unified as `Class6EED8` in `include/Class6EED8.h`
(FILERESOURCE_SLOTS/FIELDS, 0x30 bytes, one own field `loaded` at +0x02C). The
unit-local views `D_8006EED8Obj`/`D_8006EED8Methods` and the single-slot cast
views `Slot0CMethods_322b4`, `Slot08Arg0Methods_322b4` and
`CtorArg1Methods_322b4` are gone; `GetActiveDataSourceMethods` is declared
`FileResourceMethods *`. The Source block above is the round-82 text; the live
body in `src/code_322b4.c` is byte-identical.

Renamed from `Get_vtable_D8006EED8` with rename.py (the getter
convention); the table `D_8006EED8` is `gClass6EED8Methods` (rename.py).
It returns `Class6EED8Methods *` now (`&gClass6EED8Methods`).
