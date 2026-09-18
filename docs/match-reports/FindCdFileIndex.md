> Renamed from `func_800284C4` on 2026-09-18 (tools/rename.py). Address 0x800284c4.

# FindCdFileIndex

**Unit:** code_179d8_r · **Size:** 31 words · **Status:** MATCHED (31/31 words) · **Round 45**

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
code_179d8_q.c's `EnqueueCdRequest` and as `func_800284C4`'s own
pre-existing comment there put it: "code_179d8_r: name -> table index".
