# GetClass879C4Methods -- MATCHED (4/4)

> Renamed from `func_80057F58` on 2026-09-26 (tools/rename.py). Address 0x80057f58.

Unit: `src/class_3bb8c_t.c`. Class: `gClass879C4Methods` (49 slots) -- plain
no-argument getter, `return &gClass879C4Methods;`. Not itself a vtable slot.

## Body

```c
typedef struct D_800879C4Table D_800879C4Table;
extern D_800879C4Table gClass879C4Methods;

D_800879C4Table *GetClass879C4Methods(void) {
    return &gClass879C4Methods;
}
```

This same table was already established with fields
(`D_800879C4Methods`/`D_800879C4Obj`, ctor at `+0x008`, `slot0x40`) in the
neighbouring `class_3bb8c_p` unit earlier this round, which owns the
ctor (`Class879C4__Class879C4`) and this table's own leaf slots
(`Class879C4__Update/40/48/50`). This function needs none of those fields --
only the address -- so it's declared here as an opaque incomplete type,
per the multiple-independent-local-views convention (this unit does not
include `class_3bb8c_p`'s header).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GetClass879C4Methods   # 4/4
```

## Track 4 (2026-09-26, round 87, alpha)

Renamed from `func_80057F58` (tools/rename.py), tier A: a plain getter for
`&gClass879C4Methods` (formerly `D_800879C4`), named like
`GetSpriteMethods` / `GetActorMethods`. It now returns `Class879C4Methods *`
(`include/Class879C4.h`); class_3bb8c_t.c's opaque `D_800879C4Table`
typedef and extern are gone. Its callers are the class's own allocator and
ctor (class_3bb8c_p.c). Byte-identical.
