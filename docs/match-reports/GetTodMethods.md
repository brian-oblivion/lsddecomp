# GetTodMethods -- MATCHED (4/4 words)

> Renamed from `func_800441A4` on 2026-09-25 (tools/rename.py). Address 0x800441a4.

Round 82, runner echo (code_33808 session, echo #5), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `D_8006F240` (`lui/addiu; jr; nop`), declared locally as `extern s32 D_8006F240[];`.

Table slot (`tools/classtable.py`): `gFileResourceMethods` +0x0A4.

## Source

```c
extern s32 D_8006F240[];

void *GetTodMethods(void) {
    return D_8006F240;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/code_33808.c`.

## Naming

- **GetTodMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 86, charlie)

The local `extern s32 D_8006F240[];` is gone: the table is `extern TodMethods D_8006F240;` in include/Tod.h and the getter returns `TodMethods *` (`return &D_8006F240;`). Bytes unchanged.
