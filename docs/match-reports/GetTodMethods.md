# GetTodMethods -- MATCHED (4/4 words)

> Renamed from `func_800441A4` on 2026-09-25 (tools/rename.py). Address 0x800441a4.

Round 82, runner echo (GraphicsResources session, echo #5), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gTodMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gTodMethods[];`.

Table slot (`tools/classtable.py`): `gFileResourceMethods` +0x0A4.

## Source

```c
extern s32 gTodMethods[];

void *GetTodMethods(void) {
    return gTodMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/graphics/GraphicsResources.c`.

## Naming

- **GetTodMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 86, charlie)

The local `extern s32 gTodMethods[];` is gone: the table is `extern TodMethods gTodMethods;` in include/Tod.h and the getter returns `TodMethods *` (`return &gTodMethods;`). Bytes unchanged.
