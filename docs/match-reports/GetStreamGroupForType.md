# GetStreamGroupForType -- MATCHED (7/7 words), round 81

> Renamed from `func_800493C8` on 2026-09-25 (tools/rename.py). Address 0x800493c8.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** `return D_80086170[index];` over `extern s16 D_80086170[]` (sll/addu/lh through `$at`, the resolved addiu_at construct).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
s32 GetStreamGroupForType(s32 index) {
    return D_80086170[index];
}
```
