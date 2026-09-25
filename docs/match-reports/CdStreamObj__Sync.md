# CdStreamObj__Sync -- MATCHED (exact length, 10/10 words), round 81

> Renamed from `func_800478D0` on 2026-09-25 (tools/rename.py). Address 0x800478d0.

Round 81, runner echo. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot of gCdStreamObjMethods.
- **What:** `CdSync(mode, &self->cdResult)`: the object carries a CdSync result buffer at +0x24. Return type unconstrained by the bytes.
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/code_3770c.c`, declared from the Psy-Q prototypes.

## Naming

Tier A. `CdStreamObj__Sync` -- free function (not a slot), a thin wrapper: `CdSync(mode, &self->cdResult)`. Evidence: the libcd call and the result buffer it writes into name the operation directly.

## Source

```c
int CdStreamObj__Sync(CdStreamObj *self, int mode) {
    return CdSync(mode, self->cdResult);
}
```
