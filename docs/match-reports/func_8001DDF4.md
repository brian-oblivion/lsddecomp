# func_8001DDF4 -- STALL: length EXACT (199/199 words, no drift); 29/199 raw word-match; first real diff at vram 0x8001DDF8 (register identity, `self` in $s4 vs retail's $s5)

Round 46 (echo). **This is the first time C was ever written or built for
this function.** Round 45 (delta) filed a structure-only derivation with NO
score at all -- see the git history for that write-up, superseded below now
that it has an actual measurement. Two-thirds of that derivation (Part 1 and
Part 2) transcribed directly into working C; Part 3's open ambiguity is
resolved below.

## What round 45 handed this round

Signature `s32 func_8001DDF4(Class6B5CCObj *self, s32 *outFlag, Vec3S16_d294
*diff, void *list)`, with Part 1 (a fixed 2-row box-midpoint average into a
local `Vec3S16_d294 mid[2]`) and Part 2 (a `count1`-driven loop over
`func_8001F50C` planes, gated by `ClipSegmentToBox`/`func_8001F8B8`, setting
bits in `self->unk2C`) both fully derived and high-confidence. Part 3 (a
second, `list`-driven double loop) was flagged NOT fully decoded: "the
precise relationship between the middle `k` loop ... and the inner fixed-4
`m` loop" was left as the open question for whoever picked this up next.

## Part 3, resolved

Re-read `asm/nonmatchings/code_d294_b/func_8001DDF4.s` lines 197-268
(`.L8001DFD0` through `.L8001E0B4`) instruction-by-instruction:

- The innermost loop runs `m = 0..3` (4 passes, unconditional), but the
  box-test body (the `ClipSegmentToBox`/`func_8001F8B8` pair) only executes
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
unlike the signed/unsigned `unk10` case in `func_8001E7BC`'s round-19
history, which is textually a much more local fold). My build reproduced
this branch as written (did not get eliminated on my side either) -- the
residue here is register identity, not branch presence/absence.

## New symbols added (own-file, not shared header)

```c
extern s32 func_8001F8B8(void *arg0, s32 *arg1, Vec3S16_d294 *arg2, s32 *arg3, Vec3S16_d294 *arg4, Vec3S16_d294 *arg5);
extern s32 D_8008A838;
```

**Note the signature conflicts with `code_d294_c.c`'s own existing local
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
`Class6B5CC__CheckBoundsOverlap`'s own history in this unit (round 20: "fixing the frame size
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
or simple declaration reordering (see `func_8001E7BC`'s own round-44
residue #1, same symptom, same unit). Not attempted further this round --
budget went to establishing the first honest score and closing the
structural unknowns instead of iterating blind on a 10-candidate saved-
register permutation with no tool support for exploring it directly.

## Preserved body (29/199, no drift)

```c
#if 0
extern s32 func_8001F8B8(void *arg0, s32 *arg1, Vec3S16_d294 *arg2, s32 *arg3, Vec3S16_d294 *arg4, Vec3S16_d294 *arg5);
extern s32 D_8008A838;

s32 func_8001DDF4(Class6B5CCObj *self, s32 *outFlag, Vec3S16_d294 *diff, void *list) {
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

    count1 = func_8001F3A4(self->unk20);
    self->unk2C = 0;
    flag2 = 0;
    for (i = 0; i < count1; i++) {
        plane = func_8001F50C(self->unk20, i);
        if (ClipSegmentToBox(NULL, (BoundsBox_d294 *)plane, &mid[0], &mid[1])) {
            if (func_8001F8B8(self->unk20, &bigConst, diff, &outWord, &mid[0], &mid[1])) {
                if (D_8008A838 == 0 || outWord >= 0x201) {
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
        plane = func_8001F50C(self->unk20, j);
        bitJ = 1 << j;
        rowBase = (u8 *)list + 4;
        for (k = 0; k < cnt2; k++) {
            bitK = 1 << k;
            for (m = 0; m < 4; m++) {
                if (m == 1 || m == 2) {
                    u8 *rowM = rowBase;
                    u8 *rowMplus1 = rowBase + 0x18;
                    if (ClipSegmentToBox(NULL, (BoundsBox_d294 *)plane, (Vec3S16_d294 *)rowM, (Vec3S16_d294 *)rowMplus1)) {
                        if (func_8001F8B8(self->unk20, &bigConst, diff, &outWord, (Vec3S16_d294 *)rowM, (Vec3S16_d294 *)rowMplus1)) {
                            if (D_8008A838 == 0 || outWord >= 0x201) {
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
`Class6B5CC__CheckBoundsOverlap` needed in this same unit two rounds ago.** When a derivation
report is inherited with NO C ever attempted, build the solid parts first
and let the frame-size check (word 0 of `funcdiff`'s output) tell you
immediately whether an unaccounted local exists, rather than reading
further into the diff before that's settled.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `Class6B5CC__ClassifyAgainstPlanes`
(tier B). STALL, still `INCLUDE_ASM`; not attempted for a match this
round (naming pass only, and this is the documented `gp_rel`-history
blocker's successor -- see CLAUDE.md's "Open toolchain blockers" table,
now RESOLVED, so this function is ordinary matching work for whoever
picks it up next). Slot `+0x0AC` occupant: computes a fixed 2-row
box-midpoint average, then classifies it against every one of
`self->unk20`'s planes (via `ClipSegmentToBox`/`func_8001F8B8`,
setting bits in `self->unk2C`), and if none set, runs a second,
`list`-driven sliding-window pass doing the same per-plane
classification over a corner list. "ClassifyAgainstPlanes" describes
the measured mechanics (a plane-membership/clip test, not a specific
game concept); which planes `self->unk20` holds is not established
beyond "the same planes `Class6B5CC__CheckBoundsOverlap` reads." Held
back from an actual rename because this symbol is referenced (in a
comment) from `src/code_d294_c.c:414` -- a different unit. Posted to
the broadcast.
