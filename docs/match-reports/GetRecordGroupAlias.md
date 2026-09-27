# GetRecordGroupAlias -- MATCHED (8/8 words), round 81

> Renamed from `func_80048E80` on 2026-09-25 (tools/rename.py). Address 0x80048e80.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** tail wrapper: `return GetRecordGroup(index);` (GetRecordGroup returns `GetRecordTable(NULL) + gStageFirstRecord[index] * 0x1C`; its prototype is declared locally as `void *GetRecordGroup(s32 index)`). The byte match says nothing about the return type.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetRecordGroup(s32 index);

void *GetRecordGroupAlias(s32 index) {
    return GetRecordGroup(index);
}
```

## Naming

- **Name:** `GetRecordGroupAlias`
- **Tier:** A
- **Evidence:** pure one-line tail wrapper of GetRecordGroup; no distinguishing caller found (grep across src/ and include/), so named for exactly what it does and nothing more.
