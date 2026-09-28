# FindCdFileIndex

> Renamed from `func_800284C4` on 2026-09-18 (tools/rename.py). Address 0x800284c4.

**Unit:** CdDriver · **Size:** 31 words · **Status:** MATCHED (31/31 words) · **Round 45**

## What it does

Sibling of `FindCdFileEntry` over the same `gFileTable`/`gFileTableCount` record
table, but returns the matching record's **index** (`s32`) instead of a
pointer, and `-1` if none match. Here the loop counter only increments on
the not-found path (a plain instruction, not a branch-delay-slot trick),
so the returned index is exactly the position of the match.

## The C

```c
s32 FindCdFileIndex(char *arg0)
{
    char *cur = gFileTable;
    s32 i = 0;

    LockCd();
    while (strstr(cur, arg0) == NULL) {
        i++;
        if (i >= gFileTableCount) {
            return -1;
        }
        cur += 0x1C;
    }
    UnlockCd();
    return i;
}
```

Closed on the first attempt.

## Naming

**Tier A.** Sibling of `FindCdFileEntry` over the same table, returning the
matching record's index (`-1` if none), consumed as `fileIndex` by
CdDriver.c's `EnqueueCdRequest` and as `func_800284C4`'s own
pre-existing comment there put it: "CdDriver: name -> table index".

## Track 4b (2026-09-25, round 85)

The CD driver's shared globals and records are now declared once, in
`include/CdDriver.h`, and this body uses that one reading: the walk is `CdFileEntry *cur; cur++` over `cur->name` (was `char *` stepped by 0x1C). The
global's type comes from its accessors (`gFileTable` is walked at the 0x1C
`CdFileEntry` stride; `gCdSeekParam` is read for `->size` and sought to at
`+0x14`, i.e. `pos`). Byte-identical; no new `-Wall` warning.

## Round 101 (track 7 polish)

Commented as the index of the first entry whose name contains `name`, or
-1; the not-found return skips UnlockCd like FindCdFileEntry's.
