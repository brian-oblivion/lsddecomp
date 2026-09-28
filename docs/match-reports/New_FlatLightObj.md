# New_FlatLightObj -- MATCHED (24/24 words, round 81, alpha)

> Renamed from `func_8004291C` on 2026-09-25 (tools/rename.py). Address 0x8004291c.

**Unit:** `src/graphics/FlatLightObj.c` (was `src/code_3311c.c`, carved from `psyq_3311c` in FINISHING-PLAN revision 18). **Class:** method table `gFlatLightObjMethods` (class id 0x6, direct BasicClass child), read here as `FlatLightObj`: a 0x20-byte BasicClass holding a Psy-Q light id at +0x00C and a `GsF_LIGHT` (LIBGS.H) at +0x010. "FlatLight" is Sony's own name, not a guess: LIBGS.H's `GsF_LIGHT` (`int vx,vy,vz; unsigned char r,g,b;`) matches `FlatLightParams` field-for-field, and `GsSetFlatLight` is the only sink for it. Slots resolved with `python3 tools/classtable.py gFlatLightObjMethods`.

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

Declarations: `FlatLightObj`, `FlatLightObjMethods`, `FlatLightParams` and `FlatLightColor` are in `include/FlatLightObj.h`; `GsSetFlatLight` is Sony's prototype from `<libgs.h>` (round 101: the unit's local prototype, which took a `FlatLightParams *`, was replaced by Sony's, and the two calls now cast `&self->light` to `GsF_LIGHT *`; zero bytes changed).

## Notes

First spelling `if (self == NULL) return NULL; ctor; return self;` was 1 word LONG (branch offset 0xA vs 0x9: cc1 kept a separate return-NULL path). `if (self != NULL) { ctor; return self; } return NULL;` matches: retail puts `v0 = 0` in the `beqz` delay slot and falls into the common epilogue. 2 builds.

## Naming

`New_FlatLightObj`, tier A. Mechanics is its purpose: allocate, call the class's ctor slot through the table getter, return the object or NULL -- the same shape as `include/Pad.h`'s `New_Pad`. `New_Class` convention.

## History (moved from the unit banner of `src/code_3311c.c`, round 101)

The unit's banner and its first comment read, verbatim, until round 101:

```
code_3311c -- GAME code carved from psyq_3311c on 2026-09-25 (FINISHING-PLAN
revision 18). 0x3311C..0x3328C (vram 0x8004291C..0x80042A8C). It was counted
as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
into game code, a method-table entry beside game methods, or contiguity with
those, and no Sony fingerprint). What it holds: the whole of class
gFlatLightObjMethods -- allocator, ctor, its three own vtable slots
(+0x40/+0x44/+0x48), and the table getter.

All six functions matched and named in round 81 (alpha).
```

```
The class is declared in include/FlatLightObj.h (track 4, round 87): the
object, its table and the evidence for the name live there. The unit's
only outside caller of New_FlatLightObj is LightRig__LightRig
(src/Sprite.c, include/LightRig.h), with light ids 0, 1, 2.
```

## History (moved from src/FlatLightObj.c, comments pass)

The file's banner carried its edge evidence:

> Edges (track 8): the binary fixes both. The file sits between two placed
> Sony objects, libgs/gs_110 (GsSetAmbient) before and libgs/gs_107
> (GsSetFlatLight) after, so neither neighbour can be merged into it and its
> start and end are real file boundaries. Inside, tools/tuboundary.py finds
> no rodata tying or splitting the six functions (all five gaps "boundary
> possible"; the forced boundary it notes spans Sprite.c's jump table at
> 0x80011290 to DayTaskStageMap.c's at 0x8001140C and is met by Sony edges
> elsewhere, so it forces nothing here). Content decided the rest: one class,
> whole, is one file, named for it.
