# GetCdFileEntry -- MATCHED (round 46): 19/19 words, byte-exact

> Renamed from `func_80028540` on 2026-09-18 (tools/rename.py). Address 0x80028540.

**Unit:** code_179d8_s · **Round 46** · **MATCHED**

## What it does

Index-to-pointer helper over the same `gFileTable` string-record table used
by `FindCdFileEntry`/`FindCdFileIndex`: returns `gFileTable + index * 0x1C`,
bracketed by the unit's lock/unlock pair.

## Round 45's stall, and what closed it

Round 45 got to 9/19 (length-exact) with `base` computed inline in the
return expression, and separately tried `char *base = gFileTable;`
**declared and initialized together** before the lock call -- both of
those put `base` in `$s2` (an extra callee-saved register retail does not
use), 4 bytes longer. This session re-confirmed that exact result
(re-ran the combined-declaration shape, got the same `$s2`/longer
outcome, 304674 bytes of downstream drift) before reaching for the
permuter.

**Permuter, all three checks run:**

1. **Correctness** -- seed (the round-45 report's 9/19 "compute inline
   after lock" body) compiled and scored under the harness.
2. **Cost** -- base score 510 (`Register Differences: 10 (5)`,
   `Insertions: 2 (100)`, `Deletions: 2 (100)`) -- a real, non-zero,
   non-register-identity-only residue (unlike round 45's `ServiceSoundCueSet`
   pure-register-identity dead end that never moved off its seed score).
3. **Base-score agreement** -- rebuilt the identical seed body in-tree
   before searching: `9/19 words match (file 0x18D40-0x18D8C)`, length
   exact, no out-of-range drift -- matches the round-45 report's own
   figure exactly, so the scaffold is scoring the real thing.

Search: `-j 6 --stop-on-zero --best-only`, bounded `timeout 600` (`rc=0`,
well under the bound), run in the background while other functions were
worked. **Found a zero-score candidate at iteration 19.** The winning
diff:

```diff
 void *GetCdFileEntry(s32 index)
 {
   void *result;
+  char *new_var;
+  new_var = (char *) gFileTable;
   LockCd();
-  result = ((char *) gFileTable) + (index * 0x1C);
+  result = new_var + (index * 0x1C);
   UnlockCd();
   return result;
 }
```

The load-bearing change is not *when* `base` is computed (round 45 also
tried "before the lock call") but that it is **declared, then assigned in
a separate statement**, rather than declared-with-initializer. Translated
to idiomatic naming and rebuilt in-tree (`char *base; base = gFileTable;`
as two statements, ahead of the lock call) -- **19/19 words, byte-exact,
whole-image `OK: build matches retail SLPS_015.56`.**

## Final body (byte-exact)

```c
void *GetCdFileEntry(s32 index)
{
    void *result;
    char *base;

    base = gFileTable;
    LockCd();
    result = base + index * 0x1C;
    UnlockCd();
    return result;
}
```

### Proposed learning

**`T x = expr;` (declaration-with-initializer) and `T x; x = expr;`
(separate declaration then assignment) are NOT interchangeable under GCC
2.6.3 -O2's register allocator, even though they are semantically
identical and C89 permits either.** Round 45 tried exactly this value
(`gFileTable` loaded before the lock call) as a combined
declaration-with-initializer and got it allocated to an extra unused
saved register ($s2, 4 bytes longer than retail). Splitting the identical
initialization into two statements put it in $s0 as retail does, with no
other change. When a value's *timing* is already confirmed correct (by
delay-slot or call-ordering evidence) but the register class is still
wrong, try re-splitting a combined declare+init into two statements (and
vice versa) before concluding the shape itself is unreachable -- this is
a second, independent axis from statement *order*, and round 45's report
did not distinguish it from the "declared/assigned before the lock call"
attempts it had already tried. Found by the permuter, not by hand --
worth remembering as a lever the permuter reaches that manual rephrasing
attempts (which default to combined declare+init as the "obvious" idiom)
tend to skip.

## Naming

**Tier A.** Direct index-to-pointer helper over the same `gFileTable` table
(`gFileTable + index * 0x1C`), no search -- distinguished from
`FindCdFileEntry`/`FindCdFileIndex` (which scan) by the Get/Find naming
convention. Called from `CdDriver__RunRequestQueue` (code_179d8_s.c, still `INCLUDE_ASM`)
by table index.

## Track 4b (2026-09-25, round 85)

The CD driver's shared globals and records are now declared once, in
`include/CdDriver.h`, and this body uses that one reading: `&base[index]` over `CdFileEntry *` (was `char *` plus `index * 0x1C`). The
global's type comes from its accessors (`gFileTable` is walked at the 0x1C
`CdFileEntry` stride; `gCdSeekParam` is read for `->size` and sought to at
`+0x14`, i.e. `pos`). Byte-identical; no new `-Wall` warning.

## Round 101 (track 7 polish)

`base`/`result` are `table`/`entry`, both `CdFileEntry *`, and `table` is
now initialised at its declaration. With the typed pointer that shape is
byte-exact too (measured round 101), so the declare-then-assign split
above was a fact about the `char *` / `void *` spelling, not about this
function; no MATCHING line is needed. The return type stays `void *`
because code_179d8_s.c declares it that way (proposal in the round's
summary).
