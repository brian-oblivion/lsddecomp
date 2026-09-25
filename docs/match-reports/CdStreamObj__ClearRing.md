# CdStreamObj__ClearRing -- MATCHED (exact length, 8/8 words), round 81

> Renamed from `func_800478B0` on 2026-09-25 (tools/rename.py). Address 0x800478b0.

Round 81, runner echo. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x078 of gCdStreamObjMethods.
- **What:** tail wrapper: `StClearRing()` (libcd). Return type unconstrained by the bytes.
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/code_3770c.c`, declared from the Psy-Q prototypes.

## Naming

Kept `func_`. The class gCdStreamObjMethods is a CD streaming (StSetRing/CdSync) object; a naming pass can call it e.g. `CdStreamObj` (tier B: from the libcd calls only).

## Source

```c
void CdStreamObj__ClearRing(CdStreamObj *self) {
    StClearRing();
}
```
