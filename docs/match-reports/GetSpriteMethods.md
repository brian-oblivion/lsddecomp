# GetSpriteMethods -- MATCHED (4/4 words), round 82

> Renamed from `func_800422BC` on 2026-09-25 (tools/rename.py). Address 0x800422bc.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py`).
- **What:** Returns the gSpriteMethods method table (the sprite subclass `class_3bb8c_p.c` constructs through it, where it was declared `D8006EE1CMethods *` until track 4).
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* Returns the gSpriteMethods method table. */
SpriteMethods *GetSpriteMethods(void) {
    return &gSpriteMethods;
}
```

## Track 4 (2026-09-25, round 82, alpha)

Renamed from `func_800422BC`: it returns the class's table, the getter every Sprite ctor and subclass ctor calls (Sprite__Sprite, New_Sprite, ScreenSprite's ctor, D800879C4__D800879C4). Now typed `SpriteMethods *` and returns `&gSpriteMethods`: the same `lui/addiu` pair as the array-decay spelling. And the class is unified as `Sprite` in `include/Sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).
