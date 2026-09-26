# GetVariantSpriteMethods -- MATCHED (4/4)

> Renamed from `GetClass879C4Methods` on 2026-09-26 (tools/rename.py). Address 0x80057f58.

> Renamed from `func_80057F58` on 2026-09-26 (tools/rename.py). Address 0x80057f58.

Unit: `src/class_3bb8c_t.c`. Class: `gVariantSpriteMethods` (49 slots) -- plain
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
neighbouring `class_3bb8c_p` unit earlier this round, which owns the
ctor (`VariantSprite__VariantSprite`) and this table's own leaf slots
(`VariantSprite__Update/40/48/50`). This function needs none of those fields --
only the address -- so it's declared here as an opaque incomplete type,
per the multiple-independent-local-views convention (this unit does not
include `class_3bb8c_p`'s header).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GetVariantSpriteMethods   # 4/4
```

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `func_80057F58` (tools/rename.py), tier A: a plain getter for
`&gVariantSpriteMethods` (formerly `D_800879C4`), named like
`GetSpriteMethods` / `GetActorMethods`. It now returns `VariantSpriteMethods *`
(`include/VariantSprite.h`); class_3bb8c_t.c's opaque `D_800879C4Table`
typedef and extern are gone. Its callers are the class's own allocator and
ctor (class_3bb8c_p.c). Byte-identical.
