# GetVariantSpriteMethods -- MATCHED (4/4)

> Renamed from `GetClass879C4Methods` on 2026-09-26 (tools/rename.py). Address 0x80057f58.

> Renamed from `func_80057F58` on 2026-09-26 (tools/rename.py). Address 0x80057f58.

Unit: `src/world/ObjMStyleActor.c`. Class: `gVariantSpriteMethods` (49 slots) -- plain
no-argument getter, `return &gVariantSpriteMethods;`. Not itself a vtable slot.

## Body

```c
typedef struct D_800879C4Table D_800879C4Table;
extern D_800879C4Table gVariantSpriteMethods;

D_800879C4Table *GetVariantSpriteMethods(void) {
    return &gVariantSpriteMethods;
}
```

This same table was already established with fields
(`D_800879C4Methods`/`D_800879C4Obj`, ctor at `+0x008`, `slot0x40`) in the
neighbouring `ObjMStyleActor` unit earlier this round, which owns the
ctor (`VariantSprite__VariantSprite`) and this table's own leaf slots
(`VariantSprite__Update/40/48/50`). This function needs none of those fields --
only the address -- so it's declared here as an opaque incomplete type,
per the multiple-independent-local-views convention (this unit does not
include `ObjMStyleActor`'s header).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GetVariantSpriteMethods   # 4/4
```

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `func_80057F58` (tools/rename.py), tier A: a plain getter for
`&gVariantSpriteMethods` (formerly `D_800879C4`), named like
`GetSpriteMethods` / `GetActorMethods`. It now returns `VariantSpriteMethods *`
(`include/VariantSprite.h`); ObjMStyleActor.c's opaque `D_800879C4Table`
typedef and extern are gone. Its callers are the class's own allocator and
ctor (ObjMStyleActor.c). Byte-identical.

## Track 6 (2026-09-26, round 93, bravo)

The class `Class879C4` is now `VariantSprite` (`include/VariantSprite.h`,
`python3 tools/renametype.py Class879C4 VariantSprite`), tier B: the
mechanics are certain and are the whole of what the class adds to Sprite --
`variant` (0 or 1) picks the texture cell the Sprite ctor binds
(`sVariantSpriteCells`) and the CLUT row the reset slot sets
(`sVariantSpriteClutX/Y`). What the sprites are in the game is not
established (their only builder is StyleEffect, kinds 2 and 3, and every
path passes variant 0), which is why it is not tier A. The table, getter,
allocator, methods and the three data tables followed the class name.
The same tool run rewrote `Class879C4` tokens inside this report's older
history prose (the known renametype behaviour pending an operator
decision); those lines were left as the tool wrote them.
