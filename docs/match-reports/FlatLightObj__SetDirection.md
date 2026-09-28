# FlatLightObj__SetDirection -- MATCHED (20/20 words, round 81, alpha)

> Renamed from `func_80042A2C` on 2026-09-25 (tools/rename.py). Address 0x80042a2c.

**Unit:** `src/graphics/flat_light_obj.c` (was `src/code_3311c.c`, carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `gFlatLightObjMethods` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. "FlatLight" is Sony's own name, not a guess: LIBGS.H's `GsF_LIGHT` (`int vx,vy,vz; unsigned char r,g,b;`) matches `FlatLightParams` field-for-field, and `GsSetFlatLight` is the only sink for it. Slots resolved with `python3 tools/classtable.py gFlatLightObjMethods`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

slot +0x048 (`setDirection`): if `update`, widen three `s16` into the light's vx,vy,vz (+0x010..+0x018); then `GsSetFlatLight(lightId, &light)`. Per-field `self->light.vx = dir[0];` matched on first build (scored full once the sibling above stopped drifting it).

## Source

```c
void FlatLightObj__SetDirection(FlatLightObj *self, s32 update, s16 *dir) {
    if (update) {
        self->light.vx = dir[0];
        self->light.vy = dir[1];
        self->light.vz = dir[2];
    }
    GsSetFlatLight(self->lightId, (GsF_LIGHT *)&self->light);
}
```

Declarations: `FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams` and `FlatLightColor` are in `include/flat_light_obj.h`; `GsSetFlatLight` is Sony's prototype from `<libgs.h>` (round 101: the unit's local prototype, which took a `FlatLightParams *`, was replaced by Sony's, and the two calls now cast `&self->light` to `GsF_LIGHT *`; zero bytes changed).

## Notes

No iteration needed.

## Naming

`FlatLightObj__SetDirection`, tier A for the mechanics (conditionally overwrite the light's vx/vy/vz from a `s16[3]`, then push via `GsSetFlatLight`) -- `Class__Method` convention, matches the struct's own `setDirection` slot name. "Direction" itself (vs. just "vector") is tier B: GsF_LIGHT's `vx,vy,vz` are the flat-light vector Psy-Q's own doc calls the light direction, but that reading rests on the SDK header/manual, not on anything visible in this function's body.
