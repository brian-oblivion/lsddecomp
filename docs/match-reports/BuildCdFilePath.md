# BuildCdFilePath -- MATCHED (26/26 words)

> Renamed from `func_800289CC` on 2026-09-21 (tools/rename.py). Address 0x800289cc.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
char *BuildCdFilePath(char *dest, char *suffix) {
    dest[0] = '\\';
    strcpy(dest + 1, func_800270B8());
    strcat(dest, suffix);
    strcat(dest, D_8008A8A8);
    return dest;
}
```

with `extern char *func_800270B8(void);`, `extern char *strcpy(char *dest,
char *src);`, and `extern char D_8008A8A8[];` declared locally.

Byte-exact, 26/26 words.

## Notes

Builds a CD-ROM path string: a leading `\` (0x5C), the directory/disc-label
string `func_800270B8()` returns (still uncarved, `code_171e0.c`), the
caller-supplied filename `suffix`, and a fixed `";1"` suffix from
`D_8008A8A8` -- the ISO9660 file-version-number convention (`FILE.EXT;1`),
strong confirmation of this unit's CD-ROM theme alongside `CdStatus`.

`D_8008A8A8` sits in the `.sdata` region (`asm/data/7B008.sdata.s`) but this
function's OWN reference to it is a plain absolute `lui`/`addiu`, not
`%gp_rel` -- the three-grep blocker screen on this function's `.s` correctly
found zero `gp_rel` hits, so this is not an instance of the gp-relative
blocker despite the symbol's `.sdata` placement. `strcpy` is this unit's own
(defined later in ROM order, forward-declared here); `strcat` is already
declared identically in `code_171e0.h` (included for `func_80028898`), so
the local `extern` here is a harmless duplicate, not a conflict.
