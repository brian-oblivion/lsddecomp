# Sprite__SetDisplay -- MATCHED (12/12 words), round 82

> Renamed from `func_8004220C` on 2026-09-25 (tools/rename.py). Address 0x8004220c.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x060 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and D_800879C4 (`tools/classtable.py`).
- **What:** `GetSetBitField(&self->sprite.attribute, 0x1F, 1, a1 == 0) == 0` (inverted display flag, bit 31).
- **Result:** byte-exact; 12/12 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* Sprite classes slot +0x060: display on/off (attribute bit 31, inverted). */
s32 Sprite__SetDisplay(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1F, 1, a1 == 0) == 0;
}
```

## Track 4 (2026-09-25, round 82, alpha)

Renamed from `func_8004220C` for its slot: +0x060 is `setDisplay` in Class6B5CC, and this body is the same GetSetBitField edit on the embedded GsSPRITE's attribute (bit 31 = GsDOFF, inverted) that Class6B5CC__SetDisplay makes on the GsDOBJ2's. First occupant in gSpriteMethods; inherited by every sprite subclass. And the class is unified as `Sprite` in `include/Sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).
