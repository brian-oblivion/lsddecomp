# GetStageTextureRecords -- MATCHED (8/8 words), round 81

> Renamed from `GetRecordGroupAlias` on 2026-09-27 (tools/rename.py). Address 0x80048e80.

> Renamed from `func_80048E80` on 2026-09-25 (tools/rename.py). Address 0x80048e80.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** tail wrapper: `return GetStageRecords(index);` (GetStageRecords returns `GetRecordTable(NULL) + gStageFirstRecord[index] * 0x1C`; its prototype is declared locally as `void *GetStageRecords(s32 index)`). The byte match says nothing about the return type.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetStageRecords(s32 index);

void *GetStageTextureRecords(s32 index) {
    return GetStageRecords(index);
}
```

## Naming

- **Name:** `GetStageTextureRecords`
- **Tier:** A
- **Evidence:** tail call of GetStageRecords: the group's first four records are TEXA..TEXD.TIX (the record paths are retail's gRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100); PickStageTexture indexes it and ObjM hands the pick to New_TimBlockSrc (class_3bb8c_l.c).

## Naming history

- Round 100 (bravo, polish): renamed from `GetRecordGroupAlias` with tools/rename.py, on the record paths and callers above; previous tier A.
