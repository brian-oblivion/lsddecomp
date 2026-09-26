# GetTodSetMethods -- MATCHED (4/4 words)

> Renamed from `func_80045428` on 2026-09-25 (tools/rename.py). Address 0x80045428.

Round 82, runner echo (code_33808 session, echo #5), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `D_8006F590` (`lui/addiu; jr; nop`), declared locally as `extern s32 D_8006F590[];`.

Table slot (`tools/classtable.py`): `gFileResourceMethods` +0x0A8.

## Source

```c
extern s32 D_8006F590[];

void *GetTodSetMethods(void) {
    return D_8006F590;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/code_33808.c`.

## Naming

- **GetTodSetMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 88, delta)

Now `TodSetMethods *GetTodSetMethods(void)` returning `&D_8006F590` (include/TodSet.h), replacing the unit-local `extern s32 D_8006F590[]` and `void *` prototype. Bytes unchanged.
