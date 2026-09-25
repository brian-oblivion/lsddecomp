# FlatLightObj__SetColor -- MATCHED (17/17 words, round 81, alpha)

> Renamed from `func_800429E8` on 2026-09-25 (tools/rename.py). Address 0x800429e8.

**Unit:** `src/code_3311c.c` (carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `D_8006F06C` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. The name is a hypothesis from the `GsSetFlatLight` calls, not evidence. Slots resolved with `python3 tools/classtable.py D_8006F06C`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

slot +0x044 (`setColor`): if `update`, copy a 3-byte colour into the light's r,g,b (+0x01C..+0x01E); then `GsSetFlatLight(lightId, &light)`.

## Source

```c
void FlatLightObj__SetColor(FlatLightObj *self, s32 update, FlatLightColor *rgb) {
    if (update) {
        self->light.rgb = *rgb;
    }
    GsSetFlatLight(self->lightId, &self->light);
}
```

Declarations (`FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams`, `FlatLightColor`, the local `GsSetFlatLight` prototype) are at the top of `src/code_3311c.c`.

## Notes

First attempt copied the three bytes field by field (`self->light.r = rgb[0]` ...): 3 words LONG, `lbu`/`nop`/`sb` interleaved per byte. Retail issues three `lb` then three `sb`. That is GCC's block move for a struct assignment: an all-`s8` 3-byte struct `FlatLightColor` and `self->light.rgb = *rgb;` matched on the next build (2 builds total). Members are typed `s8` to match retail's signed `lb`; a `u8` member struct was not tried. The param is typed `FlatLightColor *`.

### Proposed learning

**All loads before any store, one byte each, = whole-struct assignment of a small alignment-1 struct.** Discriminator: retail `lb;lb;lb;sb;sb;sb` with no load-delay nops, versus per-field copy which compiles to `lbu;nop;sb` x3 (3 words longer). Sibling of the existing all-`s8`/`s16` alignment-2 `lwl`/`lwr` idiom: at alignment 1 and size 3 GCC 2.6.3 does not use `lwl`/`lwr` but an unrolled byte block move. (round 81, alpha)
