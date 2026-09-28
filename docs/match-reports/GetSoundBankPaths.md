# GetSoundBankPaths -- MATCHED (4/4 words), round 81

> Renamed from `GetWeeklyGroupTable` on 2026-09-27 (tools/rename.py). Address 0x80048d64.

> Renamed from `func_80048D64` on 2026-09-25 (tools/rename.py). Address 0x80048d64.

Round 81, runner echo. Unit `src/cd/game_files.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** table getter: returns `sSoundBankPaths`.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetSoundBankPaths(void) {
    return sSoundBankPaths;
}
```

## Naming

- **Name:** `GetSoundBankPaths`
- **Tier:** A
- **Evidence:** getter: returns sSoundBankPaths (renamed from gWeeklyGroupTable), whose seven words point to "SND\AMBIENT", "SND\CARTOON", "SND\ELECTRO", "SND\ETHNOVA", "SND\HUMAN", "SND\LOVELY", "SND\STANDERD" (retail data at 0x800819CC); the same seven bases as the VH/VB pairs at sRecordTable[0..13]. Nothing in the code or data is weekly.

## Naming history

- Round 100 (bravo, polish): renamed from `GetWeeklyGroupTable` with tools/rename.py, on the record paths and callers above; previous tier B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / GameApplication.h, which are themselves hypotheses, so the name is consistent but not established).
