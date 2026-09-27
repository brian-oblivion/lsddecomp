# GetDefaultDataDirectory -- MATCHED (3/3 words), round 81

> Renamed from `func_80048CF0` on 2026-09-27 (tools/rename.py). Address 0x80048cf0.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; called from src/code_1677c.c.
- **What:** returns the small-data word `D_8008A960` (`%gp_rel` load; the gp_rel blocker is RESOLVED). Return type `s32` kept as `include/GameApplication.h` declares it, although retail stores a pointer (`&D_8008A958`) there.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
s32 GetDefaultDataDirectory(void) {
    return D_8008A960;
}
```

## Naming

Round 82 (bravo, naming pass). Left as `func_`, tier C. It is mechanically a
pure getter (which would ordinarily be tier A), but its target `D_8008A960`
has no established purpose: its only consumer is `SetDataDirectory` (code_171e0.c,
deliberately left unnamed by that unit -- "a getter/setter pair for gDataDirectory
... left unnamed"), which just stores it into a second, equally unnamed
small-data global at GameApplication construction time. There is nothing here to
name the getter FOR, so `Get<Something>` would be a guess, not evidence.
`D_8008A960` itself is left unrenamed for the same reason.
