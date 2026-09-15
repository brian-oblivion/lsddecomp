# func_8001DDF4 -- STALL, round 45 (2026-09-15)

**Unit:** `code_d294_b` · **Size:** 199 words. **Status:** structural
derivation only -- **no C was written or built this round.**

Previously filed as blocked by `gp_rel` (round 12, no derivation attached).
That blocker was RESOLVED in round 42. This is a fresh derivation attempt,
not a rebuild of prior work (there was none to rebuild).

## Why this one was screened rather than coded

`code_d294_b`'s sibling `func_8001DA28` (`slotA8`, occupying the vtable slot
right before this one's `slotAC`) is a similarly-shaped function -- corner/
plane clipping tests driven by the same `func_8001F3A4`/`func_8001F50C`
PsyQ helpers -- and remains a STALL after MULTIPLE dedicated rounds (round
13, round 20), reaching only 14/243 words in-range despite fixing its frame
size and a deferred-self-materialization residue. Given that history, this
round's remaining budget went into getting the STRUCTURE right and
documented rather than into a build attempt likely to cost many iterations
for an uncertain result. The derivation below is solid enough that a future
round can start writing C directly from it rather than re-deriving from the
raw disassembly.

## Signature (derived, not yet written into `include/code_d294.h`)

```c
s32 func_8001DDF4(Class6B5CCObj *self, s32 *outFlag, Vec3S16_d294 *diff, void *list);
```

- `self` = `$a0`, saved to `$s5` at entry (this unit's usual convention).
- `outFlag` (`$a1`, saved to `sp+0x48`) -- an output pointer, written `0` or
  `1` near the end and read back at the very end to form the return value
  (`return (*outFlag != 0);`, confirmed by the tail
  `lw v0,0(t2)/sltu v0,zero,v0` sequence -- matches the header's existing
  comment on `slotAC`, "return value is also tested truthy/falsy like
  slotA8's").
- `diff` (`$a2`, saved to `sp+0x50`, typed `Vec3S16_d294*` already in
  `Class6B5CCMethods::slotAC`) -- forwarded untouched into a `func_8001F8B8`
  call.
- `list` (`$a3`, saved to `sp+0x58`) -- see "The `list` argument" below;
  its true type is NOT yet named, kept `void*` here deliberately.

Note this does NOT match the argument reading suggested by the STALLED
`func_8001D714`'s own preserved call site (`slotAC(self, other->unk2C, &diff,
&count)`) -- that call site is itself unverified (the function has never
built), and this function's own body makes `outFlag` an output pointer and
`list` a multi-field struct pointer, neither of which look like `&count`
(a pointer to one scalar). Trust THIS function's own reads over the
stalled caller's guessed argument names.

## The `list` argument's shape (MEASURED from this function's own reads)

`list` (base `$a3` at entry) is read at offset `+0x0` (a `s32 count`,
reloaded once, `sp+0x68`, used as the second double-loop's inner bound) and
at TWO fixed sub-regions relative to a moving base that starts at
`list+4` and advances by `0x18` (24 bytes) per outer-loop pass:

- `list[row]+0x0` (3x `s16`, i.e. a `Vec3S16_d294`) -- read as `s1`'s target
  (init `list+4`).
- `list[row]+0xC` (3x `s16`) -- read as `t0`'s target (init `list+0x10`,
  i.e. `+0xC` past `s1`'s own start).

For `row = 0, 1` (a FIXED 2-iteration loop, not driven by `list->count`),
these two 3-`s16` groups are averaged component-wise
(`(lo+hi)>>1`, ordinary arithmetic-shift halving, no rounding-toward-zero
correction needed since it's a plain `sra`, not the truncating-div idiom)
and the results packed into a **local 2-element `Vec3S16_d294[2]` array on
the stack** (`sp+0x18..0x24`). This is exactly the "low corner / high
corner, then midpoint" shape of `BoundsBox_d294` (`include/code_d294.h`):
`list[row]` at offset `+4` (relative to `list`) is itself a
`BoundsBox_d294`-shaped 0xC-byte span (`lo` at `+0`, `hi` at `+0xC` of the
row, i.e. `list+4+row*0x18+0x0` and `list+4+row*0x18+0xC`), with `0x18-0xC
= 0xC` bytes of the row unaccounted for (unread by this function).

So: **`list` looks like `{ s32 count; BoundsBox_d294 box[2] /* each box
padded to 0x18, only the first 0xC bytes read */; ... more, stride 0x18,
count entries starting after the fixed pair ... }`.** The SECOND
double-loop (see below) walks a DIFFERENT, `count`-driven array of 0x18-byte
rows starting at the SAME `list+4` base, stride `0x18`, so the two fixed
"box[0]/box[1]" rows read by the first loop are very likely just
`list_row[0]` and `list_row[1]` of the SAME array the second loop walks
`list->count` of -- i.e. `list` is plausibly one single
`{ s32 count; Row rows[count]; }` (`CornerList_d294`-shaped, stride 0x18
instead of that struct's stride-6 corners) where the first loop
unconditionally processes just the first two rows regardless of `count`.
Not fully confirmed -- the second loop's own row layout (below) only
confirms the `+4`/stride-`0x18` part, not that rows 0/1 are literally
`rows[0]`/`rows[1]` of the same array (could also be two separate fixed
fields that happen to sit where `rows[0]`/`rows[1]` would).

## Part 1: fixed 2-row box-midpoint loop (fully derived, high confidence)

```c
Vec3S16_d294 mid[2];   /* sp+0x18..0x24 */
s32 row;

for (row = 0; row < 2; row++) {
    /* box[row].lo at list+4+row*0x18, box[row].hi at list+4+row*0x18+0xC */
    mid[row].x = (box[row].lo.x + box[row].hi.x) >> 1;
    mid[row].y = (box[row].lo.y + box[row].hi.y) >> 1;
    mid[row].z = (box[row].lo.z + box[row].hi.z) >> 1;
}
```

Retail interleaves the 6 additions/shifts across the 2 iterations for
scheduling (three independent `lh`/`lh`/`addu`/`sra`/`sh` chains issued
back-to-back within EACH pass, not per-field loops) -- same flavor of
compiler scheduling as `func_8001CEB4`'s three interleaved `/360`
divisions this round; expect this loop to need the SAME "no naive
reshaping" treatment when actually coded.

## Part 2: per-plane clip test, sets `self->unk2C` bits (fully derived)

```c
s32 count1 = func_8001F3A4(self->unk20);
s32 i;

self->unk2C = 0;
for (i = 0; i < count1; i++) {
    Sixteen6_d294 *plane = func_8001F50C(self->unk20, i);
    if (func_8001E110(NULL, (BoundsBox_d294 *)plane, &mid[0], &mid[1])) {
        s32 outWord;   /* sp+0x44 */
        if (func_8001F8B8(self->unk20, &(s32){0x7FFFFFFF}, diff, &outWord,
                           &mid[0], &mid[1])) {
            if (D_8008A838 == 0 || outWord >= 0x201) {
                self->unk2C |= (1 << i);
            }
        }
    }
}
```

- `func_8001E110(out, box, p1, p2)` is ALREADY MATCHED (118/118, this same
  unit) -- a recursive line/segment-vs-box clip test. Passing a
  `Sixteen6_d294*` (a view-frustum-plane-shaped record) where it expects a
  `BoundsBox_d294*` is legitimate: both are 12-byte, 6x-`s16` structs, and
  `Sixteen6_d294`'s own header comment already documents that its fields
  "pair with `BoundsBox_d294` fields in a scrambled order" -- same
  underlying shape, different semantic reading. Confirms `func_8001F50C`
  returns per-plane AABB-shaped bounds, one per index `0..count1-1`.
- `func_8001F8B8` (PsyQ, `asm/psyq_fa50.s`, NOT decompiled anywhere in this
  project yet) is called `(self->unk20, &bigConst, diff, &outWord,
  &mid[0], &mid[1])` -- 4 register args plus 2 stack args (`sp+0x10`/
  `sp+0x14` holding `&mid[0]`/`&mid[1]` respectively, i.e. the SAME two
  pointers just passed to `func_8001E110`). No existing extern/prototype
  for it in this unit's header -- needs declaring when this is coded
  (6-argument PsyQ call, 2 via stack, matches `func_8001EE04`'s own
  register+stack split style already used elsewhere in this unit).
- `D_8008A838` -- a `%gp_rel` global (the ORIGINAL round-12 blocker hit,
  now ordinary gp-relative access). Loaded as a plain `s32` gate; no other
  reference found elsewhere in the currently-carved units (single
  reference, this function only) -- declare `extern s32 D_8008A838;`
  directly in `src/code_d294_b.c` when coded, same convention as
  `func_8004AA10`'s `D_8008A980` this round.
- The `1 << i` OR-accumulation into `self->unk2C` is a plain "which planes
  passed" bitmask -- `self->unk2C`'s existing role is unknown outside this
  function; not yet in `include/code_d294.h`.

## Part 3: dead(?) flag check, then a SECOND double loop (structure derived,
## NOT fully decoded -- this is where the remaining work is)

```c
if (self->unk2C != 0) {
    if (deadFlag) {          /* deadFlag initialized 0 right before Part 2's
                               * loop and NEVER WRITTEN anywhere in Part 2 --
                               * this branch is dead in every path traced
                               * this round. Confirmed by grepping every
                               * $s2 write between the two `addu s2,zero,zero`
                               * resets (asm lines 64 and 146): none exist.
                               * Reproduce faithfully regardless -- it is
                               * retail's own dead branch, not a translation
                               * error, and removing it is NOT safe (register
                               * allocation in this function is dense enough
                               * that removing a real branch WILL perturb
                               * scheduling elsewhere, per this round's other
                               * two functions' experience). */
        *outFlag = 1;
    }
    goto epilogue;
}

*outFlag = 0;
{
    s32 cnt2 = list->count;      /* reload of list->count, sp+0x68 */
    s32 j;                        /* outer index, 0..count1-1 (count1 RELOADED
                                    * from sp+0x60, the func_8001F3A4 result) */

    for (j = 0; j < count1; j++) {
        Sixteen6_d294 *plane = func_8001F50C(self->unk20, j);   /* -> $s6 */
        s32 bitJ = 1 << j;                                        /* -> $fp */
        u8 *rowBase = (u8 *)list + 4;                              /* -> $s1,
                                    * walked across the WHOLE k loop below,
                                    * advanced +0x18 once more at the j-loop
                                    * bottom (delay slot of the j-loop branch,
                                    * `addiu s1,s1,0x18`) */
        s32 k;

        for (k = 0; k < cnt2; k++) {
            s32 bitK = 1 << k;                                       /* -> $s7 */

            /* inner 4-pass loop, index "s2" (0..3), REUSES the outer
             * dead-flag register -- confirmed a genuinely separate live
             * range, not aliasing: s2 is unconditionally reset to 0 right
             * before this loop starts (asm line 146). */
            s32 m;
            for (m = 0; m < 4; m++) {
                void *rowM = rowBase;             /* -> $a2 for func_8001E110,
                                                     * also saved as sp+0x10 */
                void *rowMplus1 = rowBase + 0x18;   /* -> $a3, also sp+0x14 */

                if (func_8001E110(NULL, (BoundsBox_d294 *)plane, rowM, rowMplus1)) {
                    s32 outWord;   /* sp+0x44, reused */
                    if (func_8001F8B8(self->unk20, &(s32){0x7FFFFFFF}, diff,
                                       &outWord, rowM, rowMplus1)) {
                        if (D_8008A838 == 0 || outWord >= 0x201) {
                            self->unk2C |= bitJ;
                            *outFlag |= bitK;
                        }
                    }
                }
                rowBase += 6;    /* NOT +0x18 -- the inner 4-pass loop steps
                                  * through the row in 6-byte (one
                                  * Vec3S16_d294) increments, i.e. it is
                                  * walking OVERLAPPING/sliding 2-element
                                  * windows across a 0x18-byte-plus span, not
                                  * 4 independent rows. Exact bound/relation
                                  * between "k" (cnt2-driven) and "m" (fixed
                                  * 4) not yet resolved -- see below. */
            }
        }
    }
}

epilogue:
    lw t2, 0x48(sp);   /* reload outFlag */
    return (*outFlag != 0);
```

**What's solid:** the outer `j` loop (0..count1-1, re-fetching a plane via
`func_8001F50C` and a bit `1<<j` into `$fp`), the middle `k` loop
(0..cnt2-1, `list->count`, a second bit `1<<k` into `$s7`), and the
INNERMOST fixed-4 loop's call shape (`func_8001E110`/`func_8001F8B8`,
IDENTICAL argument shape to Part 2, but sourcing its box/point pair from
the SLIDING `rowBase`/`rowBase+0x18` pair instead of the fixed `mid[0]`/
`mid[1]`) are all confirmed against the raw disassembly (`asm/
nonmatchings/code_d294_b/func_8001DDF4.s` lines 128-215, `.L8001DFD0`
through `.L8001E0B4`).

**What's NOT resolved:** the precise relationship between the middle `k`
loop (bound `cnt2` = `list->count`) and the inner fixed-4 `m` loop, and
whether `rowBase`'s `+6`-per-`m`-pass / `+0x18`-per-`k`-pass stepping means
`m` indexes a SEPARATE, smaller stride inside each `k`-th row, or whether
`rowBase`'s reset happens once per `k` pass (making the `+6` steps genuinely
overlapping windows within one 0x18-byte row) -- this needs a closer,
uninterrupted read of `asm/nonmatchings/code_d294_b/func_8001DDF4.s` lines
146-213, which is what the next attempt on this function should start
from, not the raw disassembly cold.

## New symbols needed when this is coded

- `extern s32 D_8008A838;` in `src/code_d294_b.c` (single-reference gp_rel
  global, own-file convention).
- A prototype for `func_8001F8B8` (PsyQ, `asm/psyq_fa50.s`) -- 6 args,
  4 register + 2 stack, shape `(void *unk20, s32 *bigConst, Vec3S16_d294
  *diff, s32 *outWord, Vec3S16_d294 *p1, Vec3S16_d294 *p2)` per the call
  site (types on the last 4 args inferred from what's passed, not from the
  callee's own body, since it is a linked PsyQ object).

### Proposed learning

**When a sibling function in the SAME unit has already absorbed multiple
dedicated rounds and stayed at a low word-match (`func_8001DA28`, 14/243
after round 13 AND round 20), that is signal to budget a STRUCTURAL
derivation pass (signature, argument shapes, loop bounds, confirmed
sub-calls) rather than a blind build-and-iterate attempt on a same-shaped
neighbor.** The derivation above should cut a future attempt's ramp-up
substantially even though no C was compiled this round -- concretely,
Part 1 and Part 2 (roughly 2/3 of the function' structure) are
high-confidence and ready to transcribe directly; only Part 3's innermost
loop relationship needs fresh eyes.
