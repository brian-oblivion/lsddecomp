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
- **Tier:** B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / Class6D3C8.h, which are themselves hypotheses, so the name is consistent but not established)
- **Evidence:** table getter: returns gWeeklyGroupTable.
