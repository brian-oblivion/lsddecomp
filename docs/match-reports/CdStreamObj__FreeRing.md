# CdStreamObj__FreeRing -- MATCHED (exact length, 8/8 words), round 81

> Renamed from `func_80047870` on 2026-09-25 (tools/rename.py). Address 0x80047870.

Round 81, runner echo. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x070 of gCdStreamObjMethods.
- **What:** tail wrapper: `StFreeRing(base)` on the second argument (libcd, LIBCD.H `u_long StFreeRing(u_long *base)`). The byte match says nothing about the return type.
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/code_3770c.c`, declared from the Psy-Q prototypes.

## Naming

Tier A. `CdStreamObj__FreeRing` -- slot +0x070. Evidence: tail-wraps `StFreeRing(base)`; the libcd call names the operation.

## Source

```c
u32 CdStreamObj__FreeRing(CdStreamObj *self, u32 *base) {
    return StFreeRing(base);
}
```
