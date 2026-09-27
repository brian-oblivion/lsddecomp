# Viewport__InitDefaults -- MATCHED round 49 (41/41 words, whole-image byte-exact)

> Renamed from `Unk18Obj__InitDefaults` on 2026-09-25 (tools/rename.py). Address 0x8003e968.

> Renamed from `func_8003E968` on 2026-09-23 (tools/rename.py). Address 0x8003e968.

## ROUND 49 (runner delta): MATCHED -- the asm-label alias defeats the address CSE, then a second independent scheduling residue is exposed and closed by two bare barriers

Re-verified the inherited stall first: spliced the round-46 body
(unchanged) into `src/code_2cc8c.c` in isolation (both of this
runner's units confirmed at their starting `INCLUDE_ASM` counts first)
and rebuilt -- **0/41 raw, `WARNING: differs OUTSIDE this range too**,
reproducing this report's own documented figure exactly. Not stale.

### Lever 1: an asm-label alias defeats the address CSE

The standing open question (rounds 44/46) was whether GCC 2.6.3's CSE of
a pure symbol address across two whole-struct copies is defeatable from
C89 at all. It is. Retail computes `gDefaultViewportColor`'s address (a `lui %hi`/
`addiu %lo` pair) INDEPENDENTLY for each of the two copies; our compiler
computes it once and reuses the register, because both reads are
syntactically the same declared symbol and GCC's local CSE recognizes
them as identical.

Declaring a SECOND extern name for the SAME linker symbol via GCC's
`__asm__("name")` alternate-name extension --

```c
extern SByte3_d294 gDefaultViewportColor;
extern SByte3_d294 D_8008A8F8_b __asm__("gDefaultViewportColor");
```

-- and reading the second copy through `D_8008A8F8_b` instead of
`gDefaultViewportColor` again gives the two accesses textually distinct symbols, so
CSE never fires: two independent `la`/`lui`+`addiu` pairs are emitted,
matching retail exactly. This is not a new idiom for the project --
`code_179d8_m.c` already uses `__asm__("D_8008D988")`-style aliasing for
array reinterpretation -- but it had not been applied to defeat an
address CSE before. Verified first in isolation through the pinned
pipeline (`tools/gcc263/cpp | cc1`) on a 6-line reproducer: the aliased
read produces a second `la $6,gDefaultViewportColor`, confirmed byte-for-byte
against retail's own two `lui`/`addiu` pairs once assembled in the real
function.

`volatile` (rounds 44/46) also defeats the CSE but forces a completely
different, much worse codegen shape (its own stack frame, a runtime
memcpy-shaped call) -- the alias-name trick costs nothing extra and
reproduces retail's actual instruction shape, not merely a
semantically-equivalent-but-structurally-wrong one.

This closed the LENGTH gap outright: the function is now exactly 41
words with no outside-range drift. It did not close the whole function --
see lever 2.

### Lever 2: closing the length exposed a second, independent scheduling residue

With the CSE fixed, `asm-differ` showed the entire byte-copy tail
(both `lui a2,0x8009`/`addiu`/`lb`x3/`sb`x3 blocks and everything after)
matching byte-for-byte -- confirming the CSE was genuinely the whole
length story. What remained was a residue in the function's FIRST 21
words: retail interleaves the two unconditional zero-inits
(`self->unk90 = 0; self->unk70 = 0;`) and the two global-loaded field
stores (`self->unk34.a = gDefaultViewportWidth; self->unk34.b = gDefaultViewportHeight;`) at
their natural source position, immediately after the two `lw`s that
produce them. Our build (correctly ordered in C, unchanged from every
prior round) instead deferred ALL FOUR of those stores to just before
the byte-copy blocks, apparently to interleave them with the run of
`li`/`sw` immediate-constant stores that follows.

**This is the SAME symptom rounds 44/46 already described ("the rest of
the function additionally gets rescheduled wholesale") but it is NOT
caused by the missing address computation** -- it survived fixing that
issue outright, so it is a second, independent scheduling residue that
happened to be invisible underneath the length gap.

Two bare `__asm__("");` scheduling barriers close it:

```c
self->unk90 = 0;
self->unk70 = 0;
__asm__("");
self->unk34.a = gDefaultViewportWidth;
self->unk34.b = gDefaultViewportHeight;
__asm__("");
self->unk3C = 0xD;
...
```

The first barrier alone (after the two zero-inits) moved those two
stores to their correct early position and raised the score from 20/41
to 22/41. The second barrier (after the two global-loaded stores) closed
the remainder outright: **41/41, whole-image `build-and-verify.sh`
byte-exact.** Both are ordinary CLAUDE.md-permitted scheduling barriers
(no register pinned, no operand constraint) -- confirmed by the fact
that removing either one only changes instruction ORDER (which stores
happen early vs. late), never which register holds a value.

### Final matched body

```c
extern s32 gDefaultViewportWidth;
extern s32 gDefaultViewportHeight;
extern SByte3_d294 gDefaultViewportColor;
extern SByte3_d294 D_8008A8F8_b __asm__("gDefaultViewportColor");

void Viewport__InitDefaults(Unk18Obj *self) {
    self->unk90 = 0;
    self->unk70 = 0;
    __asm__("");
    self->unk34.a = gDefaultViewportWidth;
    self->unk34.b = gDefaultViewportHeight;
    __asm__("");
    self->unk3C = 0xD;
    self->unk44 = 0x7D0;
    self->unk48 = 0x40;
    self->unk40 = 0x100;
    self->unk4C = 0xA;
    self->unk50 = 0x10000;
    self->unk54 = 0;
    self->unk60 = 0x4E20;
    self->unk5B = gDefaultViewportColor;
    self->unk58 = D_8008A8F8_b;
    self->unkB4 = 0;
    self->unkB8 = 1;
}
```

No struct/header changes -- all fields and offsets were already known
from prior rounds (see the round-44 section below). This unit's own
`code_2cc8c.h` is untouched by this match.

### Proposed learning (round 49)

**"No known C89 idiom for defeating this CSE" is a claim worth testing
against GCC's alternate-name extension before escalating to the
toolchain.** `__asm__("existing_symbol_name")` on a second extern
declaration of the same type gives the compiler two textually distinct
symbols for one linker symbol, which is enough to defeat a local CSE
pass that keys on syntactic identity, at zero codegen cost elsewhere --
unlike `volatile`, which defeats the same CSE but by making the compiler
treat every access as an observable side effect, with a much heavier
and much less retail-like cost. This project already had the idiom on
file (`code_179d8_m.c`, for a different purpose: reinterpreting an array
element type) but had not connected it to the address-CSE class of
residue.

**And: a "whole function rescheduled around a missing instruction"
diagnosis can describe TWO stacked, independent residues, not one.**
Fixing the length gap here did not auto-fix the reordering -- it fully
exposed a second, previously-invisible-underneath-the-drift issue that
needed its own, unrelated lever (bare scheduling barriers, the same tool
this unit's sibling functions already use for a similar hoist-ordering
problem). When a length-short residue's own report also describes a
scheduling symptom, do not assume closing the length closes the
schedule too -- re-measure after the length fix before declaring victory.

## ROUND 46 (runner delta): drift-checked fresh (consistent), one new lever tried and rejected -- `volatile` on just the READ side regresses even worse than `volatile` on the declaration

Re-spliced the round-44 body (unchanged) into `src/code_2cc8c.c` in
isolation and rebuilt: **0/41 raw, `WARNING: differs OUTSIDE this range
too (176860 bytes)`** -- reproduces this report's own documented figure
exactly (the raw 0/41 is the known-misleading window-alignment artifact
of the 2-word CSE'd shortfall, not a new regression). Not stale.

**New lever, not in the existing attempt list:** attempt 3 (already on
file) qualifies the GLOBAL's own declaration as `extern volatile
SByte3_d294 gDefaultViewportColor;`, forcing every access to it through a
conservative, always-observable-side-effect path. This round instead
qualified `volatile` only at the two READ *sites*, leaving the
declaration itself plain:

```c
self->unk5B = *(volatile SByte3_d294 *)&gDefaultViewportColor;
self->unk58 = *(volatile SByte3_d294 *)&gDefaultViewportColor;
```

This tests a genuinely different hypothesis than attempt 3: does the
conservative codegen come from the SYMBOL being declared volatile
everywhere, or from any one dereference being volatile-qualified?
**Result: WORSE, not merely unchanged** -- `asm-differ` shows the
compiler now lowers the whole-struct copy via an actual runtime call
(`jal 238a8`, a memcpy-shaped helper) inside a newly-introduced stack
frame (`addiu sp,sp,-0x20`, callee-saved `$s0`/`$s1` spilled), a
structurally different and even less retail-like shape than attempt 3's
own regression. **Confirms the CSE-defeating cost is inherent to
`volatile`-qualifying an AGGREGATE dereference at all in this compiler,
regardless of whether the qualifier sits on the declaration or the
access site** -- there is no narrower `volatile` spelling left to try
for this specific residue.

Reverted immediately; restored to `INCLUDE_ASM`. Full oracle
re-confirmed green. **Skip further attempts this round**: with the
`volatile`-at-any-granularity axis now closed on both ends (declaration
and access site), the memory-clobber-barrier axis already closed (round
44), and per-field/chained-assignment axes already closed (round 44),
and a ~29,000-iteration permuter search already run (round 44) — this
report's own standing question stands unresolved: whether GCC 2.6.3's
CSE of a pure symbol address across two whole-struct copies is
defeatable from C89 at all, or is a genuine toolchain-level fact to
escalate rather than a per-function problem. Not escalating unilaterally
per CLAUDE.md's "never experiment with the toolchain mid-round" rule --
flagging it here again for head-level review, as the existing report
already does.

### Proposed learning (round 46)

**`volatile`'s cost for defeating an address CSE does not shrink by
moving the qualifier from the declaration to the access site for a
struct-typed dereference** -- both spellings abandon the inline
load/store lowering entirely in favor of a runtime copy routine. This
narrows (rather than contradicts) the existing round-44 finding that
"volatile forces a completely different, WORSE codegen shape": the
worseness is a property of volatile-qualifying an AGGREGATE access in
this compiler, not of WHERE in the source the qualifier is spelled.

Unit `code_2cc8c`, carved round 13. Reopened round 42 as `gp_rel`-blocked
(the blocker is RESOLVED, see CLAUDE.md); the stub above was never actually
attempted until round 44.

## Round 44 (echo): every field/constant/global correct, but GCC hoists a
## redundant address computation retail does not, and reschedules the whole
## function around it

Straightforward constructor: zeroes/initializes 13 fields with a mix of
globals and immediate constants, guarded by nothing (unconditional). All
new field OFFSETS and VALUES were already known from sibling functions in
this unit (`unk90`, `unk70`, `unk34` (`Pair32_d294`), `unk3C`/`unk40`/
`unk44`/`unk48`/`unk4C`/`unk50`/`unk54`/`unk60`, `unk58`/`unk5B`
(`SByte3_d294`), `unkB4`/`unkB8`) -- no new struct derivation needed, this
is a pure codegen residue.

```c
extern s32 gDefaultViewportWidth;
extern s32 gDefaultViewportHeight;
extern SByte3_d294 gDefaultViewportColor;

void Viewport__InitDefaults(Unk18Obj *self) {
    self->unk90 = 0;
    self->unk70 = 0;
    self->unk34.a = gDefaultViewportWidth;
    self->unk34.b = gDefaultViewportHeight;
    self->unk3C = 0xD;
    self->unk44 = 0x7D0;
    self->unk48 = 0x40;
    self->unk40 = 0x100;
    self->unk4C = 0xA;
    self->unk50 = 0x10000;
    self->unk54 = 0;
    self->unk60 = 0x4E20;
    self->unk5B = gDefaultViewportColor;
    self->unk58 = gDefaultViewportColor;
    self->unkB4 = 0;
    self->unkB8 = 1;
}
```

`gDefaultViewportWidth`/`gDefaultViewportHeight` are plain `.sdata` words (already in
`config/gp-symbols.txt`, ordinary `%gp_rel` accesses, no issue).
`gDefaultViewportColor` is a 4-byte all-zero `.sdata` region, read as a whole
`SByte3_d294` struct (3 signed bytes) via its ADDRESS -- retail computes
that address with an ABSOLUTE `lui`/`addiu` (not `%gp_rel`), consistent
with CLAUDE.md's rule that `la` of an sdata symbol stays absolute even
though loads/stores of it go through `$gp`.

## The residue

Retail copies the same 3-byte struct from `gDefaultViewportColor` TWICE (once to
`self->unk5B`, once to `self->unk58`), and computes the struct's address
via `lui`/`addiu` **independently each time** -- two separate 2-instruction
`lui %hi(gDefaultViewportColor)` / `addiu %lo(gDefaultViewportColor)` sequences, back to back
with nothing in between. My build's GCC 2.6.3 CSEs this: it computes the
address ONCE and reuses the same register for both copies, coming out
**2 words (8 bytes) shorter** than retail. The rest of the function
additionally gets **rescheduled wholesale** around this: retail issues the
two loads of `gDefaultViewportWidth`/`gDefaultViewportHeight` early (to hide load latency) but
stores them, and stores the two unconditional zero-inits (`unk90`,
`unk70`), almost immediately after; my build defers ALL FOUR of those
stores to just before the byte-copy blocks, apparently because removing
one redundant address computation shifts the scheduler's priority
ordering for the surrounding independent stores. Confirmed via
`asm-differ` (realigned): the first REAL difference is at the function's
own first instruction (`sw zero,0x90(a0)` retail vs the CSE'd build's
`lw v1,...(gp)` first) -- everything downstream realigns cleanly by a
constant -8 byte offset except for the two duplicated `lui`/`addiu` pairs.
Raw (non-realigned) `funcdiff.py` reports 0/41 in-range words, which is
misleading: the fixed window compare can't see past the missing 2 words,
even though nearly every instruction downstream is present, just shifted.

## What was tried (all real attempts, all reverted)

1. Straightforward whole-struct assignment as shown above -- **39/41
   instructions** (2 short), CSE'd address, rescheduled. Best reached.
2. A bare `__asm__ __volatile__("" ::: "memory")` between the two copies --
   **no change at all**, byte-for-byte identical to attempt 1. A memory
   clobber only forces MEMORY operations not to be reordered/cached across
   it; a pure address CONSTANT (no memory read involved in computing it)
   isn't affected, so it can't stop this specific CSE.
3. `extern volatile SByte3_d294 gDefaultViewportColor;` -- markedly WORSE: forces a
   completely different, more conservative codegen shape with its own
   stack frame (`addiu sp,sp,-8`) and a saved `$s0`, ~44 instructions.
   Volatile makes the compiler treat every access as an observable side
   effect, which stops the CSE but at a much higher cost that doesn't
   remotely resemble retail either.
4. Per-field byte assignments (`self->unk5B.b0 = gDefaultViewportColor.b0;` etc.,
   6 statements instead of 2 whole-struct copies) -- WORSE and in a new
   way: GCC emits `lbu` (zero-extending) loads for the per-field version
   instead of retail's `lb` (sign-extending), since the compiler can prove
   the sign bits are dead when each byte flows straight into another `s8`
   field. Confirms the whole-struct-assignment shape (attempt 1) is the
   right one; per-field access diverges further, not closer.
5. A chained assignment, `self->unk58 = (self->unk5B = gDefaultViewportColor);`,
   suggested by a bounded permuter search (below) -- compiles to reading
   the SECOND copy from `self->unk5B` (i.e. `self`+0x5B) rather than
   re-reading the global, confirmed by disassembly (`lb v0,0x5b(a0)` etc.,
   no second `lui %hi(gDefaultViewportColor)` at all). Semantically valid (both
   sides are zero either way) but structurally wrong: retail's two
   identical `lui %hi(gDefaultViewportColor)` pairs prove it re-reads the GLOBAL
   both times, not `self`. Real in-range word match: 7/41, worse than
   attempt 1.

## Permuter: ~29,000 iterations under load (bounded, `timeout 300`), no
## zero, one heuristic-better lead that measured worse for real (attempt 5
## above)

Seeded from attempt 1 (the 39/41 CSE'd body). Base score (permuter's own
heuristic, NOT a real byte count): **1045**. Search ran `-j 4
--stack-diffs --stop-on-zero --best-only` for 300 seconds under whatever
load this worktree's host had at the time (**phrased as "not closed in
~29,000 iterations under load", not permuter-exhausted**), reaching
roughly iteration 28,860 with no candidate at 0 and no zero. The saved
`output-350-1` candidate (permuter score 350, the lowest saved) is the
chained-assignment idea (attempt 5 above) -- diffed against the seed and
then verified through the REAL pinned toolchain, it scores WORSE
(7/41 real vs the seed's structurally-closer-but-unaligned 39/41), another
confirmation of CLAUDE.md's standing warning that a permuter's heuristic
score is not a real byte count and must be verified, not trusted. The
other five saved candidates (`output-425/485/605/635/725`) are all inert
`do { ... } while (0)` statement regroupings with no `diff`-visible
semantic change and (by inspection) no reason to affect codegen.

## Proposed learning

**A bare `__asm__` memory-clobber barrier does not prevent CSE of a pure
ADDRESS constant** (a `lui`/`addiu` pair with no memory read involved) --
only actual memory operations are ordered/cached relative to it. This is a
narrower, previously-undocumented corollary of the existing
register-identity-vs-instruction-order distinction in CLAUDE.md: the
barrier's reach is bounded to memory side effects, not to arbitrary
compiler optimizations. For a redundant symbol-address computation retail
does NOT eliminate, this project currently has no known permitted C-level
lever -- `volatile` overcorrects into an unrelated codegen shape, and
restructuring the copy (per-field, chained) either diverges further or is
semantically-equivalent-but-structurally-wrong. Flagging this as an open
question for a future round or head-level review: is there a legitimate
C89 idiom for "recompute this address, don't cache it" that this project
hasn't tried yet, or is this a genuine case where GCC 2.6.3's CSE pass
cannot be defeated from the C source at all (making it a to-be-escalated
toolchain question, not a per-function one)?

## Naming

`Unk18Obj__InitDefaults` -- tier A. Slot40 occupant, called once from `Viewport__Viewport`'s own constructor tail (`self->methods->slot40(self)`, `code_2cc8c.c`) immediately after the base ctor. Body is straight-line field initialization to constants. Mechanics (post-ctor default init) are the whole of its purpose.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__InitDefaults`. The +0x040 slot's occupant, named `initDefaults`. The defaults name the fields: screenSize 256x240 (gDefaultViewportWidth/gDefaultViewportHeight), otLength 13, unk44 2000, unk48 64, projH 256, nearZ 10, farZ 0x10000, drawEnabled 1. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`), three sites, all kept:

- **First bare `__asm__("")`** (after `self->otReady = 0;`) -- **justified**,
  now commented at the site. Deleting it alone: image red (12 bytes),
  `funcdiff` 37/41, asm-differ shows `lw v0,0xf4(gp)` / `lw v1,0xf8(gp)`
  (`gDefaultViewportWidth`, `gDefaultViewportHeight`) hoisted above `sw zero,0x90(a0)` and
  `sw zero,0x70(a0)`. Instruction order.
- **Second bare `__asm__("")`** (after the two `screenSize` stores) --
  **justified**, now commented at the site. Deleting it alone: image red
  (35 bytes), `funcdiff` 22/41, asm-differ shows `sw ...,0x34(a0)` /
  `sw ...,0x38(a0)` sunk from right after their loads to below the whole run
  of constant stores (`otLength` through `fogNear`, `+0x3C..+0x60`), with the
  two loaded values moving to `v1`/`a1` because `v0` is then reused by the
  constants. The primary change is instruction order.
- **The asm-label alias** `extern ViewportRgb D_8008A8F8_b
  __asm__("gDefaultViewportColor");` -- already justified at the site by the block
  comment above it (a second textual name defeats GCC's CSE of the two
  `gDefaultViewportColor` address computations). Re-measured by spelling the second
  copy `self->clearColor = gDefaultViewportColor;`: image red (the function came out two
  words shorter, 39 against 41, `funcdiff` 17/39, and the whole image after it
  drifted). So the alias is load-bearing; which instructions went missing was
  not itemised. Kept.


## Track 7 (round 95, alpha, polish pass)

Constants in decimal (otLength 13, unk44 2000, unk48 64, projH 256, nearZ 10, farZ 65536, fogNear 20000). The data it reads are renamed with `tools/rename.py`: `D_8008A8FC`/`D_8008A900` -> `gDefaultViewportWidth`/`gDefaultViewportHeight` (256, 240), `D_8008A8F8` -> `gDefaultViewportColor` ({0, 0, 0}); the local alias `D_8008A8F8_b` is now `gDefaultViewportColorAlias` (a C identifier only; its `__asm__` name is the symbol). The comment above the externs now keeps only `MATCHING:` lines; what it said, verbatim:

```c
/* MATCHED round 49. Two levers were needed, see docs/match-reports/Viewport__InitDefaults.md:
 * (1) retail reloads the address of `gDefaultViewportColor` INDEPENDENTLY for each of
 * the two whole-struct copies (two separate lui/addiu pairs); GCC 2.6.3
 * otherwise CSEs that into one shared computation. Declaring a second
 * extern name aliased to the same symbol via `__asm__("gDefaultViewportColor")` (the
 * same alternate-name idiom `code_179d8_m.c` already uses) gives the
 * second copy a textually distinct symbol, defeating the CSE without
 * `volatile`'s much worse codegen. (2) Two bare `__asm__("")` scheduling
 * barriers pin the two independent zero-inits and the two global-loaded
 * stores to retail's own early positions instead of letting the scheduler
 * defer them to just before the byte copies. */

```


## Track 7 (round 100, echo, polish pass)

## Constants

The defaults became unit-local `#define`s in code_2cc8c.c (only InitDefaults uses them): `VIEWPORT_DEFAULT_OT_LENGTH` 13 (8192 tags), `VIEWPORT_DEFAULT_MAX_PACKETS` 2000, `VIEWPORT_DEFAULT_PACKET_SIZE` 64, `VIEWPORT_DEFAULT_PROJ_H` 256, `VIEWPORT_DEFAULT_NEAR_Z` 10, `VIEWPORT_DEFAULT_FAR_Z` 65536, `VIEWPORT_DEFAULT_FOG_NEAR` 20000; `lightMode = 0` -> `GsLMODE_NORMAL` (libgs.h). Fields `unk44`/`unk48`/`unk90`/`unkB4` are now `maxPackets`/`packetSize`/`clockEventCount`/`extraSwap`.
