# GetRecordTable -- MATCHED (7/7 words), round 81

> Renamed from `func_80048D48` on 2026-09-25 (tools/rename.py). Address 0x80048d48.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; called from GetRecordGroup and src/class_39e08.c.
- **What:** `if (out != NULL) *out = 0x230; return gRecordTable;` -- the `ori v0,0x230` in the beqz delay slot is the compiler's own scheduling.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetRecordTable(s32 *out) {
    if (out != NULL) {
        *out = 0x230;
    }
    return gRecordTable;
}
```
