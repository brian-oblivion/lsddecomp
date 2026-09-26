# BgLayer__UpdateScale -- MATCHED (140/140 words)

> Renamed from `BgLayer__SetScale` on 2026-09-26 (tools/rename.py). Address 0x8004441c.

> Renamed from `func_8004441C` on 2026-09-25 (tools/rename.py). Address 0x8004441c.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 140/140 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Scale of the GsBG at +0x044: with src = four halfwords {xnum, xden, ynum, yden}, sx/sy are xnum/xden and ynum/yden in 20.12 fixed point (integer part << 12 plus (remainder << 12) / den, the same formula BgLayer__UpdateRotation uses for the rotation). With `set`: scalex (+0x60) = 0x1000 for a zero divisor, else min((s16)sx, 30000); likewise scaley (+0x62). Otherwise each is added, except that a sum over 30000 yields 30000 -- or 1 when either term of that ratio was negative.

Table slot (`tools/classtable.py`): D_8006F2C4 +0x048.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* D_8006F2C4 +0x048: the two ratios of `src` (+0 over +2, +4 over +6) in
 * 20.12 fixed point become the GsBG's scale -- stored when `set` (0x1000
 * for a zero divisor, at most 30000), else added, a sum over 30000 giving
 * 30000, or 1 when either term of that ratio was negative. */
typedef struct Scale4441C {
    /* +0x00 */ s16 xnum;
    /* +0x02 */ s16 xden;
    /* +0x04 */ s16 ynum;
    /* +0x06 */ s16 yden;
} Scale4441C;

void BgLayer__UpdateScale(Obj6F2C4 *self, s32 set, Scale4441C *src) {
    s32 negX;
    s32 negY;
    s32 den;
    s16 sx;
    s16 sy;
    s32 v;

    negX = 0;
    negY = 0;
    if (src->xnum < 0 || src->xden < 0) {
        negX = 1;
    }
    if (src->ynum < 0 || src->yden < 0) {
        negY = 1;
    }
    den = src->xden;
    if (den != 0) {
        sx = ((src->xnum / den) << 12) + (((src->xnum % den) << 12) / den);
    }
    if (src->yden != 0) {
        sy = ((src->ynum / src->yden) << 12) + (((src->ynum % src->yden) << 12) / src->yden);
    }
    if (set) {
        if (den == 0) {
            self->scalex = 0x1000;
        } else {
            v = sx;
            if (v > 30000) {
                v = 30000;
            }
            self->scalex = v;
        }
        if (src->yden == 0) {
            self->scaley = 0x1000;
        } else {
            v = sy;
            if (v > 30000) {
                v = 30000;
            }
            self->scaley = v;
        }
    } else {
        if (self->scalex + sx > 30000) {
            if (negX) {
                self->scalex = 1;
            } else {
                self->scalex = 30000;
            }
        } else {
            self->scalex = sx + self->scalex;
        }
        if (self->scaley + sy > 30000) {
            if (negY) {
                self->scaley = 1;
            } else {
                self->scaley = 30000;
            }
        } else {
            self->scaley = sy + self->scaley;
        }
    }
}
```

## Notes

Second build. The first build matched everything but the clamp in the `set` arm (89/140): with an `s16 v` temporary the extension moved to the compare and the store took the raw value; retail extends once, before the compare, so the temporary is `s32 v = sx;` (sx/sy themselves are `s16` locals, which gives retail's use-site `sll/sra` in the add arm). The first divisor is held in a local (`den`, retail keeps it in a3 for the `set` arm's zero test) while the second is re-read from `src` (retail reloads +6). sx/sy are left unset on a zero divisor, as in retail. The 0x10 leaf frame came for free. Local view `Scale4441C` just above; the GsBG scale fields are the `Obj6F2C4` fields named by BgLayer__Reset.

## Naming

- **BgLayer__UpdateScale**, tier B. Slot +0x048: two ratios become the GsBG's x/y scale, clamped to 30000, stored or added.

## Track 4 (2026-09-26, round 88, alpha)

Renamed from `BgLayer__SetScale` for the slot it overrides: D_8006F2C4 +0x048 is Class6B5CC's `updateScale` (Class6B5CC__UpdateScale), set or add from the same `WholeFrac_d294` {num, den} ratio table; this override reads entries [0] and [1] (x, y), which were the `Scale4441C` view's xnum/xden/ynum/yden. That view is gone: the live body takes `WholeFrac_d294 *src` and reads `src[0].whole`, `src[0].frac`, `src[1].whole`, `src[1].frac`; `self` is `BgLayer *` (include/BgLayer.h). The clamp to 30000 is this override's own, the slot name still says what it does. Byte-identical.
