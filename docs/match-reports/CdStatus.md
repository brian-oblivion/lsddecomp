# CdStatus -- CONVERTED to a linked SDK object (round 34). NOT game code, NOT a stall.

> **ROUND 34 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libcd/sys.o` (Psy-Q 3.3) AND IS NAMED `CdStatus` (0x80028C34).** It was
> matched C counted as game code; the object owns its bytes, so the C is gone from
> `src/` and the game-code count shrank by it -- the correction CLAUDE.md asks
> for, not a regression. The run `libc2/strcpy` + `libc2/strstr` + `libcd/sys`
> tiles 0x19378..0x19C78 and crosses the CdDriver / libcd_bios boundary;
> both units trimmed. Whole-image SHA1 green. Nothing here is assignable and
> there is no stall left to work. The text below is the pre-conversion record.

## Original report

# CdStatus -- MATCHED (4/4 words)

Unit: `CdDriver`. Runner: echo, round 17 (second assignment).

## Result

```c
extern u8 CD_status;

u8 CdStatus(void) {
    return CD_status;
}
```

Byte-exact, 4/4 words.

## Notes

Inherited name from FirecatFG's lsddecomp, treated as a hypothesis per
CLAUDE.md -- but the shape (a trivial byte accessor) and this unit's overall
CD-ROM theme (`BuildCdFilePath`'s path-building, the `;1` ISO9660 suffix) make
"CD status byte" a plausible read, not just an inherited label taken on
faith.
