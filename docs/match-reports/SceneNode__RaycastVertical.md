# SceneNode__RaycastVertical -- MATCHED (round 57, revisit): 180/180 words, byte-exact, whole-image SHA1 green. SEE ROUND 57 AT THE BOTTOM -- everything between here and it is superseded history, kept for its derivation value; the round-46 section it points to is no longer the current best.

> Renamed from `func_8001E7BC` on 2026-09-26 (tools/rename.py). Address 0x8001e7bc.

Unit: `code_d294_c` (round 14). By far the largest function in this
round's queue (200 asm lines, 0x2D0 bytes / 180 words -- more than 60%
bigger than the next-largest, `SceneNode__FaceTarget` at 110 words). Blocker
screen clean: no `gp_rel`, no `addiu $at,$at,%lo`, no `mfhi`/`mflo`-
adjacent-`mult`/`div` hit.

**No C was written or built for this function.** Given the size, the two
still-uncarved callees it depends on (`TmdModel__RaycastFaces`, `slotA4`'s
occupant `SceneNode__ComposeAndApplyRotation`), and the amount of genuinely new field/struct
derivation needed, a full structural read was the higher-value use of
this session's remaining time over a first, likely-incomplete C attempt.
This does NOT count against the 30-attempt budget -- the analysis below
is meant to let the next runner (or this runner in a future round) start
writing C directly instead of re-deriving the shape from scratch.

## Signature and top-level shape

`s32 SceneNode__RaycastVertical(SceneNodeObj *self, void *arg1, void *arg2)`
(return value is 0 or 1; `arg1`/`arg2` types not yet pinned down --
`arg2` is read as a 3-entry `u16`-ish table, `arg1` is forwarded whole
into `SubVec3S16`'s own `dest` parameter, already `s32*` per that
function's own signature).

```
if (self->unk20 == 0) return 0;
if (self->unk10 < 0) {
    <accumulate self's own "attached node" list into self->unk14->unk38>
}
<compute a 3-entry s16 delta between arg2 and self->unk14->unk38 (or {0,0,0} if self->unkC==0)>
self->methods->slotA4(self, 0, /* &sp+0x18-ish scratch */, /* &delta */, 1);  /* 5 args, occupant SceneNode__ComposeAndApplyRotation, code_d294_b, out of scope */
if (!TmdModel__RaycastFaces(self->unk20, &scratch1, &scratch2, 0, &scratch3, delta_minus_1024)) {
    if (!TmdModel__RaycastFaces(self->unk20, &scratch1, &scratch2, 0, &scratch3, delta_plus_1024)) {
        return 0;
    }
}
SubVec3S16(arg1, &scratch3, &scratch2);   /* already matched, this unit */
return 1;
```

## New field/struct knowledge NOT yet committed (needs verification against
## real C before adding to `include/code_d294.h`)

- **`SceneNodeObj` gains (at least) THREE new fields**, all read directly
  off `self` (not through `unk14`):
  - `+0x20`: ALREADY typed `void *unk20` (round 12, `SceneNode__GetModelHull`) --
    this function is a SECOND confirming use, null-checked at entry (a
    `return 0` guard) and later passed as `TmdModel__RaycastFaces`'s own first
    argument. No retype needed, just a second confirmed non-NULL-checked
    use.
  - `+0x10`: a NEW `s32` field, compared with `bgez` (signed, so `s32`
    not `u32`) -- gates the entire "accumulate list into `unk38`" block
    (only runs when `< 0`).
  - `+0xC`: **CONFLICTS with the ALREADY-TYPED `UnkOwner_d294 *unkC`**
    (an owner back-reference, established by `SceneNode__AttachToParent`/
    `SceneNode__DetachFromParent` in `code_d294.c`). This function's OWN use --
    null-checked, then (when non-null) `self->unk14` is read and treated
    as the base for a `+0x38` sub-table, exactly the SAME
    `SceneNodeSub14::unk38` field `SceneNode__LocalOffsetToWorldPos`/`SceneNode__FaceTarget`
    already established this round -- is CONSISTENT with `unkC` staying
    a simple null/non-null gate (its own POINTER value is never
    dereferenced here, only tested against 0), so this is likely NOT a
    real conflict, just a THIRD confirmed null-check use of the same
    field. Flagging explicitly per the shared-header rule since it
    touches an offset two other functions in TWO different rounds
    already named, even though no type change is implied.
- **A linked-list walk, `self->unkC`'s OWN target chases a `+0xC` "next"
  pointer** (inside the accumulation loop, `node = node->unkC` advances,
  loop continues `while (node != 0)`) -- so whatever `self->unkC` points
  to is a DIFFERENT type from `UnkOwner_d294` (which has no `+0xC` field
  of its own) OR `UnkOwner_d294` itself needs a `+0xC` "next" field added.
  Each list node also has its own `+0x14` pointing to a Vec3-shaped
  structure (`+0x18`/`+0x1C`/`+0x20`, read via `node->unk14->unk18` etc.)
  -- the SAME `EntityPos`/`SceneNodeSub14`-family "position vector
  pointer at `+0x14`" convention seen throughout this project. This
  needs its own type, not yet named.
- **`SceneNodeSub14::unk38`'s element type needs reconciling across TWO
  different access widths.** `SceneNode__LocalOffsetToWorldPos` (matched, this round) reads
  and writes it as `s32[3]` (full-word arithmetic, `dst[i] += table[i]`).
  THIS function reads the SAME field via `lhu` (unsigned HALFWORD) at the
  same stride-4 offsets (0, 4, 8) -- i.e., only the LOW 16 bits of each
  `s32` slot, consistent with values that never exceed the `s16` range in
  practice (a plausible reading for bounded in-game deltas), but not
  proven from this call site alone. Whichever runner writes this
  function's C should decide between an explicit `(u16)table[i]` cast (no
  struct change) and a genuine struct retype (e.g. `struct { s16 lo, hi;
  }` per element) based on what the pinned `cc1` actually selects for
  each candidate -- do not guess from the disassembly alone, this project
  has repeatedly found the "obvious" reading wrong for this exact
  toolchain (see this unit's other reports from this round).
- **An apparent dead/defensive check**: `if ((u8 *)self->unk14 + 0x38 ==
  0)` (adding `0x38` to a pointer, then comparing the SUM against zero,
  rather than checking `self->unk14` itself for null) gates entry to the
  accumulation loop. Algebraically this is almost never true for a real
  heap pointer; reproduce it literally rather than "fixing" it to a
  direct `self->unk14 == 0` check, which would NOT match retail's bytes
  (confirmed by the disassembly computing the sum FIRST, then testing
  it, not testing `self->unk14` before the addition).
- **New extern needed: `TmdModel__RaycastFaces`** (still uncarved, presumably the
  NEXT slice past this one or a sibling segment) -- called twice with
  identical arguments except the LAST (a candidate angle table, one
  computed with `-0x400`/`+0x400` applied to one axis, i.e. a +/-90-degree
  BAM offset), tried in sequence until one returns nonzero; reads as a
  "does this candidate angle lead to something valid" test, but that's
  inference, not confirmed.
- **New `SceneNodeMethods` slot needed: `+0xA4`** (occupant
  `SceneNode__ComposeAndApplyRotation`, `code_d294_b`, confirmed via `tools/classtable.py
  gSceneNodeMethods`, out of this carve's scope) -- called with 5 arguments
  (`self`, `0`, two scratch pointers, `1`), signature not yet pinned down
  precisely enough to commit.

## Derivation notes / open questions for whoever attempts the C

- The TWO `TmdModel__RaycastFaces` calls are near-identical (`self->unk20`,
  `&scratch1`, `&scratch2`, literal `0`, `&scratch3` as the 5th
  stack-passed argument, a locally-built 3-entry `s16` angle table as the
  6th) -- almost certainly a shared subexpression/pattern worth writing
  as a single repeated statement with only the 6th argument's
  construction differing (`-0x400` vs `+0x400` on one axis), similar in
  spirit to this unit's other "manually unrolled 3x/2x repetition"
  functions this round (`SceneNode__GetRotationDegrees`, `SceneNode__LocalOffsetToWorldPos`).
- The delta-table construction before `slotA4` (`out[i] = arg2[i] -
  table[i]`, `u16` reads, `s16` stores) is structurally identical to
  `SubVec3S16`'s OWN body (already matched, this unit) -- worth
  checking whether the SOURCE literally calls `SubVec3S16` here too
  (with `s16`-truncated inputs) rather than reproducing its logic inline;
  the disassembly doesn't show a `jal` for this specific 3-word diff
  (matching an INLINED/duplicated computation, not a call), but this is
  worth double-checking against a real `cc1` build before committing to
  either reading.
- Given the size and the two blocking uncarved callees, this is a strong
  candidate to SPLIT into sub-attempts: get the two guard checks + list
  accumulation matching first (word count 0xEFE0..0xF154, roughly the
  first 96 bytes / 24 words), confirm against retail before tackling the
  `TmdModel__RaycastFaces`-dependent tail, rather than attempting the whole 180
  words in one pass.

### Proposed learning

**When a function is disproportionately larger than the rest of its
queue (here, 60%+ bigger than the next-largest) and depends on
still-uncarved callees, a structural-analysis-only stall report --
field offsets, control flow, candidate types, open questions -- is a
legitimate and valuable contribution distinct from a "reached but didn't
match" stall.** It costs zero attempts against the 30-limit and lets the
next session start from working C immediately instead of re-reading 200
lines of MIPS. Distinguish this class explicitly from the "best-reached
body, restored to INCLUDE_ASM" class in `docs/PARALLEL-RUNS.md`'s own
staffing guidance, since the two need different next steps (write C vs.
read disassembly).

## Round 19 (echo): deeper structural read (still no C attempted -- see
## reasoning below), several open questions closed

Re-read the full disassembly instruction-by-instruction (not skimmed) to
pin down several things this report's original pass left open. **Still
no C was written or built** -- the reasoning for that holds even more
strongly after this pass: the function needs a genuine cross-unit field
retype (below) plus a 4-buffer, exactly-8-bytes-each stack layout nailed
down by trial, which is real derivation work, not a quick attempt. This
continues to not count against the 30-attempt budget.

**Closed: `self->unk10 < 0` and `self->unkC != 0` gate the SAME block
together, not independently.** The outer guard is
`if (self->unk10 < 0 && self->unkC != 0) { <backup copy + accumulation
loop> }` -- confirmed by tracing both branch targets: `bgez` (unk10>=0)
skips straight past the whole block, and a SEPARATE `beqz` on `unkC`
right after it ALSO skips the whole block (to the same landing spot the
`bgez` path also reaches), each setting up what's needed for the
FOLLOWING independent computation. This wasn't stated as a combined
condition in the original pass.

**Closed: the "apparent dead/defensive check" is real and gates only the
backup-copy + loop, not the whole block** -- `if ((u8 *)self->unk14 + 0x38
== 0)` skips just that inner section, confirmed by its own branch target.

**Closed: the ternary `(self->unkC != 0) ? self->unk14->unk38 : NULL`
(the "backup" pointer, ALREADY a committed `s32 unk38[3]` field on
`SceneNodeSub14` per this unit's own header, established independently
by `SceneNode__LocalOffsetToWorldPos`/`SceneNode__FaceTarget` this same round) is recomputed FRESH,
inline, at FOUR separate points**: once per axis (x/y/z) inside the
accumulation loop, plus once more after the loop for the final delta
computation -- never cached in a named local, each occurrence re-testing
`self->unkC` from scratch. This is the same "no cross-statement caching"
idiom already on record for this project, just at a larger scale (4
independent re-derivations of one conceptual value) than any prior
documented instance.

**Closed: `TmdModel__RaycastFaces`'s real arity is 6, not "5-ish".** Register
trace: `a0 = self->unk20`, `a1 = &buf30` (an output buffer),
`a2 = &buf28` (a second output buffer), `a3 = 0` (literal), plus TWO
stack-passed arguments at `sp+0x10`/`sp+0x14` -- `arg5 = &buf18` (an
8-byte buffer already used as `slotA4`'s own output buffer earlier in
the SAME function, reused here as an input) and `arg6 = &angleTable` (a
freshly-built 3-entry `s16` table, written into the SAME 8 bytes the
`delta` local occupied before `slotA4` consumed it -- a genuine
stack-slot lifetime reuse, not a bug). Both calls are otherwise
identical except the angle table's construction (`buf18[1] - 0x400` vs
`buf18[1] + 0x400`, confirming the report's "+/-90-degree BAM offset on
one axis" reading, specifically axis index 1).

**Closed: the local-variable stack layout is four 8-byte buffers, back
to back**, `sp+0x18` (`buf18`, written by `slotA4`, read as `s16[?]` at
indices 0/1/2 later), `sp+0x20` (`delta`, written before `slotA4`, then
its SAME memory reused for the angle table after), `sp+0x28` (`buf28`,
an output-only buffer for `TmdModel__RaycastFaces`), `sp+0x30` (`buf30`, likewise)
-- derived from the frame total (`0x58`), the saved-register block
(`0x38`..`0x54`, seven registers `s0`-`s5`+`ra`), and the outgoing-arg
area (`sp+0x00`..`sp+0x18`, 24 bytes/6 words, sized by `TmdModel__RaycastFaces`'s
own 6-argument arity, 4 in registers + 2 on the stack -- consistent with
this round's `ApplyMatrixToLVArray` finding that outgoing-arg sizing reflects
the WIDEST call in the function, though here the wide call is genuinely
live, not dead code).

**Open, and the reason a build was still not attempted: `UnkOwner_d294::
unk14` needs to be usable as a POINTER here** (`node->unk14->unk18/
unk1C/unk20`, the same "position" shape as `SceneNodeObj::unk14`'s own
`SceneNodeSub14 *`), but it is currently typed `s32` in
`include/code_d294.h`, established by `SceneNode__AttachToParent` (already matched,
this unit) which only ever COPIES the raw value
(`self->unk14->unk48 = owner->unk14;`, both currently `s32`) and never
dereferences it. A straight `lw`/`sw` word copy is emitted identically
whether the field is `s32` or a pointer, so retyping it to
`SceneNodeSub14 *` (matching the exact field-access shape this function
needs) is very likely SAFE for `SceneNode__AttachToParent`'s existing match -- but
"very likely safe" is exactly the kind of shared-struct-edit CLAUDE.md
says to verify with the full oracle before trusting, not assume. This is
the concrete next step for whoever attempts the C: retype
`UnkOwner_d294::unk14`, immediately re-run `./build-and-verify.sh` to
confirm `SceneNode__AttachToParent` (and anything else touching that field) is
unaffected, THEN write this function's body using the four-buffer stack
layout and the four-times-repeated ternary above.

### Proposed learning

**A function whose remaining blocker is a cross-function shared-field
retype (not a missing struct, not an uncarved callee) is a different
kind of "not yet attempted" than one blocked on genuinely unknown
shape.** Everything about this function's CONTROL FLOW, ARGUMENT
COUNTS, and STACK LAYOUT is now pinned down precisely; the one
remaining prerequisite is a single-field retype that risks (at low
probability, given the copy-only usage) an already-matched sibling
function elsewhere in the SAME unit. Flagging this distinction because
the retype-then-verify step is exactly one `./build-and-verify.sh` run
before real C-writing can start, not more open-ended derivation.

## Round 19 (echo) continued: first real C attempt, 29/180 (drifted),
## the retype done and verified

Went ahead with the retype flagged above: `UnkOwner_d294::unk14` changed
from `s32` to `SceneNodeSub14 *` in `include/code_d294.h`. **Verified
safe immediately** -- `SceneNode__AttachToParent` (`src/code_d294.c`, a DIFFERENT
unit this runner does not own for editing, but the shared header is
owned by this runner this round) still compiles (one new warning,
"assignment makes integer from pointer without a cast", not an error --
left as-is rather than editing a file outside this runner's assigned
units) and the whole-image SHA1 stayed green. This is now committed.

Wrote the full function body per the structural analysis above and
built it. **First result: 19/180, and the ENTIRE `self->unk10 < 0`
guarded block (backup copy + accumulation loop) was silently eliminated
by the compiler** -- traced to `SceneNodeObj::unk10` already being
typed `u32` (a packed bit-flags word, established by other already-
matched functions this project round using it that way) in the shared
header. Writing `self->unk10 < 0` against an unsigned field is a
compile-time-constant-false comparison in C, so GCC 2.6.3 proved the
entire guarded block dead and removed it, branch condition and all --
not a runtime residue, a source bug. **Fixed with an explicit
`(s32)self->unk10 < 0` cast** (no header retype needed or wanted --
`unk10`'s other established use is a genuine unsigned bitfield host, this
function just needs to read the SAME bits with a signed comparison for
ITS OWN purposes). This alone reproduced the block's presence and
raised the score to **29/180, still WITH address drift** (351943 bytes
outside range) -- not a trustworthy score yet, but real, verified
progress: the backup-copy block's instructions now appear at
approximately retail's own positions, with what looks like a register
identity difference (`$a1`/`$a3` swapped for the "node"/list-walk
pointer) as the visible residue in that section, and the tail
(`TmdModel__RaycastFaces` calls, buffer layout) not yet independently verified
correct.

**Preserved the 29/180 body in `src/code_d294_c.c` as `#if 0`** (this
function's FIRST-EVER inline preserved body -- there was none before
this round). Stopped here rather than continuing to iterate the
register-identity/buffer-layout residue: this function's remaining
distance to a match is still substantial (the four-buffer stack layout,
the two `TmdModel__RaycastFaces` calls, and `SubVec3S16`'s argument buffers are
all unverified against the real oracle beyond "compiles and produces a
plausible partial score"), and this round's remaining time was better
spent finishing verification of the rest of this runner's work list.
`INCLUDE_ASM` restored; this does count as a real attempt now (one, not
thirty), unlike the round-14 "structural analysis only" pass.

### Proposed learning

**A field already established as `unsigned` by ONE already-matched
function elsewhere in the project can silently kill a DIFFERENT
function's dead branch when reused with a signed comparison, with NO
compiler error or warning** -- `self->unk10 < 0` against a `u32` field
compiles clean and simply drops the entire guarded block, which then
LOOKS like an unrelated "why is my loop missing" mystery rather than
what it actually is (a type mismatch with an already-committed shared
field). The fix is a local cast (`(s32)self->unk10 < 0`), not a header
retype -- the field's WIDTH is right, only the road SIGNEDNESS needs
correcting for this one read site, and other established uses of the
same field are unaffected. When a compiled body is unexpectedly missing
an entire guarded block with no error, check whether one of its OWN
condition's operands is a shared field whose established type doesn't
match what this new read site needs signedness-wise, before assuming a
scheduling or optimization mystery.

## ROUND 44 (echo): 29/180-with-drift -> 136/180, EXACT LENGTH -- two structural levers, both now closed off as dead ends for the remaining residue

Picked this up from round 19's preserved 29/180 body (the `(s32)self->unk10 < 0`
signed-comparison fix was already correct and is unchanged here). Rebuilt it
first to confirm the inherited figure: **29/180, WITH drift (351899 bytes
outside range)** -- honest, matching the report's own history.

### Lever 1: the accumulation loop's "table" ternary needs to be written TWICE per axis, not cached in a variable

The loop backs up `self->unk14->unk38[i] += cur->unk14->{unk18,unk1C,unk20}`
for each axis, gated by `self->unkC != 0` (a "backup owner" ternary this
unit's `SceneNode__LocalOffsetToWorldPos`/`SceneNode__FaceTarget` also use, both matched). Retail's
disassembly shows the ternary's `lui`/reload-and-branch sequence computed
**TWICE per axis** -- once for the STORE address, once for the LOAD value --
with **NOTHING cached across them**. My first attempt cached it in a named
`table` variable (`table = ternary; table[i] = table[i] + x;`), which GCC
computes ONCE (address reused for both read and write of the same
expression) -- 2 fewer instructions per axis than retail, hence the 29/180
w/drift score (the function comes out systematically SHORTER).

**Fix: write the ternary literally twice, with no variable to (mis)cache it
in** -- `EXPR[0] = EXPR[0] + x;` where `EXPR` is the full ternary written out
both times. This alone took the score from 29/180 (drift) to **132/180
(exact length, no drift)** -- confirming the loop's shape was the dominant
piece of the residue, not the guards or the tail.

**Levers tried and REJECTED for reproducing the double evaluation without
writing it out literally, all measured, none matched retail's own timing:**
- A bare `__asm__ __volatile__("" ::: "memory")` between per-axis
  assignments of a named `table` -- irrelevant here (see Viewport__InitDefaults's
  report this same round: a memory clobber does not force recomputation of
  a pure address constant).
- Two SEPARATE named variables (`dst`/`src`), each independently assigned
  the identical ternary, one used for the store address and one for the
  load value -- **worse** (37/180, drift). GCC still recognizes the
  redundant pair and partially folds it, just not exactly as retail did.
- An explicit intermediate (`sum = table[i] + x; table = ternary (again);
  table[i] = sum;`) to force a SECOND assignment between the read and the
  write -- **worse** (31/180, drift). Same story.
- Declaration order of `table`/`node`/`cur` (several permutations) --
  **completely inert** on this residue, consistent with round 44's other
  finding on this exact axis (`Viewport__InitOt`'s report, same round):
  declaration order is not a reliable lever for a CSE/materialization
  residue, only for certain register-preference ones.

**The ONLY shape that reproduced retail's double computation is writing the
ternary expression out twice, verbatim, at the SOURCE level.** This project
already has one documented precedent for "write it twice, don't cache it"
(round 19's own note on this same function, and `SceneNode__FaceTarget`/
`SceneNode__LocalOffsetToWorldPos`'s per-axis-but-not-per-read/write reassignment) -- this
round establishes that the "twice" can mean twice **per statement**, not
just once per axis, when the ternary is used as both an l-value and an
r-value target within one update.

### Lever 2: don't index a ternary with a non-zero constant -- it silently folds the offset into the NULL branch too

The first attempt at lever 1 used a single macro,
`UNK38_TABLE(self)[i]` (`(self->unkC != 0 ? self->unk14->unk38 : (s32*)0)[i]`),
applied per-axis with `i` = 0, 1, 2. This reached 132/180 but produced a
CONCRETE SEMANTIC difference from retail for axes 1 and 2: GCC distributes
the constant index into BOTH ternary branches, i.e. rewrites
`(cond ? base : NULL)[i]` as `cond ? base[i] : NULL[i]`. For axis 1
this makes the `unkC == 0` fallback address `(s32*)0 + 1` = a LITERAL `0x4`,
not zero -- retail's own `unkC == 0` fallback is `addu $a0, $zero, $zero`
(a real zero) for every axis, confirmed at vram 0x8001E8AC. Concretely: `li
a0, 0x4` in my build vs `move a0, zero` in retail.

**Fix: give each axis its OWN macro that takes `&unk38[i]` INSIDE the
ternary's true branch**, so the NULL branch stays a bare `(s32*)0`
regardless of axis:
```c
#define UNK38_X(self) ((self)->unkC != 0 ? &(self)->unk14->unk38[0] : (s32 *)0)
#define UNK38_Y(self) ((self)->unkC != 0 ? &(self)->unk14->unk38[1] : (s32 *)0)
#define UNK38_Z(self) ((self)->unkC != 0 ? &(self)->unk14->unk38[2] : (s32 *)0)
```
This closed the false-branch bug (3 more words) but introduced a SECOND,
smaller effect: the TRUE branch's address now folds `0x38 + i*4` into ONE
`addiu` (e.g. `0x3C` for axis 1), whereas retail keeps the `addiu` at a
CONSTANT `0x38` for every axis and applies the per-axis offset only in the
load/store instruction's own displacement (`0x4(a0)`, `0x8(a0)`). Net: 132
-> 136/180. **This second-order difference (constant-folded combined
offset vs. base-plus-instruction-displacement) was NOT closed** -- every
variant tried (materializing the base in a real variable, indexing that
variable instead of the ternary) reopened the WORSE lever-1 problem
(single evaluation instead of two). The two properties -- "correct NULL
fallback" and "un-folded 0x38 base with instruction-level indexing" --
could not be obtained SIMULTANEOUSLY with anything tried this round.

### Remaining residue, precisely

1. **Register identity: `node` (`self->unk14`, used transiently for the
   backup-copy block) is $a1 in my build, $a3 in retail** -- FIRST real
   diff, vram 0x8001E810, and it propagates through every instruction in
   the backup-copy block (vram 0x8001E810-0x8001E838, 8 words). Declaration
   order (multiple permutations, including merging `node` and `table` into
   one variable) did not move it; merging them outright regressed hard (to
   91/180 with drift) because the merge changes semantics of the loop's own
   guard checks downstream, not just the register.
2. **The 0x38-vs-0x38+i "combined ADDIU vs base+displacement" difference**
   for Y and Z axes in the loop (vram 0x8001E8B8, 0x8001E8D4, 0x8001E90C,
   0x8001E928, and their paired load-side instructions) -- described above.
3. **A 4-register permutation in the tail**: retail assigns
   `$s3`=`&delta`, `$s4`=`&buf30`, `$s2`=`&buf28`, `$s1`=`&buf18` (in THAT
   order of first address-taken use); my build assigns the same FOUR
   addresses to `$s1`/`$s3`/`$s4`/`$s2` respectively -- a full permutation,
   not a simple pair swap. Every instruction in the `slotA4`/`TmdModel__RaycastFaces`
   x2/`SubVec3S16` call sequence (vram 0x8001E994 onward) that touches
   one of these four addresses shows the SAME register substituted for its
   retail counterpart, consistently. Tried reordering the four buffer
   declarations to match retail's OWN first-use order (`delta, buf30,
   buf28, buf18`) -- **regressed** to 129/180, so declaration order is
   AGAIN not the lever here.

All three residues are the SAME class (register/materialization identity,
not a missing feature or wrong value) and none responded to the levers this
project has documented for other register residues (declaration order,
`__asm__` barriers, merging/splitting variables). This may need either a
permuter search (not yet run for this function -- it is large, 180 words,
so a search will be slow) or a head-level read.

### Header/struct state

No NEW struct knowledge this round -- everything used
(`SceneNodeObj::unk20/unk10/unkC/unk14`, `SceneNodeSub14::unk18/1C/20/38`,
`UnkOwner_d294::next/unk14`) was already committed by round 19 or earlier
in this unit. This round is pure codegen-shape derivation.

### Proposed learning

**A residue where retail computes the SAME conditional expression twice
(once as an l-value target, once as an r-value source) needs to be written
out twice at the SOURCE level -- caching it in one variable, however
plausible-looking, reliably comes out 2 fewer instructions per occurrence
and desyncs the whole function's length.** This is the same family as
Viewport__InitDefaults's CSE finding this round (GCC not merging what looks
mergeable) but the INVERSE direction: there, GCC merged what retail did NOT;
here, GCC would happily merge what retail also did NOT, and the fix in both
cases is to give the compiler no plausible-looking single value to (fail
to, or successfully) cache -- write the redundant computation out by hand.

**A constant array index applied to a ternary distributes into BOTH
branches, and a NULL/zero fallback branch is not immune** -- `(cond ? ptr :
NULL)[i]` is `cond ? ptr[i] : NULL[i]`, and `NULL[i]` for `i != 0` is a
non-zero literal address, not "still null, offset". Any future ternary
written as an array base with a non-zero constant index should take the
index INSIDE the true branch (`&ptr[i]`) and leave the false branch a bare
NULL, never index the ternary as a whole.

## Round 46 (echo): first-ever permuter search, one real lever found (136 -> 142/180)

Per this round's assignment, `SceneNode__RaycastVertical` was flagged NEVER SEARCHED
despite three prior hand-lever rounds (19, 44). Re-verified 136/180 live
first (rebuilt the round-44 preserved body, `funcdiff.py` confirmed
136/180 exactly, no drift -- check 3, base score agrees with the real
in-tree build).

**Scaffold validation (`--debug --stack-diffs`):** base score **1503**
(stack differences 8, branch differences 0, register differences 35,
reorderings 2, insertions 6, deletions 6) -- NOT a 0/0 scaffold, so
(per this round's cost-test guidance) worth spending a bounded search on.

**Search:** `-j 4 --stop-on-zero --best-only`, bounded at 900s (PATH scoped
to this worktree, `permuter-work/bin`). Ran to completion under load
(other runners' own concurrent searches active in their own worktrees, not
in this one): 56437 iterations, one improvement found and saved
(`output-875-1`, permuter score 875 down from base ~1495-1503 -- the
permuter's own internal base score fluctuates slightly run to run, a
known internal-scoring-mode artifact per round 41's own note, not a
discrepancy in the residue), never beaten again across the remaining
~56000 iterations.

**Per CLAUDE.md's "a permuter score drop is a LEAD, not a RESULT":**
read the actual diff rather than trusting the permuter's own score.

```diff
   delta[0] = buf18[0];
+  delta[1] = ((u16) buf18[1]) - 0x400;
   delta[2] = buf18[2];
-  delta[1] = ((u16) buf18[1]) - 0x400;
```

A pure SOURCE-ORDER swap: writing `delta[1]` before `delta[2]` (instead of
after) right after the `slotA4` call, no expression change at all.
Translated into the real `src/code_d294_c.c` and verified against the real
oracle (not the permuter's isolated scaffold): **136 -> 142/180, no drift,
`build exit=2`, no compile errors.** A genuine, oracle-confirmed 6-word
improvement from a one-line statement reorder.

**The three residues round 44 named are UNCHANGED** -- confirmed by
re-reading the new diff: the first real difference is still at vram
`0x8001E810` (`node`'s register identity, $a1 vs retail's $a3), and the
0x38-vs-0x38+i combined-ADDIU pair and the tail's 4-register permutation
both still appear at their same addresses. The permuter's search, run to
completion (56437 iterations), never found anything touching any of the
three -- consistent with round 41's own finding on a DIFFERENT function in
this unit's neighborhood (`SceneNode__NotifyTaggedParents`) that a pure register-identity
residue lies outside what a bounded source-mutation search can reach.

**Verdict: the search closed one real, independent lever (statement
order) and is otherwise a clean negative on the three named
register-identity residues** -- not "not closed in N iterations," but
"closed what it could reach, the rest needs a different kind of fix."
Filing as STALL at the new figure (142/180), `INCLUDE_ASM` restored,
preserved body updated below.

### Proposed learning

**A permuter's `--debug` scaffold showing non-zero insertions/deletions
(here: 6/6) does NOT mean the eventual improvement will itself be an
insertion or deletion** -- the one real lever this search found was a
pure STATEMENT-ORDER swap (a "reordering," which the scaffold already
separately reported as 2), not a change to instruction count at all. The
insertion/deletion count is a go/no-go signal for whether to spend the
search budget (round 45's own framing), not a prediction of what SHAPE
the eventual lever will take.

## Preserved body (round 46 best, 142/180, no drift)

```c
#if 0
extern s32 TmdModel__RaycastFaces(void *arg0, void *arg1, void *arg2, s32 arg3, void *arg4, s16 *arg5);
extern void SubVec3S16(s32 *dest, s16 *b, s16 *a);

s32 SceneNode__RaycastVertical(SceneNodeObj *self, s32 *arg1, s32 *arg2) {
    s32 *table;
    s16 buf18[4];
    s16 delta[4];
    s16 buf28[4];
    s16 buf30[4];
    SceneNodeSub14 *node;
    UnkOwner_d294 *cur;

    if (self->unk20 == NULL) {
        return 0;
    }
    if ((s32)self->unk10 < 0 && self->unkC != NULL) {
        node = self->unk14;
        if ((u8 *)node + 0x38 != NULL) {
            node->unk38[0] = node->unk18;
            node->unk38[1] = node->unk1C;
            node->unk38[2] = node->unk20;

            cur = self->unkC;
            if (cur != NULL) {
                do {
                    (self->unkC != 0 ? &self->unk14->unk38[0] : (s32 *)0)[0] =
                        (self->unkC != 0 ? &self->unk14->unk38[0] : (s32 *)0)[0] + cur->unk14->unk18;
                    (self->unkC != 0 ? &self->unk14->unk38[1] : (s32 *)0)[0] =
                        (self->unkC != 0 ? &self->unk14->unk38[1] : (s32 *)0)[0] + cur->unk14->unk1C;
                    (self->unkC != 0 ? &self->unk14->unk38[2] : (s32 *)0)[0] =
                        (self->unkC != 0 ? &self->unk14->unk38[2] : (s32 *)0)[0] + cur->unk14->unk20;

                    cur = cur->next;
                } while (cur != NULL);
            }
        }
    }

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    delta[0] = (u16)arg2[0] - (u16)table[0];
    delta[1] = (u16)arg2[1] - (u16)table[1];
    delta[2] = (u16)arg2[2] - (u16)table[2];

    self->methods->slotA4(self, 0, buf18, delta, 1);

    delta[0] = buf18[0];
    delta[1] = (u16)buf18[1] - 0x400;
    delta[2] = buf18[2];
    if (!TmdModel__RaycastFaces(self->unk20, buf30, buf28, 0, buf18, delta)) {
        delta[1] = (u16)buf18[1] + 0x400;
        if (!TmdModel__RaycastFaces(self->unk20, buf30, buf28, 0, buf18, delta)) {
            return 0;
        }
    }
    SubVec3S16(arg1, buf18, buf28);
    return 1;
}
#endif
```

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **KEPT as `SceneNode__RaycastVertical`. Tier C.** What IS known, written down so the
  next reader does not re-derive it:
  - It is a **method of SceneNode** (first parameter `SceneNodeObj *`,
    dispatches `slotA4`), and it is the function that MAINTAINS the world
    position: its first block rewrites `unk14->unk38` (== Sony's
    `GsCOORDINATE2.workm.t`, see the PSY-Q IDENTIFICATION note in
    include/code_d294.h) as `coord.t` plus every owner's `coord.t`, walking
    the `self->unkC` list. `SceneNode__LocalOffsetToWorldPos` and
    `SceneNode__FaceTarget` are both consumers of that field.
  - The rest computes the target point `arg2` relative to that world
    position, rotates the delta into the object's own frame via `slotA4`
    (SceneNode__ComposeAndApplyRotation, the inverse-chain matrix: `slot84` with the NEGATED
    angle flag, composed down the owner list with `MulMatrix2`), then calls
    `TmdModel__RaycastFaces` twice -- once with the Y component minus 0x400 and, on
    failure, once plus 0x400, i.e. +/- 90 degrees -- and finally returns
    `SubVec3S16(arg1, buf18, buf28)`.
  - **Why no name.** `TmdModel__RaycastFaces` lives in the Psy-Q block
    (`psyq_fa50`), is not decompiled, and is the whole point of the second
    half; without it, any verb for this function ("probe", "trace", "clip",
    "aim") is a guess about what the two +/-90-degree attempts are FOR. The
    first half alone would justify something like
    `SceneNode__UpdateWorldPos`, but that would name a third of the body
    and mislead about the return value, which is the second half's result.
  - Reopening it is cheap once `TmdModel__RaycastFaces` is identified (track 2).
- Naming touched nothing in this function's preserved `#if 0` body except
  the callee names `rename.py` rewrote (`SubVec3S16`). It is still
  `INCLUDE_ASM`; the stall verdict above is unchanged.


## Round 57 (charlie, REVISIT): MATCHED, 142/180 -> 180/180, four levers

**REVISITED, round 57: MATCHED 180/180 byte-exact; names/types used**

Honest split, because the revisit rule is measuring its own hypothesis: **two
of the four levers came from post-track-3 TYPES and two came from plain
re-reading of the disassembly.** The two that came from types are the two
that had been open the longest (round 44's residues 1 and 2), and the
function does not match without them, so this revisit is a positive for the
rule rather than another round-55-style "closed it by re-reading".

Baseline re-verified live first: the round-46 preserved body rebuilt to
exactly **142/180, no drift** (`tools/stalesyms.py` lists no stale callee for
this report, and the `composeAndApplyRotation` rename was already in the
body). `build exit=2`, no compile-error grep hits.

### Lever 1 (re-reading): retail's tail is CROSS-JUMPED -- write the success body TWICE. 142 -> 155

The tell-tale is in the delay slots. Retail has `addu $a0, $s5, $zero` in the
delay slot of BOTH `TmdModel__RaycastFaces` result branches (vram 0x8001EA18 and
0x8001EA48) but only ONE `jal SubVec3S16`. That is `jump.c` cross-jumping two
identical tails and hoisting the first surviving insn of the shared block
into the earlier branch's delay slot. So the source duplicates the body:

```c
if (TmdModel__RaycastFaces(...)) { SubVec3S16(arg1, buf18, buf28); return 1; }
delta[1] = (u16)buf18[1] + 0x400;
if (TmdModel__RaycastFaces(...)) { SubVec3S16(arg1, buf18, buf28); return 1; }
```

rather than round 46's single trailing call under a doubly-negated `if`.

**What this closed is NOT what it looks like.** The instruction sequence is
almost identical either way; what changed is round 44's residue 3, the
"4-register permutation" of `$s1`-`$s4` over the four stack buffers, which
vanished completely. Round 44 had tried to move it by reordering the buffer
DECLARATIONS and regressed. The s-register assignment follows the live ranges
the CALL structure creates, not declaration order -- with the `SubVec3S16`
call inside each probe's body, `buf18`/`buf28` are live across a different
set of calls than when the call sits after the whole `if`.

### Lever 2 (re-reading): the null-`unk20` guard is an ENCLOSING `if`, not an early return. 155 -> 164

Retail's shared `addu $v0, $zero, $zero` block sits at 0x8001EA60, AFTER the
success tail, and falls through to the epilogue. Writing

```c
if (self->unk20 == NULL) { return 0; }
```

at the top puts that block BEFORE the success tail instead, costing an extra
`j` and inverting the second probe's branch. Wrapping the whole body in
`if (self->unk20 != NULL) { ... }` with one bare `return 0;` as the
function's last statement reproduces retail exactly, including the early
branch's own target (`beqz $v0, 0x8001EA60` at 0x8001E7E8, which had been
wrong since round 19 and was never separately itemised).

### Lever 3 (TYPES): cast the ternary to `LongVec3 *` and reach it by FIELD. 164 -> 172

This is round 44's residue 2, and round 44 recorded it as a dead end: "the
two properties -- correct NULL fallback and un-folded 0x38 base with
instruction-level indexing -- could not be obtained SIMULTANEOUSLY with
anything tried this round."

They can. The problem is that `(cond ? p : NULL)[i]` is a tree PLUS_EXPR over
a COND_EXPR, and `fold()` distributes it into both arms -- which both folds
`0x38 + i*4` into one `addiu` on the true arm and turns the false arm into
the literal `i*4`. A **COMPONENT_REF offset is not a tree PLUS_EXPR**: it is
applied at expand time as the MEM's own displacement. So

```c
((LongVec3 *)(self->unkC != 0 ? self->unk14->unk38 : (s32 *)0))->y
```

leaves the cond-expr's arms untouched (`addiu $a0, $v0, 0x38` / `addu $a0,
$zero, $zero`, both exactly retail's) and puts the axis in the `4($a0)`
displacement. Round 44's two properties are not in tension at all; they were
only in tension while the axis was expressed as an index.

`LongVec3` is the unit's own committed `{s32 x, y, z}` (include/code_d294.h),
and `SceneNodeSub14::unk38` is deliberately `s32 unk38[3]` rather than a
`LongVec3` because `SceneNode__LocalOffsetToWorldPos` needs to index it.
Nothing about the header changed here -- the CAST is local to this function,
which is exactly the split that header note anticipates.

### Lever 4 (TYPES): the backup copy is ONE `LongVec3` struct assignment. 172 -> 180, MATCHED

The last residue, open since round 44 and filed as a pure register-identity
stall (`node` in `$a1` vs retail's `$a3`, 8 words at vram
0x8001E810-0x8001E838). Round 44 tried declaration order and variable
merging; round 46's permuter search ran 56437 iterations and never touched
it; this round re-swept all 12 permutations of the three pointer locals
(`table`/`node`/`cur`, block front and back) and every one scored exactly
172 -- declaration order is **completely inert** on this function, now
measured three times.

The fix is not an allocation lever at all. Three scalar assignments

```c
node->unk38[0] = node->unk18;
node->unk38[1] = node->unk1C;
node->unk38[2] = node->unk20;
```

and the whole-struct assignment

```c
*(LongVec3 *)node->unk38 = *(LongVec3 *)&node->unk18;
```

emit the **same six instructions** (`lw`/`lw`/`lw` then `sw`/`sw`/`sw`, same
temps `$v0`/`$v1`/`$a0`, same displacements). They differ only in which hard
register `node` itself lands in: the scalar form gives `$a1`, the struct copy
gives retail's `$a3`. A 12-byte struct move is expanded by `emit_block_move`
as one unit, so `node`'s pseudo is referenced by a different number of insns
and with a different live shape than when three independent statements each
reference it -- enough to change where it sorts in `global_alloc` and which
hard register it is offered.

The header already says these three words ARE a vector ("copied wholesale
into `SceneNodeSub14::unk18/unk1C/unk20`"), so the struct copy is also the
more faithful source shape, not a trick.

### Negatives measured this round (all at the 172 base unless noted)

| variant | score | note |
| --- | --- | --- |
| 12 declaration-order permutations of `table`/`node`/`cur` | 172 each | inert, third independent confirmation |
| `cur` hoisted above `node = self->unk14` | 172 | dead store, eliminated |
| `(cur = self->unkC) != NULL` inside the `&&` | 153 | moves `cur` to `$a0` and splits the block's base register |
| no `node` variable, `self->unk14` written inline | 32 (drift) | the stores invalidate CSE, so `self->unk14` is reloaded per access |
| `while (cur != NULL)` instead of `if (...) do {} while` | 76 (drift) | genuinely different loop rotation |
| guard as `node->unk38 != NULL` instead of `(u8 *)node + 0x38 != NULL` | 172 | equivalent; kept the explicit form, which reads as the defensive check retail actually performs |

### Gate 3 / permuter

**No search was spent, and the reason is a result rather than a skip.** Round
46 had already run one to completion on this function (56437 iterations,
`--stop-on-zero --best-only -j 4`, 900s bound, base permuter score 1503) and
recorded a clean negative on all three named residues. Gate 3 check 1
(scaffold compiles and scores) and check 3 (scaffold signature agrees with
the real in-tree build) were both satisfied by that round's own record; check
2's insertion/deletion count (6/6) was likewise already on file. Re-spending
against the same three residues would have re-measured a recorded negative.
All four levers here are source-SHAPE changes -- statement duplication, guard
nesting, a cast, and a struct assignment -- and only the first of those is in
the permuter's mutation vocabulary at all, which is consistent with round
46's search having found only a statement-order swap.

### Proposed learning

**`(cond ? p : NULL)[i]` and `((T *)(cond ? p : NULL))->field` are not the
same construct to GCC, and the difference decides whether a constant offset
folds into the ternary's arms.** An array index on a conditional expression
is a tree `PLUS_EXPR` that `fold()` distributes into BOTH arms -- combining
the offset with the true arm's own `addiu` and, worse, silently turning a
NULL false arm into the literal `i * sizeof(T)`. A struct field reference is
a `COMPONENT_REF`, whose offset is applied at expand time as the MEM's
displacement, so the arms are left exactly as written. Whenever retail shows
a **constant base `addiu` plus a per-element instruction displacement** on
the far side of a ternary, reach for a struct-pointer cast and a field, not
an index. This retires the "could not be obtained simultaneously" dead end
round 44 recorded.

**Three scalar assignments and one struct assignment can emit byte-identical
instructions and still allocate a different register to the POINTER they go
through.** `emit_block_move` expands a small struct copy as a single unit, so
the base pointer's pseudo has a different reference count and live shape than
when three separate statements each mention it, which is enough to move it in
`global_alloc`'s ordering. **So a register-identity residue on a pointer used
by a run of same-shaped field copies is worth one struct-assignment attempt
before it is filed as a stall** -- it is cheap, it is not a register pin, and
here it was the difference between 172/180 and a match after three rounds had
classified it as unreachable by source shape.

**A "register identity" classification is a claim about the RESIDUE, not
about the function, and it decays as the rest of the function changes.** All
three of round 44's residues were filed as one class ("register/
materialization identity, not a missing feature or wrong value") and two of
them turned out to be ordinary source-shape differences elsewhere in the
function: the `$s1`-`$s4` permutation was the tail's call structure, and the
`$a1`/`$a3` one was the copy idiom. Only re-deriving the surrounding shape
exposed them. When a stall carries several same-class residues, fixing the
STRUCTURE first and re-measuring is worth more than attacking any of them
directly -- here levers 1 and 2 together removed one "register identity"
residue outright without ever addressing it.

## Track 6 (round 91, echo): named `SceneNode__RaycastVertical`, tier B

A SceneNode method `(self, out, target)`: refreshes coord2's workm.t as coord.t plus every parent's coord.t, rotates `target` minus that into the node's frame (composeAndApplyRotation), then casts TmdModel__RaycastFaces from the point along -Y by 0x400 and, failing that, +Y by 0x400; on a hit writes hit minus point to `out` and returns 1. Mechanics; its one caller is AcceptGridElem (class_3bb8c_p.c), a grid-query filter. Was `func_8001E7BC`, kept tier C by track 3 for want of a verb. The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.

## Round 98 (echo): track 7, moved from src/code_d294_c.c

Locals and parameters named for their roles, all byte-identical: `arg1` -> `offset` (out, s32 x3), `arg2` -> `target` (in, a world position), `buf18` -> `origin`, `delta` -> `end` (it first holds `target` less the world position, the input composeAndApplyRotation rotates into `origin`), `buf28` -> `hit`, all three now `SVECTOR` instead of `s16[4]`; `buf30` -> `s32 best` (TmdModel__RaycastFaces's nearest-distance out; the frame is unchanged, measured), `table` -> `worldPos`. The probe offset `0x400` is `RAYCAST_PROBE_LENGTH` (1024), a distance along y in model units: the old comment's reading of it as "-90 and +90 degrees in BAM" was wrong, since it is added to a coordinate, not an angle.

The GsDOFF guard is now `(self->attribute & GsDOFF)` (libgs.h's `1<<31`), byte-identical to the `(s32)self->attribute < 0` it replaces: GCC folds the sign-bit mask into the same `bltz`. The round-19 note that the cast was needed was about a `u32 < 0` comparison, which is constant-false; the mask test is not.

Field names in the old comment, against include/SceneNode.h: `unk10` is `attribute`, `unkC` is `parent`, `unk20` is `model`, `unk14->unk38` is `coord2->workm.t`.

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* First it maintains the object's world translation, `coord2->workm.t`:
 * the guarded block rewrites it as this object's own coord.t plus every
 * owner's coord.t, walking the `parent` chain. Then it takes the target
 * point `arg2` relative to that world translation, rotates the delta into
 * the object's own frame through slotA4 (SceneNode__ComposeAndApplyRotation, the
 * inverse-chain matrix), and probes `TmdModel__RaycastFaces` twice -- Y minus 0x400
 * and, on failure, Y plus 0x400, i.e. -90 and +90 degrees in BAM. On
 * success `arg1` receives `buf28 - buf18` and the function returns 1.
 *
 * Four source shapes here are load-bearing; each was measured against the
 * oracle and the report records what the alternatives scored.
 *
 *  - `(s32)self->unk10 < 0`: `unk10` is a `u32` bitfield word, so without
 *    the cast the comparison is constant-false and GCC deletes the whole
 *    guarded block silently (round 19).
 *  - The backup copy is ONE `LongVec3` struct assignment, not three
 *    scalar ones. Both spell lw/lw/lw + sw/sw/sw, but only the struct copy
 *    puts `node` in retail's $a3; the scalar form gives it $a1. That was
 *    the last residue, open since round 44.
 *  - The `self->unkC != 0 ? ... : 0` ternary is written out TWICE per axis
 *    -- once for the store, once for the load -- because retail evaluates
 *    it twice (the diamond blocks it from CSE). Caching it in a variable
 *    costs two instructions per axis. It is also cast to `LongVec3 *` and
 *    reached by FIELD, not indexed as `[i]`: `(cond ? p : NULL)[i]`
 *    distributes the index into both arms, which turns the NULL arm into
 *    the literal `i*4` and folds `0x38 + i*4` into one addiu, where retail
 *    keeps a constant 0x38 base and puts the axis in the load/store
 *    displacement.
 *  - The success body is written out twice, once per probe, with the bare
 *    `return 0` last and the null-`unk20` guard as an enclosing
 *    `if (self->model != NULL)` rather than an early return. jump.c
 *    cross-jumps the two copies into the single `jal SubVec3S16` retail
 *    has (the giveaway is `move a0,s5` appearing in BOTH probe delay
 *    slots), and that placement is what puts the shared `v0 = 0` block
 *    after the success tail instead of before it.
 *
 * `node->workm.t != NULL` is retail's own check, not a typo for
 * `node != NULL`: the disassembly forms &workm.t (node + 0x38) first and
 * tests THAT. */
```

## History: track 12 (round 106, charlie), comments moved out of the source

The API documentation pass moved these comments' process text here,
verbatim; the source keeps a one-line `MATCHING:` note or the API doc.

From `src/graphics/scene_node.c`:

```c
 * MATCHING, each measured (the report has the alternatives):
 *  - coord.t is copied as one LongVec3 assignment, not three;
 *  - the parent ternary is written twice per axis and reached by field;
 *  - the hit branch is written once per probe, inside the model test;
 *  - `node->workm.t != NULL` tests the array's address, as retail does. */
```
