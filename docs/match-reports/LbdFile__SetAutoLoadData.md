# LbdFile__SetAutoLoadData -- MATCHED (2/2 words), round 81

> Renamed from `Class81940__SetAutoLoadData` on 2026-09-26 (tools/rename.py). Address 0x80048cd8.

> Renamed from `DataSrc39094__SetAutoLoadData` on 2026-09-26 (tools/rename.py). Address 0x80048cd8.

> Renamed from `func_80048CD8` on 2026-09-25 (tools/rename.py). Address 0x80048cd8.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x088 of gLbdFileMethods (`tools/classtable.py gLbdFileMethods`).
- **What:** setter: stores the second argument at `self+0x38`. The object is a local view `DataSrc39094` (FileResource fields from the unified `include/FileResource.h`, then `pad2C[0xC]`, `s32 autoLoadData`).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
/* The gLbdFileMethods object: a FileResource data source with its own fields from
 * +0x2C (local view; only this unit's methods read them). */
typedef struct DataSrc39094 {
    FILERESOURCE_FIELDS(FileResourceMethods);
    /* +0x02C */ u8 pad2C[0xC];
    /* +0x038 */ s32 autoLoadData;
} DataSrc39094;

extern u8 gLbdFileMethods[];   /* method table, 34 slots */
extern s32 D_8008A960;
extern s32 gForcedWeeklyGroup;
extern s32 gForcedVariant;
extern u8 gWeeklyGroupTable[];
extern u8 gRecordTable[];
extern char *gSoundEffectDirPtr;  /* -> "SND\\SE" */
extern const char sAsmkStreamPath[];
extern s16 gStreamTypeToGroupTable[];

/* slot +0x088 of gLbdFileMethods */
void LbdFile__SetAutoLoadData(DataSrc39094 *self, s32 value) {
    self->autoLoadData = value;
}
```

## Naming

- **Name:** `LbdFile__SetAutoLoadData`
- **Tier:** A
- **Evidence:** slot +0x088; a one-line field setter (self->autoLoadData = value), a pure setter is tier A by the leaf-mechanics rule.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__SetAutoLoadData` -> `LbdFile__SetAutoLoadData` with `rename.py` (class rename only; +0x088, the last slot). Its one caller is Class866E8__Class866E8, forwarding its own arg2. The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.
