# BasicClass__func_18040

**Unit:** code_8220 · **Size:** 31 instructions · **Status:** MATCHED (31/31 words)

BasicClass vtable slot `+0x018` (`removeAllChildren`) — see
`BasicClass__BasicClass.md` for the class's overall design. This was the
hardest of this round's 8 functions; recorded in detail because the
winning shape is non-obvious and generalizes.

## What it does

Iterates `self->children` via `GetNextBasicClass` (still `asm/code_8220_b.s`,
not yet carved to C — see signature below), calling `removeChild` (slot
`+0x014`, this unit's `BasicClass__func_17ff0`) on each extracted child
until the list is exhausted.

## `GetNextBasicClass`'s signature, established here

`GetNextBasicClass(BasicClass **outValue, BasicClassListNode **cursor)`,
`void`. Pop-and-advance: reads `node = *cursor`; if null, `*outValue = 0`;
else `*outValue = node->value`, `*cursor = node->next`. Does NOT free the
popped node (that's `removeChild`'s/`func_80018208`'s job, which is called
separately on each extracted value by THIS function's caller-facing use of
it — `GetNextBasicClass` only walks, it never frees). Also used, identically,
by the still-`INCLUDE_ASM` `BasicClass__func_180bc`/`BasicClass__func_1816c`
("get next child"/"get next parent ref" iterators) — not derived
independently here, just cross-checked for a consistent call shape.

## The final C

```c
void BasicClass__func_18040(BasicClass *self)
{
    BasicClass *child;
    BasicClass **childPtr;
    BasicClassListNode *cursor;

    childPtr = &child;
    cursor = self->children;
    if (GetNextBasicClass(childPtr, &cursor), child != NULL) {
        do {
            self->methods->removeChild(self, child);
        } while (GetNextBasicClass(childPtr, &cursor), child != NULL);
    }
}
```

## Why this shape, in three separately-necessary pieces

Retail's own instruction sequence is unusual for a loop this simple: ONE
physical `jal GetNextBasicClass` (not duplicated), reached two ways — an
initial unconditional jump straight to it (skipping the loop body on the
very first pass) and the loop's own backward branch — with `$s1` cached
once, outside the loop, to hold `&child`'s address (never recomputed), and
a genuine RELOAD of `child` from the stack (`lw $a1,0x10($sp)`) inside the
loop body immediately before the `removeChild` call, even though nothing
writes to that stack slot between the extraction and the reload. Three
independent C-shape choices were needed to reproduce all of this at once;
getting only one or two right left a real, measurable gap (never a
plausible-looking near-miss):

1. **A named `BasicClass **childPtr = &child;` local, not a bare `&child`
   inline at each call site.** Passing `&child` directly at both call
   sites (initial + loop-back) let GCC re-derive the stack address
   (`addiu $a0,$sp,0x10`) fresh each time — cheap for the compiler, but two
   physical recomputations where retail caches ONE value into `$s1`. This
   alone was worth 8 bytes/2 words and was necessary but not sufficient
   (see below): without it, the frame doesn't even allocate `$s1`,
   producing a visibly smaller/wrong frame.

2. **The comma-operator `while`/`if`+`do`+`while` combination, not a plain
   `while`.** A single `while (GetNextBasicClass(...), child != NULL) { body }`
   — the natural first reading of "extract, test, loop" — compiles with the
   test/extraction block placed FIRST (straight-line fallthrough, no
   initial jump needed) and the loop body AFTER, looping back with a
   `bnez`/`j` pair. That is the WRONG physical layout: retail has the body
   FIRST and the test/extraction block AFTER, entered via an initial
   unconditional jump. Restructuring as
   `if (extract(), cond) { do { body } while (extract(), cond); }` —
   syntactically TWO separate call sites, one for the guard and one for the
   loop's own back-test — lets GCC 2.6.3's cross-jump/tail-merge pass
   (already documented in DECOMPILATION_LEARNINGS: "2.6.3's cross-jump/
   tail-merge can merge two blocks whose guards differ") fold them into
   the SAME physical `jal`, reached from both the initial guard and the
   loop backedge — which is exactly retail's shape. This is the
   generalizable finding: **do not assume "a single physical call site
   reached two ways" implies "write it once, in a `while`'s condition."**
   Sometimes the source has TWO textually-separate, semantically-identical
   call expressions, and it's the COMPILER that merges them into one
   physical instruction — write the two calls in C and let cross-jump do
   the folding, rather than trying to write the merged form directly.

3. **`childPtr` (lever 1) combined with the `if`+`do`+`while` shape (lever
   2) — NEITHER alone reached 31/31.** The comma-`while` form (lever 2
   absent) with `childPtr` present gave the right SIZE (29 words) and the
   right general machinery (single call site, `$s1` cached) but the WRONG
   physical layout (test-block-first) and was missing the loop body's
   reload, landing at a 2-word gap with a scattered byte diff. The
   `if`+`do`+`while` form (lever 2) WITHOUT `childPtr` (bare `&child`
   inline) got the physical layout and the reload right but dropped back
   to the un-cached, `$s1`-less frame (28 words, word 0 itself differing —
   the frame size). Only combining both reached 31/31 on the first try
   after landing on the combination.

## What did NOT work (tried and reverted, in order)

- `volatile BasicClass *child` (to force the loop-body reload another way)
  — broke the `$s1` caching entirely (worse: 27 words, wrong frame size).
- `*childPtr` instead of `child` inside the loop body's call argument (to
  force a memory reload via aliasing) — no effect at all; GCC's alias
  analysis correctly proves nothing between the extraction and the use
  could have changed `*childPtr`, so it optimizes the dereference away
  regardless of spelling.
- A literal `goto test; do { body } test: extract(); while(cond);` (an
  explicit hand-written goto-into-the-middle, rather than trusting
  cross-jump merging via lever 2's two-call-sites form) — regressed to the
  un-cached frame, same as omitting lever 1; the explicit `goto` form
  apparently doesn't get the SAME register-allocation treatment as the
  natural `if`+`do`+`while`.
- A bare `__asm__("")` scheduling barrier at the top of the function body
  — no effect on this particular residue (tried before finding lever 2;
  the problem was never instruction ORDER within a fixed CFG, it was the
  CFG/physical-block-layout itself, which a barrier cannot change).

## Proposed learning

**"One physical call site, reached two ways" is not always written in C as
a single call textually shared between two control-flow edges (e.g. inside
a `while`'s condition) — it can equally be TWO separate, textually
identical call expressions (an `if`-guard and a `do`/`while`'s own test)
that GCC 2.6.3's cross-jump/tail-merge pass folds into one physical
instruction.** The two source forms are NOT interchangeable: a single
`while (sideEffectingCall(), cond) { body }` produced a different physical
block layout (test-block-first, no initial jump) than
`if (sideEffectingCall(), cond) { do { body } while (sideEffectingCall(),
cond); }` (body-first, entered via initial jump) — matching retail
required the SECOND, two-call-site form, letting the compiler's own
cross-jump pass do the merging that the FIRST form skips because there was
only ever one call site to merge. This is a distinct, more specific case of
DECOMPILATION_LEARNINGS' existing cross-jump entry (which was about two
SIBLING blocks with differing guards merging a shared tail); here it's a
LOOP guard and a loop BACK-test, and the two-call-site rewrite is the
concrete lever that exposes the merge to the compiler.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220 (fresh carve). ~14
build/measure iterations before landing on the final shape; well inside the
30-attempt budget.
