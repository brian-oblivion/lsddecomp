# GetMetaEvent -- STALL: length 2 words SHORT (compiled 211/213); raw word-match 69/213 (funcdiff, drift-poisoned by the length gap -- see round-26 update below for the trustworthy per-block figure); first real diff at word 56 (`tools/asm-differ/diff.py GetMetaEvent`), a register-identity re-read of `rec->unk4A`

> Renamed from `func_80035B2C` on 2026-09-23 (tools/rename.py). Address 0x80035b2c.

**Historical title, superseded by the round-26 update below**: STALL (near
miss, 20 words short). Kept for history; do not read the 20-word figure as
current.

**20 words SHORT: compiled length 193/213.** Measured directly from
`build/src/libsnd_seqread.c.o` (`objdump -d`, symbol-to-symbol distance),
not from `tools/funcdiff.py`, which cannot report a meaningful word-match
figure once length drifts (confirmed: it printed "the build differs
OUTSIDE this range too" at every intermediate attempt below).
**First real diff at word 59** (`tools/funcdiff.py GetMetaEvent`), which
is a register-identity symptom (`lui a0` vs `lui a3` for `VBLANK_MINUS`)
downstream of the true cause described below -- fixing the real cause
would very likely move or resolve this word too, so it was not chased
independently.

## What it does

The unit's meta-event handler, reached from `GetSeqData`'s dispatch
(both the new-status `0xF0` arm and the `0xFF` running-status arm) with
`a2` = the meta-event TYPE byte. Only two MIDI meta-event types are
understood -- **0x2F (End of Track)** and **0x51 (Set Tempo)** -- and
their real MIDI-standard meaning is exactly what the disassembly
computes, which is strong independent confirmation of the whole reading:

- **Set Tempo (0x51)**: reads a 3-byte big-endian
  microseconds-per-quarter-note value, converts it to a BPM-like rate
  via the textbook MIDI formula `60000000 / value` into `rec->unk8C`,
  then recomputes the scheduling threshold (`unk6E`/`unk70`) against
  `unk4A` and the global tick-rate constant `VBLANK_MINUS`, in whichever of
  two regimes avoids losing precision to integer truncation.
- **End of Track (0x2F)**: bumps a repeat counter (`unk48`). `unk46 == 0`
  means "loop forever" (rewind the cursor `unk4` to the saved track
  start `unk8`); otherwise, while the counter is still under the limit
  (`unk46`), rewind both `unk4` and `unkC`. Once the limit is reached,
  clear playback-state flags (`unk90`) and run the stop-sequence
  callbacks.

**New struct fields derived** (all same-size renames of existing padding,
each verified non-shifting by rebuilding with the function still
`INCLUDE_ASM`'d before trusting any of this function's own C):
`unk0` (u8, a byte passed to `func_80036410` alongside `unk3C`), `unk8`
(`u8 *`, the saved "track start" cursor), `unk2B` (u8, cleared on
end-of-track stop), `unk3C` (u8, compared against `0xFF`), `unk46` (s16,
repeat-count limit), `unk48` (u16, repeat counter -- sign-checked via an
explicit `(s16)` cast at its compare site, the same idiom
`libsnd_decre.c` already established for its own `unk40`), `unk4A` (s16,
the tempo-recompute scaling factor), `unk8C` (s32, the recomputed BPM),
`unk90` (u32, playback-state flags -- the SAME field name and bit
positions `libsnd_decre.c`'s independent local view already manipulates
via `_ss_score[a0][a1].unk90 &= ~2;` / `&= ~0x100;`, corroborating both
readings).

Cross-unit prototypes added (local guesses, kept in this .c only):
`SpuVmSeqKeyOff(s32)` and `func_80036410(s32, s32)`, both already matched
elsewhere (`libsnd_vmanager.c` and `libsnd_cres.c` respectively) with
exactly these signatures and this file's own established
"`(slot<<8)|channel`" packed-argument idiom for the former.

## Two real fixes that closed most of the gap

1. **Block order, the same lever as `GetSeqData`, but through an
   if/else rather than a switch.** The compare order in the disassembly
   checks `a2 == 0x2F` FIRST (jumping far forward to the end-of-track
   body if it matches), and only then checks `a2 == 0x51`. A direct
   transliteration -- `if (a2 == 0x2F) { ...; return; } if (a2 != 0x51)
   return; ...tempo...;` -- put the 0x2F body inline (first, adjacent to
   the compare) and the tempo body reached via a forward jump: exactly
   backwards from retail, and it cost 24 words outright (92/172-style
   damage, scored at the time as 29/213 raw with heavy drift). The fix
   is the coordinator-verified form of the lever: **source declaration
   order**, not case value order, decides which body is the fallthrough.
   Restructuring as
   ```c
   if (a2 != 0x2F) {
       if (a2 != 0x51) return;
       ...tempo body, ending in `return;`...
   }
   ...end-of-track body (unconditionally reached only when a2==0x2F,
      since the branch above never falls out)...
   ```
   makes the tempo body the textually-last, adjacent-to-compare
   (fallthrough) arm and the end-of-track body the one reached by a
   forward jump -- matching retail exactly, and recovering from 29/213
   drifted to a clean, drift-free 71/213 in one change.
2. **Unsigned comparison.** Retail's guard between the two tempo-rate
   regimes is `sltu` (`product * 10 < VBLANK_MINUS * 60`), not `slt`. `s32
   product * 10 < s32 divisor` written with BOTH operands `s32` compiles
   to a SIGNED `slt`. Declaring the two locals that come from
   `VBLANK_MINUS` (`base`, `divisor`) as `u32` -- matching the global's own
   type -- makes C's usual arithmetic conversions promote the whole
   comparison to unsigned automatically, no cast needed, and produces
   `sltu` verbatim. This is also why the two divisions in the `else`
   branch are `divu`, not `div`: same mechanism, no casts required.
   This single type change (and reordering `base`/`divisor`'s
   computation to precede the `unk4A * unk8C` multiply, matching where
   retail schedules it relative to the `mult`/`mflo` latency) took the
   function from 92 (with the WRONG block order already fixed once, then
   corrected again) to 199/213 -- exact for everything except a single
   optimization difference.

## The residue that would not move: a fused vs. unfused division

After both fixes, the function's ENTIRE remaining gap is one thing:
retail's Set-Tempo `else` branch (when `unk4A * unk8C * 10 >= VBLANK_MINUS
* 60`) computes `q = A / B` and `r = A % B` (`A` = `unk4A * unk8C * 10`,
`B` = `VBLANK_MINUS * 60`) via **two complete, independent instruction
sequences** -- `lh`/`lw`/`mult`/`mflo`/shift-chain/`divu` REPEATED
verbatim, once per operation, each reloading `rec->unk4A` and
`rec->unk8C` fresh from memory. This pinned toolchain's GCC 2.6.3, given
the equivalent C (`q = A/B; r = A%B;` with `A` written out identically
both times, whether as a struct-field expression, a named local, or raw
pointer arithmetic), instead recognizes the shared dividend/divisor and
emits ONE `divu` feeding both `mflo` (quotient) and `mfhi` (remainder) --
five to six words cheaper, and NOT what retail does.

**This was confirmed as a genuine compiler-optimization difference, not
a misreading, with an isolated reproducer through the pinned pipeline**
(`tools/gcc263/cpp | cc1 -O2 | maspsx --expand-div | as`, the
project's own escalation-diagnostic recipe): a minimal
`s32 q = a/b; s32 r = a%b;` on either plain register-resident values or
memory-resident struct fields fuses into one `divu` every time, whether
the two occurrences of `a`/`b` are written identically, through
differently-shaped but value-equal expressions (`base*4` vs
`VBLANK_MINUS*60`), or through raw pointer-arithmetic casts instead of
field syntax. GCC's `cse.c` matches on VALUE, not surface syntax, and
none of those reshapes broke the match.

**Reshapes/experiments tried, all through the pinned pipeline, none
reproduced retail's unfused shape without an unacceptable side effect:**

1. Direct duplication of the `unk4A * unk8C * 10` expression at both the
   `q` and `r` sites (the natural transliteration) -- fuses.
2. Introducing a `divisor`/`base` local computed once and reused, vs.
   inlining `VBLANK_MINUS * 60`/`* 600` fresh at each site -- no effect on
   the fusion either way (confirmed both directions).
3. Raw pointer-arithmetic reads (`*(s16 *)((u8 *)rec + 0x4A)`) in place
   of `rec->unk4A` for one of the two occurrences -- still fuses; `cse.c`
   recognizes the two access paths as the same value.
4. **`volatile Entry90902E8 *` for the whole record, or for just one
   field, DOES break the fusion** (confirmed: produces two genuinely
   separate `divu` sequences with fresh reloads) **but changes the
   instruction SHAPE, not just the count, for the `s16` field**: this
   compiler reads a `volatile` sub-word signed field via `lhu` plus an
   explicit `sll`/`sra` widen, never a plain `lh` -- confirmed with an
   isolated reproducer casting directly to `volatile s16 *`, ruling out
   an artifact of struct-pointer volatility specifically. Retail's own
   reload is a plain `lh`. This is the documented "a `volatile` cast is
   a codegen lever with a re-mask side effect" caution
   (`DECOMPILATION_LEARNINGS.md`) confirmed for a NEW case (a
   sub-word field, not the previously-documented `func_8002C048`
   scenario) -- the side effect makes `volatile` the wrong tool here,
   not merely an inconvenience, since it changes WHICH instruction
   retail uses, not just how many.
5. **A bare bare `__asm__("")` scheduling barrier between the `q` and
   `r` statements has NO effect** -- still fuses. This is the
   CLAUDE.md-sanctioned lever and it does not reach this residue.
6. **Diagnostic only, NOT used in the preserved body**: an
   `__asm__ __volatile__("" ::: "memory")` barrier (a full compiler
   memory-clobber, stronger than the bare form CLAUDE.md names) DOES
   force the exact retail shape in isolation -- two separate `divu`
   sequences, each with a plain `lh` reload, byte-identical to retail's
   pattern. This is reported as a toolchain-adjacent finding, not
   adopted: it is not the specific construct CLAUDE.md authorizes ("a
   bare `__asm__(\"\")`"), it does not fit the given register-identity
   test cleanly (it changes instruction COUNT via forcing a reload, not
   instruction order or register choice), and adopting an unsanctioned
   stronger barrier without authorization is exactly what "escalate,
   don't experiment" warns against for a borderline construct. Flagging
   it here rather than using it, in case the head wants to authorize the
   memory-clobber form as a named lever for this residue class (which
   would plausibly recur -- any redundant divmod-by-hand-duplicated-code
   idiom would hit the same fusion).

## Body as reached (193/213 -- 20 words short, all in the one divmod-fusion residue)

```c
/* stalesyms --fix 2026-09-22: func_80036410 -> _SsSndNextSep -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
/* Cross-unit calls, local guesses per project convention. */
extern s32 SpuVmSeqKeyOff(s32 a0);
extern void _SsSndNextSep(s32 a0, s32 a1);
extern u32 VBLANK_MINUS;

void GetMetaEvent(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];

    if (a2 != 0x2F) {
        if (a2 != 0x51) {
            return;
        }
        {
            u8 *p = rec->unk4;
            s32 tempo;
            s32 bpm;
            u32 base;
            u32 divisor;

            rec->unk4 = p + 1;
            tempo = (s32)p[0] << 16;
            rec->unk4 = p + 2;
            tempo |= (s32)p[1] << 8;
            rec->unk4 = p + 3;
            tempo |= p[2];

            bpm = 60000000 / tempo;
            base = VBLANK_MINUS * 15;
            divisor = base * 4;
            rec->unk8C = bpm;
            if (rec->unk4A * rec->unk8C * 10 < divisor) {
                rec->unk6E = (VBLANK_MINUS * 600) / (rec->unk4A * rec->unk8C);
                rec->unk70 = rec->unk6E;
            } else {
                s32 q = (rec->unk4A * rec->unk8C * 10) / divisor;
                s32 r = (rec->unk4A * rec->unk8C * 10) % divisor;

                rec->unk6E = -1;
                rec->unk70 = (base * 2 < r) ? q + 1 : q;
            }
        }
        rec->unk88 = ReadDeltaValue(a0, a1);
        return;
    }
    {
        u16 newCount = rec->unk48 + 1;
        s16 limit = rec->unk46;

        rec->unk48 = newCount;
        if (limit == 0) {
            rec->unk80 = 0;
            rec->unk27 = 0;
            rec->unk88 = 0;
            rec->unk4 = rec->unk8;
            return;
        }
        if ((s16)newCount < limit) {
            rec->unk80 = 0;
            rec->unk27 = 0;
            rec->unk88 = 0;
            rec->unk4 = rec->unk8;
            rec->unkC = rec->unk8;
            return;
        }
        _ss_score[a0][a1].unk90 &= ~1;
        _ss_score[a0][a1].unk90 &= ~8;
        _ss_score[a0][a1].unk90 &= ~2;
        _ss_score[a0][a1].unk90 |= 0x200;
        _ss_score[a0][a1].unk90 |= 0x4;
        rec->unkC = rec->unk8;
        rec->unk2B = 0;
        if (rec->unk3C != 0xFF) {
            _SsSndNextSep(rec->unk3C, rec->unk0);
            rec->unk2B = 0;
        }
        SpuVmSeqKeyOff((a1 << 8) | a0);
        rec->unk88 = rec->unk70;
    }
}
```

### Proposed learnings

1. The block-order lever (source DECLARATION order decides the
   fallthrough arm, per the coordinator's verified correction) applies
   to plain `if`/`else` chains exactly as it does to `switch` case
   bodies -- confirmed independently on this function after
   `GetSeqData` established it for switches.
2. A NEW divmod-fusion residue class: GCC 2.6.3 fuses `A/B` and `A%B`
   into one hardware `div`/`divu` whenever it can prove the two
   occurrences of `A` and `B` are value-identical, REGARDLESS of surface
   syntax (struct field, named local, or raw pointer arithmetic) --
   confirmed with an isolated reproducer. When retail shows two
   complete, independent divide sequences instead of one fused pair,
   that is evidence the ORIGINAL source held the same "redundant
   hand-duplicated division" idiom the game's programmers apparently
   used elsewhere, not evidence of a different formula. No portable C
   reshape reproduces the un-fused shape without a side effect (plain
   `volatile` changes `lh` to `lhu`+widen for sub-word fields); the only
   thing that worked in isolation was an unsanctioned memory-clobber
   `__asm__` barrier, reported above as a toolchain-adjacent finding
   for the head to weigh, not adopted.

## ROUND 26 (head): the memory-clobber barrier is NOT authorized — and it is not the only thing that works

Runner alpha flagged the `__asm__ __volatile__("" ::: "memory")` barrier as a
toolchain-adjacent finding and correctly did NOT adopt it, asking the head to
weigh authorizing it as a named lever for this residue class. **Ruling: not
authorized. And the premise that nothing else reaches the shape is false —
alpha's own rejected lever gets closer than alpha measured.**

### What retail actually does

Two `divu` at `0x80035CA4` and `0x80035CD8`, with the ENTIRE operand
computation duplicated between them and nothing stored in between:

```
lh   $v1, 0x4A($s0)      lh   $v1, 0x4A($s0)
lw   $v0, 0x8C($s0)      lw   $v0, 0x8C($s0)
mult $v1, $v0            mult $v1, $v0
mflo $v1                 mflo $v1
sll  $v0, $v1, 2         sll  $v0, $v1, 2
addu $v0, $v0, $v1       addu $v0, $v0, $v1
sll  $v0, $v0, 1         sll  $v0, $v0, 1
divu $zero, $v0, $a1     divu $zero, $v0, $a1
mflo $a0   (quotient)    mfhi $v1   (remainder)
```

### Probes through the pinned pipeline

| probe | result |
| --- | --- |
| adjacent `/` and `%`, same expression | **fused** — one `divu`, `mflo`+`mfhi` |
| shared named temp `t`, then `t/d`, `t%d` | **fused**, identical output |
| an intervening STORE between them | still fused |
| an intervening CALL between them | still fused |
| two independent pointer copies `p1`, `p2` | still fused, `lh` kept |
| **both fields `volatile`** | **UN-FUSES** — full duplicate computation, but `lh` -> `lhu`+widen |
| **only the `s32` field `volatile`** | **UN-FUSES *and* keeps retail's plain `lh`** |

That last row is the one that matters and **it was never tried**. Alpha
rejected `volatile` wholesale on the strength of the all-volatile variant's
`lh`/`lhu` side effect. Applying it to the WORD-sized field alone has no
width to get wrong, so the side effect cannot arise:

```
lh   $5, 0($6)          <- retail's plain lh, preserved
lw   $2, 4($6)
mult $5, $2 / mflo / sll 2 / addu / sll 1
divu $3, $3, $4
lw   $2, 4($6)          <- volatile forces the re-read; the fusion is broken
mult $5, $2 / mflo / sll 2 / addu / sll 1
```

The one remaining difference from retail is that retail ALSO re-reads
`lh 0x4A` in the second sequence, where this keeps the earlier `lh` result
live in `$5`. That is a much smaller gap than the 20 words this report
records, and it is an ordinary source question, not a barrier question.

### Why the barrier is refused

1. **It is not the construct CLAUDE.md authorizes.** The rule permits "a bare
   `__asm__("")`". A memory clobber is a different and much stronger thing.
2. **It does not fit the stated test.** The test is register identity versus
   instruction order; a memory clobber changes neither — it changes which
   OPERATIONS are emitted, by suppressing an optimization. So authorizing it
   would be writing NEW policy, not applying existing policy, and alpha was
   right that this is not a runner's call.
3. **The premise is false.** A plain-C lever reaches the same shape and keeps
   `lh`, per the table above.
4. **It would generalize in the worst direction.** "Barrier away any CSE or
   fusion the compiler does and retail did not" would become the standard
   answer to a large class of residues — the same slippery slope HARD RULE 6
   already describes for whole-function `__asm__`: it becomes the precedent
   for anything hard to express, and it stops anyone looking for the real
   source shape. Here, looking for the real source shape took four probes and
   found one.

Recorded as an operator-facing policy question in the round report, with a
recommendation to decline. It blocks nothing: the plain-C route above is the
live lead.

### Proposed learning

**Rejecting a lever because ONE application of it had a side effect is not
the same as the lever failing.** `volatile` was rejected here on an
all-fields variant whose only fault was a sub-word load-width change; the
field that actually needed to be volatile is the WORD-sized one, where that
fault cannot occur. When a qualifier-based lever "works but with a side
effect", check whether the side effect is intrinsic to the lever or an
artifact of WHERE it was applied — narrow the application before discarding
it.

## ROUND 26 (runner alpha, bounded 8-attempt work order): narrowed volatile applied — 211/213, 2 words short

Applied the narrowed lever exactly as specified: `volatile s32 *pbpm =
&rec->unk8C;`, with `rec->unk4A` left as an ordinary (non-volatile)
read. Verified in isolation through the pinned pipeline first (matches
the head's probe table row for row: two separate `divu` sequences,
retail's plain `lh` preserved, `unk4A`'s value kept live in a register
across both — the one predicted gap). Applied to the real function:

**193/213 -> 211/213 in one change.** First real diff moved from word 59
to word 56, and is now a pure register-identity symptom of the single
remaining gap, not a separate defect: retail's second division re-reads
`rec->unk4A` fresh (`lh $v1, 0x4A($s0)`), this body's second division
reuses the value the first division already loaded.

**Six further reshapes tried against that one `lh`, all through the real
function (not just the isolated reproducer), none closed it:**

1. Inlining `VBLANK_MINUS * 60` fresh at both the `q` and `r` division
   sites instead of the cached `divisor` local -- **regressed** to
   210/213 (one word worse; `divisor` computed once and reused is
   better, not neutral, for this specific residue).
2. An explicit `(s32)` cast on the second occurrence of `rec->unk4A`
   only -- no effect, still 211/213.
3. Wrapping the `q` and `r` statements each in their own nested `{ }`
   block, hypothesizing block-scope would end `unk4A`'s cached
   liveness -- no effect.
4. Swapping the multiplication operand order for the second occurrence
   (`*pbpm * rec->unk4A` vs `rec->unk4A * *pbpm`) -- no effect; GCC's
   `cse.c` canonicalizes commutative operands before matching, as
   expected.
5. A bare `__asm__("")` scheduling barrier between the `q` and `r`
   statements, WITH the narrowed-volatile fix already in place (i.e.
   re-testing the already-known-ineffective bare barrier now that the
   surrounding fusion is broken, in case breaking the fusion changed
   its reach) -- confirmed in isolation first, no effect there either;
   not spent against the real file's budget.
6. (Isolation only, to rule out an interaction) confirming the
   REGRESSION in (1) reproduces outside the real function too -- it
   does not reproduce in the minimal reproducer (which has no
   competing register pressure), which is itself informative: this
   residue's exact behavior depends on the REAL function's register
   pressure, not just the local expression shape, the same conclusion
   `DECOMPILATION_LEARNINGS.md` already draws for cached-vs-reread
   struct fields generally ("cache a re-read struct field only across
   a CALL-FREE span" -- this is the same family, just for a value kept
   live across two *sibling* uses rather than across a call).

**Stopping here at 211/213 per the work order's own instruction ("if you
plateau, append the negative and stop").** The preserved body below is
the 211/213 state (narrowed volatile only, `divisor` cached and reused,
no cast/nesting/reordering) since that was the best measured and every
further reshape either did nothing or regressed. `src/libsnd_seqread.c` is
back to `INCLUDE_ASM` and the whole-image build is confirmed green.

**Process note, not a code finding:** running `make extract` while this
function was mid-experiment (i.e. NOT wrapped in `INCLUDE_ASM` at that
moment) silently dropped its own `asm/nonmatchings/.../GetMetaEvent.s`
stub -- splat only regenerates a nonmatching stub for a function that is
currently `INCLUDE_ASM`'d in `src/`, so extracting while a function is
live C loses the ability to put it back. Caught immediately (the very
next build failed with "can't open ... for reading"), fixed by
re-running `make extract` after restoring `INCLUDE_ASM`, and confirmed
the whole-image build was clean before touching anything else. Worth
knowing before running `extract` mid-round for any reason other than a
genuinely fresh carve.

## Body as reached, round 26 update (211/213 -- 2 words short, supersedes the 193/213 body above for future reference; the 193/213 body above is left as the historical record of the pre-lever state)

**Round 49 note (runner charlie): rewrapped in literal `#if 0`/`#endif`,
and `func_80036410` CORRECTED to `_SsSndNextSep`** (`src/code_179d8_k.c`'s
actual preserved body already carries the real Psy-Q name from a later
SDK-object round; only this report's copy was stale), per
`tools/stalesyms.py`'s finding relayed by the head. No behavioral change.

```c
#if 0
extern s32 SpuVmSeqKeyOff(s32 a0);
extern void _SsSndNextSep(s32 a0, s32 a1);
extern u32 VBLANK_MINUS;

void GetMetaEvent(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &_ss_score[a0][a1];

    if (a2 != 0x2F) {
        if (a2 != 0x51) {
            return;
        }
        {
            u8 *p = rec->unk4;
            s32 tempo;
            s32 bpm;
            u32 base;
            u32 divisor;

            rec->unk4 = p + 1;
            tempo = (s32)p[0] << 16;
            rec->unk4 = p + 2;
            tempo |= (s32)p[1] << 8;
            rec->unk4 = p + 3;
            tempo |= p[2];

            bpm = 60000000 / tempo;
            base = VBLANK_MINUS * 15;
            divisor = base * 4;
            rec->unk8C = bpm;
            if (rec->unk4A * rec->unk8C * 10 < divisor) {
                rec->unk6E = (VBLANK_MINUS * 600) / (rec->unk4A * rec->unk8C);
                rec->unk70 = rec->unk6E;
            } else {
                /* Narrowed volatile lever (round 26 head ruling): only the
                 * WORD-sized field needs to be volatile to defeat GCC's
                 * div/mod fusion -- there is no load-width to get wrong for
                 * a full-word read, so retail's plain `lh` for unk4A is
                 * unaffected. */
                volatile s32 *pbpm = &rec->unk8C;
                s32 q = (rec->unk4A * *pbpm * 10) / divisor;
                s32 r = (rec->unk4A * *pbpm * 10) % divisor;

                rec->unk6E = -1;
                rec->unk70 = (base * 2 < r) ? q + 1 : q;
            }
        }
        rec->unk88 = ReadDeltaValue(a0, a1);
        return;
    }
    {
        u16 newCount = rec->unk48 + 1;
        s16 limit = rec->unk46;

        rec->unk48 = newCount;
        if (limit == 0) {
            rec->unk80 = 0;
            rec->unk27 = 0;
            rec->unk88 = 0;
            rec->unk4 = rec->unk8;
            return;
        }
        if ((s16)newCount < limit) {
            rec->unk80 = 0;
            rec->unk27 = 0;
            rec->unk88 = 0;
            rec->unk4 = rec->unk8;
            rec->unkC = rec->unk8;
            return;
        }
        _ss_score[a0][a1].unk90 &= ~1;
        _ss_score[a0][a1].unk90 &= ~8;
        _ss_score[a0][a1].unk90 &= ~2;
        _ss_score[a0][a1].unk90 |= 0x200;
        _ss_score[a0][a1].unk90 |= 0x4;
        rec->unkC = rec->unk8;
        rec->unk2B = 0;
        if (rec->unk3C != 0xFF) {
            _SsSndNextSep(rec->unk3C, rec->unk0);
            rec->unk2B = 0;
        }
        SpuVmSeqKeyOff((a1 << 8) | a0);
        rec->unk88 = rec->unk70;
    }
}
#endif
```

### Proposed learning

The single remaining `lh` re-read is registry-pressure-sensitive in a
way the isolated reproducer cannot show: recomputing `VBLANK_MINUS * 60`
fresh (removing a cached local) made the SAME residue worse, not
neutral, in the real function while having no measurable effect in
isolation. This reinforces the existing "cache a re-read struct field
only across a CALL-FREE span" finding, extended to a value re-read
across two *sibling* statements rather than across a call boundary —
register pressure from the surrounding function is doing real work here
that a minimal reproducer will not surface, so a probe that plateaus in
isolation is not proof a real-file variant will too, and vice versa.

## ROUND 35 (runner alpha): permuter run, no zero -- residue confirmed permuter-resistant

Rebuilt the round-26 preserved body (211/213, narrowed-`volatile` lever) in
isolation first to reconfirm the recorded score before spending any search
time: clean build, `funcdiff.py GetMetaEvent` reproduces **69/213 words
match** (drift-poisoned by the 2-word length gap, as this report's title
now states directly rather than quoting the pre-drift 20-word figure), and
the compiled length cross-checked via `mipsel-linux-gnu-objdump`
symbol-to-symbol distance is **211 words**, matching this report exactly.

**Built a minimal, hand-written seed** (not the whole `src/` file --
`tools/setup-permuter.sh`'s own header warns that a seed carrying OTHER
functions' still-`INCLUDE_ASM` bodies pulls their raw `.s` includes into the
scaffold verbatim via the `PERMUTER`-gated macro, inflating the base score
by two orders of magnitude; this was hit and corrected on the SAME function's
scaffold this round before it was reused here) with only the types/externs
this function's own body needs (`Entry90902E8`, `_ss_score`,
`ReadDeltaValue`, `SpuVmSeqKeyOff`, `_SsSndNextSep`, `VBLANK_MINUS`). Sanity
check (`--debug --stack-diffs`): **base score 1425**, **`Stack Differences:
0`** -- a clean, small, register/reordering-only score consistent with a
narrow residue, unlike `ContDataEntry`'s scaffold this same round (rejected
outright, nonzero stack differences). This one was trusted and searched.

**Searched `-j 4 --stop-on-zero --best-only` for 3 minutes (15862+
iterations, bounded with `timeout 180`, `permuter rc=0` meaning the bound
was not hit -- the search pool was exhausted / plateaued on its own).
No candidate ever scored below the base 1425; the best score seen
repeatedly across many iterations was 1425 itself (i.e. equal to base, not
an improvement), with every other sampled candidate scoring higher (1730 to
16420+).** No zero found. This is a genuine negative result, not a timeout:
`permuter rc=0` (not 124), and the search had already sampled tens of
thousands of candidates without moving off the base score.

This reinforces rather than contradicts the round-26 finding: the residue
(retail re-reads `rec->unk4A` fresh via a plain `lh` before the second
division; this build keeps the first read's value live in a register) is a
register-pressure/allocator decision, not a local expression-shape question
a random AST permutation is likely to stumble into by chance, and the
permuter's own mutation set (reordering, decl motion, cast/paren
insertion) does not include the register-pressure-shaping mechanism this
report's own round-26 update already identified as underlying the residue
(the un-cached-vs-cached-divisor experiment showing the same edit has
OPPOSITE effects in isolation vs. in the real function's register-pressure
context). Marking this residue **permuter-exhausted** per
`docs/MATCHING-GUIDE.md`'s guidance ("if only undefined-behaviour or
duplicate-arm forms reach zero and no idiomatic translation scores the
same, mark the class permuter-exhausted in the report and move on").

`src/libsnd_seqread.c` restored to `INCLUDE_ASM`; whole-image build confirmed
green (`build-and-verify.sh` exit 0) before moving on.

### Round 35 lever checklist

- **Rebuild-before-trust**: done, reconfirmed 211/213 unchanged.
- **Permuter**: sanity-checked clean (base 1425, zero stack diff, unlike the
  SAME round's `ContDataEntry` scaffold), then SEARCHED (this report's
  queue entry said "ZERO permuter" going in) -- **negative**, no zero in
  15862+ iterations. Marked permuter-exhausted for this residue.
- **Lever 6 (narrowed-volatile precedent)**: not re-tried; round 26 already
  established this is the residue's own class and found the narrowing
  refinement that got from 20 to 2 words short.

### Proposed learning

A clean permuter sanity check (`Stack Differences: 0`, base score in the
low thousands) is worth building even for a function whose recorded residue
is described as "register-pressure-sensitive" and therefore seems like a
poor permuter target on paper -- the sanity check itself is cheap (seconds)
and, unlike `ContDataEntry`'s scaffold the SAME round, this one confirmed
the scaffold really was scoring the documented residue before any search
time was spent. The search coming back negative is still informative: it is
the second data point (after round 26's own manual reshapes) that this
specific "which register holds a cached vs. re-read struct field" choice is
not reachable through either manual reshaping OR randomized AST mutation,
which is a stronger claim than either alone.

## Round 39 update (runner alpha): re-verified; hoist-both-before-either checked, not applicable

Rebuilt the round-26 preserved body (narrowed-`volatile` lever) and
cross-checked compiled length directly via `objdump` symbol-to-symbol
distance: `GetMetaEvent` is `0x34c` bytes (211 words), reproducing
"length 2 words SHORT (211/213)" exactly.

**Hoist-both-before-either: not applicable.** The one remaining word is
retail re-reading `rec->unk4A` fresh via a plain `lh` before EACH of two
divisions (`q`/`r`), where this build keeps the first read's value live in
a register and reuses it for the second. This is the OPPOSITE shape from
what the lever addresses: the lever fixes a case where two VALUES that
retail computes together (adjacent loads, later shared consumer) get
split apart by naive source order; here retail deliberately does NOT
reuse a single cached read across two consumers that this build's compiler
WOULD LIKE to reuse (via CSE) given the current source shape -- there is
no "hoist both before use" available since the fix would require
PREVENTING reuse of an already-loaded non-volatile value, which is exactly
what round 26's narrowed-`volatile` lever already targets (and round 35's
permuter search already confirmed exhausted at this specific residue,
15862+ iterations, no zero, marked permuter-exhausted). No new experiment
run this round given both the manual reshape space (round 26, six
variants) and the permuter route (round 35) are already exhausted for this
specific single-word residue. `INCLUDE_ASM` unchanged, `build-and-verify.sh`
confirmed byte-exact.

**NON_MATCHING body promoted, round 66** (runner charlie).

## Round 97 types pass (echo)

code_179d8_k's local `Entry90902E8` view retired onto `include/SsScore.h`:
the same 0xAC-byte (`SS_SEQ_TABSIZ`) `_ss_score[access][seq]` record that
libsnd_cres, libsnd_decre and libsnd_vmanager already use. The header gained
this unit's fields by splitting padding (no offset, size or existing type
moved); field names stay offset-only (`unkNN`) as the header's convention for
Sony-only fields, with each one's mechanics in its comment. The unit's
`(u8 *)rec + unk12 + 0x17/0x2C` and `(s16 *)((u8 *)rec + 0x4E + ch * 2)`
arithmetic became the header's per-channel arrays `unk17[16]` (pan),
`unk2C[16]` (program) and `unk4E[16]` (volume): `unk12` is the event's MIDI
channel (GetSeqData stores a status byte's low nibble), not a byte offset to
an "embedded state block" as the old local comment read it. Byte-exact
unchanged; the NON_MATCHING object is identical too (objdump of
`build/nonmatching/src/code_179d8_k.c.o` before/after).

## History (moved from src/libsnd_seqread.c, comments pass)

A comment inside the NON_MATCHING body, on the `volatile s32 *pbpm`, read:

> Narrowed volatile lever (round 26 head ruling): only the
> WORD-sized field needs to be volatile to defeat GCC's
> div/mod fusion -- there is no load-width to get wrong for
> a full-word read, so retail's plain `lh` for unk4A is
> unaffected. See docs/match-reports/GetMetaEvent.md.

The comment above this function's NON_MATCHING body in src/libsnd_seqread.c read:

> NON_MATCHING: 211/213 words, length 2 SHORT. Residue: register-identity
> re-read of rec->unk4A (retail's second divu re-reads it fresh via a
> plain `lh`; this body keeps the first read's value live in a register)
> (docs/match-reports/GetMetaEvent.md). Hand-derived -- reaches 211/213
> via a narrowed `volatile` qualifier on the word-sized field only
> (round 26 head ruling, ordinary C semantics defeating div/mod fusion,
> not a banned register pin); round 35's permuter search (15862+
> iterations) found no zero and never beat the base score, residue
> marked permuter-exhausted.

The function comment above GetMetaEvent carried this stall paragraph between its summary and its per-type notes:

> STALL -- see docs/match-reports/GetMetaEvent.md. 2 words SHORT
> (211/213, compiled length measured off build/src/libsnd_seqread.c.o since
> funcdiff's word-match number is not trustworthy once length drifts).
> First real diff at word 56 (`tools/funcdiff.py GetMetaEvent`), a
> register-identity symptom: retail re-reads `rec->unk4A` fresh (a plain
> `lh`) before EACH of the Set-Tempo rate recompute's two divisions; this
> body keeps the first read's value live in a register instead. The
> round-26 head's narrowed-`volatile` lever (only the WORD-sized
> `rec->unk8C` field marked `volatile`, not the sub-word `unk4A`) closed
> 18 of the 20 words this stalled at previously by defeating GCC's
> div/mod fusion without retail's plain `lh` turning into `lhu`+widen.
> The one remaining word resisted six further reshapes (see report) --
> every one of them either had no effect or regressed.

The declarations above GetMetaEvent carried:

> Cross-unit calls, local guesses per project convention. SpuVmSeqKeyOff is
> matched in libsnd_vmanager.c and already has this exact "(slot<<8)|channel"
> single-argument reading in both libsnd_cres.c and libsnd_decre.c;
> _SsSndNextSep is Sony's `libsnd/next`, linked from the SDK object since
> round 34; this signature is the one libsnd_cres.c's matched C used
> before the conversion.
