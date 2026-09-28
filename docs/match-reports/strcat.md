# strcat -- CONVERTED to a linked SDK object (round 34). NOT game code.

> **ROUND 34 (2026-09-12), runner bravo. THIS FUNCTION IS NOW LINKED FROM
> SONY'S OWN OBJECT `libc2/strcat.o` (Psy-Q 3.3), WHOSE 0xA8 OF TEXT COVERS
> EXACTLY IT.** It was `game_shell`'s LAST function, so the conversion is a
> pure suffix split -- `[c game_shell 0x171E0][o libc2/strcat 0x17930]` -- with
> no new unit name, no function reordering and no rodata attach to move. The C
> body is deleted from `src/app/game_shell.c`; callers keep spelling it `strcat`
> and now resolve to the object. Whole-image SHA1 green.
>
> **This RECLASSIFIES a matched function out of the game-code count, and that
> is the correction, not a regression** (CLAUDE.md: never write C for a
> function a Sony object owns). It is also the case round 32's `sdkstalls.py`
> lesson predicts and does not catch: that tool crosses the STALL queue
> against placed objects, and this function was not stalled, it was closed.
>
> **The round-8 work below called it.** Its opening paragraph says the body
> "carries a guard textbook `strcat` has no reason to" and reads as "Psy-Q
> library defensive code rather than game code" -- a correct classification,
> three rounds before the project had a way to act on one. Everything below is
> kept as the derivation it was, not as live guidance: the C shown no longer
> compiles into the image, but the two load-bearing source shapes it found (the
> post-increment scan worth 25 words, `return dest` over `return NULL` worth
> one) are the durable finding and generalise past this function.

**Unit:** game_shell (until round 34) · **Size:** 42 instructions (0xA8 bytes) ·
**Status: was MATCHED 42/42**, whole-image SHA1 green. Closed by the head in
round 8 (2026-09-02) with the project's second permuter run.

> **Kept in full.** Everything below the RESOLUTION section is the state of
> knowledge before the fix, across two earlier rounds and a head
> adjudication. The instruction-level readings in it are accurate and the
> 25-word post-increment finding is what made the last instruction reachable
> at all; only the final classification ("compiler-internal, unreachable")
> was wrong.

## RESOLUTION — the last instruction was one word of source

The residue was retail having `move a0,s1` in the first call's delay slot
where ours had `nop` — retail restating a value that was already live. The
fix is on the NULL-`dest` guard, and it is one word:

```c
    if (dest == NULL) {
        return dest;      /* NOT `return NULL` */
    }
```

`dest` *is* null on that path, so the two spellings return the identical
value and the C is equally correct either way. But `return dest;` **uses**
`dest`, and that use keeps it live through the guard, which is what makes
the compiler establish it in `$a0` the way retail does. `return NULL;`
mentions it zero times, the value dies at the branch, and the instruction
disappears. The other two exits do return a real `NULL` and share one tail,
exactly as the earlier adjudication found.

**Measured** (pinned pipeline, instruction-text diff against retail):

| source form | diff |
| --- | --- |
| `if (dest == NULL) return dest;` | **0** |
| permuter's raw output: dead `origDest = dest;` in the NULL arm | **0** |
| `if (dest == NULL) goto fail;` (the old 41/42) | 2 |
| `char *origDest = dest;` at declaration | 36 |
| single shared `fail: return origDest;` with `origDest = NULL` init | 38 |

### The permuter's zero was NOT the answer, and this is the pattern to copy

The permuter reached zero at iteration 320 with a **dead store** —
`origDest = dest;` inside the `if (dest == NULL)` arm, on a path where
`origDest` is never read. That is precisely the duplicate-arm artifact Gate 3
warns about, and committing it would have put provably dead code in `src/`
with no explanation attached.

But it was a *lead*, and it pointed at the right thing: it said retail's
source **uses `dest` on the null path**. The idiomatic way to use it there is
to return it. That form also reaches zero, and it is code a person would
write.

**So: run the permuter, then throw its output away and keep only what it told
you.** Gate 3's "a zero is a LEAD, not an answer" is not a formality — here
the difference between the lead and the answer was dead code versus a
one-word idiom, and both scored identically.

> **HEAD ADJUDICATION, round 2026-08-30-a.** The runner filed this at 16/42 and
> classified the whole residue as an unreachable compiler-internal choice. That
> classification was **wrong for the larger of the two gaps**, and the head
> reached 41/42 by changing the source. Everything below the runner's byline is
> its original text, kept because its derivation and its ruled-out attempts are
> still accurate and still useful; read this section first for what actually
> holds. See "Head adjudication" immediately below.

## Head adjudication — most of this was source-reachable

The runner's own diff contained the evidence that its classification was too
broad, and it is worth naming the tell, because it generalises.

**The two branches had DIFFERENT TARGETS, not just different delay slots.**

```
retail:  beqz v0,17998        <- targets the `addiu s1,s1,-1` fixup
         addiu s1,v1,1            and undoes the increment on the taken path
mine:    beqz v0,179a0        <- targets PAST the fixup, skipping it
         nop
         addiu s1,s1,1
```

A differing delay slot is a scheduler choice. A differing branch *target* is a
different control-flow graph, and a different CFG comes from different source.
That is the discriminator: **before classifying a residue as a scheduler whim,
check whether the branch targets agree.** If they do not, the source shape is
still wrong and there is nothing compiler-internal about it yet.

Here the CFG says retail increments the scan pointer **unconditionally** and
then backs it up, which is the post-increment idiom, not the pre-test one:

```c
while (*dest) {     /* runner's shape  -> 16/42 */
    dest++;
}

while (*dest++) {   /* retail's shape  -> 41/42 */
}
dest--;
```

That single change is worth **25 words**. With it, every instruction in the
function matches retail except one.

### What actually remains: one redundant move

```
retail:  jal 13348 / move a0,s1     ($a0 already holds dest; retail restates it)
mine:    jal 13348 / nop
```

This *is* the class the runner named, and its reasoning about this half stands:
same register, same value, retail's delay-slot filler restates an already-live
value and ours does not. It is now isolated to exactly one instruction, which
makes it directly comparable to `New_GameApplication` (23/24, one instruction, same
phenomenon).

Nine further source shapes were built against the 41/42 body and none moved it:
`__asm__("")` before the overlap check, inside the taken branch, a named
temporary for the first length, combined null checks, `!=`-negated and
subtraction-form guards, and assigning `origDest` before the checks. Two made it
much worse (`origDest` hoisted to the top, 22/42; the overlap check written
through `origDest`, 3/42), which is itself informative — it confirms retail
copies the original pointer *after* the guard, in the `beq` delay slot.

**Disposition.** Restored to `INCLUDE_ASM`; 41/42 is not a match and no score
short of byte-exact may stay in `src/`. This is now the project's best-posed
permuter target: two instances, both one instruction, both the same phenomenon.

## What it does

Not the textbook libc `strcat`. It has a guard textbook `strcat` doesn't:
after computing `strlen(dest)` and `strlen(src)` (via a still-uncarved helper,
`func_80013348`, address only), it bails returning `NULL` if
`dest + strlen(dest) == src + strlen(src)` — i.e. the two strings' *end*
pointers coincide. It also returns `NULL` (not `dest`) if either input is
`NULL`. Otherwise it behaves like ordinary `strcat`: scans `dest` for its
terminator, then copies `src` (including the terminator) onto that point,
returning the original `dest`.

This is exactly the case CLAUDE.md's task brief calls out: a named
libc-shaped routine whose real source may be Sony's rather than the game's.
The overlap/end-pointer guard is not something an ordinary game-code
`strcat` reimplementation would have any reason to add, and reads as
Psy-Q-library-shaped defensive code.

## Derivation (control flow, confirmed correct — see Residue for the actual gap)

```
if (dest == NULL) return NULL;
if (src == NULL) return NULL;
if (dest + strlen(dest) == src + strlen(src)) return NULL;
d = <pointer to dest's terminator>;      /* while(*d) d++; idiom */
do { *d++ = c = *src++; } while (c);     /* while((*d++ = *src++)) idiom */
return dest;                              /* the ORIGINAL dest, preserved */
```

Confirmed instruction-for-instruction correct at the control-flow level: with
the C below, every non-delay-slot-filler instruction in the function matches
retail exactly, in the same order, using the same registers. The residue is
two isolated missing/differently-filled delay slots, not a structural or
register-identity problem.

## Residue — two delay-slot-filler gaps, not register or control-flow bugs

> **SUPERSEDED IN PART.** Gap 2 below was not a delay-slot-filler gap; it was a
> different CFG, and the post-increment scan loop closes it. Gap 1 stands.
> Kept for the record because the instruction-level reading is accurate.

Read with `tools/asm-differ/diff.py strcat` at the best (16/42) attempt:

**Gap 1 — a redundant register reload retail keeps, mine elides:**

```
retail:  jal 13348 / move a0,s1     (redundant: a0 already == s1 here)
mine:    jal 13348 / nop
```

`$a0` already holds `dest` unchanged since function entry at this point (the
first call, `func_80013348(dest)`); nothing between entry and here writes
`$a0`. Retail's compiler re-established it anyway with an explicit `move`,
filling the branch-delay slot that would otherwise be a `nop`. My build's
compiler recognized the value was already correct and left the slot empty.
Both are semantically identical — same register, same value — this is a
pure delay-slot-filler *choice*, not a register-identity change (nothing here
is fixable or bannable per CLAUDE.md rule 6's register-pin test, since no
register identity differs).

**Gap 2 — a delay slot retail fills, mine leaves empty, plus an operand-source choice:**

```
retail:  beqz v0,X / addiu s1,v1,1     (fills the delay slot, reads FROM v1)
mine:    beqz v0,X / nop  ...  addiu s1,s1,1   (as a separate, later instruction, reads FROM s1)
```

Both compute the identical result (`v1 == s1` at this point — `v1` was just
copied from `s1` two instructions earlier), but retail's scheduler filled the
branch's delay slot with the increment (sourcing it from the freshly-copied
`v1`, which has no dependency on the just-tested branch condition), while
mine left the slot as `nop` and emitted the increment separately afterward
(sourcing it from `s1` itself). This costs exactly one extra word, which is
also why Gap 1 and Gap 2 together shift everything after them by 2 words
total in a naive size count, though after Gap 1 alone the size differential
is 1 word and after both it is netted against retail's own count correctly
(42/42 either way once both close — this was cross-checked with
`build-and-verify.sh`'s whole-image diff, not just the isolated window).

## Why this is classified TOOLCHAIN/COMPILER-INTERNAL, not a reshaping target

> **SUPERSEDED.** This section's conclusion was wrong for Gap 2, which was
> reachable from C and is now closed. It remains correct for Gap 1 alone.

Both gaps are the same phenomenon documented in
`docs/match-reports/New_GameApplication.md` (a different unit, `game_shell`,
found independently by a different runner): GCC 2.6.3's `-O2` delay-slot
filler (`fill_eager_delay_slots`/`fill_slots_from_thread` in reorg.c-era
GCC) sometimes duplicates an already-live value into a delay slot and
sometimes doesn't, for reasons that report's author could not find a
source-level lever to control after 14+ attempts on an even simpler
function (a 24-word allocator wrapper, one delay-slot residue, single
merge point). This function has the *same* residue class, twice, in a much
larger (42-word) body with three-way branching, guard clauses, and two
independent loops — strictly harder terrain for the same problem.

## Attempts tried (did not change the residue)

> These were all run against the 16/42 scan-loop shape. They are still valid
> negative results about the guard and the two `strlen` calls, which the 41/42
> body keeps unchanged.

1. Named `s32 lenDest, lenSrc;` locals for both `strlen` results, then
   `if (dest + lenDest == src + lenSrc)` — worse (11/42): forces GCC to spill
   `lenDest` into a callee-saved register across the second `strlen` call
   (retail never does this — it reuses `$v0` directly in the *second* call's
   delay slot, before that call's own return overwrites it).
2. Inline compound expression, no named length locals:
   `if ((dest + func_80013348(dest)) == (src + func_80013348(src)))` —
   **16/42, the best result**, matches retail's delay-slot reuse of `$v0`
   across the two `strlen` calls exactly. All further attempts start from
   this shape.
3. Swapped addition operand order (`func_80013348(dest) + dest` instead of
   `dest + func_80013348(dest)`) — no change, 16/42.
4. Named `s32 lenDest` for *only* `dest`'s length, left `src`'s inline —
   no change, 16/42.
5. `char *d;` as a genuinely separate scan variable (not mutating the `dest`
   parameter itself), returning the untouched `dest` parameter directly —
   **worse (7/42)**: this flips which physical register holds the
   "preserved original pointer" vs. "advancing scan pointer" in a way that
   diverges further from retail's choice (retail mutates the *parameter's
   own* register in place for the scan, and copies the original into a
   fresh temp — the opposite of what this attempt does).
6. Bare `__asm__("");` as the very first statement of the function (per the
   head's `Pad__DispatchEvents` broadcast, Lever 1) — **worse (6/42)**. Confirms
   this residue is not the prologue-store-order class that lever addresses;
   it perturbed unrelated scheduling instead.
7. Bare `__asm__("");` immediately before the overlap-check expression
   (attempt #2's shape) — no change, 16/42. Neither helps nor hurts; the
   barrier has no effect on this specific filler choice, same as
   `New_GameApplication`'s finding that an `asm("")` barrier didn't touch its
   analogous residue either.

None of these are register-identity changes (CLAUDE.md rule 6's test: would
removing/changing the attempt move a value to a DIFFERENT register? No —
every attempt either matches retail's registers or fails outright by
choosing different ones for unrelated reasons, e.g. attempt 5). This is
consistent with the finding in `New_GameApplication.md` that this residue class
does not yield to `if`/`goto`/`return` spelling, temp-variable placement, or
scheduling barriers.

## Preserved body (41/42 — the head's shape, supersedes the runner's 16/42)

```c
#if 0
/* include/data_source.h already declares:  extern s32 func_80013348(char *s); */

char *strcat(char *dest, char *src) {
    char *origDest;

    if (dest == NULL) {
        goto fail;
    }
    if (src == NULL) {
        goto fail;
    }
    if ((dest + func_80013348(dest)) == (src + func_80013348(src))) {
        goto fail;
    }
    origDest = dest;
    while (*dest++) {
    }
    dest--;
    while ((*dest++ = *src++) != 0) {
    }
    return origDest;
fail:
    return NULL;
}
#endif
```

The runner's original body differed from this only in the scan loop
(`while (*dest) { dest++; }`) and scored 16/42.

## Head broadcast levers — applicability

- **goto-vs-return (New_Pad lever):** **applied, and it is the reason
  this function reached 16/42 rather than something much worse.** All three
  early exits return a value (`NULL`) different from the main path's
  (`origDest`) — exactly the shape the lever describes. Using `goto fail;`
  for all three, landing on a single shared `return NULL;`, matches retail's
  single shared tail (`move v0,zero` immediately before the common epilogue)
  with a single `move v0,v1`-shaped return on the success path — no
  duplicate epilogues, no extra `j`. This part of the function is fully
  correct; the residue is entirely within the delay-slot-filler class above,
  unrelated to how the early exits are spelled.
- **loop-invariant hoisting (Pad__DispatchEvents lever 2):** **checked, not
  applicable in the form described.** Neither loop here walks a named array
  against a hoisted base/end pointer — both are simple forward pointer scans
  (`while (*d) d++;` and `while ((*d++ = *s++))`) with no bound/array
  identifier for GCC to hoist in the first place. No `base`/`end` temp was
  introduced in any attempt, consistent with the lever's advice; it simply
  doesn't have a target to apply to here.
- **prologue store-order barrier (Lever 1):** **tried (attempts 6–7 above),
  did not close either gap.** Confirmed by checking register allocation
  before/after: removing the barrier changed nothing (attempt 7) and adding
  it at the top changed unrelated scheduling for the worse (attempt 6) without
  touching the two delay slots this report is about — this residue is a
  delay-slot **filler** choice (which independent instruction gets moved into
  an already-existing slot), not a prologue callee-save **store order**
  choice (which stack slot gets written first), so the lever's mechanism
  doesn't reach it either way.

## Proposed learning

> The three entries below all still hold; the third one's conclusion
> ("resists reshaping, best-posed permuter target") was acted on this round
> and the permuter closed it. Added on top:

**Counting mentions beats reasoning about schedulers.** Both one-instruction
residues closed this round were a mismatch in how many times the SOURCE
mentions a value, not a scheduling choice:

- `New_GameApplication` mentioned its return value once too MANY (a `return` on a
  path where the value was already in `$v0`) — the fix removed a mention.
- `strcat` mentioned `dest` once too FEW on the null path (`return NULL`
  where retail returned the pointer itself) — the fix added a mention.

Same phenomenon, opposite directions, and in both cases the surplus or
missing copy landed in a delay slot, which is what made both look like
delay-slot filler choices for three rounds. **When a diff is one redundant or
one missing `move`, count where the value is mentioned in your source before
theorising about `reorg.c`.**

**Check whether the branch TARGETS agree before classifying a residue as a
scheduler or delay-slot choice.** A differing delay slot is a scheduling
artifact; a differing branch target is a differing control-flow graph, and a
differing CFG always comes from the source. Here that distinction was worth 25
words, and reading past it produced a confident "compiler-internal, unreachable"
classification for a residue that a one-line source change closed.

**`while (*p) { p++; }` and `while (*p++) { } p--;` are different code.** The
post-increment idiom increments unconditionally and backs up at the merge, so
its guard branch targets the fixup; the pre-test idiom skips it. When retail's
guard branch jumps to an `addiu rN,rN,-1`, the source used post-increment.

**A genuinely isolated one-instruction residue class now has two instances.**
Retail's delay-slot filler restates an already-live value (`move a0,s1` where
`$a0` already holds it); ours emits `nop`. Same register, same value, no
control-flow difference, and the branch targets agree — which is what makes
this one a real instance of the class rather than a misread. See
`New_GameApplication.md` (23/24) for the other. Both are one instruction, both
resist reshaping and barriers, and together they are the project's best-posed
permuter target.

## History moved from src/code_171e0.c (round 99, charlie, track 7)

This block sat at the end of `src/code_171e0.c`, where `strcat` used to be
defined, until the track-7 pass. It is moved here unchanged:

> ROUND 34: `strcat` (0x80027130, this unit's last function, 42 words) LEFT
> THIS FILE. It is Sony's -- `libc2/strcat.o`, Psy-Q 3.3, 0xA8 of text
> covering exactly it -- and the unit's segment now ends at 0x17930 with an
> `o` entry after it. It had been matched as C since round 8, and the head
> had noticed at the time that it reads as library code rather than game
> code ("carries a guard textbook strcat has no reason to"); it was right,
> and the reclassification is the correction CLAUDE.md asks for, not a
> regression.
>
> The C body and the two load-bearing source shapes it turned on (the
> post-increment scan, worth 25 words; `return dest` rather than
> `return NULL` on the NULL-dest path, worth one) are preserved in full in
> docs/match-reports/strcat.md. Nothing is lost by deleting them here.
>
> Callers in this unit (BuildFileName, just above) keep calling `strcat`
> under that name -- the declaration in include/code_171e0.h still serves,
> and now resolves to the linked object.
