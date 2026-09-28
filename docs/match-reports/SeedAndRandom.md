# SeedAndRandom -- MATCHED (11/11 words)

> Renamed from `func_80048CFC` on 2026-09-25 (tools/rename.py). Address 0x80048cfc.

Round 82, runner echo, 2026-09-25. Unit `game_files` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
11/11, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Seeds the Sony RNG when the argument is nonzero, then returns `rand()`. GameApplicationFileResource.c calls it with a day number (`*(s32 *)0x1F800000 % 365`) and a second argument the body never reads.

## Source

```c
extern int rand(void);
extern void srand(unsigned int seed);

s32 SeedAndRandom(s32 seed, s32 unused) {
    if (seed != 0) {
        srand(seed);
    }
    return rand();
}
```

## Notes

- GetRecordTable (already matched) returns sRecordTable and writes 0x230 to
  `*out`; every `+0x70`/`+0xFC`/`+0x3D40`/`+0x3E04`/`+0x3E20` offset in this
  unit is a whole number of 0x1C-byte records into that table, so the unit
  types the table as `FilePathRecord` (size only). The record's fields are unknown.
- Callers in other units still declare their own prototypes (`s32` returns in
  DayTaskStageMap.h / class_3bb8c.h); those are independent declarations and were
  not touched.

## The second parameter (round 82, alpha, track 3 externcheck)

The source block above had drifted from `src/cd/game_files.c`, which has
always compiled the 2-parameter form. The second parameter is dead in the
body (`$a1` is never read), but it is real at every call: two callers load
it explicitly (GameApplication__SeedRandom: `move a1,zero` at
0x800260F4; PickSoundBank: `move a1,a0` at 0x80048D78), and the other
three (PickStageTexture, PickStageBgm, PickOpeningMovie) forward their
own incoming second parameter. PickStageBgm and PickOpeningMovie used
to pass an uninitialised local instead; round 82 gave each the parameter it
forwards (byte-identical, two cc1 warnings gone). So the dead parameter
stays: dropping it would make both explicit callers' loads unexplainable.

## Naming

- **Name:** `SeedAndRandom`
- **Tier:** A
- **Evidence:** leaf: `if (seed) srand(seed); return rand();` -- mechanics are the whole purpose.
