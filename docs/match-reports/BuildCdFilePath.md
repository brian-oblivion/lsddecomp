# BuildCdFilePath -- MATCHED (26/26 words)

> Renamed from `func_800289CC` on 2026-09-21 (tools/rename.py). Address 0x800289cc.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
char *BuildCdFilePath(char *dest, char *suffix) {
    dest[0] = '\\';
    strcpy(dest + 1, GetDataDirectory());
    strcat(dest, suffix);
    strcat(dest, gCdFileVersionSuffix);
    return dest;
}
```

with `extern char *GetDataDirectory(void);`, `extern char *strcpy(char *dest,
char *src);`, and `extern char gCdFileVersionSuffix[];` declared locally.

Byte-exact, 26/26 words.

## Notes

Builds a CD-ROM path string: a leading `\` (0x5C), the directory/disc-label
string `GetDataDirectory()` returns (`code_171e0.c`, still `func_`-named,
carved), the caller-supplied filename `suffix`, and a fixed `";1"` suffix
from `gCdFileVersionSuffix` -- the ISO9660 file-version-number convention
(`FILE.EXT;1`), strong confirmation of this unit's CD-ROM theme alongside
`CdStatus`.

`gCdFileVersionSuffix` sits in the `.sdata` region (`asm/data/7B008.sdata.s`) but this
function's OWN reference to it is a plain absolute `lui`/`addiu`, not
`%gp_rel` -- the three-grep blocker screen on this function's `.s` correctly
found zero `gp_rel` hits, so this is not an instance of the gp-relative
blocker despite the symbol's `.sdata` placement. **CORRECTION (round 64):**
`strcpy` was this unit's own at the time this was written; round 34's
SDK-object conversion reclassified it as Sony's (`lib/libc2/strcpy.o`), so
the "defined later in ROM order" clause is stale -- it is declared LOCAL
here (per-call-site typed) the same way `CdSearchFile`/`printf` are
elsewhere in this unit, never defined in this file. `strcat` is already
declared identically in `code_171e0.h` (included for
`FileResource__InstallCdReadDriver`), so the local `extern` here is a
harmless duplicate, not a conflict.

## Naming (round 64, runner alpha)

`func_800289CC` -> `BuildCdFilePath`, tier A. The body's four operations
(prepend `\`, append the base directory, append `suffix`, append the
ISO9660 `;1` version suffix) fully account for what the function returns; a
pure string-building leaf whose mechanics are its whole purpose. Called by
`OpenCdFile` (this unit) as the first step of resolving a file by name.

## Naming (round 99, echo, track 7)

Parameter `suffix` -> `name`, tier A: both callers (`OpenCdFile`,
`CdDriver__Open`/`ResolveFileEntries`) pass a file name, and the body builds
`"\\" + <data directory> + name + ";1"`; the suffix is
`gCdFileVersionSuffix`. `GetDataDirectory` (code_171e0.c) is the data directory
getter; its name is proposed, not applied (not this unit's function).
