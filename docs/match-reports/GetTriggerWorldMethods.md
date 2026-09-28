# GetTriggerWorldMethods -- MATCHED (4/4 words)

> Renamed from `func_80044CC4` on 2026-09-25 (tools/rename.py). Address 0x80044cc4.

Round 82, runner echo (graphics_resources session, echo #5), 2026-09-25. Unit `graphics_resources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gTriggerWorldMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gTriggerWorldMethods[];`.

Table slot (`tools/classtable.py`): `gFileResourceMethods` +0x0B0.

## Source

```c
extern s32 gTriggerWorldMethods[];

void *GetTriggerWorldMethods(void) {
    return gTriggerWorldMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/graphics/graphics_resources.c`.

## Naming

- **GetTriggerWorldMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 88, bravo)

Now `TriggerWorldMethods *GetTriggerWorldMethods(void)` returning `&gTriggerWorldMethods` (include/TriggerWorld.h), replacing the unit-local `extern s32 gTriggerWorldMethods[]` and `void *` prototype. Bytes unchanged.
