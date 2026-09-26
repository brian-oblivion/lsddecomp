# SceneNode__NotifyTaggedParents — MATCHED (round 76, bravo: 54/54 on the first build of the new shape, whole image green; was 48/54 length-exact)

> Renamed from `Class6B5CC__NotifyTaggedParents` on 2026-09-26 (tools/rename.py). Address 0x8001e4a4.

> Renamed from `func_8001E4A4` on 2026-09-18 (tools/rename.py). Address 0x8001e4a4.

Unit: `code_d294_b`. Round 13, runner delta; improved round 19 (echo), see
that section at the end. Best score reached: 48/54 words
in-range, build clean at that point (verified: this score was read with
`build exit=0` before reverting). ~15 real attempts (round 13) + a further
round-19 pass. Restored to
`INCLUDE_ASM` per project rule — no score short of byte-exact stays in
`src/`.

REVISITED, round 76: MATCHED 54/54 (build exit=0, whole-image SHA1 green); names/types not relevant -- the fix is loop KIND (goto -> two nested do/while), with the literal 4 unnamed.

## Round 76 (bravo): MATCHED -- the goto CFG was the wall

**Rebuild first.** The round-19 preserved body rebuilt live: **48/54,
insertions 0 / deletions 0, positional skeleton diffs 6**, `nm -S` `0xd8`,
the same as retail. So `plan.py`'s "length off" reading was wrong: length is
exact. The six diffs are the `$s1`/`$s2` exchange of `self` and the literal
4 (prologue save and set, `beq`, `move a1`).

**The lever (loop kind).** Retail's `ori s1,zero,4` is a loop.c invariant
hoist. A constant that sits in a callee-saved register and is read only by a
`beq` inside the loop is what loop.c's move_movables produces. But loop.c only
sees loops that have NOTE_INSN_LOOP_BEG/END, that is SYNTACTIC loops. Every
earlier round wrote this function as a `goto` CFG. That form has no loop for
loop.c to optimise, so the 4 was rematerialized (the 8/54 shape round 13
started from) until round 13 named it `tag = 4`. A named local set at entry
is a different pseudo, with different refs and live length, and global-alloc
priority put it after `self`. That is why no assignment order, declaration
order or barrier in rounds 13, 19, 20, 37, 41 or 55 (plus 73118 permuter
iterations) could fix it. None of them changed the loop kind, and a permuter
never turns a goto into a loop.

The body is two nested do/while loops: an inner "find the next kind-4 entry"
loop that leaves by `goto found`, and an outer loop over the rest of the
refs. The 4 is a bare literal:

```c
entry = NULL;
do {
    do {
        BasicClass__GetNextParentRef(node, &entry, &cursor);
        if (entry != NULL && (entry->methods->header & 0xF) == 4) {
            goto found;
        }
    } while (cursor != NULL);
    entry = NULL;
found:
    if (entry != NULL && *(u8 *)entry->methods == 0x34) {
        entry->methods->slot10(entry, self);
    }
} while (cursor != NULL);
```

This matched 54/54 on its first build, with no `s`/`n`/`tag` copies. Round
13's `for(;;)`/`continue`/`break` attempt (9/54, four saved registers) was a
different CFG. It was not a proof that syntactic loops cannot work. One
build, no permuter, so Gate 3 was not needed.

### Proposed learning (round 76)

**A constant in a callee-saved register, set in the prologue and read only
by a compare inside a loop, is a loop.c hoist, so the loop must be
SYNTACTIC.** A `goto` loop has no loop notes, and loop.c never touches it.
Naming the constant (`tag = 4`) gets the register COUNT right but not the
register ORDER: global-alloc priority depends on the pseudo's refs and live
length, and a named local has different ones from a hoisted constant.
Screen: a `li sN,K` in the prologue with no other use of K, and a goto-form
body. Rewrite the loop as `do/while` before touching locals. This is the
mirror of echo's round-76 finding (a goto loop AVOIDS strength reduction):
loop kind decides what loop.c does, in both directions.

## History (rounds 13-55)

## Signature (as attempted)

```c
void SceneNode__NotifyTaggedParents(SceneNodeObj *self, void *node);
```

Confirmed from the disassembly's own register roles, not guessed: `a0`
(SceneNode__NotifyTaggedParents's 1st param) is forwarded, unmodified, as the 2nd argument to
the final dispatch call (`entry->methods->slot10(entry, self)`) — a "self"
role. `a1` (2nd param) is what gets walked via `BasicClass__GetNextParentRef` —
the "node" being scanned for tag-4 parents.

## What it does

Walks `node`'s parent-ref list (via `BasicClass__GetNextParentRef`, already
matched in `code_8220`) looking for entries whose header's low nibble is 4.
For each such entry whose header's FULL low byte is also `0x34`, dispatches
the entry's own `+0x010` vtable slot as `(entry, self)`. Keeps scanning the
whole list (there can be more than one qualifying entry) rather than
stopping at the first match.

```c
void SceneNode__NotifyTaggedParents(SceneNodeObj *self, void *node) {
    SceneNodeObj *s;
    void *n;
    GenericObj_d294 *entry;
    void *cursor;
    s32 tag;

    s = self;
    n = node;
    tag = 4;
    entry = NULL;
loop:
    BasicClass__GetNextParentRef(n, &entry, &cursor);
    if (entry == NULL) {
        goto check_cursor;
    }
    if ((entry->methods->header & 0xF) == tag) {
        goto dispatch;
    }
check_cursor:
    if (cursor != NULL) {
        goto loop;
    }
    entry = NULL;
dispatch:
    if (entry == NULL) {
        goto tail;
    }
    if (*(u8 *)entry->methods != 0x34) {
        goto tail;
    }
    entry->methods->slot10(entry, s);
tail:
    if (cursor != NULL) {
        goto loop;
    }
}
```

The control flow (the `goto` graph above) is confirmed byte-exact against
retail: every branch target, every instruction in the body except the
prologue/epilogue register-save shuffle, and the frame size (0x28, 3
callee-saved regs + `ra`) all match exactly. The gap is purely which of two
values — `self` and the literal `4` — lives in `$s1` vs `$s2`.

## The residue: a clean 2-value register-identity swap

Retail's register assignment: `$s0` = `node`, `$s1` = the constant `4`
(loaded once, `ori $s1,$zero,0x4`, in the *prologue*, and read every loop
iteration via `beq $v0,$s1,...` — a register-register compare, never a
fresh `li`), `$s2` = `self` (read exactly once, at the very end).

The best C reached here gets `node` correctly into `$s0`, but swaps the
other two: `self`→`$s1`, `tag`(4)→`$s2`. Every instruction is otherwise
identical — same opcodes, same offsets, same branch targets — just this one
pair of callee-saved register numbers exchanged, which also flips the
matching pair of prologue/epilogue save/restore instructions.

### What moved the score, and what didn't

Starting point (literal `4` in the comparison, `self`/`node` used directly,
no extra locals): 8/54, and critically only **2** callee-saved registers
allocated (`s0`,`s1`) instead of retail's 3 — the literal wasn't being
kept in a register across the loop at all.

1. **Extracting the literal into `s32 tag = 4;`** (a named local, assigned
   before the loop): 8/54 → 42/54. This got the register *count* right (3
   saved regs) by making GCC treat the repeated comparison value as
   something worth caching across the loop-carried call, matching retail's
   own choice to cache it rather than re-issue `li` each iteration. Without
   this extraction, GCC never promotes the bare literal to a callee-saved
   register at all (confirmed twice — reverting the extraction on later,
   better attempts always dropped straight back to the 2-register,
   ~8-10/54 shape).
2. **Copying `self` into a local `SceneNodeObj *s;` before the loop, and
   using `s` (not `self`) at the one dispatch call site**: 42/54 → 45/54.
3. **Also copying `node` into a local `void *n;` before the loop, and using
   `n` (not `node`) in the `BasicClass__GetNextParentRef` call**: 45/54 → 47/54,
   and this is what fixed `node`'s OWN register to `$s0`, matching retail.

So indirecting every one of the three "needs a callee-saved register"
values through its own named local, assigned in the order `s = self; n =
node; tag = 4;`, got 2 of the 3 assignments (`node`→`s0`, and getting the
COUNT of registers right) to match retail exactly, and left exactly one
pairwise swap (`self`↔`tag` between `$s1`/`$s2`).

### What did NOT move the score further (all tried, all reverted)

- Every permutation of the three assignment statements' ORDER (`s=self;
  tag=4;n=node;` and five other orderings) — only the specific order in the
  source above reaches 47; every other tried ordering regressed to 42, 43,
  or worse.
- DECLARATION order of the four locals (`s`,`n`,`entry`,`cursor`,`tag` in
  various orders) — zero effect on codegen once assignment order was fixed,
  consistent with C89 declarations only affecting scope, not RTL emission
  order.
- A bare `__asm__("");` as the function's FIRST statement (the permitted
  scheduling-barrier lever for prologue store-order residues) — made it
  WORSE (41/54), reordering the `ra` save relative to the others without
  fixing the `self`/`tag` identity swap.
- Restructuring the `goto`-based flat loop into an equivalent `for(;;)` with
  `continue`/`break` — much worse (9/54, plus a whole different frame size:
  GCC synthesized a 4th callee-saved register and a completely different
  block layout). The `goto` form is required just to keep the CFG shape
  retail has; this was expected going in, per the file's own comment, and
  confirmed rather than assumed.
- Extracting `entry->methods` into its own temp pointer at the dispatch
  site (`GenericMethods_d294 *m = entry->methods;`) — no change (still the
  same 42/54 baseline it was tested against).
- Swapping the comparison operand order (`tag == (...)` vs `(...) == tag`)
  — no change to the score, only a cosmetic difference in the one
  already-differing `beq` instruction's encoding.

## New class knowledge (kept — measured from disassembly, independent of
whether the function itself matched)

- `GenericMethods_d294` gained a new slot at `+0x010`:
  `void (*slot10)(GenericObj_d294 *self, SceneNodeObj *arg)`. **Caught a
  self-inflicted struct-layout bug while deriving this**: the first attempt
  at this edit put `slot10` immediately after `header` with no padding,
  silently shifting the already-matched `slot50` from its correct `+0x050`
  to `+0x044`. This didn't fail the build (compiles clean either way) —it
  broke the WHOLE-IMAGE SHA1 by one byte, in `SceneNode__DetachAttachedChildren` (`code_d294.c`,
  a different unit, already matched, calling `entry->methods->slot50`).
  Caught by running the full `./build-and-verify.sh` and then `cmp -l
  build/SLPS_015.56 disk/SLPS_015.56` to localize the single differing byte
  to `0x8001D24C`, inside `SceneNode__DetachAttachedChildren`. Fixed with the missing
  `pad004[0x010-0x004]`. **This is exactly the scenario CLAUDE.md's vtable
  slot-retype warning describes** ("check every other caller first... a
  slot retype that breaks another function shows up as a red build, not as
  a diff in the function you are working on") — except here it wasn't even
  a retype, just an ordinary new-field insertion with a forgotten pad, and
  it still broke a sibling unit silently until the full-image oracle was
  re-run. See Proposed learning below.
- `BasicClass__GetNextParentRef` (already matched, `src/code_8220.c`) is called
  directly here (not through a vtable) — same "verbatim inherited BasicClass
  method, called by symbol" pattern already established for
  `BasicClass__Release` etc. Declared locally with this unit's own
  opaque/pointer types rather than `#include "code_8220.h"`.

## Header changes kept

`include/code_d294.h`:
- `GenericMethods_d294`: added `pad004[0x010-0x004]` + `slot10` (see above;
  the padding fix is the load-bearing part).
- New extern `BasicClass__GetNextParentRef(void *self, GenericObj_d294 **outParent,
  void **cursor)`.
- Prototype `void SceneNode__NotifyTaggedParents(SceneNodeObj *self, void *node);` left in
  place even though the function is back to `INCLUDE_ASM` — harmless (no
  caller references it yet), and saves the next attempt from re-deriving
  the signature.

## Proposed learning

**A new struct field inserted without its leading padding does not fail the
build — it silently shifts every field after it, and the failure surfaces
as a whole-image SHA1 mismatch with NO diagnostic pointing at the struct
edit.** This is a variant of CLAUDE.md's vtable-retype warning, but broader:
it isn't specific to retyping a *shared vtable slot* — an ordinary "add a
newly-discovered field to a struct that's shared by another already-matched
function elsewhere" carries the identical risk, and is easy to trigger by
simply forgetting one `u8 padNN[...]` line when splicing a new member
between an existing field and its neighbor. The check that catches it is
exactly `./build-and-verify.sh`'s exit code plus, if it fails with no
compile error, `cmp -l build/SLPS_015.56 disk/SLPS_015.56 | head` to
localize the single differing byte to a VRAM address (`file_offset -
0x800 + 0x80010000`) and from there to a function via `build/lsdde.map`.
Worth adding to CLAUDE.md's "four ways a score lies" family as a fifth:
*a struct edit for one function's sake can silently break a DIFFERENT,
already-matched function's build, with zero signal in that function's own
`build exit=`/funcdiff output* — the signal only ever shows up in the
whole-image byte count, which is why the loop insists on re-running the
full oracle after every source change, not just the one you were editing
for.

**On the register-identity residue itself:** confirms and extends the
existing "declaration order or lifetime difference" guidance. Here, it
wasn't declaration order that mattered (verified: zero effect from
permuting it) — it was **whether each value-that-needs-a-callee-saved-
register was indirected through an explicit local assignment, and in what
ORDER those assignments were written**, even when the local is otherwise
functionally inert (`s = self;` then using `s` in place of `self` at the
one call site changes nothing about the function's behavior). Two of three
such indirections, in the right order, closed 39 of 46 missing words; the
third pairwise swap did not yield to any further reordering tried. Given
this is a clean, isolated 2-register permutation with an otherwise
byte-identical body, it reads as squarely within the "register identity...
if reshaping does not move it, it is a stall" class from
DECOMPILATION_LEARNINGS, now with a data point that reshaping CAN close
most but not all of such a gap before running out of orderings to try.

## Round 19 (echo): one more word closed (47/54 -> 48/54) via the
## split-combined-expression axis; verdict otherwise unchanged

Re-verified the round-13 47/54 claim first: rebuilt with the preserved
body verbatim, `funcdiff.py` confirmed 47/54 with the exact residue
described (the `self`/`tag` pair swapped between `$s1`/`$s2`), no
contamination.

Tried the axis that closed the sibling register-pair-swap residue
`GetSetBitField` (`docs/match-reports/GetSetBitField.md`, same project,
round 18): splitting a combined expression into two statements. Applied
it to the tag-comparison test, which combines a mask and an equality
check in one expression:

```c
/* was: if ((entry->methods->header & 0xF) == tag) */
masked = entry->methods->header & 0xF;
if (masked == tag) {
    goto dispatch;
}
```

(`masked` a new `s32` local, declared alongside `tag`.) Result: **48/54,
no size drift** — one additional word closed relative to the round-13
best, with the SAME residue shape remaining (the `self`/`tag` pair still
swapped across the same 6 words: the two prologue saves, the `ori`
loading the constant `4`, the loop's `beq` comparison register, and the
final call's `self` argument register).

Tried extending the same lever to the OTHER combined expression in the
function (`*(u8 *)entry->methods != 0x34`, the dispatch-eligibility
check) the same way — **worse, back down to 47/54** (and a new word
diverged that hadn't before). Reverted that half immediately, keeping
only the tag-comparison split. Also tried swapping the three assignment
statements' order (`tag = 4;` moved before `s = self; n = node;`, since a
new local existed to interact with) — **worse, 47/54 with a different
diff pattern** (word 2 area shifted). Reverted; kept the original
`s = self; n = node; tag = 4;` order this report's round-13 section
already established as the only one that scores highest.

The remaining 6-word residue (all still describable as the single
`self`<->`tag` `$s1`/`$s2` swap) did not move further under this round's
attempts. Filing as STALL at 48/54 (up from 47/54), `INCLUDE_ASM`
restored; preserved body below updated to the new best.

### Proposed learning (round 19)

**The split-combined-expression axis that fully closed `GetSetBitField`'s
register-pair swap only PARTIALLY closed this one (47/54 -> 48/54, not
54/54), and applying the SAME axis to a second combined expression in the
same function made things worse rather than compounding the gain.** A
lever that helps once is not guaranteed to help again on a second,
textually similar expression in the same body — treat each combined
expression as its own experiment rather than assuming the axis
generalizes within a function just because it worked on one expression
in it.

## Round 20 (alpha): three more axes tried, all negative; per the head's
## guidance, not ground further given the pattern

Re-verified 48/54 first (matches exactly, no drift). Tried three axes not
in the prior two rounds' tables:

1. **Drop the `s = self;` indirection, use `self` directly at the
   dispatch call site** (keeping `n`/`tag`'s indirection): **worse,
   43/54** -- the register COUNT itself regressed (a 4th callee-saved
   register class no longer matched retail's 3), not just the swap.
   Confirms round 13's finding that `self`'s own indirection is load-
   bearing, from the opposite direction (removing it, not adding it).
2. **Retype `tag` from `s32` to `u8`** (it only ever holds the literal 4,
   and is compared against `entry->methods->header & 0xF`, itself
   naturally `<= 0xF`): **no change, 48/54, byte-identical diff.** The
   type-narrowing lever that closed one function in round 18
   (`TaskObjF__WriteMemcardSaveFile`) does nothing here.
3. **A second `__asm__("")` immediately before the loop label** (after
   `entry = NULL;`, in addition to the existing barrier before `s =
   self;`): **no change, 48/54, byte-identical diff** -- inert, not
   harmful, consistent with round 13's finding that a second barrier
   elsewhere in this same function was actively harmful in ONE position
   (right after `s=self;n=node;`) but neutral in this different one.

Per the head's round-20 guidance ("give #1 an honest attempt, but do not
grind it if it presents as the same wall") and consistent with this
report's own two prior rounds, this residue continues to resist every
axis tried across three rounds now (13 assignment/declaration/barrier
variants in round 13, split-expression in round 19, these three in round
20). Filing unchanged as STALL at 48/54, `INCLUDE_ASM` restored, `src/`
confirmed clean after each test.

### Proposed learning

No new lever found; recording the negatives so a future attempt doesn't
re-try them. This function's `self`/`tag` `$s1`/`$s2` swap has now
survived: all 6 assignment-order permutations, declaration-order
permutations, a `goto`-vs-`for` restructure, a `methods` temp extraction,
comparison-operand-order, a first-statement barrier (positive, partial),
a second barrier (harmful in one position, neutral in another), the
split-combined-expression axis (partial), dropping `self`'s own
indirection (harmful), and retyping `tag` to `u8` (neutral). This is a
strong candidate for genuinely resistant register-identity -- the kind
CLAUDE.md's "if reshaping does not move it, it is a stall" framing was
written for.

## Preserved near-miss body (updated, round 19, 48/54)

```c
#if 0
void SceneNode__NotifyTaggedParents(SceneNodeObj *self, void *node) {
    SceneNodeObj *s;
    void *n;
    GenericObj_d294 *entry;
    void *cursor;
    s32 tag;
    s32 masked;

    s = self;
    n = node;
    tag = 4;
    entry = NULL;
loop:
    BasicClass__GetNextParentRef(n, &entry, &cursor);
    if (entry == NULL) {
        goto check_cursor;
    }
    masked = entry->methods->header & 0xF;
    if (masked == tag) {
        goto dispatch;
    }
check_cursor:
    if (cursor != NULL) {
        goto loop;
    }
    entry = NULL;
dispatch:
    if (entry == NULL) {
        goto tail;
    }
    if (*(u8 *)entry->methods != 0x34) {
        goto tail;
    }
    entry->methods->slot10(entry, s);
tail:
    if (cursor != NULL) {
        goto loop;
    }
}
#endif
```

## Round 37 (echo): first permuter search, clean negative

Rebuilt the round-19 preserved body live first (per this round's "build
the inherited body before trusting its score" discipline): reproduces
exactly, `build exit=2`, no compile errors, `funcdiff.py` confirms 48/54
with no length-drift warning (this function's own window is trustworthy).
This was this function's **first-ever permuter search** -- one of the
round's identified never-searched near-misses, despite already carrying
three prior rounds of hand-lever attempts.

`tools/setup-permuter.sh SceneNode__NotifyTaggedParents <seed>` scaffolded cleanly (seed:
the round-19 48/54 body, `#include "code_d294.h"` for the project's own
struct/extern declarations rather than re-declaring them locally, since
this unit already shares that header). `--debug --stack-diffs` sanity
check: **base score = 38** (8 stack-difference points, 6
register-difference points, **0 insertions, 0 deletions** -- the cleanest
possible scaffold signature: a pure register/frame-shuffle with no
structural difference at all, exactly matching this report's own framing
of "every instruction otherwise identical, just one register-pair
swapped").

Ran the bounded search: `timeout 900 ... permuter.py -j 6 --stop-on-zero
--best-only --stack-diffs permuter-work/SceneNode__NotifyTaggedParents`, rc captured on
the very next command. **rc=124** (900-second bound fired; nothing
external killed it). **73118 iterations** -- the highest iteration count
of any search this round, consistent with this being the smallest/fastest
function to recompile of the three searched. **No candidate ever beat the
base score of 38** -- confirmed two ways: the raw score log's minimum
value across the entire run is 38, and `permuter-work/SceneNode__NotifyTaggedParents/`
contains no `output-*` directories at all (the permuter only creates one
when `--best-only` finds something strictly better than the running
best). This is the same "no candidate ever improved on the seed" signature
`CD_ready` produced earlier this round.

**Verdict: not closed in 73118 iterations under load (rc=124); the
permuter found nothing better than the base score at any point.** This is
consistent with -- and independently strengthens -- this report's own
three-round conclusion that this is genuinely resistant register-identity,
not an unexplored source-shape question: a full bounded permuter search,
which explores far more of the source-shape space than any hand-driven
session could in the same time, still found zero improvement on a residue
that is ALREADY known (per the round-13/19/20 sections above) to respond
partially to some reshapes and not at all to others. Filing as STALL,
figures unchanged (48/54), `INCLUDE_ASM` confirmed restored. This function
has now had a full round-13 hand-lever sweep, a round-19/20 continuation,
AND a full bounded permuter search all converge on the same 6-word
residue -- about as strong a case for "genuinely resistant register
identity" as this project's tooling can currently produce.

### Proposed learning (round 37)

**A permuter search's `--debug` scaffold check can itself be diagnostic
evidence, independent of the search outcome.** A base score with zero
insertions and zero deletions (as here, 38 = pure stack+register penalty)
confirms a residue is EXACTLY what the report already claims -- a clean
register/frame permutation with no missing or extra instructions -- before
spending any search budget on it. That is a useful confirmation step in
its own right for any function classified as "register identity," not
just a sanity gate to get past before the real work starts.

## Round 41 (bravo): dead-reload lever checked by hand, explicit negative

This round's head broadcast a new lever discovered on a different unit
(`Class866E8__BuildRateEntries`/`Class866E8__LoadElementResources`, `code_55dd4`): a source-level dead
reload (`p = x->y; ... p->z = 0;` where `p` already holds `x->y` from
earlier in the same block) can, once removed, reshape register
allocation across the WHOLE enclosing block and close unrelated words
elsewhere. Per this round's assignment, checked this function's
round-19 preserved body (48/54) for the same shape BEFORE re-running
round 37's already-exhausted 73118-iteration permuter search.

**Rebuilt the preserved body live first**, per this round's "build any
inherited body before trusting its score" discipline: reproduces
exactly, 48/54, `build exit=2`, no compile errors, no drift warning --
matches this report's own figures with no contamination.

**The lever does not apply here, and the negative is clean, not just
"didn't find one":**

- Every one of the four locals that gets its own callee-saved register
  (`s`, `n`, `tag`, plus `entry`/`cursor` on the stack) is assigned
  EXACTLY ONCE and read at exactly one call site each (`s` at the
  dispatch call, `n` at the `BasicClass__GetNextParentRef` call, `tag` in the
  loop's comparison). None of them is a pointer-chain value that gets
  reloaded through a DIFFERENT expression later after already being
  captured in a local -- the dead-reload shape requires exactly that
  ("`p` already holds `x->y`... `p->z=0`" reloads the SAME chain via the
  original path instead of through `p`), and no such pair of expressions
  exists in this function's body.
- The one field that IS read more than once, `entry->methods` (read at
  the loop's masked-comparison line, then again at the dispatch
  eligibility check, then again at the `slot10` call), is not a source-level
  redundancy needing a manual fix -- reading retail's own disassembly
  (`asm/nonmatchings/code_d294_b/SceneNode__NotifyTaggedParents.s`) shows GCC 2.6.3
  ALREADY reuses the SAME register (`$a1`) across the eligibility check
  (`lbu $v1,0x0($a1)`, `0x8001E530`) and the `slot10` load
  (`lw $v0,0x10($a1)`, `0x8001E540`) with no re-fetch of `entry->methods`
  in between -- confirmed byte-identical in the current 48/54 build at
  those exact addresses. There is no reload to remove here; the compiler's
  own CSE already matches retail exactly on this axis, which is why this
  report's original round-13 section could already say "every instruction
  ...  except the prologue/epilogue register-save shuffle... match
  exactly."
- The four `lw`/`lbu` reloads of `entry` itself (from its stack slot,
  `0x10($sp)`) at `0x8001E4D8`, `0x8001E504`, `0x8001E518`, `0x8001E550`
  are NOT a source-level redundancy either -- they are the necessary
  re-reads of an out-parameter written by `BasicClass__GetNextParentRef(n,
  &entry, &cursor)` on each loop iteration, and retail performs the
  identical four reloads at the identical addresses (confirmed by
  reading the `.s` directly, not inferred). Removing or caching any of
  them would change behavior, not just allocation.

**Explicit answer: the dead-reload lever does NOT apply to this
function.** The entire body outside the 3-instruction prologue/epilogue
register-save shuffle is already byte-identical to retail (established
in round 13, reconfirmed here); there is no redundant load anywhere in
the preserved C for the lever to remove. This is consistent with round
37's `--debug --stack-diffs` finding that the base permuter score here
is a PURE 38-point stack+register penalty with zero insertions and zero
deletions -- a lever that works by eliminating an instruction (an
insertion/deletion-class fix) has nothing to act on in a residue that is
already structurally minimal. Filing unchanged as STALL at 48/54,
`INCLUDE_ASM` confirmed restored, `git diff --stat src/code_d294_b.c`
clean after the check (verbatim re-restore, byte-for-byte identical to
the committed state).

### Proposed learning (round 41)

**The dead-reload lever is scoped to residues that still contain a
removable instruction (an insertion/deletion, not a pure
register/stack-shuffle).** This function is the cleanest possible
control case: a `--debug` scaffold with 0 insertions and 0 deletions
(round 37) meant there was, by construction, no dead load left to find --
and hand-reading the preserved body against retail's own disassembly
confirms it directly (every multi-read field access already reuses a
register exactly as retail does). Worth recording alongside the lever's
positive results so the next round doesn't spend a check on a function
whose own scaffold history already rules it out: check the DEBUG
insertion/deletion counts first, and treat a 0/0 scaffold as a signal
that dead-reload-class levers have nothing to remove, before re-reading
the body by hand to confirm.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only, and NOT attempted for a match this
round** (this unit's assignment explicitly excludes matching this
function -- it is a live `INCLUDE_ASM` stall handled by a different
track; naming only). Proposed name: `SceneNode__NotifyTaggedParents`
(tier B). Walks `node`'s parent-ref list (via the already-matched
`BasicClass__GetNextParentRef`, `code_8220`) looking for entries whose
header's low nibble is `4`; for each such entry whose header's FULL low
byte is also `0x34`, dispatches the entry's own `+0x010` vtable slot as
`(entry, self)`, continuing to scan the WHOLE list rather than stopping
at the first hit. "NotifyTaggedParents" describes the measured
mechanics (scan parents, filter by a tag byte, dispatch to matches);
the game-level meaning of tag `4`/`0x34` and what the `+0x010`
dispatch actually does to `entry` is not established. This is also the
class's own table-slot BOUNDARY -- `code_d294_c.c`'s own file banner
already documents "`tools/classtable.py gSceneNodeMethods` stops at
SceneNode__NotifyTaggedParents" -- i.e. it is `SceneNodeMethods`'s LAST slot
(`+0x0B4`), not evidence of anything about this function's own
purpose beyond position. Held back from an actual rename because this
symbol is referenced (in a comment) from `src/code_d294_c.c:13` -- a
different unit's own file banner, making exactly that boundary
observation. Posted to the broadcast.

(Later renamed for real in round 54's own second head commit, "apply
bravo's eight held-back cross-unit function renames" -- the function is
`SceneNode__NotifyTaggedParents` in `src/` as of round 55.)

## Round 55 (charlie): REVISITED (round 55) -- confirmed unchanged, 48/54

Assigned as a track-1 REVISIT job per FINISHING-PLAN.md's revisit rule: this
unit passed track 3 naming last round (round 54), so every score on file here
predates that naming. Rebuilt the round-19 preserved body live first, per this
round's "build any inherited body before trusting its score" discipline:
reproduces exactly, `build exit=2`, no compile errors, `funcdiff.py` confirms
**48/54**, no drift, the exact same `self`/`tag` `$s1`/`$s2` swap this report's
round-13/19/20/37/41 sections already describe.

**Explicit answer to the revisit's own question: the round-54 renaming gave
NO new shape here.** This function's own body references `GenericObj_d294`,
`GenericMethods_d294::slot10`, `SceneNodeObj`, `BasicClass__GetNextParentRef` --
none of those symbols were touched by round 54's naming pass (which renamed
only seven `SceneNodeMethods` vtable slots -- `getRotMatrix`,
`readUnk20Data`, `transformAndNotifyParents`, `tryAttachNearby`,
`composeAndApplyRotation`, `checkBoundsOverlap`, `classifyAgainstPlanes` --
plus eight cross-unit function symbols, none of which this function calls or
is called through). The naming pass changed nothing about ANY type, offset,
or struct this function reads. This is a clean instance of "the changed state
named in the brief does not reach this particular function" -- worth
recording since the revisit rule's own justifying precedent (four warm bodies
in the sister project) was about types actually used by the function, not
merely present somewhere in the unit.

Checked DECOMPILATION_LEARNINGS 3d's newest entry (round 53, "the 'mention a
value twice' lever needs a value with a genuine SECOND, INDEPENDENT USE
POINT") against this function's residue before spending a build: `self`
(here, `s`) and `tag` each have exactly ONE static use site (the one dispatch
call and the one loop comparison, respectively) -- neither qualifies as a
lifetime-splitting candidate, so the lever does not apply here. This was
confirmed by inspection, not by a build, since the precondition (a second,
independent use point) is visibly absent from the 12-line function body.

No new attempt was made beyond the live re-verification and this
applicability check -- this residue has now survived a full round-13
hand-lever sweep (13 assignment/declaration/barrier variants), round-19's
split-expression axis, round-20's three further axes, a 73118-iteration
permuter search (round 37) that never beat the base score, and round-41's
dead-reload-lever check (ruled inapplicable, clean negative). Filing
unchanged as STALL at 48/54, `INCLUDE_ASM` confirmed restored,
`git diff --stat src/code_d294_b.c` empty after the check.

REVISITED (round 55): confirmed unchanged at 48/54; round-54 naming reached
none of this function's own symbols; no new lever found or attempted beyond
an applicability check of the one new lever added to DECOMPILATION_LEARNINGS
since this function's last attempt (round 53's "mention twice", ruled
inapplicable by inspection).

### Proposed learning (round 55)

**A REVISIT job's premise -- "the unit's types changed, re-derive from them"
-- can be true of the UNIT while being false of the specific function
assigned.** This unit's track-3 pass renamed seven vtable slots and eight
cross-unit functions; this function's entire body touches none of them. The
revisit rule should be read as "check whether the changed state reaches this
function's own symbol set" before spending a build, not as a blanket
license to assume new shape exists -- confirming absence by reading the
symbol list cost nothing, re-deriving blind would have cost a full attempt
cycle for a function whose inputs provably did not change.

NON_MATCHING body promoted, round 69
