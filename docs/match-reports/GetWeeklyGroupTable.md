# GetWeeklyGroupTable -- MATCHED (4/4 words), round 81

> Renamed from `func_80048D64` on 2026-09-25 (tools/rename.py). Address 0x80048d64.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** table getter: returns `gWeeklyGroupTable`.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetWeeklyGroupTable(void) {
    return gWeeklyGroupTable;
}
```

## Naming

- **Name:** `GetWeeklyGroupTable`
- **Tier:** A
- **Evidence:** table getter: returns gWeeklyGroupTable.
