# BgLayer__UpdateRotation -- MATCHED (39/39 words)

> Renamed from `BgLayer__SetRotation` on 2026-09-26 (tools/rename.py). Address 0x80044380.

> Renamed from `func_80044380` on 2026-09-25 (tools/rename.py). Address 0x80044380.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 39/39 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Computes the 20.12 fixed-point ratio of two s16 fields of the third argument (+0x08 / +0x0A) as (q << 12) + ((r << 12) / den) from one div's quotient and remainder, and stores it at +0x64 when the second argument is nonzero, otherwise adds it to +0x64.

Table slot (`tools/classtable.py`): D_8006F2C4 +0x044 (a SceneNode subclass; slot +0x044 is its first own slot past the SceneNode reset slot at +0x040).

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
typedef struct Obj6F2C4 {
    SCENENODE_FIELDS(SceneNodeMethods);
    /* +0x044 */ u8 pad44[0x10];
    /* +0x054 */ Vec3S8 unk54;
    /* +0x057 */ u8 pad57[0xD];
    /* +0x064 */ s32 unk64;   /* 20.12 fixed point */
} Obj6F2C4;

typedef struct Ratio44380 {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ s16 num;
    /* +0x0A */ s16 den;
} Ratio44380;

void BgLayer__UpdateRotation(Obj6F2C4 *self, s32 set, Ratio44380 *src) {
    s32 num = src->num;
    s32 den = src->den;
    s32 v = ((num / den) << 12) + (((num % den) << 12) / den);

    if (set) {
        self->unk64 = v;
    } else {
        self->unk64 += v;
    }
}
```

## Notes

First build. `/` and `%` of the same operands share one div (mflo then mfhi). The unit-local `Obj6F2C4` gained +0x64 (s32) after a pad, additively; its size is now 0x68, the allocator's (New_BgLayer) BMemPMgrAlloc size.

## Naming

- **BgLayer__UpdateRotation**, tier B. Slot +0x044: a ratio converted to 20.12 fixed point, stored or added into the rotate field.

## Track 4 (2026-09-26, round 88, alpha)

Renamed from `BgLayer__SetRotation` for the slot it overrides: D_8006F2C4 +0x044 is SceneNode's `updateRotation` (SceneNode__UpdateRotation), and the body does what that slot does, set (flag nonzero) or add, from the same three-entry `WholeFrac_d294` {num, den} ratio table SceneNode's version reads through RatioToFixed12 (entries at +0/+4/+8). A GsBG has one rotation, so this override reads only entry [2], the z angle: the `Ratio44380` view's +0x08/+0x0A. That view is gone; the live body takes `WholeFrac_d294 *table` and reads `table[2].whole` / `table[2].frac`, `self` is `BgLayer *` (include/BgLayer.h) and +0x064 is `rotate` (GsBG.rotate; was unk64). Byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
