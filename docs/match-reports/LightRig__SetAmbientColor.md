# LightRig__SetAmbientColor -- MATCHED (42/42 words), round 82

> Renamed from `D8006EFAC__SetAmbientColor` on 2026-09-26 (tools/rename.py). Address 0x8004283c.

> Renamed from `func_8004283C` on 2026-09-25 (tools/rename.py). Address 0x8004283c.

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** gLightRigMethods slot +0x0BC (`tools/classtable.py`).
- **What:** Sets the 3-byte ambient colour at +0x050 from `*rgb`; when the third argument is non-zero it first swaps (old colour written back into `*rgb` via a stack temp). Then `GsSetAmbient(r << 4, g << 4, b << 4)` read back unsigned (`lbu`).
- **Result:** byte-exact; 42/42 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** the unit-local `D_8006EFACObj` view; `ambient` is a `SpriteRgb` (the all-s8 3-byte struct), so all three copies are lb/lb/lb + sb/sb/sb, and the final reads are `(u8)` casts.

## Source

```c
/* gLightRigMethods slot +0x0BC: set the ambient colour (swapping the old one out
 * into *rgb when asked) and hand it to GsSetAmbient. */
void LightRig__SetAmbientColor(D_8006EFACObj *self, SpriteRgb *rgb, s32 swap) {
    SpriteRgb old;

    if (swap) {
        old = self->ambient;
        self->ambient = *rgb;
        *rgb = old;
    } else {
        self->ambient = *rgb;
    }
    GsSetAmbient((u8)self->ambient.r << 4, (u8)self->ambient.g << 4, (u8)self->ambient.b << 4);
}
```

## Naming

- `D8006EFAC__SetAmbientColor` -- tier A. Slot +0x0BC: sets the ambient colour (optionally swapping the previous one out to the caller) and forwards it to GsSetAmbient. Round-82 broadcast: "GsSetAmbient" evidence.
