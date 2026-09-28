# OpenCdFile -- MATCHED round 74 (43/43, length exact, whole image byte-exact)

REVISITED, round 74: MATCHED (43/43); names/types not relevant (existing
field names used unchanged; the lever was control flow).

> **ROUND 74 (2026-09-24), runner charlie -- MATCHED.**
>
> **Rebuild first.** The promoted `#ifdef NON_MATCHING` body, made live
> unchanged (ReadCdFile held at INCLUDE_ASM): `build exit=2`, 13/43,
> `insertions 13 / deletions 13`, 27 positional skeleton diffs, built length
> 44/43 (one word long) -- matches the round-72 title.
>
> **The diff, read.** Retail computes `&path` (`addiu $s2,$sp,0x28`) at the
> TOP of the loop body, every iteration, and passes BuildCdFilePath a
> separate fresh `addiu $a0,$sp,0x28`. The built body computes `&path` once
> before BuildCdFilePath into a saved register and reuses it everywhere --
> that is the extra word, and the saved-register rotation (i/self/path in
> s1/s2/s0 vs retail s0/s1/s2) follows from the changed live ranges. That is
> loop-invariant motion: a C `while`/`for`/`do` emits loop notes, loop.c
> hoists the invariant address out, and CSE then merges it with the
> pre-loop argument.
>
> **Shape tried 1 (while + post-increment):** `while (CdSearchFile(...) ==
> 0) { if (i++ >= 100) { printf; return; } }` -- fixes the retail
> `move v0,s0; slti v0,v0,100; bnez; addiu s0,s0,1` compare-before-increment
> form (ins/del 13/13 -> 7/7) but still hoists &path, still 44 words.
>
> **Shape tried 2 (label + backward goto) -- MATCH.** A goto loop has no
> loop notes, so loop.c never sees it and never hoists; `&path` is
> recomputed in the loop's first block exactly as retail does. Built on the
> first try: 43/43, `insertions 0 / deletions 0`, `./build-and-verify.sh`
> OK, `tools/check-nonmatching.sh` green. Store order
> `pos; size; isOpen = 1` was written to follow retail's load order; not
> separately measured whether the other order also matches.
>
> ```c
> void OpenCdFile(ObjA34_179D8H *self, char *suffix) {
>     s32 i;
>     StatBuf179D8H statBuf;
>     char path[0x40];
>
>     i = 0;
>     if (self->isOpen == 0) {
>         BuildCdFilePath(path, suffix);
>     retry:
>         if (CdSearchFile(&statBuf, path) == 0) {
>             if (i++ < 100) {
>                 goto retry;
>             }
>             printf(gCdFileNotFoundFmt, path);
>             return;
>         }
>         self->pos = statBuf.pos;
>         self->size = statBuf.size;
>         self->isOpen = 1;
>     }
> }
> ```
>
> ### Proposed learning
>
> **A loop-invariant stack address recomputed INSIDE a loop in retail means
> the loop was a label+goto, not while/for.** Discriminator: retail has
> `addiu $sN,$sp,K` at the top of the loop body while a call before the
> loop passes the same `$sp+K` as its own fresh `addiu $a0,$sp,K`; the
> built C instead computes it once before the loop into a saved register
> (one word long, saved registers rotated). GCC 2.6.3's loop.c only
> hoists invariants out of loops bracketed by loop notes, which goto loops
> never get. Seven prior rounds (17/36/47/54/64/72) filed this as
> "path-address CSE + register-role rotation"; it was control flow.

---

(Historical record below; its title was: OpenCdFile -- STALL, NON_MATCHING body promoted round 72 (promoted body: 13/43 words at length 44/43 [1 word long], structural; round-54 reshape 2 measured 14/43 at the same length but was reverted and is NOT the promoted body -- see round-72 note))


> **ROUND 72 (2026-09-23), runner charlie -- NON_MATCHING body promoted.**
> Per `docs/FINISHING-PLAN.md` track 1b: the "## Best result" body (the one
> the round-64 note says is kept current against the present struct field
> names) is hand-derived across rounds 17/36/47/54 -- no permuter-found edit
> is in it; round 47's permuter check (b) explicitly DECLINED the search
> (insertions=4, deletions=3, reorderings=1, base score 863), so nothing
> from a search ever entered this body. Live-measured this round under the
> current pinned maspsx flags (rounds 42/63) by making the body live C in
> place of `INCLUDE_ASM`, running the real oracle, and reading
> `funcdiff.py`: **13/43 raw word-match, length 44/43 words (one word
> long)** -- `build/lsdde.map` confirms `BuildCdFilePath - OpenCdFile =
> 0xB0` = 44 words against retail's 43, unchanged from every prior
> measurement. This is NOT byte-exact, so it was restored to the
> `#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` shape (not left as
> live C) with a comment carrying this score, its residue class
> (structural: path-address CSE across the loop's calls + a register-role
> rotation), and this report. `./build-and-verify.sh` and
> `tools/check-nonmatching.sh` both green afterward; verified build bytes
> unchanged (INCLUDE_ASM still drives the linked build).

> Renamed from `func_80028920` on 2026-09-21 (tools/rename.py). Address 0x80028920.

> **ROUND 64 (2026-09-21), runner alpha -- field rename note.** `ObjA34_179D8H`'s
> fields were renamed this round: `unk0C` -> `isOpen`, `unk18` -> `pos`,
> `unk1C` -> `size`; `StatBuf179D8H`: `unk0` -> `pos`, `unk4` -> `size`;
> `MethodsA34_179D8H`: `slot48` -> `onError`. All prose and code below this
> note PREDATES the rename and uses the old field names throughout (it is
> historical narrative, left as written); the `## Best result` block's actual
> function body has been updated to compile against the CURRENT struct
> definitions in `src/code_179d8_h.c` -- that is the one to splice if you
> pick this function up again.

> **ROUND 54 (2026-09-18), runner charlie -- rebuilt, then two more structural
> reshapes, both negative.**
>
> **Rebuild-before-trusting-the-score.** Spliced the preserved (round-36)
> body into `src/code_179d8_h.c` in place of the `INCLUDE_ASM` unchanged and
> ran the real oracle: `build exit=2`, no compile-error grep hits,
> `build/lsdde.map` confirms `BuildCdFilePath - OpenCdFile = 0xB0` = 44
> words against retail's 43 (one word long, unchanged since round 36/47).
> Restored immediately; `./build-and-verify.sh` -> `OK: build matches retail
> SLPS_015.56`.
>
> **Two new reshapes tried this round, both on the untested "first call
> needs a genuinely different C-level expression" axis this report already
> flagged (line "The untested axis" below) — both negative, one badly so:**
>
> 1. **Forward-goto to relocate the found-handling block physically after
>    the exhausted-tail**, matching the asm's literal layout (`found` block
>    sits right before the epilogue, reached by a forward branch from inside
>    the loop, positioned AFTER the `printf`+return tail — the OPPOSITE of
>    where a natural `while(1){ if(found) break; ...} self->unk18=...;`
>    places it):
>    ```c
>    i = 0;
>    if (self->unk0C == 0) {
>        BuildCdFilePath(path, suffix);
>        do {
>            if (CdSearchFile(&statBuf, path) != 0) {
>                goto found;
>            }
>        } while (i++ < 100);
>        printf(gCdFileNotFoundFmt, path);
>        goto end;
>    found:
>        self->unk18 = statBuf.unk0;
>        self->unk0C = 1;
>        self->unk1C = statBuf.unk4;
>    }
>    end:
>        ;
>    ```
>    **Result: 3/43, with drift (249182 bytes outside range)** — much worse
>    than the recorded 12/43. The goto/label restructuring did not merely
>    fail to help; it broke the shape that was already closest. Reverted
>    immediately, clean revert confirmed byte-for-byte, `OK: build matches
>    retail`.
> 2. **`do { if(found) break; } while (i++ < 100)` — retail's own increment
>    timing** (test `i` BEFORE incrementing, then increment only when
>    looping again, matching the asm's `slti v0,s0,0x64` / delay-slot
>    `addiu s0,s0,1` exactly), keeping the found-handling AFTER the loop as
>    plain fall-through (the shape that scored 12/43 previously):
>    ```c
>    i = 0;
>    if (self->unk0C == 0) {
>        BuildCdFilePath(path, suffix);
>        while (1) {
>            if (CdSearchFile(&statBuf, path) != 0) {
>                break;
>            }
>            if (!(i++ < 100)) {
>                printf(gCdFileNotFoundFmt, path);
>                return;
>            }
>        }
>        self->unk18 = statBuf.unk0;
>        self->unk0C = 1;
>        self->unk1C = statBuf.unk4;
>    }
>    ```
>    **Result: 14/43 raw match, SAME length as before (44/43, one word
>    long)** — a numerically higher raw count than the recorded 12/43, but
>    `asm-differ` shows it is the IDENTICAL residue class, not an
>    improvement: the address computation is still hoisted one instruction
>    earlier than retail (an extra `addiu s0,sp,0x28` appears BEFORE the
>    `jal BuildCdFilePath` where retail computes it in the jal's own delay
>    slot), and the register-role rotation persists in a different exact
>    permutation (`s2`=self/`s1`=i/`s0`=path here, vs retail's
>    `s1`=self/`s0`=i/`s2`=path — still a three-way rotation, not resolved).
>    The FIRST real diff is now at file offset `0x19124` (word 2, the very
>    first `sw` after the stack adjust — a register-name difference that
>    persists through the whole function), which is earlier than the raw
>    count alone suggests. Recorded as the new "best" raw figure since it is
>    genuinely higher at the SAME length (a fair comparison, unlike the
>    goto attempt's different-length regression), but it is not a step
>    toward matching — the underlying residue (address-CSE-across-3-call-sites,
>    documented below) is untouched.
>
> **Neither reshape defeats the address CSE**, which is the actual blocker
> (residue #1 below); reshape 2 only moved which register ends up rotated
> where, at the cost of hoisting the first call's address one instruction
> earlier than before. No new lever found. Not re-running the permuter:
> round 47's check-(b) decline (insertions=4, deletions=3, base score 863)
> is unaffected by either of this round's reshapes, since both preserve the
> same fundamental CSE and only perturb block order / increment timing
> around it. Restored to `INCLUDE_ASM`; `git status --porcelain` clean.
>
> **Proposed learning (posted to broadcast this round):** the corpus's
> "GCC 2.6.3's loop optimisations are SYNTAX-GATED, not CFG-gated" learning
> (a documented `label:...if(cond) goto label` LICM-defeat, for a
> loop-carried literal comparison) does NOT generalise to a repeated LOCAL
> STACK-ADDRESS argument CSE'd across non-adjacent call sites in different
> blocks — tried on two distinct call-count/shape variants here, one
> regressed sharply (3/43) and the other left the CSE fully intact while
> only relocating the rotation. The established fix for an address-CSE
> residue (`extern T name_b __asm__("name");`, "A repeated-global-address
> CSE is defeatable from C89") is explicitly for a GLOBAL's linker symbol;
> there is no known C89 lever in this corpus for the LOCAL-stack-address
> case this function needs, and this round's two attempts to find one via
> restructuring came back negative.

---

> **ROUND 36 (2026-09-12), runner charlie -- MEASURED, not just re-named.**
> Round 34's SDK-object conversion renamed this function's two callees
> (`func_8002B640` -> `CdSearchFile`, `func_80012C20` -> `printf`; both
> confirmed in `config/symbols.slps01556.lsdde.txt` and already declared,
> per-call-site typed, in `src/code_179d8_h.c` itself). The preserved body
> below still spelled the old names and was never rebuilt under the new
> ones, so its 12/43 figure was carried forward UNVERIFIED (flagged by
> `tools/stalesyms.py`). Corrected the two names, spliced the body into
> `src/code_179d8_h.c` in place of the `INCLUDE_ASM`, and ran the real
> oracle: `build exit=0`, `funcdiff.py` shows the function is exactly one
> word longer than retail's 43 (`asm-differ` confirms it is the SAME
> structural residue this report already documents -- `path`'s address
> cached once via `addiu s0,sp,0x28` ahead of the loop, then reloaded for
> the first call, instead of being recomputed fresh at each of the three
> call sites as retail does), matching this report's own already-recorded
> shape exactly. The unverified figure is now measured, and it did not
> move. Restored to `INCLUDE_ASM`; the corrected, linkable body is
> preserved below (replacing the stale-symbol version). Not spending
> further attempts re-deriving the five already-tried axes (see below) or
> the untested one already named (a genuinely different C-level expression
> for the first call's `path` use) -- out of scope for this round's time
> budget, still open for whoever picks this up next.

Unit: `code_179d8_h`. Runner: echo, round 17 (second assignment). Restored
to `INCLUDE_ASM`.

## Class: mixed (address-expression caching + register-role rotation), not
one of the two documented blockers

Screened clean on both. Confirmed via `tools/m2ctx.py code_179d8_h --sig
'void OpenCdFile(ObjA34_179D8H *self, char *suffix)' --run`, which
independently reconstructs the same algorithm and confirms the field
offsets/types this report uses.

## What it does (high confidence)

Builds a CD path via `BuildCdFilePath` (this unit, matched) into a local 64-byte
buffer, then retries `func_8002B640` (CD stat lookup, still uncarved,
BLOCKED addiu_at in its own unit `libcd_bios`) up to 100 times; on success,
copies the stat buffer's first two fields into `self->unk18`/`self->unk1C`
and marks `self->unk0C = 1`; on exhausting the retries, logs via
`func_80012C20` (Psy-Q print wrapper) and gives up. `self` is the SAME
`ObjA34_179D8H` this unit's `CloseCdFile`/`GetCdFileSize` already
established (fields `unk0C`, `unk1C` line up exactly) -- this function adds
a new field, `unk18`, a 4-byte alignment-2 pair (the `lwl`/`lwr` +
`swl`/`swr` idiom CLAUDE.md documents), copied from the same offset in the
stat buffer.

## Best result (12/43, `build exit=0`, size drift present)

```c
#if 0
/* Pair16_179D8H, StatBuf179D8H, ObjA34_179D8H and MethodsA34_179D8H are
 * ALREADY declared earlier in src/code_179d8_h.c (current field names:
 * ObjA34_179D8H::isOpen/pos/size, StatBuf179D8H::pos/size) -- do not
 * re-paste these typedefs when splicing, only the function body below. Shown
 * here again only so this block reads standalone. */
typedef struct Pair16_179D8H {
    s16 unk0;
    s16 unk2;
} Pair16_179D8H;

typedef struct StatBuf179D8H {
    Pair16_179D8H pos;
    u32 size;
    u8 pad8[0x18 - 0x8];
} StatBuf179D8H;

/* CdSearchFile/printf (was func_8002B640/func_80012C20): Sony's, linked
 * from lib/libcd/iso9660.o and the Psy-Q C runtime since round 34's
 * SDK-object conversion -- corrected round 36. */
extern s32 CdSearchFile(StatBuf179D8H *statBuf, char *path);
extern void printf(const char *fmt, void *arg1);
extern char gCdFileNotFoundFmt[];
char *BuildCdFilePath(char *dest, char *suffix);  /* forward decl, ROM order */

void OpenCdFile(ObjA34_179D8H *self, char *suffix) {
    s32 i;
    StatBuf179D8H statBuf;
    char path[0x40];

    i = 0;
    if (self->isOpen == 0) {
        BuildCdFilePath(path, suffix);
        while (1) {
            if (CdSearchFile(&statBuf, path) != 0) {
                break;
            }
            i++;
            if (i >= 100) {
                printf(gCdFileNotFoundFmt, path);
                return;
            }
        }
        self->pos = statBuf.pos;
        self->isOpen = 1;
        self->size = statBuf.size;
    }
}
#endif
```

`ObjA34_179D8H::unk18` (added this attempt) and the field order in
`ObjA34_179D8H`/`StatBuf179D8H` are worth keeping even though this function
stalled -- `CloseCdFile`/`GetCdFileSize` (already matched) are unaffected
(neither reads `unk18`), and the STACK LAYOUT this version reaches
(`statBuf` at `sp+0x10`, `path` at `sp+0x28`, matching retail's `lwl
$v0,0x13(sp)` / `lwr $v0,0x10(sp)` source and frame size `-0x78`) is
correct -- confirmed by diffing against retail's exact addresses.

## The residue (two distinct issues, best attempt has both)

1. **GCC unifies `path`'s address across all three call sites** (the
   `BuildCdFilePath` dest argument, the `func_8002B640` second argument, and
   the `func_80012C20` second argument) into ONE cached register value,
   computed once. Retail does NOT: it computes `$a0 = sp+0x28` freshly for
   the FIRST call (a plain `addiu`, no caching), and only starts caching
   the address in a callee-saved register (`$s2`) once inside the retry
   loop, recomputing it fresh EACH loop iteration rather than hoisting it
   out. Removing an explicit separate pointer variable (using the array
   name `path` directly at all three sites, tried explicitly) did not
   change this -- GCC still recognises the repeated `path` reference as the
   same value and reuses one register throughout.
2. **Register roles are rotated relative to retail even where the shape is
   otherwise close.** Retail: `$s1` = `self`, `$s0` = the retry counter
   `i`, `$s2` = the cached path address. My best attempt: `$s2` = `self`,
   `$s1` = `i`, `$s0` = `path`. Every local variable is present and live in
   the right ranges; only which callee-saved register each lands in
   differs.

## Attempts (5, all build-verified, same overall shape after the first)

1. `do { ... } while(i < 100);` with the "found" case handled via early
   `return` inside the loop body -- 4/43, size drift (wrong CFG entirely;
   didn't share the "not found" tail).
2. m2c-seeded `while (func_8002B640(...) == 0) { i++; if (i>=100) {...
   return; } }` -- 3/43, similar CFG issue, slightly different framing of
   the same problem.
3. `while (1) { p = path; if (found) break; ...}` with an explicit `char
   *p` pointer variable, reused for the 2nd/3rd calls -- 12/43 (first time
   the stack layout and frame size matched retail's exactly); register-role
   rotation and address-caching residues both present.
4. Attempt 3 with the explicit `p` variable removed (bare `path` used at
   all three call sites) -- identical 12/43 result; confirms the caching
   isn't caused by introducing a named pointer variable, GCC does it for
   the bare array reference too.
5. `do { if (found) { ...; return; } } while (i++ < 100);` (post-increment
   loop condition, matching the delay-slot increment's apparent timing) --
   5/43, worse; the post-increment condition changed the CFG more than
   intended.

**The untested axis: whether the FIRST call (`BuildCdFilePath`) needs a
GENUINELY different C-level expression for `path` than the loop's two later
calls** -- e.g. if retail's source takes `path`'s address into a variable
only INSIDE the loop (never before it), the first call might use a
literal `&path[0]` while the loop uses a name introduced fresh at that
point; I did not find a spelling that stops GCC treating them as the same
cacheable value while keeping the rest of the structure intact (attempt 3's
`p` was assigned before AND used for the first call implicitly via `path`,
which may have been the wrong split).

### Proposed learning

GCC 2.6.3 at `-O2` will unify a repeated array-name argument
(`path`/`&path[0]`) into ONE cached register across multiple calls,
including across an intervening function call, REGARDLESS of whether the
repetition is spelled via the bare array name or an explicit pointer
variable holding the same value. When retail does NOT do this (a fresh
`addiu` at each use, at least for the first occurrence), that is a real
residue worth chasing, not a false lead -- but the fix is not simply "avoid
naming a variable for it," since the bare-array-name form gets the same
treatment. The right lever (not yet found) is likely a structural
difference in when/how the FIRST use's address is computed relative to the
loop, not a naming choice within an otherwise-identical control-flow
shape.

---

## Round 18 (echo) — one more hypothesis, also negative

Tried using `BuildCdFilePath`'s own return value (it returns `dest`
verbatim, confirmed by reading its now-matched body) as the address
reused across the loop's later calls, instead of referencing the `path`
array by name a second time:

```c
char *p = BuildCdFilePath(path, suffix);
while (1) {
    if (CdSearchFile(&statBuf, p) != 0) { break; }
    ...
}
```
(`func_8002B640` corrected to `CdSearchFile` round 64 -- Sony's,
`lib/libcd/iso9660.o` since round 34; this fragment is illustrative/partial
and was never itself a build target, but `tools/stalesyms.py` flags any
LIVE preserved-block reference to a since-renamed name regardless.)

Hypothesis: if the loop's calls consume a value that arrived via a
CALL's return register ($v0) rather than a locally-recomputed `addiu`,
GCC might not treat it as a CSE-able address the same way. **Result: 14/43
with an address-drift warning (function came out longer than retail)** --
worse than the existing 12/43 baseline and not a clean comparison (the
drift means the per-function score isn't even trustworthy). Reverted
immediately; `build-and-verify.sh` confirms a clean revert
(`OK: build matches retail SLPS_015.56`).

This rules out "the loop's cursor comes from a call's return value rather
than a re-taken address" as the lever; combined with the existing report's
5 attempts, that's 6 total structural variations tried on the same
"path address caching" residue with no success. Not escalated further
this round (lower priority than this round's assigned four functions;
still flagged for permuter per the existing report's own suggestion, not
yet spent this round due to time).

---

## Round 47 (2026-09-16), runner delta -- rebuilt in-tree, then permuter DECLINED on check (b)

**Rebuild-before-trusting-the-score, per this round's brief.** Spliced the
preserved body (unchanged from round 36's, above) into `src/code_179d8_h.c`
in place of the `INCLUDE_ASM` and ran the real oracle:
`build exit=2`, no compile-error grep hits, `build/lsdde.map` shows
`BuildCdFilePath - OpenCdFile = 0xB0` = 44 words against retail's 43 (one
word longer, matching round 36's own note exactly). `funcdiff.py` read
13/43 raw word-match this time (round 36 recorded 12/43) -- a one-word
difference in the RAW count that does not change the verdict; the
STRUCTURE (one extra cached-address instruction, same register-role
rotation) is bit-for-bit the same class of residue round 36 already
measured. Restored to `INCLUDE_ASM` immediately after
(`diff src/code_179d8_h.c` against the pre-splice copy: identical);
`./build-and-verify.sh` confirms `OK: build matches retail SLPS_015.56`
afterward.

**Permuter pre-checks, per this round's three-point protocol -- (a) and
(b) run, (c) not reached because (b) already said no:**

- **(a) scaffold compiles and scores:** yes.
  `tools/setup-permuter.sh OpenCdFile permuter-work/seed_80028920.c`
  built clean (`scaffold built ... base compiles, target assembled`).
- **(b) insertion/deletion penalties, `--debug --stack-diffs`:** **NOT**
  near 0/0. Measured: `Insertions: 4 (100)`, `Deletions: 3 (100)`,
  `Reorderings: 1 (60)`, `Register Differences: 19 (5)`, `Stack
  Differences: 8 (1)`, **base score = 863**. The debug diff itself shows
  the exact residue this report already documents in prose, now in
  registers: retail's `s1`/`s2` map to this attempt's `s2`/`s1`
  (a genuine role swap, not a naming accident) and retail computes `a0 =
  sp+0x28` fresh for the FIRST `BuildCdFilePath` call (`addiu a0,sp,0x28`)
  where this attempt caches it into `s0` one instruction earlier and reuses
  the cached copy (`move a0,s0`) -- the identical "address computed once,
  hoisted above the first use" residue five prior attempts already
  independently rediscovered, now visible directly in the permuter's own
  diff output rather than inferred from asm-differ.
- **(c) scaffold-vs-real-build agreement:** not run as a separate step --
  the round-36 and this round's own in-tree rebuild (above) already pin the
  real build's residue (one word long, same register-role class), and the
  permuter's debug diff reproduces the SAME structural shape (address
  cached before the loop, s0/s1/s2 rotated) rather than a contradictory one
  the way the `DayTaskStageMap` family's void scaffolds did in round 46. No
  disagreement to flag.

**Verdict: search DECLINED, per the round-47 brief's explicit warning to
weigh check (b) "carefully" for this exact residue class.** Insertions=4
and deletions=3 (not 0/0) mean the permuter would need to both ADD and
REMOVE instructions relative to the current best attempt to close the
gap -- a source-MUTATION search over the current seed's expression tree is
not shaped to discover "hoist this address computation out of the loop
AND swap which callee-saved register holds it," which is a CONTROL-FLOW/
allocation-order change, not an expression rewrite. Combined with 6 prior
manual attempts (this report + the round-18 addendum) that already
independently confirm the residue is about WHEN/WHERE the address is
computed relative to the loop rather than how it is spelled, this is
recorded as NOT SEARCHED (declined on evidence), not as a negative search
result -- a different seed (one that already hoists the address correctly
and only needs the register-role swap) might score very differently, and
is worth trying before spending a real search budget here.

### Proposed learning

**Insertions/deletions being far from 0/0 is itself diagnostic, not just a
gate to clear before searching.** For this residue, the permuter's own
`--debug --stack-diffs` output named the exact same two defects (address
hoisted before the loop; register roles swapped) that five independent
hand-written C attempts had already converged on separately -- meaning the
debug diff can be read as a FREE cross-check of a "structural" verdict
before deciding whether to spend a search on it at all, not only after.

---

## Naming (round 64, runner alpha)

- **`func_80028920` -> `OpenCdFile`, tier B.** Mechanics: if not already
  open, builds the CD path (`BuildCdFilePath`), retries `CdSearchFile` up to
  100 times, and on success records the result and marks the object open.
  Not derived from this function's own (stalled) body alone: independently
  confirmed by `src/code_179d8_s.c`'s `CdDriver__Open`, which calls this
  function directly when CD-async mode is off, and otherwise reimplements
  the identical algorithm (same field offsets, same `CdSearchFile`/`CdControl`
  sequence) for its own async path. Paired with `CloseCdFile`/`GetCdFileSize`/
  `ReadCdFile` (also this unit) as an Open/Close/Size/Read quad; see those
  reports and `src/code_179d8_h.c`'s unit header comment.
- Field renames on `ObjA34_179D8H`/`StatBuf179D8H` this function reads
  (`unk0C`->`isOpen`, `unk18`->`pos`, `unk1C`->`size`; `StatBuf179D8H`
  `unk0`->`pos`, `unk4`->`size`) are recorded in `CloseCdFile.md`'s and
  `GetCdFileSize.md`'s `## Naming` sections and in `src/code_179d8_h.c`
  directly; not re-derived here.

## Round 97 (runner bravo): `StatBuf179D8H` is Sony's `CdlFILE`

The local `sp+0x10` buffer was this unit's own `StatBuf179D8H`
(`CdLoc16 pos; u32 size; u8 pad8[0x10]`, 0x18 bytes). Field by field it is
`<libcd.h>`'s `CdlFILE` -- `CdlLOC pos` (4 bytes), `u_long size`, `char
name[16]` -- and it is the output of `CdSearchFile`, whose Sony prototype
takes a `CdlFILE *`. The type was deleted and the unit now includes
`<libcd.h>`; the accessors `statBuf.pos`/`statBuf.size` keep their names as
Sony's own fields. `CdDriver::pos` is still the project's `CdLoc16`
(include/FileResource.h), so the copy is spelled `*(CdLoc16 *)&statBuf.pos`,
the round 96 precedent in `src/code_179d8_q.c`'s `ResolveFileEntries`.
Byte-exact; whole-image SHA1 green.

## Source comment history (round 99, echo, track 7)

The comment above `OpenCdFile` in `src/code_179d8_h.c` before round 99's
track 7 pass, kept verbatim; the source now keeps a one-line `MATCHING:`
note. Names as of round 98 (the parameter `suffix` is now `name`, the local
`i` is `retries`, `statBuf` is `file`).

```c
/* MATCHED round 74 (charlie). The retry loop is a label + backward goto,
 * not while/for: a real loop gets loop notes, loop.c hoists &path out of it
 * and CSEs it with BuildCdFilePath's argument (one word long, rotated
 * saved registers). Retail recomputes &path inside the loop body --
 * docs/match-reports/OpenCdFile.md. */
```

## Naming (round 99, echo, track 7)

- Parameter `suffix` -> `name`, tier A: the one caller, `CdDriver__Open`
  (`src/code_179d8_s.c`), passes its own `name`, and `BuildCdFilePath`
  appends it after the data directory and before `;1`, so it is the file
  name, not a suffix.
- Locals: `i` -> `retries` (counts the lookups after the first), `statBuf` ->
  `file` (Sony's `CdlFILE`, `CdSearchFile`'s output).
- Constants: `path[0x40]` -> `path[CD_PATH_SIZE]` (64, `include/CdDriver.h`;
  `CdDriver__Open` and `ResolveFileEntries` declare the same 0x40 buffer for
  the same `BuildCdFilePath` call). `100` -> `CD_SEARCH_ATTEMPTS - 1`:
  `retries++ < 100` searches 101 times in all, the same 101 that
  `ResolveFileEntries` spells as its local `CD_SEARCH_RETRIES 0x65`.
