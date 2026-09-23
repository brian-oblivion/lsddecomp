# GetObj865C8Methods

> Renamed from `func_8004A060` on 2026-09-23 (tools/rename.py). Address 0x8004a060.

**Unit:** class_39e08 · **Size:** 4 words (0x10 bytes) · **Status:** MATCHED (4/4 words)

## What it does

The `Get_vtable_X`-shaped accessor for the class implemented by most of this
unit's remaining functions: returns `&D_800865C8`, a 33-slot method table
(`tools/classtable.py 0x800865C8`). No parameters, matching the
`Get_vtable_DreamSys` / `GetClass6D3C8Methods` shape from
`docs/research/class-framework.md`.

## Derivation

```
lui   $v0, %hi(D_800865C8)
addiu $v0, $v0, %lo(D_800865C8)
jr    $ra
 nop
```

Written as:

```c
Class865C8Methods *GetObj865C8Methods(void) {
    return &D_800865C8;
}
```

`Class865C8Methods` and `D_800865C8`'s extern declaration are established in
`include/class_39e08.h`, added this round. The table's DATA itself is still
raw (`asm/data/76DC8.data.s`) -- out of this round's scope; only the pointer
type needed for callers is declared.

## Proposed learning

None beyond what's already documented.

## Naming

`GetObj865C8Methods` -- tier A. Plain accessor, `return &D_800865C8;` -- matches the established `GetXMethods`/`Get_vtable_X` accessor convention used site-wide for vtable getters (e.g. `GetClass86668Methods`, `Get_vtable_IntermediateBase`).
