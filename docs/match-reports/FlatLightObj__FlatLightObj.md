# FlatLightObj__FlatLightObj -- MATCHED (25/25 words, round 81, alpha)

> Renamed from `func_8004297C` on 2026-09-25 (tools/rename.py). Address 0x8004297c.

**Unit:** `src/code_3311c.c` (carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `D_8006F06C` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. The name is a hypothesis from the `GsSetFlatLight` calls, not evidence. Slots resolved with `python3 tools/classtable.py D_8006F06C`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

slot +0x008 (ctor): `Get_vtable_BasicClass()->ctor(self)`, install `func_80042A7C()` as the method table, then call its own slot +0x040 with the light id. Matched first build.

## Source

```c
void FlatLightObj__FlatLightObj(FlatLightObj *self, s32 lightId) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = func_80042A7C();
    self->methods->setLightId(self, lightId);
}
```

Declarations (`FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams`, `FlatLightColor`, the local `GsSetFlatLight` prototype) are at the top of `src/code_3311c.c`.

## Notes

No iteration needed.
