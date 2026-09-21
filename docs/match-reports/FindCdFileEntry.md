# FindCdFileEntry

> Renamed from `func_80028448` on 2026-09-18 (tools/rename.py). Address 0x80028448.

**Unit:** code_179d8_r · **Size:** 31 words · **Status:** MATCHED (31/31 words) · **Round 45**

## What it does

Linear scan over a flat table of 0x1C-byte string records based at
`gFileTable`, `gFileTableCount` entries long. Returns a pointer to the first
record for which `strstr(record, needle) != NULL`, or `NULL` if none
match. Brackets the whole scan with the unit's lock/unlock pair, but
**only calls the unlock on the found path** -- the not-found path returns
directly without calling `UnlockCd()`. That asymmetry is retail's own
behaviour, reproduced exactly; it is not a bug introduced here.

## The C

```c
void *FindCdFileEntry(char *arg0)
{
    char *cur = gFileTable;
    s32 i = 0;

    LockCd();
    do {
        if (strstr(cur, arg0) != NULL) {
            UnlockCd();
            return cur;
        }
        i++;
        cur += 0x1C;
    } while (i < gFileTableCount);
    return NULL;
}
```

Closed on the first attempt. The `do { } while` shape (rather than a
`for`/`while` with a pre-loop test) is what reproduces retail falling
straight into the first `strstr` call with no initial bound check --
GCC 2.6.3 -O2 does not hoist a test ahead of a `do-while` body.

### Proposed learning

`gFileTable` (base pointer) / `gFileTableCount` (count) is a flat array of
fixed-size string records; treat `gFileTable` as `char *` and step by
`+= 0x1C` rather than declaring an array-of-struct type, since the
per-record byte layout past the leading string is otherwise unknown to
this unit.

## Naming

**Tier A.** Linear scan over the `gFileTable`/`gFileTableCount` record
table, `strstr`-matching `name` against each 0x1C-byte record; returns the
matching record pointer or `NULL`. Distinguished from `GetCdFileEntry`
(direct index-to-pointer, no search) by the Find/Get convention. Caller
`func_80027800` (code_179d8_s.c, the `Class6D4E8__RequestLoadFile` worker)
uses the returned record's `pos`/`size` fields to seek to and size the read,
confirming "find the file's table entry by name" as the purpose.
