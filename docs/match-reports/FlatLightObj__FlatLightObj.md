# FlatLightObj__FlatLightObj -- MATCHED (25/25 words, round 81, alpha)

> Renamed from `func_8004297C` on 2026-09-25 (tools/rename.py). Address 0x8004297c.

**Unit:** `src/graphics/FlatLightObj.c` (was `src/code_3311c.c`, carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `gFlatLightObjMethods` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. "FlatLight" is Sony's own name, not a guess: LIBGS.H's `GsF_LIGHT` (`int vx,vy,vz; unsigned char r,g,b;`) matches `FlatLightParams` field-for-field, and `GsSetFlatLight` is the only sink for it. Slots resolved with `python3 tools/classtable.py gFlatLightObjMethods`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

slot +0x008 (ctor): `Get_vtable_BasicClass()->ctor(self)`, install `Get_vtable_FlatLightObj()` as the method table, then call its own slot +0x040 with the light id. Matched first build.

## Source

```c
void FlatLightObj__FlatLightObj(FlatLightObj *self, s32 lightId) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_FlatLightObj();
    self->methods->setLightId(self, lightId);
}
```

Declarations: `FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams` and `FlatLightColor` are in `include/FlatLightObj.h`; `GsSetFlatLight` is Sony's prototype from `<libgs.h>` (round 101: the unit's local prototype, which took a `FlatLightParams *`, was replaced by Sony's, and the two calls now cast `&self->light` to `GsF_LIGHT *`; zero bytes changed).

## Notes

No iteration needed.

## Naming

`FlatLightObj__FlatLightObj`, tier A. The constructor called through the ctor vtable slot: chains the BasicClass base ctor, installs the class's own method table, then delegates to `setLightId`. `Class__Class` convention (`include/Pad.h`'s `Pad__Pad` is the same shape: base ctor, install own table, call a slot).

## History (moved from include/FlatLightObj.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * BasicClass * to FlatLightObj * and its s32 sources to the slots' types
 * (track 4, round 89).
```
