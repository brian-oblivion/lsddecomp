# CdStream__NoOpSlot5C -- MATCHED (exact length, 2/2 words), round 81

> Renamed from `CdStreamObj__func_800475C8` on 2026-09-26 (tools/rename.py). Address 0x800475c8.

> Renamed from `func_800475C8` on 2026-09-25 (tools/rename.py). Address 0x800475c8.

Round 81, runner echo. Unit `src/code_3770c.c` (carved from psyq_3770c in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x05C of gCdStreamMethods.
- **What:** an empty method (`jr $ra; nop`).
- **Levers:** none needed.
- **Context:** local view `CdStreamObj` (BASICCLASS_FIELDS + `cdResult[8]` at +0x24, `s32 unk2C` at +0x2C, `u32 *ring` at +0x50) and the libcd/libspu externs are in `src/code_3770c.c`, declared from the Psy-Q prototypes.

## Naming

Tier C. Kept the tier-C `Class__func_xxxxx` form (class known, purpose not): slot +0x05C, an empty override (`jr $ra; nop`) with no evidence of what it would do if implemented.

## Source

```c
void CdStream__NoOpSlot5C(CdStreamObj *self) {
}
```
