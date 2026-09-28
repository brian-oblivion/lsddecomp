# SetPickOverrides -- MATCHED (8/8 words), round 81

> Renamed from `func_80048D28` on 2026-09-25 (tools/rename.py). Address 0x80048d28.

Round 81, runner echo. Unit `src/cd/GameFiles.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** two independent guarded stores: `if (a >= 0) sForcedSoundBank = a; if (b >= 0) gForcedStageBgm = b;` (the `bltz` pair; both globals are sdata, stored via `%gp_rel`).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void SetPickOverrides(s32 a, s32 b) {
    if (a >= 0) {
        sForcedSoundBank = a;
    }
    if (b >= 0) {
        gForcedStageBgm = b;
    }
}
```

## Naming

- **Name:** `SetPickOverrides`
- **Tier:** A
- **Evidence:** leaf: two independent guarded stores into sForcedSoundBank/gForcedStageBgm; mechanics are the whole purpose.
