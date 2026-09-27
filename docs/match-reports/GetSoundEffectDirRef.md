# GetSoundEffectDirRef -- MATCHED (4/4 words), round 81

> Renamed from `func_80048DF8` on 2026-09-25 (tools/rename.py). Address 0x80048df8.

Round 81, runner echo. Unit `src/GameFiles.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot.
- **What:** returns the ADDRESS of sdata word `gSoundEffectDirPtr` (which holds a pointer to the string "SND\\SE" at D_8008A970). An address-take of an sdata symbol stays absolute lui/addiu, per the gp_rel rule.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
char **GetSoundEffectDirRef(void) {
    return &gSoundEffectDirPtr;
}
```

## Naming

- **Name:** `GetSoundEffectDirRef`
- **Tier:** B
- **Evidence:** returns &gSoundEffectDirPtr, the sdata pointer to the confirmed string "SND\\SE" (asm/data/7B12C.sdata.s); only used by GetSoundEffectDir.
