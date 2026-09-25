# GetSoundEffectDir -- MATCHED (9/9 words)

> Renamed from `func_80048E08` on 2026-09-25 (tools/rename.py). Address 0x80048e08.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
9/9, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Returns the string `*GetSoundEffectDirRef()` (the "SND\\SE" pointer D_8008A96C). Callers pass one ignored argument (class_39e08.c declares it `s32 GetSoundEffectDir(s32)`); the body never reads `a0`, so it is written `(void)` here.

## Source

```c
char **GetSoundEffectDirRef(void);   /* defined earlier in this unit */

char *GetSoundEffectDir(void) {
    return *GetSoundEffectDirRef();
}
```

## Notes

- GetRecordTable (already matched) returns D_80081A04 and writes 0x230 to
  `*out`; every `+0x70`/`+0xFC`/`+0x3D40`/`+0x3E04`/`+0x3E20` offset in this
  unit is a whole number of 0x1C-byte records into that table, so the unit
  types the table as `Rec1C` (size only). The record's fields are unknown.
- Callers in other units still declare their own prototypes (`s32` returns in
  class_39e08.h / class_3bb8c.h); those are independent declarations and were
  not touched.
