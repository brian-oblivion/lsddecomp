# New_FlatLightObj -- MATCHED (24/24 words, round 81, alpha)

> Renamed from `func_8004291C` on 2026-09-25 (tools/rename.py). Address 0x8004291c.

**Unit:** `src/code_3311c.c` (carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `gFlatLightObjMethods` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. "FlatLight" is Sony's own name, not a guess: LIBGS.H's `GsF_LIGHT` (`int vx,vy,vz; unsigned char r,g,b;`) matches `FlatLightParams` field-for-field, and `GsSetFlatLight` is the only sink for it. Slots resolved with `python3 tools/classtable.py gFlatLightObjMethods`.

Verified: `./build-and-verify.sh` exit 0, `OK: build matches retail SLPS_015.56`; `tools/funcdiff.py` full match, 0 insertions / 0 deletions.

## What it does

the allocating constructor ("new"): `BMemPMgrAlloc(0x20)`, and if non-NULL call the class's own ctor slot +0x008 through `Get_vtable_FlatLightObj()` with the light id; returns the object or NULL.

## Source

```c
FlatLightObj *New_FlatLightObj(s32 lightId) {
    FlatLightObj *self;

    self = BMemPMgrAlloc(sizeof(FlatLightObj));
    if (self != NULL) {
        Get_vtable_FlatLightObj()->ctor(self, lightId);
        return self;
    }
    return NULL;
}
```

Declarations (`FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams`, `FlatLightColor`, the local `GsSetFlatLight` prototype) are at the top of `src/code_3311c.c`.

## Notes

First spelling `if (self == NULL) return NULL; ctor; return self;` was 1 word LONG (branch offset 0xA vs 0x9: cc1 kept a separate return-NULL path). `if (self != NULL) { ctor; return self; } return NULL;` matches: retail puts `v0 = 0` in the `beqz` delay slot and falls into the common epilogue. 2 builds.

## Naming

`New_FlatLightObj`, tier A. Mechanics is its purpose: allocate, call the class's ctor slot through the table getter, return the object or NULL -- the same shape as `include/Pad.h`'s `New_Pad`. `New_Class` convention.
