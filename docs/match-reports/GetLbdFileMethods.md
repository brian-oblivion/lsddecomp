# GetLbdFileMethods -- MATCHED (4/4 words), round 81

> Renamed from `GetClass81940Methods` on 2026-09-26 (tools/rename.py). Address 0x80048ce0.

> Renamed from `GetDataSrc39094Methods` on 2026-09-26 (tools/rename.py). Address 0x80048ce0.

> Renamed from `func_80048CE0` on 2026-09-25 (tools/rename.py). Address 0x80048ce0.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; the table getter of gLbdFileMethods.
- **What:** returns the method table address `gLbdFileMethods` (lui/addiu).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetLbdFileMethods(void) {
    return gLbdFileMethods;
}
```

## Naming

- **Name:** `GetLbdFileMethods`
- **Tier:** A
- **Evidence:** table getter: returns gLbdFileMethods, matches the project's 'table getter' convention exactly.

## Track 4 (2026-09-26, round 87)

Renamed `GetDataSrc39094Methods` -> `GetLbdFileMethods` with `rename.py`: the table getter, returns &gLbdFileMethods. It is gFileResourceMethods's gDataSourceClientGetters entry at +0x0A0, so SetActiveDataSource rebinds this class's interface slots. The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.
