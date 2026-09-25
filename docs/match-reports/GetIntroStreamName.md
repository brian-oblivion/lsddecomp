# GetIntroStreamName -- MATCHED (7/7 words), round 81

> Renamed from `func_800490F4` on 2026-09-25 (tools/rename.py). Address 0x800490f4.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; called from src/code_1677c.c.
- **What:** `if (typeCodeOut != NULL) *typeCodeOut = 0x31; return sAsmkStreamPath;` -- signature copied from `include/Class6D3C8.h` (`const char *`; rodata `sAsmkStreamPath` referenced as a symbol, no literal).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
const char *GetIntroStreamName(s32 *typeCodeOut) {
    if (typeCodeOut != NULL) {
        *typeCodeOut = 0x31;
    }
    return sAsmkStreamPath;
}
```

## Naming

- **Name:** `GetIntroStreamName`
- **Tier:** A
- **Evidence:** its one caller, Class6D3C8__LoadIntroLogoSequence, uses the returned path directly as a StreamTask's stream name; matches exactly.
