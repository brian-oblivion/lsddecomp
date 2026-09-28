# FlatLightObj__SetLightId -- MATCHED (2/2 words, round 81, alpha)

> Renamed from `func_800429E0` on 2026-09-25 (tools/rename.py). Address 0x800429e0.

**Unit:** `src/code_3311c.c` (carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `gFlatLightObjMethods` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. "FlatLight" is Sony's own name, not a guess: LIBGS.H's `GsF_LIGHT` (`int vx,vy,vz; unsigned char r,g,b;`) matches `FlatLightParams` field-for-field, and `GsSetFlatLight` is the only sink for it. Slots resolved with `python3 tools/classtable.py gFlatLightObjMethods`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

slot +0x040 (`setLightId`): stores the light id at +0x00C. One-line setter, matched first build.

## Source

```c
void FlatLightObj__SetLightId(FlatLightObj *self, s32 lightId) {
    self->lightId = lightId;
}
```

Declarations: `FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams` and `FlatLightColor` are in `include/FlatLightObj.h`; `GsSetFlatLight` is Sony's prototype from `<libgs.h>` (round 101: the unit's local prototype, which took a `FlatLightParams *`, was replaced by Sony's, and the two calls now cast `&self->light` to `GsF_LIGHT *`; zero bytes changed).

## Notes

No iteration needed.

## Naming

`FlatLightObj__SetLightId`, tier A. Pure leaf, mechanics is its purpose by definition (a setter): stores the argument at +0x00C. `Class__Method` convention, matches the struct's own `setLightId` slot name.
