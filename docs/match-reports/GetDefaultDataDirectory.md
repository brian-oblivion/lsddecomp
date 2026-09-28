# GetDefaultDataDirectory -- MATCHED (3/3 words), round 81

> Renamed from `func_80048CF0` on 2026-09-27 (tools/rename.py). Address 0x80048cf0.

Round 81, runner echo. Unit `src/cd/game_files.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; called from src/app/game_shell.c.
- **What:** returns the small-data word `sDefaultDataDirectory` (`%gp_rel` load; the gp_rel blocker is RESOLVED). Return type `s32` kept as `include/game_application.h` declares it, although retail stores a pointer (`&D_8008A958`) there.
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
s32 GetDefaultDataDirectory(void) {
    return sDefaultDataDirectory;
}
```

## Naming

Round 82 (bravo, naming pass). Left as `func_`, tier C. It is mechanically a
pure getter (which would ordinarily be tier A), but its target `sDefaultDataDirectory`
has no established purpose: its only consumer is `SetDataDirectory` (game_shell.c,
deliberately left unnamed by that unit -- "a getter/setter pair for sDataDirectory
... left unnamed"), which just stores it into a second, equally unnamed
small-data global at GameApplication construction time. There is nothing here to
name the getter FOR, so `Get<Something>` would be a guess, not evidence.
`sDefaultDataDirectory` itself is left unrenamed for the same reason.

## Naming (round 99, head, track 7)

`func_80048CF0` -> `GetDefaultDataDirectory`, tier A (a getter): it returns
`sDefaultDataDirectory` (was `D_8008A960`), an sdata word initialised to the
address of the sdata string `"CDI\\"` (`D_8008A958`). Its one caller,
GameApplication's ctor, passes it straight to `SetDataDirectory`
(game_shell.c), which installs the directory BuildCdFilePath and
CdStream__Open put between the root `\` and a file name. Retyped `char *`
here, in game_shell.c's externs, and in cd_stream.c's `GetDataDirectory`
prototype; zero bytes. Proposed by charlie's game_shell pass.
