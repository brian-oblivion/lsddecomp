# GetClass81940Methods -- MATCHED (4/4 words), round 81

> Renamed from `GetDataSrc39094Methods` on 2026-09-26 (tools/rename.py). Address 0x80048ce0.

> Renamed from `func_80048CE0` on 2026-09-25 (tools/rename.py). Address 0x80048ce0.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; the table getter of D_80081940.
- **What:** returns the method table address `D_80081940` (lui/addiu).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetClass81940Methods(void) {
    return D_80081940;
}
```

## Naming

- **Name:** `GetClass81940Methods`
- **Tier:** A
- **Evidence:** table getter: returns D_80081940, matches the project's 'table getter' convention exactly.

## Track 4 (2026-09-26, round 87)

Renamed `GetDataSrc39094Methods` -> `GetClass81940Methods` with `rename.py`: the table getter, returns &D_80081940. It is D_8006D430's gDataSourceClientGetters entry at +0x0A0, so SetActiveDataSource rebinds this class's interface slots. The class (method table D_80081940, id 0x903, a Class6D430 subclass) was named `Class81940` for its table address, as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every Class6D430 subclass is. The unified definition is `include/Class81940.h`.
