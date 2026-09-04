# CdStatus -- MATCHED (4/4 words)

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment).

## Result

```c
extern u8 D_8006D60C;

u8 CdStatus(void) {
    return D_8006D60C;
}
```

Byte-exact, 4/4 words.

## Notes

Inherited name from FirecatFG's lsddecomp, treated as a hypothesis per
CLAUDE.md -- but the shape (a trivial byte accessor) and this unit's overall
CD-ROM theme (`func_800289CC`'s path-building, the `;1` ISO9660 suffix) make
"CD status byte" a plausible read, not just an inherited label taken on
faith.
