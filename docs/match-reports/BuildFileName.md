# BuildFileName

> Renamed from `func_800270C4` on 2026-09-18 (tools/rename.py). Address 0x800270c4.

**Unit:** GameApplicationFileResource · **Size:** 27 instructions · **Status:** MATCHED (27/27 words, whole-image build verified byte-exact)

## What it does

Builds a string in `dest` by truncating it to empty, then conditionally
appending `arg2` (only if non-NULL), then unconditionally appending `arg1`,
then `arg3` — in that order — and returns `dest`. All three appends go
through this unit's own `strcat` (still `INCLUDE_ASM`, see `strcat.md`).

## Derivation

```
move  $s0, $a0            ; s0 = dest
move  $s1, $a1            ; s1 = arg1
move  $a1, $a2            ; a1 (renamed) = arg2, used for the check + call1
move  $s2, $a3            ; s2 = arg3
beqz  $a1, SKIP           ; if (arg2 == 0) skip the first strcat
 sb   $zero, 0($s0)       ; dest[0] = 0  -- ALWAYS runs (branch delay slot)
jal   strcat               ; strcat(dest, arg2)   [$a0 still = dest, untouched since entry]
 nop
SKIP:
move  $a0, $s0
jal   strcat                ; strcat(dest, arg1)
 move $a1, $s1
move  $a0, $s0
jal   strcat                 ; strcat(dest, arg3)
 move $a1, $s2
move  $v0, $s0                ; return dest
```

The `dest[0] = 0` truncation is the branch's delay slot, so it executes on
*both* the taken and not-taken paths — matching plain C where the statement
simply precedes the `if`, with GCC's own scheduler (not source order)
choosing to fill the delay slot with it.

## Final C

```c
char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3) {
    dest[0] = '\0';
    if (arg2 != NULL) {
        strcat(dest, arg2);
    }
    strcat(dest, arg1);
    strcat(dest, arg3);
    return dest;
}
```

## Attempt log

Matched on the first attempt.

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable. The one branch here (skip the first
  `strcat`) never affects the return value — every path returns `dest`
  unconditionally at the end, so there is no early-exit-with-different-value
  shape for the lever to act on.
- **loop-invariant hoisting:** not applicable, no loop (the loops are inside
  `strcat` itself, a separate function).
- **prologue store-order barrier:** not applicable, no store-order residue.

## Proposed learning

None new. Straightforward confirmation that `strcat`'s prototype
(`char *strcat(char *dest, char *src);`, declared in `include/GameApplicationFileResource.h`
so this forward reference to a later-defined-in-file function resolves) is
right in shape even though `strcat`'s own body is still stalled — the two
are independent findings.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800270C4` | `BuildFileName` | A |

**Evidence.** `dest[0]=0`; conditionally `strcat`s an optional prefix
(`arg2`, only if non-NULL); unconditionally `strcat`s `arg1` then `arg3`;
returns `dest`. Confirmed against real call sites across the tree
(`TextEntryItemList.c`: memory-card icon/font paths with a directory prefix and
an extension suffix; `PlacementGridVabSound.c`: name+suffix with no prefix) that this
is a general-purpose "optional-prefix + name + suffix" path/filename
composer, not guessed from this function's body alone. Pure string
composition whose mechanics are its purpose.
