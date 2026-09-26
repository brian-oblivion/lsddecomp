# FadeClutRow -- MATCHED (122/122 words)

> Renamed from `func_80043648` on 2026-09-25 (tools/rename.py). Address 0x80043648.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 122/122 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Fade one 256-colour CLUT row toward the entry's colour. StoreImage reads the row (x 0, y (index << gTimBlockClutShift) + 0x1E0, 256 x 1) into a local buffer; then for each of mask - 1 steps, with f = (step + 1) << (12 - shift), every non-zero 15-bit colour is blended channel by channel ((c * (0x1000 - f) + colour * f) >> 15 on the <<3-scaled 5-bit channels, keeping the STP bit) into a second buffer, which LoadImage uploads to the next row down (dst.y = src.y + step + src.h), DrawSync between. Also copies the entry's mask to its +0x0A.

Table slot (`tools/classtable.py`): not in any method table (called by TimBlockSrc__FadeEntry, gTimBlockSrcMethods +0x080).

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* Fade one 256-colour CLUT row (the entry's `index`, from VRAM y 0x1E0)
 * toward the entry's colour: read the row back, then for each of
 * mask - 1 steps blend every non-zero colour (step << (12 - shift)) / 0x1000
 * of the way to the colour and upload the result to the next row down. */
typedef struct Rect43648 {       /* LIBGPU.H RECT */
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect43648;

/* LIBGPU.H */
extern int StoreImage(Rect43648 *rect, u32 *p);
extern int LoadImage(Rect43648 *rect, u32 *p);
extern int DrawSync(int mode);

void FadeClutRow(Ent6F0B8 *e, s32 index) {
    Rect43648 dst;
    Rect43648 src;
    u16 out[256];
    u16 in[256];
    s32 i;
    s32 j;
    s32 shift;
    s32 r;
    s32 g;
    s32 b;
    s32 f;
    s32 rr;
    s32 gg;
    s32 bb;
    u32 c;
    s32 cr;
    s32 cg;
    s32 cb;

    src.x = 0;
    src.w = 0x100;
    src.h = 1;
    src.y = (index << gTimBlockClutShift) + 0x1E0;
    StoreImage(&src, (u32 *)in);
    DrawSync(0);
    dst.h = 1;
    dst.x = 0;
    dst.y = 0;
    dst.w = 0x100;
    r = (u8)e->vec.x;
    g = (u8)e->vec.y;
    b = (u8)e->vec.z;
    shift = 12 - e->shift;
    e->unkA = e->mask;
    for (i = 0; i < e->mask - 1; i++) {
        f = (i + 1) << shift;
        rr = r * f;
        gg = g * f;
        bb = b * f;
        f = 0x1000 - f;
        for (j = 0; j < src.w; j++) {
            c = in[j];
            if (c == 0) {
                out[j] = in[j];
            } else {
                cr = (in[j] & 0x1F) << 3;
                cg = (c >> 2) & 0xF8;
                cb = (c >> 7) & 0xF8;
                cr = (cr * f + rr) >> 15;
                cg = (cg * f + gg) >> 15;
                cb = (cb * f + bb) >> 15;
                out[j] = (in[j] & 0x8000) | cr | (cg << 5) | (cb << 10);
            }
        }
        dst.y = src.y + i + src.h;
        DrawSync(0);
        LoadImage(&dst, (u32 *)out);
    }
}
```

## Notes

Seventh build. Levers, each measured: (1) the colour held in a `u32 c` read ONCE for the zero test and the g/b channels, while the r channel and the STP bit re-read `in[j]` -- retail reloads in[j] in both arms; a `u16 c` let cc1 merge every read, add an `andi 0xffff` and cross-jump the copy arm into the blend's store (49/122); `s32 c` got the reload shape but `sra` for `c >> 2` (90/122); (2) per-channel `s32 cr/cg/cb` temporaries so the unsigned `c` does not make the products unsigned (`srl` where retail has `sra` for `>> 15`), and each channel's blend assigned back to its temporary before one final OR -- retail computes all three then ORs (92 -> 110/122); (3) the blend factor reused in place, `f = 0x1000 - f;` rather than a separate `inv` -- the last register difference (t0 for both, t1 for j) (110 -> 122/122). The frame (0x450: two RECTs and two 256-entry buffers) matched from the first build. The unit-local `Ent6F0B8` view gained `u16 unkA` at +0x0A (its `pad4[8]` became `pad4[6]` + unkA; TimBlockSrc__SetEntryShift/TimBlockSrc__FadeEntry still byte-exact, whole image green). StoreImage/LoadImage/DrawSync are Sony's (LIBGPU), extern only, with a unit-local RECT view `Rect43648`.

## Naming

- **FadeClutRow**, tier B. Free function: blends one 256-colour CLUT row toward a target colour over `mask-1` steps and uploads each step via LIBGPU StoreImage/LoadImage; a pure mechanics leaf (tier B since 'fade' is a purpose word, but the blend math itself is the whole body).

## Track 4 (2026-09-25, round 83, bravo)

Takes `TimBlockSrcEntry *` (merged from the unit's Ent43068/Ent6F0B8 views); +0x0A is `clutH`, set to `mask` here, +0x0C `color`. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/code_33808.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
