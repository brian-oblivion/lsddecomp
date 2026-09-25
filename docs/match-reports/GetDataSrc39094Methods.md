# GetDataSrc39094Methods -- MATCHED (4/4 words), round 81

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
void *GetDataSrc39094Methods(void) {
    return D_80081940;
}
```

## Naming

- **Name:** `GetDataSrc39094Methods`
- **Tier:** A
- **Evidence:** table getter: returns D_80081940, matches the project's 'table getter' convention exactly.
