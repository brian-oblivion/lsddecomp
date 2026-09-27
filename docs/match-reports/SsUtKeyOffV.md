# SsUtKeyOffV -- MATCHED (round 62 REVISIT: 73/73 words, insertions 0 / deletions 0, whole-image oracle green)

> Renamed from `func_80031890` on 2026-09-24 (tools/rename.py). Address 0x80031890.

**REVISITED, round 62: MATCHED; names/types used -- the closing lever came
from the MATCHED sibling `SsUtKeyOff`'s idiom, and the local names
(`chan`, `mask0`, `mask1`) are that sibling's own, reused deliberately rather
than re-invented.**

Unit: `src/code_179d8_j_b.c`. 73 words (0x124 bytes), file offset `0x22090`,
vram `0x80031890`.

## The closing body

```c
s32 SsUtKeyOffV(s16 idx)
{
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (_snd_ev_flag == 1) {
        goto fail_nolock;
    }
    _snd_ev_flag = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    D_8008EA26 = idx;
    chan = D_8008EA26;
    if (chan < 0x10) {
        mask0 = 1 << chan;
        mask1 = 0;
    } else {
        mask0 = 0;
        mask1 = 1 << (chan - 0x10);
    }
    D_8008D9A3[chan].unk0 = 0;
    D_8008D98C[chan].unk0 = 0;
    _svm_voice[chan].unk0 = 0;
    _snd_ev_flag = 0;
    D_80090C60 = mask0 | D_80090C60;
    D_80090C64 |= mask1;
    D_8008E228 &= ~D_80090C60;
    D_8008E22C &= ~D_80090C64;
    return 0;

fail:
    _snd_ev_flag = 0;
fail_nolock:
    return -1;
}
```

No new declarations: `_snd_ev_flag`, `D_8008EA26` (`volatile u16`, per this
report's round-23 CLOSED finding, which stands and is still load-bearing),
`Rec34Byte D_8008D9A3[]`, `Rec34Half _svm_voice[]`/`D_8008D98C[]` and the four
mask scalars were all already in the unit. Nothing was added to `include/`.

## What four rounds had wrong, and the lever

The function stood at 51/73 through rounds 23, 31, 32 and 40 -- four rebuilds,
a documented axis list, and a 123134-iteration permuter search -- classified as
"scattered register-identity/scheduling residue, not reachable through any
legal lever", explicitly invoking HARD RULE 6. **It was neither register
identity nor scheduling.** Step (a) of the revisit rule is what exposed that:
the inherited body rebuilds at exactly the recorded 51/73 with exactly the
recorded length, but its ins/del line reads **insertions 4 / deletions 4**. A
register-identity residue is 0/0, so four of those twenty-two differing words
were structural all along and rule 6 never covered them.

**The lever: compile the MATCHED sibling and diff its OBJECT against your
target's retail `.s` -- do not read the sibling's C.**

`SsUtKeyOff` sits about 1500 bytes up the same unit, is byte-exact against
retail, and its `else` branch is this function's whole body. Dumping
`build/src/code_179d8_j_b.c.o` and reading its tail gives, in order:

```
sb   zero, D_8008D9A3[chan]
lhu  D_80090C60 ; lhu D_80090C64
sh   zero, D_8008D98C[chan] ; sh zero, _svm_voice[chan]
lhu  D_8008E228
or / sh D_80090C60 / nor / and / sh D_8008E228
lhu  D_8008E22C
or / sh D_80090C64 / nor / and / sh D_8008E22C
sw   zero, _snd_ev_flag
```

That is **instruction for instruction** retail's `SsUtKeyOffV` from word 33
to word 65 -- same four-global load order (C60, C64, E228, E22C), same
or/store/nor/and/store pairing per channel, same `or` operand order (the mask
first). The one difference is *where the `_snd_ev_flag = 0;` store lands*: the
sibling releases the lock after the mask block, retail's `SsUtKeyOffV`
releases it before (`sw zero, %lo(_snd_ev_flag)` at 0x80031950, between the
`D_8008E228` load and the first `or`). Moving that one statement is the entire
delta. First build of that shape: **73/73, ins 0 / del 0, whole-image SHA1
green.**

**Round 32 looked at this exact sibling and drew the wrong thing from it**,
and the reason is the point worth keeping. It compared the two functions'
**C** side by side, noticed the sibling's `u32 mask0` / `u16 mask1` type
asymmetry against this function's matched pair of `u16`s, tried the retype,
got byte-identical output, and concluded -- in a "Proposed learning" section --
that a working idiom from a matched sibling need not transfer. The types were
never the transferable part. What transfers is that **the sibling caches
nothing**: it writes `D_80090C60 = mask0 | D_80090C60;` and
`D_8008E228 &= ~D_80090C60;` directly, where every attempt on this function
since round 23 carried four cached locals (`old60`, `old64`, `e228`, `e22c`).
That difference is invisible in a C-to-C reading -- a cached read of a global
looks like a faithful transcription of the same operation, and rounds 23 and 40
both explicitly reasoned that naming the reads could not matter (round 23:
"named as separate locals ... **zero effect**, byte-identical to the
direct-expression form (GCC already treats them equivalently)"). That claim was
true for the pair it was tested on and false for the set of four.

**Both documented residue clusters were one artifact of that caching**, which
is why neither ever moved:

- The `loBit`/`hiBit` `$a2`/`$a3` swap. With four extra live locals competing
  for the same call-clobbered registers, the allocator put the mask pair the
  other way round -- and that propagated into the `or`'s operand order
  (`or $a0,$a0,$a2` where retail has `or $a0,$a3,$a0`). With no cached locals
  the masks land in retail's registers unprompted.
- The "second-half interleaving". Measured precisely this round rather than
  described: the cached-locals build loads the four globals in the order
  **C60, E228, C64, E22C** (confirmed from the relocations) where retail loads
  **C60, C64, E228, E22C**, and it sinks the `D_8008E228` store six
  instructions past retail's position, below the `D_8008E22C` load. Removing
  the locals restores both.

## Negatives confirmed this round (not stale -- re-measured on the current pipeline)

Both of these were recorded by round 23, which predates the round-42 maspsx
flag changes (`--gp-symbols`, `--no-nop-mflo-mfhi`), so they were worth
re-measuring before being trusted. Both reproduce exactly:

- **`__asm__("")` between the two channel groups: 51/73 -> 28/73**, ins 16 /
  del 16. Round 23's figure to the word. The barrier is a legitimate
  order-only construct here and it still makes things sharply worse.
- **Swapping the `loBit`/`hiBit` declaration order: byte-identical**, 51/73,
  ins 4 / del 4. Inert, as recorded.

One new negative, worth recording because it is the near-miss on the right
idea: **the sibling's shape with the two channels' statements interleaved**
(`C60 or` / `E228 and` / `C64 or` / `E22C and`, mirroring retail's *emitted*
order rather than the sibling's source order) scores **43/73, ins 12 / del 12**
-- worse than the cached-locals baseline. Retail's emitted order is
channel-60-complete-then-channel-64, and writing that order in C does not
produce it; writing both `or`s first and both `and`s after does. That is an
ordinary instance of "the emitted order is not the source order", but it is the
one that would have made this lever look wrong if it had been tried first.

## Attempts (round 62 -- 6 real builds)

1. Round 23's preserved body verbatim (revisit step (a)): 51/73 confirmed,
   length exact, **ins 4 / del 4** -- the figure that falsified the
   register-identity classification.
2. Sibling idiom, no cached locals, but with the two channels interleaved and
   the lock released early: 43/73, ins 12 / del 12.
3. Back to baseline; full side-by-side against the sibling's compiled object.
4. `loBit`/`hiBit` declaration order swapped: byte-identical. INERT.
5. `__asm__("")` between the channel groups: 28/73, ins 16 / del 16.
6. **Sibling idiom with the sibling's own statement order and the lock
   released before the mask block: 73/73, ins 0 / del 0, oracle green.**

No permuter search was spent; the match came on the sixth build.

### Proposed learning

**When a stall has a byte-exact sibling, diff the sibling's COMPILED OBJECT
against your target's retail `.s` -- comparing the two functions' C is a
different and much weaker check, and it is the one that fails.**
`SsUtKeyOffV` stood for four rounds beside a matched twin 1500 bytes up its
own file. Round 32 did compare them, in C, and came away with the one
difference that was cosmetic (the `u32`/`u16` mask types) while missing the one
that was structural (the sibling caches no globals). The object-level diff
makes the structural difference unmissable, because it is *in the instruction
stream you are trying to reproduce*: the sibling's tail and retail's tail line
up word for word, and whatever single statement does not line up is the answer.
This is cheap -- the sibling is already in your build -- and it is checkable
without retail at all.

Two corollaries, both paid for here:

- **A cached local for a global read is not a neutral transcription.** Four of
  them turned a matchable function into a plausible register-identity stall,
  and the mechanism is indirect enough that every axis anyone varied *inside*
  the cached-locals shape (declaration order, types, barriers, statement order,
  123k permuter iterations) was searching a space that did not contain the
  answer. That is the matching guide's "a long attempt list is not a broad one"
  with a new tell: when every attempt keeps the same set of locals, the untested
  axis is whether those locals should exist -- and this project has hit that
  exact axis before (`func_8002C048`, twenty-five variations all keeping the
  cached `c1`/`c2` locals).
- **Invoking HARD RULE 6 needs the ins/del figure, not a reading of the diff.**
  Four rounds described this residue as "same instructions, same operands'
  values, different physical registers" -- which is the rule-6 shape, and which
  four separate readers agreed on -- while `funcdiff.py` would have said 4/4.
  A report that cites rule 6 without quoting `insertions 0 / deletions 0` has
  not established that rule 6 applies.

## Disposition

**MATCHED.** C committed in `src/code_179d8_j_b.c` in ROM-address order (last
function in the unit), with a comment pointing at the sibling relationship so
the next reader does not have to re-derive it. `./build-and-verify.sh` green,
`git status --porcelain` empty. The round-23 `volatile D_8008EA26` and
`goto`-to-shared-tail findings below are both still load-bearing in the
matching body and are unchanged.

---

# SsUtKeyOffV -- STALL (length EXACT at 73/73 words; 51/73 raw word-match; first real diff at word 37 / retail 0x80031924 -- genuine scattered residue, not a shift)

**Measurement note (both numbers, not conflated):** compiled LENGTH is
exactly retail's 73 words (funcdiff reports no out-of-range drift). The
RAW hex word-match count is 51/73. Because the length is exact, this is
NOT "51/73 with the rest explained by a shift" -- `tools/asm-differ/diff.py`
realigns the two streams and confirms the first REAL divergence is at word
37 (file offset 0x022124, retail vram 0x80031924), with the preceding 37
words matching exactly. So of the 36 words from 37..72, 14 further match
and 22 genuinely differ, in the two clusters described below (the
`loBit`/`hiBit` register swap, and the second-half interleaving) -- these
are independent register-identity/scheduling residues, not an artifact of
address drift.

Unit `code_179d8_j`, round 23 (2026-09-07). Not a class method. Bounds-checks
`idx` against 0x18 under a `_snd_ev_flag` reentrancy lock (same lock/idiom as
`code_179d8_i.c`'s `func_80033738`), converts the channel into a 32-bit-wide
`(loBit, hiBit)` bitmask pair, clears three per-channel fields, then ORs the
new bits into two running masks (`D_80090C60`/`D_80090C64`) and clears the
corresponding bits out of two "active" masks (`D_8008E228`/`D_8008E22C`).

## What it is (best-reached body, 51/73 words, whole-image SHA1 red)

```c
#if 0
extern volatile u16 D_8008EA26;

s32 SsUtKeyOffV(s16 idx)
{
    u16 bankIdx;
    u16 hiBit, loBit;
    u16 old60, old64, e228, e22c;

    if (_snd_ev_flag == 1)
        goto fail_locked;
    _snd_ev_flag = 1;
    if ((u16) idx >= 0x18)
        goto fail_unlock;
    D_8008EA26 = idx;
    bankIdx = D_8008EA26;
    if (bankIdx < 0x10) {
        loBit = 1 << bankIdx;
        hiBit = 0;
    } else {
        loBit = 0;
        hiBit = 1 << (bankIdx - 0x10);
    }
    D_8008D9A3[bankIdx].unk0 = 0;
    D_8008D98C[bankIdx].unk0 = 0;
    _svm_voice[bankIdx].unk0 = 0;
    old60 = D_80090C60;
    old64 = D_80090C64;
    e228 = D_8008E228;
    _snd_ev_flag = 0;
    old60 = loBit | old60;
    D_80090C60 = old60;
    D_8008E228 = e228 & ~old60;
    e22c = D_8008E22C;
    old64 = hiBit | old64;
    D_80090C64 = old64;
    D_8008E22C = e22c & ~old64;
    return 0;

fail_unlock:
    _snd_ev_flag = 0;
fail_locked:
    return -1;
}
#endif
```

(Uses this unit's already-shared `_snd_ev_flag`, `Rec34Byte D_8008D9A3[]`,
`Rec34Half _svm_voice[]`/`D_8008D98C[]`, and the scalar `D_80090C60`,
`D_80090C64`, `D_8008E228`, `D_8008E22C` globals declared near the top of
`code_179d8_j.c`.)

## Two findings worth keeping regardless of the stall

### 1. CLOSED: `D_8008EA26` needs `volatile` to reproduce retail's reload

Retail stores `idx` into `D_8008EA26` and then genuinely reloads it (`sh`
immediately followed by `lhu` from the same address) before using it as
`bankIdx`. A plain `D_8008EA26 = idx; bankIdx = D_8008EA26;` does **not**
reproduce this: GCC 2.6.3's cse pass recognises the global is provably equal
to the just-stored `idx` and reuses the live pseudo, fusing the whole
`(u16)idx < 0x18` bound check and the `bankIdx < 0x10` check into a single
computation -- 29 words shorter than retail; a bare `__asm__("")`
scheduling barrier between the store and the read does **not** block this
(it only reorders instructions, it never blocks value-forwarding, since
CSE/value-numbering happens well before the scheduling pass). Declaring the
global `volatile` (`extern volatile u16 D_8008EA26;`) forces a genuine
reload and reproduces retail's exact two-instruction `lui`/`lhu` sequence at
that point. A pointer-indirection idiom (`u16 *cur = &D_8008EA26; *cur =
idx; bankIdx = *cur;`) that works for this unit's `D_8008EA22` in
`SpuVmGetSeqVol` does **not** reproduce it here -- GCC still traces the
pointer back to the known constant address and folds it exactly like the
non-pointer form. Since `SpuVmSeqKeyOff` and `SsUtChangePitch` (this same
round, this same unit) write-then-immediately-reread this identical global,
this fix generalises to all three; the type is declared once, shared, in
this unit's top block.

### 2. CLOSED: the two early-return paths tail-merge only via explicit `goto`

Writing the lock/bounds guards as ordinary `if (cond) { ...; return -1; }`
blocks produces an **extra, unconditional `j`+delay-slot pair** for the
first (`_snd_ev_flag == 1`) check, because GCC did not perform cross-jump
merging between that check's return and the bounds-check's return even
though both ultimately return the same `-1`. Retail's actual shape has the
very first check's `beq` branch **directly** to the shared tail that also
handles the bounds-check failure (clearing the lock only on that second
path). Restructuring with explicit `goto fail_locked;` / `goto
fail_unlock;` targeting one shared tail (mirroring retail's actual
label topology, not just its net effect) closed a 2-word/~279KB-drift gap
outright, taking the function from 5/73 (with a whole-image size mismatch,
untrustworthy) to 51/73 (exact retail length, no drift). This is the
"branch TARGETS disagree" class from the matching guide, generalised: it
is not just about lining up branch targets inside one already-matched
control-flow shape, it is that **naive sequential `if / return` sometimes
does not reach the tail-sharing retail's source achieved**, and an explicit
`goto` to a hand-placed shared label is the correct, permitted lever (not a
register fix -- it changes control flow, which is squarely source-level).

## The unresolved residue (register identity / scheduling, not reshaped away)

Two clusters, 22 words total, isolated with `objdump`-vs-retail diffing
rather than guessed from the funcdiff hex:

- **`loBit`/`hiBit` land in the wrong scratch registers.** Retail's `if
  (bankIdx < 0x10)` branch fills its delay slot with `sllv $a3,$a1,$v1`
  (i.e. `loBit` materializes in `$a3`) and the explicit `else` arm sets
  `$a2` to 0/the other shift (`hiBit` in `$a2`). This build's compiled
  code swaps them throughout (`loBit` in `$a2`, `hiBit` in `$a3`) --
  same instructions, same operands' *values*, different registers.
  Tried and rejected: swapping the `hiBit`/`loBit` **declaration** order
  (no effect on this cluster, but combined with other changes made a
  *different*, unrelated part of the function regress -- see below);
  swapping the **statement order inside each branch** (`hiBit = 0;
  loBit = ...;` vs `loBit = ...; hiBit = 0;` -- regressed to 36/73, so
  the delay-slot filler's register choice is not driven by textual
  order within the branch either).
- **The two mask-update sequences interleave differently.** Retail
  fully completes channel-60's `or`/store/`nor`/`and`/store sequence
  (including a freshly-computed `~newBits` complement) before starting
  channel-64's (with `D_8008E22C`'s read deferred until *after*
  `D_80090C60`'s store, not prefetched early like `D_8008E228`'s read
  is). This build's compiled output, despite the C statements appearing
  in that exact same order, computes both `or`s first, then interleaves
  the `nor`/`and`/store pairs across the two channels, reusing `$a0`/`$a1`
  across both halves in an order this project's toolchain-pinned
  pipeline chose independently of the C statement order. Tried: naming
  `D_8008E228`bandwidth's and `D_8008E22C`'s reads as separate locals
  (`e228`, `e22c`) matching retail's own early/late fetch split --
  **zero effect**, byte-identical output to the direct-expression form
  (GCC already treats them equivalently); a bare `__asm__("")` barrier
  between the two channel's statement groups -- **regressed** to 28/73
  (moved *more* things out of position, not fewer).

Per CLAUDE.md's rule, a register-identity mismatch reachable only through
`register T v asm("$N")` or an operand constraint is a STALL, not a lever to
pull. Both clusters here are exactly that shape (same instructions/operands,
different physical registers), and every legal reshaping axis tried moved
the residue sideways or made it worse, never smaller.

## Axes tried (for the next attempt)

- `D_8008EA26` access idiom: plain global (fused, wrong), `__asm__("")`
  barrier (no effect on fusion), pointer indirection (no effect), `volatile`
  qualifier (fixed it) -- **closed**.
- Early-return control flow: naive `if/return` (extra `j`, 5/73 with drift),
  explicit `goto` to a shared tail matching retail's label topology (closed,
  51/73).
- `loBit`/`hiBit`: declaration order (no effect alone), statement order
  within each `if`/`else` arm (regressed to 36/73), separate `new60`/`new64`
  names instead of reusing `old60`/`old64` in place (regressed to 43/73 --
  also perturbs the SECOND cluster, so the two clusters are not fully
  independent in how the register allocator responds to local pressure).
- Second-half interleaving: named early/late locals for `D_8008E228`/
  `D_8008E22C` mirroring retail's own fetch timing (no effect, byte-identical
  to the unnamed form), a mid-sequence `__asm__("")` barrier (regressed to
  28/73).

**Untested axis:** deliberately changing which values are cached in named
locals for `old60`/`old64` themselves (e.g. reading them through a
`volatile`-qualified access, or via the `D_8008EA26`-style pointer idiom) to
see whether forcing a different liveness shape for those two changes the
`$a2`/`$a3` allocation for `loBit`/`hiBit`. Not attempted this round for lack
of remaining budget after the two closed findings above consumed most of it.

### Proposed learning

**A bare `__asm__("")` scheduling barrier is not a universal lever, and
"the test is register vs. order" needs a corollary: it can also move a
residue in the WRONG direction, not just fail to move it.** Two placements
tried on this function each made the total match score sharply worse (73
words 51->28 and 51->36 respectively) despite each individually satisfying
CLAUDE.md's permitted-use test (removing them only changes instruction
order, never which register holds a value). The existing guidance already
says "if it doesn't move the residue, it is a stall" for the register-fix
case; this round's finding is that for a scheduling residue specifically,
a barrier can ALSO active make things worse by pinning the wrong
instruction to the wrong side of the boundary, so a regression from adding
a barrier is not by itself evidence that the surrounding reshape is wrong --
it may just mean the barrier itself was placed on the wrong side of a
genuinely floating instruction.

**Second, confirmed-independently learning, worth restating because it
generalizes beyond this function:** a value known-equal to a live register
by a simple, unconditional assignment to a global (`G = x;` immediately
followed by a read of `G`) gets FUSED by this pinned GCC 2.6.3 -- the
global round-trip is not "free" scaffolding, it is an optimisation
target, and reproducing retail's *actual* memory reload requires
`volatile` on the global's declared type. A bare scheduling barrier and a
pointer-indirection idiom (which works for a DIFFERENT case in this same
file, `SpuVmGetSeqVol`'s `s16 *cur = &D_8008EA22;`) both fail to force it
here; the difference between the two cases is not yet understood and is
worth a head-level look if the `D_8008EA22` idiom is ever needed on a third
global in this codebase.

## ROUND 31 (runner delta): rebuild confirms both title figures exactly

Rebuilt the preserved body verbatim through the current pinned pipeline.
**Length is exact (73/73 words, no drift) and the raw word-match is
51/73**, matching this report's title precisely. Re-ran
`tools/asm-differ/diff.py` and confirmed the same two residue clusters
described above: the `loBit`/`hiBit` register swap starts immediately
(`sllv a3,a1,v1` vs `sllv a2,a1,v1` at file offset 0x0220D8), shown as a
register RENAME (not new content) until the first genuinely NEW content
divergence at file offset 0x022124 (retail vram 0x80031924, an
immediate-operand difference against a different displacement) -- exactly
the "word 37" the title cites once register-only renames are set aside.

No new axis was attempted this round: the one flagged as untested (caching
`old60`/`old64` through a different access idiom) was not reached given
this round's per-function time budget, and this function's own findings
already show that barrier placement here is actively counterproductive
(51->28 or 51->36), so a cheap re-try of that axis was not worth the
attempt. Restored to `INCLUDE_ASM`; still a STALL, register-identity/
scheduling per CLAUDE.md rule 6.

## ROUND 32 (runner alpha): rebuild confirms both figures exactly; sibling-derived axis tried and rejected

Rebuilt the preserved body verbatim through the current pinned pipeline.
**Length is exact (73/73 words, no drift) and the raw word-match is
51/73**, matching this report's title precisely.

Noticed that `SsUtKeyOff` -- an ALREADY-MATCHED sibling in this same
unit whose `else` branch is the identical bit-mask/store idiom used here
(same `D_8008D9A3`/`D_8008D98C`/`_svm_voice` clears, same
`D_80090C60`/`D_80090C64`/`D_8008E228`/`D_8008E22C` mask-update sequence)
-- declares its two mask locals with ASYMMETRIC types: `u32 mask0` (the
`D_80090C60`-bound one) against `u16 mask1` (the `D_80090C64`-bound one),
not the matched pair of `u16`s this function's preserved body uses for
`loBit`/`hiBit`. Since `SsUtKeyOff` reproduces retail byte-for-byte with
that asymmetry, it looked like a promising, evidence-backed (not guessed)
lever for the `loBit`/`hiBit` register-swap residue this report's "The
unresolved residue" section describes.

**Tried: retyping `loBit` to `u32` (matching `mask0`) while leaving `hiBit`
as `u16` (matching `mask1`), everything else unchanged.** Result: **byte-
identical to the baseline, 51/73, same two residue clusters in the same
places** (confirmed via `tools/funcdiff.py`'s diff line list, not just the
raw count -- the exact same file offsets diverge in the exact same way).
The type asymmetry that matters in `SsUtKeyOff` does NOT transfer here.
The likely reason: in `SsUtKeyOff`, `1 << chan`/`1 << (chan-0x10)` are
directly the VALUES eventually stored into `D_80090C60`/`D_80090C64`
without any intervening OR against a cached local of a specific width in
the same way -- whereas here (per the CLOSED finding above) the -O2
pipeline's own promotion/narrowing rules make the `u16`-vs-`u32` distinction
on `loBit`/`hiBit` invisible once each is OR'd with a `u16` cached global
and stored back into a `u16` global: both declared widths produce the same
generated code. This is a genuinely different mechanism sharing only a
surface resemblance (both are the "which mask goes in which register"
question), not the same lever under two names -- worth recording so a
future attempt does not re-try this exact retype expecting the sibling's
result to carry over.

Restored to `INCLUDE_ASM`; `git status --porcelain` empty; whole-image
build re-verified green. Still a STALL, same classification as before
(scattered register-identity/scheduling residue, not reachable through any
legal lever tried across two rounds).

### Proposed learning

**A source-level fix that closes a byte-exact match in one function does
not automatically transfer to a sibling with superficially the same
C shape, even within the same source file and the same surrounding
declarations.** `SsUtKeyOff`'s `u32 mask0` / `u16 mask1` asymmetry is
load-bearing THERE, but produces zero-effect, byte-identical output when
applied to this function's `loBit`/`hiBit` -- the two functions differ in
how the mask value flows into its eventual store (a direct value in one
case, an OR-with-a-cached-global in the other), and that difference is what
determines whether the declared width can influence codegen at all. Before
assuming a working idiom from a matched sibling transfers, check whether
the VALUE'S PATH to its final store is actually the same shape, not just
whether the surface expression (`1 << x`) looks the same.

## ROUND 40 (runner charlie): rebuild confirms both title figures exactly; first real permuter search run, two sub-base leads found and both fail to reproduce

Rebuilt the preserved body verbatim through the current pinned pipeline.
**Length is exact (73/73 words, no drift) and the raw word-match is
51/73**, matching this report's title precisely -- no rot, no stale-symbol
gap (unlike the two `SsUtKeyOn`/`SsUtKeyOnV` siblings in this same
unit, this function's own declarations were untouched by round 34's split).

**This function had never been permuter-searched before this round** (per
this round's assignment, corrected from an earlier over-count of which
functions in this unit had already had the lever pulled). Set up via
`tools/setup-permuter.sh SsUtKeyOffV permuter-work/seed_func_80031890.c`;
scaffold built clean, `--debug --stack-diffs` base score **550**, consistent
with this report's two residue clusters (no stack or branch-target
penalty, all cost in register/reorder/insertion/deletion buckets).

Searched **123134 iterations** in 600s (`timeout 600 ... -j 12 --stack-diffs
--stop-on-zero --best-only`; `rc=124`, i.e. the bound fired, not a natural
stop). No zero was found. Two candidates below base score were found and
saved, both tried on the real oracle per CLAUDE.md's "a permuter zero (or
near-zero) is a LEAD, not an answer" -- **neither reproduced; both are
scaffold-local artifacts, not real levers**:

- **Score 150** (`output-150-1`/`-2`): hoists `bankIdx < 0x10` into a named
  boolean temp (`e228 = bankIdx < 0x10; if (e228)`, reusing the existing
  `e228` local for a second, unrelated purpose in the permuter's own
  output). Translated idiomatically (a distinct `u16 isLoBank` local,
  `isLoBank = bankIdx < 0x10; if (isLoBank) {...}`) and rebuilt through
  `./build-and-verify.sh` + `funcdiff.py`: **byte-identical to baseline,
  51/73, same exact diff offsets** (confirmed via the full diff list, not
  just the raw count). Zero effect on the real oracle.
- **Score 145** (`output-145-1`): a dead-store idiom
  (`new_var = 0; _snd_ev_flag = new_var;` instead of `_snd_ev_flag = 0;`)
  combined with folding the `D_80090C60` store into the `old60` assignment
  expression (`old60 = (D_80090C60 = loBit | old60);`). Tried the
  assignment-fold half in isolation (the dead-store half is unambiguous
  permuter noise, not worth a real-C translation): **regressed to 48/73**,
  three words worse than baseline. Reverted.

Both candidates confirm CLAUDE.md's and the matching guide's standing
warning in a new instance: **the permuter's isolated scaffold and the real
translation unit do not share the same register-allocation pressure**, so
a scaffold-local score improvement can be a pure artifact of the smaller
surrounding declaration set, not evidence about the real function. Neither
candidate is a new axis beyond what rounds 23/31/32 already explored
(`loBit`/`hiBit` register identity, mask-update interleaving) -- the
un-tested axis those rounds flagged (caching `old60`/`old64` through a
different access idiom) was not reached by the permuter's random search
either, since it never proposed a `volatile`/pointer-indirection variant
for those two locals specifically.

**Restored to `INCLUDE_ASM`; `git status --porcelain` empty for this
function; whole-image build re-verified green with all three of this
round's functions back at their baseline.** Still a STALL, same
classification as rounds 23/31/32: register-identity/scheduling residue,
now confirmed exhausted under a real (not merely reasoned-about) permuter
search of 123k+ iterations, not just hand reshaping.

### Proposed learning

**A permuter score improvement measured only in `--debug`/search output
(never round-tripped through the real oracle) is not evidence, no matter
how large the score drop looks (550 -> 150 is a 73% reduction in permuter
units) or how plausible the diff reads.** Two different candidates from
this round's search, of two different shapes (a hoisted boolean, an
assignment-expression fold), both scored well below the baseline in the
scaffold and both were byte-identical-or-worse against the real
`build-and-verify.sh` pipeline. The project's standing guidance to
"translate and re-verify every candidate, including the ones that look
best" is not a formality here -- it is the only thing that prevented two
false leads from being reported as progress.

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (`D_8008D988` itself now reads `_svm_voice` above) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

`src/` now clears `_svm_voice[chan].unk1B/unk04/unk00`. Byte-exact.

## Naming (round 95, track 6, delta)

Return type is now Sony's `short` (`<libsnd.h>`: `short SsUtKeyOffV(short
voice)`), byte-identical.
