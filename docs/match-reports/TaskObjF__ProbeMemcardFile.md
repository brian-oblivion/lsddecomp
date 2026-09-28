# TaskObjF__ProbeMemcardFile — MATCH (35/35 words)

> Renamed from `func_8004E9AC` on 2026-09-24 (tools/rename.py). Address 0x8004e9ac.

**Unit:** title_menu (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__ProbeMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *filterName)`. Guards
`filterName`: if it's NULL or points at an empty string, returns 0
immediately without calling anything. Otherwise calls
`TaskObjF__OpenAndReadMemcardFile(self, destBuf, filterName)` — with a retry-loop skeleton
identical in shape to `TaskObjF__FormatCard`/`TaskObjF__CheckCardSpace` but initialized to
`retries = 0`, so it structurally never loops (a single attempt).

## Result

Matched on the first attempt.

```c
extern s32 TaskObjF__OpenAndReadMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *filterName);

s32 TaskObjF__ProbeMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *filterName)
{
    s32 retries;
    s32 result;

    retries = 0;
    if (filterName == NULL || *filterName == 0) {
        return 0;
    }
    do {
        result = TaskObjF__OpenAndReadMemcardFile(self, destBuf, filterName);
    } while (result == 0 && retries-- != 0);
    return result;
}
```

## Parameter naming, corrected mid-derivation

Reading the disassembly forward, it initially looked like this function's
2nd parameter (forwarded as `TaskObjF__OpenAndReadMemcardFile`'s 2nd argument) might be an
opaque "arg1" distinct from the "name" string tested at the top — but
`TaskObjF__OpenAndReadMemcardFile`'s own body (see its report) reveals its 2nd parameter is
the `strcpy` DESTINATION buffer, and its 3rd parameter (this function's
`filterName`, the one actually tested here) is never read by
`TaskObjF__OpenAndReadMemcardFile` at all. Named accordingly once `TaskObjF__OpenAndReadMemcardFile` was read.

### Proposed learning

The `do { } while (result == 0 && retries-- != 0)` skeleton, seen three
times in this unit (`TaskObjF__FormatCard` with `retries = 10`,
`TaskObjF__CheckCardSpace` with `retries = 10`, this function with `retries = 0`) is
a single retry-loop idiom parameterized purely by the initial constant —
including the degenerate "run once" case. Reproduce the loop skeleton
literally rather than special-casing `retries == 0` into a plain `if`; the
skeleton's bytes are present in the disassembly even when the loop body
only ever executes once.

## Head correction, round 75

The two-argument / `filterName` reading of `BuildMemcardPath` above is
superseded. Round 75 matched `TaskObjF__OpenAndReadMemcardFile` by calling it with THREE
arguments `(pathBuf, self->unkC, suffix)`, the third forwarded from the
caller's own third parameter already in `$a2` (so no `$a2` set-up is
emitted, which is why it read as two). `src/ui/title_menu.c` now declares
one real prototype, `extern void *BuildMemcardPath(void *dest, s32 selector,
void *suffix);`, replacing the unprototyped `arity-ok` declarations; this
function's bytes are unchanged (see `TaskObjF__OpenAndReadMemcardFile.md`).

## Naming (round 78, track 3)

`func_8004E9AC` -> `TaskObjF__ProbeMemcardFile`. **Tier B.** Sits at `gTaskObjFMethods` +0x054, matching `Node3bb8cE`'s own `SelfMethods3bb8cE.slot54(self, s32, char*)` -- the slot `TaskObjF__FindUnusedMemcardName`/`TaskObjF__CollectExistingMemcardFiles` dispatch through on themselves. Guards an empty/NULL suffix, then makes a single (`retries = 0`, structurally never loops) call to `TaskObjF__OpenAndReadMemcardFile`. Named for what it mechanically is (a single-attempt existence/read probe used as the vtable's file-check slot); the two different calling conventions its callers use (`destBuf == NULL` for existence-only, non-NULL to also read) keep this tier B rather than A.

## Source comment moved here (round 98, track 7)

"TaskObjF__OpenAndReadMemcardFile's 3rd parameter is the file-name suffix,
forwarded verbatim by TaskObjF__ProbeMemcardFile (after rejecting NULL/empty)
and by TaskObjF__OpenAndReadMemcardFile as BuildMemcardPath's 3rd argument.
Round 75 corrected the earlier reading that TaskObjF__OpenAndReadMemcardFile
never used it: it never TOUCHES $a2, because the value is already where the
call wants it." The source now says what the function returns; `*suffix ==
0` reads `'\0'`. Zero bytes changed.
