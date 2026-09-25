# Get_vtable_CdStreamObj -- MATCHED (exact length, 4/4 words), round 81

> Renamed from `func_80047900` on 2026-09-25 (tools/rename.py). Address 0x80047900.

Round 81, runner echo. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** returns the address of the class method table gCdStreamObjMethods (game code per revision 18, not the SDK adjacency lead).
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/code_3770c.c`, declared from the Psy-Q prototypes.

## Naming

Tier A. `Get_vtable_CdStreamObj` -- returns `&gCdStreamObjMethods`. Evidence: matches the established `Get_vtable_<Class>` convention used by `Get_vtable_BasicClass`, `Get_vtable_Pad`, `Get_vtable_Entity`, etc.

## Source

```c
CdStreamObjMethods *Get_vtable_CdStreamObj(void) {
    return &gCdStreamObjMethods;
}
```
