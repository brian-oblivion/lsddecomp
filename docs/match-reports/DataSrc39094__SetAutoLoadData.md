# DataSrc39094__SetAutoLoadData -- MATCHED (2/2 words), round 81

> Renamed from `func_80048CD8` on 2026-09-25 (tools/rename.py). Address 0x80048cd8.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x088 of D_80081940 (`tools/classtable.py D_80081940`).
- **What:** setter: stores the second argument at `self+0x38`. The object is a local view `DataSrc39094` (Class6D430 fields from the unified `include/Class6D430.h`, then `pad2C[0xC]`, `s32 autoLoadData`).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
/* The D_80081940 object: a Class6D430 data source with its own fields from
 * +0x2C (local view; only this unit's methods read them). */
typedef struct DataSrc39094 {
    CLASS6D430_FIELDS(Class6D430Methods);
    /* +0x02C */ u8 pad2C[0xC];
    /* +0x038 */ s32 autoLoadData;
} DataSrc39094;

extern u8 D_80081940[];   /* method table, 34 slots */
extern s32 D_8008A960;
extern s32 gForcedWeeklyGroup;
extern s32 gForcedVariant;
extern u8 gWeeklyGroupTable[];
extern u8 gRecordTable[];
extern char *gSoundEffectDirPtr;  /* -> "SND\\SE" */
extern const char sAsmkStreamPath[];
extern s16 gStreamTypeToGroupTable[];

/* slot +0x088 of D_80081940 */
void DataSrc39094__SetAutoLoadData(DataSrc39094 *self, s32 value) {
    self->autoLoadData = value;
}
```

## Naming

- **Name:** `DataSrc39094__SetAutoLoadData`
- **Tier:** A
- **Evidence:** slot +0x088; a one-line field setter (self->autoLoadData = value), a pure setter is tier A by the leaf-mechanics rule.
