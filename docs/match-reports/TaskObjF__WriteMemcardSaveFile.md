# TaskObjF__WriteMemcardSaveFile -- MATCHED 51/51 (round 18, echo, via permuter -- targeted PERM macros)

> Renamed from `func_8004EEA0` on 2026-09-20 (tools/rename.py). Address 0x8004eea0.

**Unit:** class_3bb8c_f · **Size:** 51 words (0xCC)

## What it does

`s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5,
s32 arg6, s32 arg7)`. The same bounded-retry shape as `TaskObjF__ReadMemcardFile`
(this unit, matched): calls `CopyMemcardIconTemplate(handle, a1)` once up front,
then retries `TaskObjF__TryWriteMemcardSaveFile(self, a1, handle, a3 & 0xFF, arg5, arg6,
arg7)` (this unit, also a predicted-hard stall — see its own report) up
to 11 times via the identical `do { ...; if (result) break; } while
(count-- != 0);` idiom, and — only if every attempt returned 0 — calls
`CopyMemcardIconTemplate(handle, 0)` before returning the last result.

Register census: **9 distinct callee-saved registers, fully saturated**
(`$fp`/`$s8` + `$s0`-`$s7` — confirmed with the `$fp`-vs-`$s8` naming
caveat from CLAUDE.md, both `objdump`'s `s8` rendering and splat's `$fp`
counted as the same register). Blocker screen clean (no `gp_rel`, no
`addiu $at,$at,%lo`, no jump table).

## The residue

**Every branch target, every field/argument, and the total compiled
LENGTH are settled** — the best attempt compiles to exactly 51 words with
**zero address drift** (no "differs outside this range" warning), the
same "confirmed zero-drift register PERMUTATION" signature CLAUDE.md
documents for `StageMap__BuildFootprintSlots` in the `class_3bb8c_b`/`_c` header family.
Retail's own register assignment: `self`→`$fp`, `a1`→`$s3`, `handle`→
`$s2`, `a3`→`$s7`, `arg5`→`$s6`, `arg6`→`$s5`, `arg7`→`$s4`, plus `$s1`
(retry counter) and `$s0` (result). The best attempt reached (preserved
below) assigns the identical SET of 9 values to the identical 9
registers, but permuted: `self`→`$s4` (not `$fp`), `a3`→`$s5` (not
`$s7`), `arg5`→`$s8`, `arg6`→`$s7`, `arg7`→`$s6` — a genuine permutation,
not a missing or extra value.

## What was tried

1. **Direct transcription** (the exact `TaskObjF__ReadMemcardFile` retry idiom,
   proven correct there). Score 31/51, full permutation as described
   above.
2. **Reordering the two independent top-of-function statements**
   (`count = 10;` before vs. after the initial `CopyMemcardIconTemplate(handle,
   a1);` call) — moved the score from 31/51 to 33/51 (2 more words) but
   did NOT change which register any of the 9 live values landed in; the
   2 extra matching words are incidental instruction-encoding overlap,
   not progress on the permutation itself.
3. **Local-declaration order** (`count`/`result` swapped) — reverted
   immediately, made things worse (back to 31/51).

Given `StageMap__BuildFootprintSlots`'s own report (this same header-family class,
documented in CLAUDE.md) already tried declaration-order and
branch-structure reshaping across 7 variants with an isolated
sub-second reproducer and never moved a SINGLE value off its
mis-assigned register, and this function's own attempt 3 reproduces
that same non-responsiveness on a much smaller scale (2 variants, one
direction), further reshaping attempts were not pursued past the two
above — see CLAUDE.md's explicit finding that this residue class is
"not fixable by reshaping."

## Preserved near-miss body (`#if 0`, best variant, 33/51, zero drift)

```c
#if 0
s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7) {
    s32 count;
    s32 result;

    count = 10;
    CopyMemcardIconTemplate(handle, a1);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, a1, handle, a3 & 0xFF, arg5, arg6, arg7);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    if (result == 0) {
        CopyMemcardIconTemplate(handle, 0);
    }
    return result;
}
#endif
```

Needs a forward declaration of `TaskObjF__TryWriteMemcardSaveFile` (defined later in this
unit's ROM order; already present near the top of `src/class_3bb8c_f.c`)
and `CopyMemcardIconTemplate`'s extern (declared in `include/class_3bb8c.h`).

## Header additions (`include/class_3bb8c.h`, additive only)

- `extern s32 CopyMemcardIconTemplate(s32 handle, s32 flag);` — **an extern for a
  function outside this unit**, typed from this call site's own register
  usage (the pre-loop call's `a1` is EEA0's OWN incoming `a1` parameter,
  left untouched in its hardware register rather than re-set, which is
  how its value was recovered without a caller elsewhere in this unit to
  cross-check against).

## Proposed learning

**A second confirmed instance of the zero-drift register-PERMUTATION
class outside the `class_3bb8c_b`/`_c` header family it was first
documented in** (this is `class_3bb8c_f`) — this residue is not specific
to one header's functions; it recurs whenever a function has ~8-9
simultaneously-live values and GCC 2.6.3 happens to pick a different
(but equally valid) bijection from values to registers than retail's
source did. Worth flagging in `docs/DECOMPILATION_LEARNINGS.md`'s
existing entry on this class as a THIRD confirmed instance, now spanning
two units.

## RESOLUTION (round 18) — the permuter found the real lever, and it was a TYPE, not a register

Given this function's own STALL classification cited CLAUDE.md's
"register-identity permutation, not fixable by reshaping" class (confirmed
independently by `StageMap__BuildFootprintSlots`, 7 variants, zero movement), a permuter
run was set up with TARGETED `PERM_GENERAL` macros over the two axes
round-17's manual attempts had already identified as levers (declaration
order of `count`/`result`; statement order of `count = 10;` vs. the
pre-loop `CopyMemcardIconTemplate` call), wrapped in `PERM_RANDOMIZE` for open
search beyond those two switches.

**Note on setup:** `tools/setup-permuter.sh` pipes the seed through the
REAL `cpp`/`cc1` to prove it compiles before handing the scaffold back --
this rejects `PERM_` macros outright (`cc1` has no idea what they are).
The fix: run `setup-permuter.sh` with a PLAIN (macro-free) seed to get a
valid, fully-preprocessed `base.c`, then hand-edit the PERM macros into
that generated `base.c` directly (the permuter's own `perm/parser.py`
expands them at each iteration; they were never meant to survive real
`cpp`). `--debug` afterward confirmed the base score (500) matched the
plain seed's own `--debug` score exactly, proving the macro edit didn't
change the DEFAULT (first-listed) expansion.

Bounded search (`timeout 600`, `-j 6 --stop-on-zero`): reached **score 0
at iteration 4160**. `permuter exit=0` (captured cleanly this time --
`--stop-on-zero` stopped the run itself well inside the 600s bound, no
race with the outer Bash timeout).

**The winning diff, verbatim from `output-0-1/diff.txt`:**

```diff
-s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7)
+s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, char a3, s32 arg5, s32 arg6, s32 arg7)
```

**Neither PERM_GENERAL axis mattered at all.** The permuter's own random
mutation found something round 17's hand attempts never considered
because the whole investigation had been framed as "which register does
each already-`s32`-typed value land in" -- the actual gap was the
PARAMETER'S DECLARED TYPE. `a3` is used in the body only as `a3 & 0xFF`
(masked to a byte); retyping the parameter itself to `char` (unsigned, per
this project's `-funsigned-char`) rather than `s32` changes how GCC 2.6.3
handles the incoming register at the function's own entry point, which
was enough to re-derive retail's ENTIRE 9-register assignment -- this
was never a rotation-of-independent-values problem at all, it was one
mistyped parameter cascading into what LOOKED like a register permutation
across the whole function.

**Applied verbatim to `src/class_3bb8c_f.c`** (body otherwise unchanged
from the preserved near-miss):

```c
s32 TaskObjF__WriteMemcardSaveFile(TaskObjF *self, s32 a1, s32 handle, char a3, s32 arg5, s32 arg6, s32 arg7) {
    s32 count;
    s32 result;

    count = 10;
    CopyMemcardIconTemplate(handle, a1);
    do {
        result = TaskObjF__TryWriteMemcardSaveFile(self, a1, handle, a3 & 0xFF, arg5, arg6, arg7);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    if (result == 0) {
        CopyMemcardIconTemplate(handle, 0);
    }
    return result;
}
```

Also added a forward declaration for `TaskObjF__TryWriteMemcardSaveFile` (defined later in
this unit, ROM order) at the top of the file -- the previous report
claimed one was "already present", which was not the case; without it,
`cc1` implicitly declares `TaskObjF__TryWriteMemcardSaveFile` returning `int` with unpromoted
argument types, which HAPPENED to still byte-match here (`int`==`s32`,
and `char`/other args promote to the same registers either way under this
ABI) but is not something to rely on -- the explicit prototype is
free and removes the compiler's own warning.

**`build exit=0`, `funcdiff`: `TaskObjF__WriteMemcardSaveFile: 51/51 words match`,
whole-image SHA1 verified.**

### Proposed learning

**A function classified into the "register-identity permutation, zero
drift" stall class on the strength of "every value is already the right
TYPE, only the register differs" should still have its ARGUMENT/LOCAL
TYPES individually re-examined, not assumed correct, before accepting the
classification.** This function's own residue was filed as a 3-instance
confirmation of that class alongside `StageMap__BuildFootprintSlots`/`ItemList__LoadResources` (see
those reports, also round 18) -- but unlike those two (where nine
combined declaration-order attempts across three functions moved nothing),
this one's actual cause was a narrower, single-parameter type mismatch
that happened to cascade into a register reassignment affecting the WHOLE
function's frame, presenting identically to a "pure rotation" residue.
**The tell, in hindsight: the parameter in question (`a3`) is masked with
`& 0xFF` at its only use site** -- a byte-range operation on a value
declared wider than a byte is exactly the shape worth re-typing before
accepting a register-identity stall, the same way CLAUDE.md's `s8`/`s16`
struct-field guidance already treats a narrow access as a signal about the
DECLARED width. Suggest cross-referencing this note from
`StageMap__BuildFootprintSlots`'s and `ItemList__LoadResources`'s entries in
`docs/DECOMPILATION_LEARNINGS.md`'s existing class writeup, since this
round closed one of three instances that class currently claims and the
mechanism that closed it does not generalize to the other two (both
re-verified inert to declaration-order changes this same round, and
neither has an obvious narrow-masked parameter to retype).

## Naming (round 60, track 3)

`func_8004EEA0` -> `TaskObjF__WriteMemcardSaveFile`. **Tier A.** A
bounded (11-attempt) retry wrapper around
`TaskObjF__TryWriteMemcardSaveFile` (see that function's own report for
why "SaveFile": the 0x200-byte buffer it submits is structurally exact
to the documented PS1 memory-card save file header format), bracketed by
a `CopyMemcardIconTemplate(handle, ...)` registry mark/unmark call (mark before
the retry loop, unmark only if every attempt failed). The registry call
itself is a different unit's own helper (`src/class_3bb8c_g.c`) and its
exact purpose is not re-derived here.

## Track 4 (2026-09-26, round 89)

Parameters retyped with TaskObjF's unification (include/TaskObjF.h), byte-identical: `a1` is `char *fileName` (TaskObjF::fileName; BuildMemcardPath's suffix), `handle` is `char *title` (TaskObjF::title; strcpy'd into the header), `arg5` is `struct TimImage *icon` (TaskObjF::iconImage, from Class86B60's iconHandle; the icon source is its FileResource `buffer`, +0x010, so the local McIconSourceRef view is gone), `arg6` is `void *data` and `arg7` is `s32 size`. Only CopyMemcardIconTemplate's own `(s32, s32)` view still takes casts.
