# GetClass865C8Methods

> Renamed from `GetObj865C8Methods` on 2026-09-26 (tools/rename.py). Address 0x8004a060.

> Renamed from `func_8004A060` on 2026-09-23 (tools/rename.py). Address 0x8004a060.

**Unit:** class_39e08 · **Size:** 4 words (0x10 bytes) · **Status:** MATCHED (4/4 words)

## What it does

The `Get_vtable_X`-shaped accessor for the class implemented by most of this
unit's remaining functions: returns `&gClass865C8Methods`, a 33-slot method table
(`tools/classtable.py 0x800865C8`). No parameters, matching the
`Get_vtable_DreamSys` / `GetClass6D3C8Methods` shape from
`docs/research/class-framework.md`.

## Derivation

```
lui   $v0, %hi(gClass865C8Methods)
addiu $v0, $v0, %lo(gClass865C8Methods)
jr    $ra
 nop
```

Written as:

```c
Class865C8Methods *GetClass865C8Methods(void) {
    return &gClass865C8Methods;
}
```

`Class865C8Methods` and `gClass865C8Methods`'s extern declaration are established in
`include/class_39e08.h`, added this round. The table's DATA itself is still
raw (`asm/data/76DC8.data.s`) -- out of this round's scope; only the pointer
type needed for callers is declared.

## Proposed learning

None beyond what's already documented.

## Naming

`GetClass865C8Methods` -- tier A. Plain accessor, `return &gClass865C8Methods;` -- matches the established `GetXMethods`/`Get_vtable_X` accessor convention used site-wide for vtable getters (e.g. `GetClass86668Methods`, `Get_vtable_IntermediateBase`).

## Track 4 (2026-09-26, round 88, Class865C8)

The class (table D_800865C8, id 0x1F230, Class86668's subclass) is unified as Class865C8 in include/Class865C8.h; the Obj865C8/Class865C8Methods views in class_39e08.h are gone. Renamed from GetObj865C8Methods with the class (returns &gClass865C8Methods, was D_800865C8). Tier A.
