# DataSrc39094__SetAutoLoadData -- MATCHED (2/2 words), round 81

> Renamed from `func_80048CD8` on 2026-09-25 (tools/rename.py). Address 0x80048cd8.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** slot +0x088 of D_80081940 (`tools/classtable.py D_80081940`).
- **What:** setter: stores the second argument at `self+0x38`. The object is a local view `D_80081940Obj` (Class6D430 fields from the unified `include/Class6D430.h`, then `pad2C[0xC]`, `s32 unk38`).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
/* The D_80081940 object: a Class6D430 data source with its own fields from
 * +0x2C (local view; only this unit's methods read them). */
typedef struct D_80081940Obj {
    CLASS6D430_FIELDS(Class6D430Methods);
    /* +0x02C */ u8 pad2C[0xC];
    /* +0x038 */ s32 unk38;
} D_80081940Obj;

extern u8 D_80081940[];   /* method table, 34 slots */
extern s32 D_8008A960;
extern s32 D_8008A964;
extern s32 D_8008A968;
extern u8 D_800819CC[];
extern u8 D_80081A04[];
extern char *D_8008A96C;  /* -> "SND\\SE" */
extern const char D_800113DC[];
extern s16 D_80086170[];

/* slot +0x088 of D_80081940 */
void DataSrc39094__SetAutoLoadData(D_80081940Obj *self, s32 value) {
    self->unk38 = value;
}
```
