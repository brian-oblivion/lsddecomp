# SceneNode__RaycastHullAgainstFaces -- MATCHED (round 76, bravo: 199/199, whole image green; was 29/199 length-exact)

> Renamed from `SceneNode__ClassifyAgainstPlanes` on 2026-09-27 (tools/rename.py). Address 0x8001ddf4.

> Renamed from `Class6B5CC__ClassifyAgainstPlanes` on 2026-09-26 (tools/rename.py). Address 0x8001ddf4.

> Renamed from `func_8001DDF4` on 2026-09-18 (tools/rename.py). Address 0x8001ddf4.

REVISITED, round 76: MATCHED 199/199 (build exit=0, whole-image SHA1 green); names/types used -- `list` retyped to the corner-list struct TryAttachNearby's match established (`AttachCornerList_d294b`), `flag2` renamed `hit`.

## Round 76 (bravo): MATCHED -- four levers, none of them register allocation

**Rebuild first.** The round-69 `#ifdef NON_MATCHING` body (the round-46/55
preserved body) rebuilt live: **29/199, insertions 12 / deletions 12,
positional skeleton diffs 166**. So the "register identity" title was never
a same-length swap. There was real structure to find.

Twelve builds took it to the match, in this order (`funcdiff` / ins-del):

1. **Argument type, one loop variable for both plane loops, and
   `self->unk2C = 0` before the `TmdModel__GetBoundsCount` call.** `list` is the same
   `{s32 count; Vec3S16_d294 v[8];}` local that TryAttachNearby passes (same
   round). Retail keeps the plane index for Part 2 AND Part 3 in `$s4`, so
   the source uses ONE variable `i` where the old body had `i` and `j`. And
   retail's `sw zero,0x2C(s5)` sits in the call's delay slot, which reorg can
   only fill from BEFORE the call. With these three changes `self` moved from
   `$s4` to retail's `$s5` and every Part 2/3 saved register agreed. (Round
   55 had measured the `unk2C` reorder alone as inert. It was, alone.) Part 1
   written indexed at this stage was 196 words, 7/199.
2. **Part 1 is three pointer walks.** `for (p = mid; p < &mid[2]; p++)`
   gives retail's un-folded entry test (`sltu a3,t1; beqz`) and its
   x-pointer plus y/z-pointer split (`sh -2(a2)`/`sh 0(a2)`, the second a
   giv at `&p->z`). `v` and `hi = list->v + 2`, each `+= 4`, give the four
   read pointers. A single `v` with `v[2]` combined all reads into one giv
   (20/199). Two pointers: 59/199, ins/del 5/5.
3. **Part 3 has no `sHitHeightGate` gate.** Retail's inner test is only
   `lw 0x44(sp); slti 0x201; bnez`. The derived body copied Part 2's
   `sHitHeightGate == 0 ||`. Removing it: 56/199 raw, but ins/del **1/1**, and
   the `m`-loop's early `addiu s2,s2,1` in the delay slot fixed itself.
4. **The last residue was a loop.c movable decision, read from `cc1 -dL`.**
   Retail hoists the constant `1` of `1 << i` into `$s1` in the Part 2 loop
   preheader (`li s1,1` after `addiu s0,sp,0x1e`). The build rematerialized
   `li v0,1` in two delay slots. The loop dump said
   `Insn 193: regno 125 (life 1), move-insn savings 1 not desirable`. The
   test is `threshold * savings * lifetime >= insn_count`, with threshold
   about `1 + n_non_fixed_regs - 3` for a loop with calls, against 37 real
   insns. Negatives, each one build: `x = x | (1 << i)` was inert. A named
   `bit = 1` before the loop reached 195/199, but in the wrong place (before
   the entry test). `bit = 1` as the first loop statement reached 197/199,
   but its movable came BEFORE `&mid[1]`'s, so the two preheader insns
   swapped. `bit = 1` inside either `if` was worse (183/199 and 56/199).
   **Closing form:** the `||` is two arms that each set the bit,

   ```c
   if (sHitHeightGate == 0) {
       self->unk2C |= 1 << i;
   } else if (outWord >= 0x201) {
       self->unk2C |= 1 << i;
   }
   ```

   That gives loop.c two equal constant-1 movables, which it combines
   (savings 2). The pair is then desirable and hoisted, in its textual
   position after `&mid[1]`, and cross-jumping merges the two arms back into
   retail's single tail. 199/199, build exit=0.

A final cleanup build (`for (i = 0; ...)` restored, with `self->unk2C = 0`
still before the call) stayed 199/199. The `u8 pad[0x18]` from round 46
stays: it is `sp+0x24..0x3B`, which nothing references. No permuter was run,
so Gate 3 was not needed. `tools/check-nonmatching.sh` is green (22 bodies:
this function's `#ifdef` is gone).

### Proposed learning (round 76)

**A constant that retail hoists into a saved register and your build
rematerializes is a loop.c DESIRABILITY miss, not register allocation.**
`cc1 -dL` says it outright: `(life 1) ... savings 1 not desirable`. The rule
is `threshold * savings * lifetime >= insn_count`. `savings` goes up when
the loop holds two equal invariant loads, so an `a || b` guarding one
statement that was really two arms setting the same thing (`if (a) S; else
if (b) S;`) doubles it. Cross-jumping then merges the arms, so the only
trace left in the bytes is the hoist. A named variable set to the constant
moves the hoist to the wrong place (before the entry test, or in the wrong
movable order), and that difference is the discriminator.

**A body that was DERIVED rather than written can copy one arm's condition
into another.** Part 3's `sHitHeightGate == 0 ||` was never in retail. Before
anything else, check every branch in the derived body against a `lw`/`beqz`
in the asm.

## History (rounds 45-55)

Round 46 (echo). **This is the first time C was ever written or built for
this function.** Round 45 (delta) filed a structure-only derivation with NO
score at all -- see the git history for that write-up, superseded below now
that it has an actual measurement. Two-thirds of that derivation (Part 1 and
Part 2) transcribed directly into working C; Part 3's open ambiguity is
resolved below.

## What round 45 handed this round

Signature `s32 SceneNode__RaycastHullAgainstFaces(SceneNodeObj *self, s32 *outFlag, Vec3S16_d294
*diff, void *list)`, with Part 1 (a fixed 2-row box-midpoint average into a
local `Vec3S16_d294 mid[2]`) and Part 2 (a `count1`-driven loop over
`TmdModel__GetBoundsBuffer` planes, gated by `ClipSegmentToBox`/`TmdModel__RaycastFaces`, setting
bits in `self->unk2C`) both fully derived and high-confidence. Part 3 (a
second, `list`-driven double loop) was flagged NOT fully decoded: "the
precise relationship between the middle `k` loop ... and the inner fixed-4
`m` loop" was left as the open question for whoever picked this up next.

## Part 3, resolved

Re-read `asm/nonmatchings/SceneNode/SceneNode__RaycastHullAgainstFaces.s` lines 197-268
(`.L8001DFD0` through `.L8001E0B4`) instruction-by-instruction:

- The innermost loop runs `m = 0..3` (4 passes, unconditional), but the
  box-test body (the `ClipSegmentToBox`/`TmdModel__RaycastFaces` pair) only executes
  when `(u32)(m - 1) < 2`, i.e. **`m == 1` or `m == 2`** -- confirmed at
  `sltiu $v0, $v0, 2` / `beqz $v0, .L8001E094` (vram 0x8001E00C-0x8001E010).
  `m == 0` and `m == 3` fall straight through to the loop-continue check with
  no call.
- `rowBase` (`$s1`) advances by **6 bytes on EVERY `m` pass** (`addiu
  s1,s1,6`, the delay slot of the `m`-loop's own `bnez`, vram 0x8001E09C) --
  all 4 passes, whether or not the box test ran -- **then ANOTHER `+0x18`
  once per `k` pass**, in the delay slot of the `k`-loop's own `bnez` (vram
  0x8001E0B0). Net advance per `k` iteration: `4*6 + 0x18 = 0x30` (48 bytes,
  two 0x18-byte rows), not the single `0x18` a naïve reading suggests.
  `rowBase` resets to `list+4` at the top of every `j` (outer) pass.
- The box test itself passes `rowBase` (current, pre-increment value) and
  `rowBase+0x18` as the two `Vec3S16_d294*`/`BoundsBox_d294*` arguments --
  the SAME call shape as Part 2, just sourcing its pointers from the sliding
  `rowBase` instead of the fixed `&mid[0]`/`&mid[1]`.

This confirms round 45's own suspicion ("sliding/overlapping windows, not 4
independent rows") precisely: the four `m`-passes step through FOUR
consecutive 6-byte slots of one nominal `0x18`-byte "row" (matching Part 1's
row stride), but only the middle two slot-positions (`+6`, `+12`) are ever
tested, each paired with the same slot position one row+extra later
(`+0x18` further).

## The `flag2`/$s2 "dead" branch -- transcribed literally, not eliminated

At `.L8001DF78` (after Part 2), if `self->unk2C != 0`, retail:
1. Always sets `*outFlag = 1` (in a branch delay slot, unconditionally --
   the store executes regardless of the immediately-following branch's
   outcome).
2. Branches to the shared epilogue (`.L8001E0DC`) with `$v0` **already
   holding the return value** (`1` on one path, `2` on the other, both
   nonzero) -- this path does NOT go through the normal
   `lw v0,(outFlag); sltu v0,zero,v0` recomputation at `.L8001E0C8`, it jumps
   straight past it.
3. The branch condition is `$s2` (`flag2` here), which per round 45's own
   grep is written exactly ONCE in the whole function (`= 0`, right before
   Part 2's loop) and NEVER again before this test -- i.e. it is
   dynamically always `0`> in every execution this binary can produce.

Transcribed as:
```c
if (self->unk2C != 0) {
    *outFlag = 1;
    if (flag2 != 0) {
        return 2;
    }
    return 1;
}
```
with `flag2` a plain `s32` local initialized to `0` and never written again.
**Retail's own compile did not eliminate this branch**, even though it is
provably dead from a pure dataflow standpoint -- that in itself is a data
point about what this exact GCC 2.6.3/-O2 configuration does and does not
constant-fold across a loop containing calls (it did NOT eliminate it here,
unlike the signed/unsigned `unk10` case in `SceneNode__RaycastVertical`'s round-19
history, which is textually a much more local fold). My build reproduced
this branch as written (did not get eliminated on my side either) -- the
residue here is register identity, not branch presence/absence.

## New symbols added (own-file, not shared header)

```c
extern s32 TmdModel__RaycastFaces(void *arg0, s32 *arg1, Vec3S16_d294 *arg2, s32 *arg3, Vec3S16_d294 *arg4, Vec3S16_d294 *arg5);
extern s32 sHitHeightGate;
```

**Note the signature conflicts with `scene_node.c`'s own existing local
extern** (`s32 arg3` there, literal `0` at its only call site) -- THIS
function's own call site passes `&outWord`, a genuine pointer, in that
position. Both externs are legitimate, independent local views (per this
project's own documented convention) since they live in different
translation units; not committed to the shared header since the two
readings disagree on arg3's type and neither is proven to be the callee's
real signature (it is a linked PsyQ object, never decompiled here).

## Lever: the frame was 0x18 bytes short -- an unaccounted stack buffer

First build, all locals as directly derived from the report: frame came out
`0x98` (152 bytes) against retail's `0xB0` (176), a flat 24-byte gap visible
in the very FIRST instruction (`addiu sp,sp,-0xN`) -- score **14/199**, and
every subsequent word differed as a result (nothing downstream can line up
while the frame pointer itself is wrong).

**Fix: add an unused `u8 pad[0x18];` local.** This is the same shape as
`SceneNode__CheckBoundsOverlap`'s own history in this unit (round 20: "fixing the frame size
(0x98 -> 0xF8, a 24-word unused-buffer padding)") -- GCC 2.6.3 reserves
stack space for locals whose source declaration doesn't survive into any
generated reference (dead-but-declared buffers, or -- more likely here,
since this function is *never* been compiled before -- a buffer this
derivation's field-offset reading simply never needed to name because
nothing in the traced control flow reads or writes it). Adding the pad
alone took the score to **29/199, word 0 now matching exactly** (frame size
confirmed correct) -- real, measured progress, not a guess left unverified.

**This is a proposed learning, not yet a settled rule**: the pad's exact
byte count matching `0x18` (the same size as this function's own outgoing
6-argument call area) may be coincidence rather than the true cause; nobody
has identified an actual field/local at that offset. Flagging for whoever
next attempts this function rather than asserting a diagnosis this round
didn't verify further.

## Remaining residue at 29/199

First real diff, vram `0x8001DDF8` (word 1): retail `sw $s5,0x9c($sp)` /
mine `sw $s4,0x98($sp)` -- `self` occupies `$s5` in retail, `$s4` in this
build, a one-register-and-one-slot shift that propagates through most of
the function's saved-register block and beyond (170 of 199 words differ in
total). This looks like the classic "too few/too many other values compete
for saved registers ahead of `self` in program order" register-identity
class this project has repeatedly found NOT responsive to `__asm__` barriers
or simple declaration reordering (see `SceneNode__RaycastVertical`'s own round-44
residue #1, same symptom, same unit). Not attempted further this round --
budget went to establishing the first honest score and closing the
structural unknowns instead of iterating blind on a 10-candidate saved-
register permutation with no tool support for exploring it directly.

## Preserved body (29/199, no drift)

```c
#if 0
extern s32 TmdModel__RaycastFaces(void *arg0, s32 *arg1, Vec3S16_d294 *arg2, s32 *arg3, Vec3S16_d294 *arg4, Vec3S16_d294 *arg5);
extern s32 sHitHeightGate;

s32 SceneNode__RaycastHullAgainstFaces(SceneNodeObj *self, s32 *outFlag, Vec3S16_d294 *diff, void *list) {
    Vec3S16_d294 mid[2];
    s16 *loPtr;
    s16 *hiPtr;
    s32 row;
    s32 count1;
    s32 i;
    Sixteen6_d294 *plane;
    s32 flag2;
    s32 cnt2;
    s32 j;
    u8 *rowBase;
    s32 k;
    s32 bitJ;
    s32 bitK;
    s32 m;
    s32 bigConst;
    s32 outWord;
    u8 pad[0x18];

    bigConst = 0x7FFFFFFF;

    loPtr = (s16 *)((u8 *)list + 4);
    hiPtr = (s16 *)((u8 *)list + 0x10);
    for (row = 0; row < 2; row++) {
        mid[row].x = (loPtr[0] + hiPtr[0]) >> 1;
        mid[row].y = (loPtr[1] + hiPtr[1]) >> 1;
        mid[row].z = (loPtr[2] + hiPtr[2]) >> 1;
        loPtr = (s16 *)((u8 *)loPtr + 0x18);
        hiPtr = (s16 *)((u8 *)hiPtr + 0x18);
    }

    count1 = TmdModel__GetBoundsCount(self->unk20);
    self->unk2C = 0;
    flag2 = 0;
    for (i = 0; i < count1; i++) {
        plane = TmdModel__GetBoundsBuffer(self->unk20, i);
        if (ClipSegmentToBox(NULL, (BoundsBox_d294 *)plane, &mid[0], &mid[1])) {
            if (TmdModel__RaycastFaces(self->unk20, &bigConst, diff, &outWord, &mid[0], &mid[1])) {
                if (sHitHeightGate == 0 || outWord >= 0x201) {
                    self->unk2C |= (1 << i);
                }
            }
        }
    }

    if (self->unk2C != 0) {
        *outFlag = 1;
        if (flag2 != 0) {
            return 2;
        }
        return 1;
    }

    *outFlag = 0;
    cnt2 = *(s32 *)list;
    for (j = 0; j < count1; j++) {
        plane = TmdModel__GetBoundsBuffer(self->unk20, j);
        bitJ = 1 << j;
        rowBase = (u8 *)list + 4;
        for (k = 0; k < cnt2; k++) {
            bitK = 1 << k;
            for (m = 0; m < 4; m++) {
                if (m == 1 || m == 2) {
                    u8 *rowM = rowBase;
                    u8 *rowMplus1 = rowBase + 0x18;
                    if (ClipSegmentToBox(NULL, (BoundsBox_d294 *)plane, (Vec3S16_d294 *)rowM, (Vec3S16_d294 *)rowMplus1)) {
                        if (TmdModel__RaycastFaces(self->unk20, &bigConst, diff, &outWord, (Vec3S16_d294 *)rowM, (Vec3S16_d294 *)rowMplus1)) {
                            if (sHitHeightGate == 0 || outWord >= 0x201) {
                                self->unk2C |= bitJ;
                                *outFlag |= bitK;
                            }
                        }
                    }
                }
                rowBase += 6;
            }
            rowBase += 0x18;
        }
    }

    return (*outFlag != 0);
}
#endif
```

## Permuter

NOT run this round -- given the residue is a single saved-register identity
shift affecting the vast majority of the function's 199 words (170/199
differ), and this project's own round-45 guidance that a pure
register-identity residue lies outside a source-mutation search's reach,
hand analysis (declaration-order experiments) looked like the better use of
remaining time than a slow (199-word) permuter search whose scaffold would
first need its own base-score-agreement check. Flagging as a candidate for
whoever picks this up next, but NOT claiming it was tried and failed --
it simply was not attempted.

### Proposed learning

**A function that was previously "screened, not built" can go from a
literal ZERO figure to a real, if incomplete, score in one sitting by
transcribing the high-confidence parts of an inherited derivation and
building it -- the frame-size gap alone (a single missing padding buffer)
cost 15/199 words before it was found, exactly the same lever
`SceneNode__CheckBoundsOverlap` needed in this same unit two rounds ago.** When a derivation
report is inherited with NO C ever attempted, build the solid parts first
and let the frame-size check (word 0 of `funcdiff`'s output) tell you
immediately whether an unaccounted local exists, rather than reading
further into the diff before that's settled.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `SceneNode__RaycastHullAgainstFaces`
(tier B). STALL, still `INCLUDE_ASM`; not attempted for a match this
round (naming pass only, and this is the documented `gp_rel`-history
blocker's successor -- see CLAUDE.md's "Open toolchain blockers" table,
now RESOLVED, so this function is ordinary matching work for whoever
picks it up next). Slot `+0x0AC` occupant: computes a fixed 2-row
box-midpoint average, then classifies it against every one of
`self->unk20`'s planes (via `ClipSegmentToBox`/`TmdModel__RaycastFaces`,
setting bits in `self->unk2C`), and if none set, runs a second,
`list`-driven sliding-window pass doing the same per-plane
classification over a corner list. "ClassifyAgainstPlanes" describes
the measured mechanics (a plane-membership/clip test, not a specific
game concept); which planes `self->unk20` holds is not established
beyond "the same planes `SceneNode__CheckBoundsOverlap` reads." Held
back from an actual rename because this symbol is referenced (in a
comment) from `src/graphics/scene_node.c:414` -- a different unit. Posted to
the broadcast.

## Round 55 (charlie): REVISITED (round 55) -- register-identity framing
## incomplete; the real Part-1 residue is a different LOOP SHAPE, not just
## `self`'s register; one structural rewrite tried, net negative

Assigned as a track-1 REVISIT job (FINISHING-PLAN.md's revisit rule): this
unit passed track 3 naming last round. Rebuilt the round-46 preserved body
live first: reproduces exactly, `build exit=2`, no compile errors,
`funcdiff.py` confirms **29/199**, no drift, isolated (only this function's
own `#if 0` wrapper removed; every sibling confirmed still wrapped via
`grep -c '^INCLUDE_ASM' src/graphics/scene_node.c`).

**The round-46 title's own framing -- "register identity: `self` lands in
$s4 here, $s5 in retail" -- undersells the residue.** Reading
`asm/nonmatchings/SceneNode/SceneNode__RaycastHullAgainstFaces.s` against
the built object's own disassembly line by line (not just the funcdiff word
count) for the FIRST loop (the 2-row box-midpoint average, this report's own
"Part 1") shows retail does not index through `mid[row].x/.y/.z` at all.
Instead it walks the `mid` array with TWO separate raw pointers advancing by
0x18 (one row) per iteration -- an X-only pointer (`$a3`, writes `mid[row].x`
then `+=6`) and a combined Y/Z pointer (`$a2`, writes `mid[row].y` at `-2`
then `mid[row].z` at `0`, then `+=6`) -- fed by FOUR separate read pointers
(`$s1`/`$t0` for the X field of lo/hi, `$a0`/`$a1` for the Y and Z fields of
lo/hi via the same `-2`/`0` offset trick), all four read pointers ALSO
advancing by 0x18 per iteration. This is a completely different induction
variable decomposition than the indexed `for (row = 0; row < 2; row++)
{ mid[row].x = ...; mid[row].y = ...; mid[row].z = ...; }` this report's own
preserved body writes, even though both compute the identical result.

**Confirmed this is not an artifact of `self`'s placement by testing it
directly:** rewrote Part 1 with the matching pointer decomposition (four
read pointers `xLo`/`xHi`/`yzLo`/`yzHi`, two write pointers `xDst`/`yzDst`,
a `do { } while` loop bounded by a raw pointer comparison against `xEnd`,
mirroring retail's exact read/write pattern including the `-2`/`0` offset
pairing for Y/Z). Built clean, but this made things WORSE, not better:
`nm -S` gives `0x328` = 202 words (+3 over retail's 199), `funcdiff.py`
score dropped to **16/199** with the out-of-range byte count blowing out to
345316 bytes (the whole rest of the image shifted). Reading the resulting
disassembly showed the loop BODY itself came out close to retail's shape
(mostly register renames), but the surrounding call-argument setup picked up
two EXTRA spill/reload instructions (`lw t3`/`addiu t2` in place of a single
`lw t2`) that retail's version doesn't have, and outFlag's own stack store
moved earlier -- i.e. matching Part 1's internals in isolation perturbed
something in the PROLOGUE/argument-marshalling area enough to cost more than
it saved. Reverted immediately; confirmed byte-identical to the round-46
preserved body afterward.

**This is a genuine, verified negative, not a null result:** the residue
here is not simply "get `self` into the right register" -- it is a
whole-function register-pressure interaction where Part 1's own induction
variables (four read pointers plus two write pointers, all candidates for
callee-saved registers even though the loop makes no calls) compete for the
SAME register pool as `self`, the loop counters `i`/`j`/`k`/`m`, and every
other long-lived value in the ~150 lines of code that follow. Reproducing
Part 1's own instruction sequence exactly is necessary but not sufficient,
and doing it in isolation (without also getting the REST of the function's
register pressure profile to match) actively regressed the score. This
matches the class CLAUDE.md's HARD RULE 6 discussion and
DECOMPILATION_LEARNINGS 3d describe as "too few/too many other values
compete for saved registers ahead of X in program order" -- except here the
competing values are FOUR pointers from a loop that never even reaches a
function call, which the original round-46 framing (a single self-vs-tag-
style swap) did not anticipate.

**Permuter, Gate 3 checks run (PARALLEL-RUNS 3.5), first-ever for this
function:** scaffold built clean (`tools/setup-permuter.sh`, seed = the
round-46 preserved body verbatim). `--debug --stack-diffs`: base score
**3056** (136 stack-difference points, 60 register-difference points, 7
reorderings, **11 insertions, 11 deletions**) -- unlike
`SceneNode__AddToActorParents`'s clean 0/0 insertion/deletion signature
(a pure register-shuffle wall), this scaffold shows REAL structural
insertions/deletions, consistent with the Part-1-loop-shape finding above:
there is genuine room for a source-level fix, not just a register swap.
This satisfies Gate 3 check 2 (cost) and check 1 (compiles and scores); check
3 (scaffold/real-build signature agreement) holds by construction since the
scaffold was seeded from the exact body just rebuilt and confirmed 29/199 in
the real tree with no drift.

**Update: the bounded search WAS run** (`timeout 700`, `-j 6`,
`--stop-on-zero --best-only --stack-diffs`), after `TryAttachNearby`'s own
second search freed up the round's remaining budget. **rc=124 (bound
fired), 85100 iterations.** Three `output-*` candidates were produced --
2803, 2661 and **2563** (down from base 3056), so the search DID find
things the base seed didn't have.

**Per CLAUDE.md's "a permuter score drop is a LEAD, not a RESULT," each
candidate was read and, where safe, translated to real source and verified
against `nm -S`/`funcdiff.py` -- not adopted on the permuter's own score:**

- **output-2661 (safest, most plausible):** reorders `self->unk2C = 0;`
  to BEFORE `count1 = TmdModel__GetBoundsCount(self->unk20);` (matching retail's own
  disassembly, which places the `self->unk2C` store in the delay slot of
  the `jal TmdModel__GetBoundsCount`) plus a benign `new_var` split of the `hiPtr`
  initialization (same "aliased local" shape as `TryAttachNearby`'s own
  successful `countList` lever). Applied both pieces to the real tree:
  **zero effect, still 29/199, length still exactly 199 (`0x31c`).**
  Retail's delay-slot placement is evidently a SCHEDULING decision the
  compiler makes independent of source statement order here, not something
  reachable by reordering the two statements.
- **output-2803:** `self->unk2C |= 1 << (j = i);` -- steals `j`'s dead
  storage (unused until Part 3's own `for (j = 0; ...)` loop) as a place to
  stash `i`'s value. Not applied to the real tree: even if behaviorally
  inert (Part 3 reinitializes `j` before any read), it did not appear
  promising enough on its own to spend a build on given the other two
  candidates' clean negative, and the round's time budget was closing.
- **output-2563 (REJECTED, not tested against the real oracle):** reuses
  `bitJ`'s storage as a scratch temp for `outWord` at Part 2's own check
  (`(bitJ = outWord) >= 0x201`), which is a legitimate "dead variable as
  scratch space" move IF `bitJ` is truly unread before Part 3's own
  `bitJ = 1 << j;` -- but the SAME diff hunk also rewrites the analogous
  check INSIDE Part 3's own `m`-loop (`bitJ >= 0x201` instead of
  `outWord >= 0x201`), and at THAT point `bitJ` already holds a real,
  semantically different value (`1 << j`, the plane bit mask) written
  earlier in the same iteration. Reading `bitJ` there instead of `outWord`
  is not a cosmetic rename -- it changes which value the `sHitHeightGate`
  gate compares against, i.e. it can change the game's actual runtime
  behavior (whether `self->unk2C`/`*outFlag` get a bit set), not just the
  compiled bytes. This is exactly the "reject UB candidates" case CLAUDE.md
  and PARALLEL-RUNS 3.5 warn about: a permuter mutation can score well by
  exploiting the fixed/degenerate test input the scorer runs against
  without being behaviorally equivalent to the source it mutated. Declined
  to test this one in the real tree at all -- adopting a candidate that
  reads the wrong variable would not be "preserving a near-miss body," it
  would be committing an incorrect transcription of the function's own
  logic even if it happened to score well.

**Net result: the search found real byte-level improvements to its own
scorer, but none of the safe-to-adopt candidates moved the real oracle's
score, and the one candidate that DID look promising by score was
semantically unsound and was not adopted.** The scaffold is left in place
(`permuter-work/SceneNode__RaycastHullAgainstFaces/`, gitignored) for whoever
picks this up next, with all three candidates' diffs preserved under
`output-*/diff.txt` and this report's read of each one, so the next attempt
does not have to re-derive which are safe.

**Explicit answer to the revisit's own question: the round-54 naming gave NO
new shape here either**, for the same reason as `NotifyTaggedParents` --
this function's own symbols (`TmdModel__RaycastFaces`, `sHitHeightGate`,
`self->unk2C`/`unk20`, `ClipSegmentToBox`, `TmdModel__GetBoundsBuffer`,
`TmdModel__GetBoundsCount`) were untouched by round 54's `SceneNodeMethods` slot
renames. What DID move the investigation forward was reading the
disassembly's own pointer arithmetic directly rather than trusting the
round-46 title's "self register identity" summary -- the real structural
finding here came from that re-read, not from anything the revisit's
premise (naming) actually supplied.

REVISITED (round 55): confirmed unchanged at 29/199 as the filed score;
one structural rewrite of Part 1 tried and reverted as a verified negative
(worse, 16/199, larger drift); Gate 3 permuter checks run and passed
(11/11 insertions/deletions -- real structural room, unlike a pure register
wall) and the bounded search itself WAS run (85100 iterations, rc=124,
three candidates found) -- two safe candidates translated and verified
inert against the real oracle (still 29/199), one candidate rejected
outright as semantically unsound (reads a bit-mask value where the source
means a plane-test result) rather than tested. Restored to `INCLUDE_ASM`,
`git diff --stat src/graphics/scene_node.c` confirmed clean after the check.

### Proposed learning (round 55)

**A register-identity title can undersell a residue that is actually a
different LOOP INDUCTION VARIABLE DECOMPOSITION -- and reproducing the loop
shape in isolation is not sufficient when the loop's own pointers compete
for callee-saved registers with everything else in the function, even
though the loop itself makes no calls.** This is a variant of the
"whole-function register pressure" class already in DECOMPILATION_LEARNINGS
3d, but the specific trap here is worth naming: a loop with NO function
calls inside it can still need its induction variables in callee-saved
registers (retail's own disassembly proves this -- `$s1`/`$t0`/`$a0`-class
regs used for pointers that are never live across a call), so "does this
variable cross a call" is not a reliable filter for "does this variable need
a saved register" on this pipeline. Before spending a rewrite on matching a
sub-loop's own instruction sequence, check whether the REST of the function
also needs to be re-shaped simultaneously -- a locally-correct rewrite that
regresses the whole-function score is a real, verified negative, not a
sign the loop-shape theory was wrong (the loop body itself DID come out
closer to retail; the cost showed up in the surrounding marshalling code
instead).

**A permuter candidate that reuses a "dead" variable's storage needs its
EVERY occurrence checked, not just the first.** `output-2563` looked, at
its first hunk, like the same legitimate "steal a not-yet-live variable's
stack slot as scratch space" move that closed real words on
`TryAttachNearby` this same round (the `countList` lever) and on
`output-2803`/`output-2661` here. Its SECOND hunk reused the same variable
(`bitJ`) at a point where it was no longer dead -- it already held a real,
different, semantically load-bearing value from earlier in the same loop
iteration. A diff that touches a variable in two places needs each site
checked against that variable's OWN liveness at that point, not just
pattern-matched against "this looks like the lever that worked elsewhere."
Scoring well is not evidence of correctness -- the permuter's fixed test
input can fail to distinguish "reads the right value" from "reads a
different value that happens not to matter for this particular input."

NON_MATCHING body promoted, round 69

## Track 6 (round 91, echo)

AttachCornerList_d294b is TmdModel.h's TmdHull and Vec3S16_d294 is TmdVec3. Byte-identical.

## Round 100 (delta): track 7

Renamed `SceneNode__ClassifyAgainstPlanes` -> `SceneNode__RaycastHullAgainstFaces`
(tier B), slot +0x0AC `classifyAgainstPlanes` -> `raycastHullAgainstFaces`.
Evidence, from the body: the "planes" it walks are TmdModel__GetBoundsBuffer's
records, which are TmdBoxes (min and max corners), not planes; each is used
only as a gate (ClipSegmentToBox), and the test that sets the bits is
TmdModel__RaycastFaces, a segment cast against every face of the model. The
segments are the hull's centre line (face centre to face centre) and, per
box, the two edges joining corners 1 and 2 of its first face to the second.
What the game uses a hit for is not established (tier B).

Parameters: `outFlag` -> `hullHits` (bit k per hull box, or 1 for a
centre-line hit), `diff` -> `hitPoint` (it is RaycastFaces's `hitOut`: the
caller's delta is overwritten with the hit point), `list` -> `hull`; the
prototype and slot follow. Locals: `mid` -> `center`, `p` -> `c`, `hi` ->
`opposite`, `v` -> `corner`, `count1` -> `boundsCount`, `plane` -> `bounds`,
`cnt2` -> `hullCount`, `m` -> `j`, `bigConst` -> `nearest`, `outWord` ->
`height`. Constants: `0x7FFFFFFF` -> `DIST_NONE` (added to TmdModel.h,
token-identical to TmdModel.c's own define, which cpp accepts; RaycastFaces
sets `*best = DIST_NONE` itself, so the caller's store is redundant but
retail's), `>= 0x201` -> `> HIT_HEIGHT_THRESHOLD` 512 (unit-local), the
corner strides 4 -> `HULL_FACE_CORNERS` (TmdModel.h). The redundant
`(TmdVec3 *)` casts on TmdVec3 pointers are gone. `u8 pad[0x18]` -> `[24]`.

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* Tests the corner list against every model plane. Part 1 averages two
 * diagonal corner pairs into mid[0]/mid[1] and tests that segment against
 * each plane, setting bit i of self->hitMask on a hit; any hit returns at once.
 * Otherwise every 8-corner box k in `list` has its two vertical edges
 * (corner m against corner m+4, m = 1, 2) tested against every plane, and a
 * hit sets plane bit i in self->hitMask and box bit k in *outFlag. `hit` is
 * written only as 0, but retail still tests it. Round 76. Byte levers:
 * the gHitHeightGate gate is two arms that each set the bit, so loop.c sees two
 * equal constant-1 loads (savings 2) and hoists the 1 into $s1; Part 1 walks
 * `p`, `v` and `hi` as pointers. */
```
