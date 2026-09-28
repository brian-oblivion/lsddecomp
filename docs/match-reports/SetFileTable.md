# SetFileTable

> Renamed from `func_80027FD8` on 2026-09-17 (tools/rename.py). Address 0x80027fd8.

**Unit:** CdDriver (fresh carve) · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What this function does

A plain `$gp`-relative setter: stores its single `s32` argument into the
scalar global `sFileTable` (in `.sdata`). No return value.

## The C

```c
extern s32 sFileTable;

void SetFileTable(s32 a0)
{
    sFileTable = a0;
}
```

## Provenance

round 45 (2026-09-15), runner echo, unit CdDriver (fresh carve). See
IsCdBusy.md for the sibling-accessor context.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027FD8` | `SetFileTable` | A |
| `D_8008A868` | `sFileTable` | A |

**Evidence.** The global is the base of an array of 0x1C-byte records:
`FindCdFileEntry` and `FindCdFileIndex` (CdDriver) walk it with a literal
0x1C stride doing `strstr(record, needle)` from offset 0, `GetCdFileEntry`
indexes it as `base + index * 0x1C`, and `ResolveFileEntries` (this unit)
fills each record's `+0x14`/`+0x18` from a `CdSearchFile` lookup on the name
at offset 0. `GameApplicationFileResource.c`'s `RegisterFileTableEntries` sets the base, then the count,
then resolves `base + idx * 0x1C` -- the three-call sequence that makes the
array a file table. Setter of a base pointer: tier A.

Note for track 4: the symbol is still declared `s32` here and `char *` in
`CdDriver`. That is a type, not a name, so this round did not touch it.

## Track 4b (2026-09-25, round 85)

The CD driver's shared globals and records are now declared once, in
`include/CdDriver.h`, and this body uses that one reading: the parameter is `CdFileEntry *` (was `s32`). The
global's type comes from its accessors (`sFileTable` is walked at the 0x1C
`CdFileEntry` stride; `sCdSeekParam` is read for `->size` and sought to at
`+0x14`, i.e. `pos`). Byte-identical; no new `-Wall` warning.
