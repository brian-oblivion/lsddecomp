# GetFlatLightObjMethods -- MATCHED (4/4 words, round 81, alpha)

> Renamed from `Get_vtable_FlatLightObj` on 2026-09-28 (tools/rename.py). Address 0x80042a7c.

> Renamed from `func_80042A7C` on 2026-09-25 (tools/rename.py). Address 0x80042a7c.

**Unit:** `src/graphics/flat_light_obj.c` (was `src/code_3311c.c`, carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `gFlatLightObjMethods` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. "FlatLight" is Sony's own name, not a guess: LIBGS.H's `GsF_LIGHT` (`int vx,vy,vz; unsigned char r,g,b;`) matches `FlatLightParams` field-for-field, and `GsSetFlatLight` is the only sink for it. Slots resolved with `python3 tools/classtable.py gFlatLightObjMethods`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

the class's table getter (`return &gFlatLightObjMethods;`), the `Get_vtable_*` shape. Matched first build.

## Source

```c
FlatLightObjMethods *GetFlatLightObjMethods(void) {
    return &gFlatLightObjMethods;
}
```

Declarations: `FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams` and `FlatLightColor` are in `include/flat_light_obj.h`; `GsSetFlatLight` is Sony's prototype from `<libgs.h>` (round 101: the unit's local prototype, which took a `FlatLightParams *`, was replaced by Sony's, and the two calls now cast `&self->light` to `GsF_LIGHT *`; zero bytes changed).

## Notes

No iteration needed.

## Naming

`GetFlatLightObjMethods`, tier A. Pure getter, mechanics is its purpose by definition: returns `&gFlatLightObjMethods`. `Get_vtable_<Class>` convention (`GetPadMethods`, `GetBasicClassMethods`), preferred here over the sibling `Get<Class>Methods` spelling seen elsewhere because `FlatLightObj` is a direct BasicClass child like `Pad`.
