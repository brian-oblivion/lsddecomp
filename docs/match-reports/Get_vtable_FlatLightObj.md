# Get_vtable_FlatLightObj -- MATCHED (4/4 words, round 81, alpha)

> Renamed from `func_80042A7C` on 2026-09-25 (tools/rename.py). Address 0x80042a7c.

**Unit:** `src/code_3311c.c` (carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `D_8006F06C` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. The name is a hypothesis from the `GsSetFlatLight` calls, not evidence. Slots resolved with `python3 tools/classtable.py D_8006F06C`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

the class's table getter (`return &D_8006F06C;`), the `Get_vtable_*` shape. Matched first build.

## Source

```c
FlatLightObjMethods *Get_vtable_FlatLightObj(void) {
    return &D_8006F06C;
}
```

Declarations (`FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams`, `FlatLightColor`, the local `GsSetFlatLight` prototype) are at the top of `src/code_3311c.c`.

## Notes

No iteration needed.
