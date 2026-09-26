# GetTimArraySrcMethods -- MATCHED (4/4 words)

> Renamed from `func_80043E74` on 2026-09-25 (tools/rename.py). Address 0x80043e74.

Round 82, runner echo (code_33808 session, echo #5), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gTimArraySrcMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gTimArraySrcMethods[];`.

Table slot (`tools/classtable.py`): `D_8006D430` +0x08C.

## Source

```c
extern s32 gTimArraySrcMethods[];

void *GetTimArraySrcMethods(void) {
    return gTimArraySrcMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/code_33808.c`.

## Naming

- **GetTimArraySrcMethods**, tier A. Table getter.


## Track 4 (2026-09-26, round 88, runner alpha)
Class unified as TimArraySrc: returns `TimArraySrcMethods *` (`&gTimArraySrcMethods`, declared in include/TimArraySrc.h) instead of `void *` over a local `extern s32 gTimArraySrcMethods[]`, which is deleted. Byte-identical.
