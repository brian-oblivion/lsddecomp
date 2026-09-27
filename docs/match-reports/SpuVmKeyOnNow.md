# SpuVmKeyOnNow -- STALL (round 65 revisit: **EXACT LENGTH 316/316** built words, up from round 45's 332-recorded / 334-rebuilt 18-words-LONG; raw word-match **201/316** after the round's one permuter search (195/316 before it) up from 33/316, measured with NO out-of-range drift; first real diff at file 0x1DA2C / vram 0x8002D22C -- unchanged in POSITION but no longer the same thing: it is now only the frame SIZE, `addiu sp,sp,-8` against retail's `-0x10`, per tools/asm-differ/diff.py. Round 45's `volatile s16 D_8008EA26` model is RETRACTED below -- the global is an ordinary non-volatile `s16` declared as an incomplete array)

> Renamed from `func_8002D1B4` on 2026-09-24 (tools/rename.py). Address 0x8002d1b4.

> Round 44's stub report (kept below the divider) predicted this would be
> "promising rather than hard" by analogy with its unit-siblings
> `ServiceSoundCueSet` and `vmNoiseOn`. That held for roughly the first 30
> instructions (full byte-for-byte match, see below) and then did not: this
> function needed real derivation work (a second, independent divisor
> family -- 16129 = 127² used DIRECTLY rather than via two chained /127
> divisions, a THIRD data table, and a per-note lookup with an early-out
> branch `vmNoiseOn` doesn't have), and closes with a genuine,
> not-yet-solved frame-allocation-POSITION residue on top of this unit's
> already-documented register-identity class. Round 45 is this function's
> first-ever attempt.

Unit: `src/code_179d8_l.c` (carved round 24, 2026-09-08) · Size: 316 words
(0x4F0 bytes), file offset `0x1D9B4`, vram `0x8002D1B4`.

## What it computes

Sibling of `vmNoiseOn` in the same unit (same two-level `_ss_score`
entry table, same three-stage `D_8008EA1A`/`17`/`11` blend cascade, same
`D_8008E8C0`-gated clamp, same low/high enable-bit split) but a DIFFERENT
source object and a DIFFERENT tail:

- **Level source**: reads `_svm_vh->unk18` (a per-session "priority"-ish
  byte, NOT the per-note `_ss_score` entry) and combines it with
  `D_8008EA10` through a SIGNED division by `16129` (`=127²`, confirmed
  against the SAME magic constant `0x82061029`/shift-13 the sibling unit's
  own report `SetAutoPan.md` -- `code_179d8_m.c`, read for reference
  only, not edited -- already brute-force-identified as divisor 16129 via a
  batch-compiled `int f(int){return a/N;}` sweep through the pinned
  pipeline). The result is then multiplied by `D_8008EA16` and `D_8008EA19`
  and divided by `16129` AGAIN, this time UNSIGNED (`multu`, the standard
  unsigned-division-by-constant correction shape, magic `0x040C2051`
  matching that same sibling report's already-published constant for the
  same divisor). **Both divisions are literal single `/16129`, not two
  chained `/127`s** -- m2c's own pattern-matcher recognized the first
  (signed) division automatically as `/16129`.
- **Per-note lookup with an early-out** `vmNoiseOn` does not have: reads
  `D_8008EA22` (same two-level `_ss_score[lowbyte][highbyte]` indexing as
  D8E0) and compares the FULL SIGNED 16-bit value against `0x21` (33) --
  when equal, both channel levels stay at the raw `lvl1` (no per-note
  scaling); otherwise each is separately scaled by the entry's `unk74`/
  `unk76` (`lvl1 * entry->field / 127`, single division, matching D8E0's own
  established `x/127` idiom).
- **The three-stage `D_8008EA1A`/`17`/`11` blend cascade is IDENTICAL in
  shape to `vmNoiseOn`'s** (each stage: `byte<0x40` picks which of the
  pair gets scaled by `byte` vs `127-byte`, divided by 63) -- reused
  verbatim from that function's already-proven-correct C shape.
- **A genuinely different tail**: after the clamp, BOTH pan values are
  SQUARED and divided by a THIRD divisor, `16383` (`=0x3FFF`, confirmed via
  the pinned-pipeline reproducer -- see below), then written together with
  the RAW `a1` parameter into a THIRD 16-byte-stride table, `D_8008D7F4`
  (distinct from `vmNoiseOn`'s `_svm_sreg_buf`/`D_8008D7F2`), at constant
  BYTE offsets `-4`/`-2`/`0` from one base-index computation
  (`D_8008EA26 * 8`, i.e. an s16-ELEMENT index of `D_8008D7F4[idx-2]`,
  `[idx-1]`, `[idx]`) -- retail computes ONE `lui`/`addiu` for `D_8008D7F4`
  and does constant -4/-2 arithmetic from it, not three separate symbol
  loads, which is why the model here is `D_8008D7F4[idx-2]` rather than
  reusing `_svm_sreg_buf`/`D_8008D7F2` (those ARE separate, independently-named
  16-byte-stride tables elsewhere in the project -- see
  `src/code_179d8_j_c.c`'s own `_svm_sreg_buf`/`D_8008D7F4` pair, declared
  "independent array, same shape" there -- so this is NOT presumed to be the
  same physical memory as D8E0's pair, just a THIRD table with the same
  stride).
- **`D_8008EA26` (the "currently selected channel" scratch global other
  units already declare `volatile u16`) needs a SIGNED per-unit view here**:
  the first read in this function compiles to `lh`, not `lhu` (confirmed by
  direct `objdump -dr` comparison against retail -- see Attempts). Declared
  `extern volatile s16 D_8008EA26;` locally in this file (a legitimate
  per-unit divergence from other units' `u16` view, per CLAUDE.md's
  multiple-independent-local-views convention) rather than reusing another
  unit's header.
- **The write order in the enable-bit tail is REVERSED from `vmNoiseOn`**:
  this function updates the `D_8008EA20`-gated `_svm_orev1`/`234` pair
  FIRST, then `D_8008E228`/`22C`/`80090C60`/`64` SECOND -- the opposite
  order from D8E0. Confirmed structurally correct by m2c's own independent
  decode; not treated as an error.

## The `/16383` divisor -- verified against the pinned pipeline

```c
unsigned int probeu(unsigned int a) { return a / 16383; }
```

compiles (through the project's pinned `cpp | cc1 -O2 | maspsx | as`
pipeline, exact `MASPSX_FLAGS` read from the Makefile) to:

```
lui v0,0x4; ori v0,v0,0x11; multu a0,v0; mfhi v0
subu a0,a0,v0; srl a0,a0,0x1; addu v0,v0,a0; srl v0,v0,0xd
```

byte-for-byte retail's magic constant (`0x00040011`) and correction
sequence for the two squared-value divisions at the end of this function.
Plain `u32 x / 16383` is the correct spelling.

## Struct/global model (all local to this unit)

```c
/* Shared with vmNoiseOn (declared once, before this function since it
 * is ROM-earlier). */
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} SsScore;
extern SsScore *_ss_score[];

extern u8 D_8008EA16;
extern u8 D_8008EA19;
extern u8 D_8008EA17;
extern u8 D_8008EA11;
extern u8 D_8008EA1A;
extern s16 D_8008E8C0;
extern u16 D_8008EA22;
extern u8 D_8008EA20;
extern u8 _svm_sreg_dirty[];
extern u8 D_8008D98C[];
extern u8 D_8008D9A3[];
extern u16 D_8008E228;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 _svm_orev1;
extern u16 _svm_orev2;

/* Object holding a per-note "priority"-ish scale byte at +0x18; only field
 * this function needs. */
typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *_svm_vh;

extern u8 D_8008EA10;
extern volatile s16 D_8008EA26; /* SIGNED here -- see "What it computes". */

/* Independent 0x10-byte-stride s16 array. */
extern s16 D_8008D7F4[];
```

## Best-derived body (332/316 words, 16 words LONG, preserved for the next attempt)

```c
void SpuVmKeyOnNow(s32 a0, s32 a1) {
    SsScore *e;
    s32 prio;
    s32 lvl0;
    u32 lvl1;
    u32 pan1;
    u32 pan2;
    u32 pan1sq;
    u32 pan2sq;
    s32 chanIdx;
    s32 lowBit;
    s32 highBit;

    prio = _svm_vh->unk18 * 0x3FFF;
    lvl0 = D_8008EA10 * prio / 16129;
    lvl1 = (u32)lvl0 * D_8008EA16 * D_8008EA19 / 16129;

    e = &_ss_score[D_8008EA22 & 0xFF][D_8008EA22 >> 8];
    pan1 = lvl1;
    pan2 = lvl1;
    if ((s16)D_8008EA22 != 0x21) {
        pan1 = lvl1 * e->unk74 / 127;
        pan2 = lvl1 * e->unk76 / 127;
    }

    if ((u8)D_8008EA1A < 0x40) {
        pan2 = (pan2 * D_8008EA1A) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA1A)) / 63;
    }

    if ((u8)D_8008EA17 < 0x40) {
        pan2 = (pan2 * D_8008EA17) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA17)) / 63;
    }

    if ((u8)D_8008EA11 < 0x40) {
        pan2 = (D_8008EA11 * pan2) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA11)) / 63;
    }

    pan1sq = pan1 * pan1;
    if (D_8008E8C0 == 1) {
        if (pan1 < pan2) {
            pan1 = pan2;
        } else {
            pan2 = pan1;
        }
        pan1sq = pan1 * pan1;
    }
    pan2sq = pan2 * pan2;

    chanIdx = D_8008EA26 * 8;
    D_8008D7F4[chanIdx] = (s16)a1;
    D_8008D7F4[chanIdx - 2] = (s16)(pan1sq / 16383);
    D_8008D7F4[chanIdx - 1] = (s16)(pan2sq / 16383);

    _svm_sreg_dirty[D_8008EA26] |= 7;
    *(u16 *)(D_8008D98C + D_8008EA26 * 0x34) = (s16)a1;
    *(u8 *)(D_8008D9A3 + D_8008EA26 * 0x34) = 1;

    if (D_8008EA26 < 0x10) {
        lowBit = 1 << D_8008EA26;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (D_8008EA26 - 0x10);
    }

    if (D_8008EA20 & 4) {
        _svm_orev1 |= lowBit;
        _svm_orev2 |= highBit;
    } else {
        _svm_orev1 &= ~lowBit;
        _svm_orev2 &= ~highBit;
    }

    D_8008E228 |= lowBit;
    _svm_okof1 &= ~D_8008E228;
    _svm_okon2 |= highBit;
    _svm_okof2 &= ~_svm_okon2;
}
```

**The opening ~30 instructions match retail BYTE-FOR-BYTE** (confirmed with
`tools/asm-differ/diff.py SpuVmKeyOnNow`) once the multiply order was fixed
to an explicit `prio` intermediate (see Attempts) -- including both
magic-multiply divisions, the two-level `_ss_score` entry lookup, and the
`id != 0x21` early-out branch with its per-note scaling. The gap starts at
the STACK FRAME ALLOCATION.

## Residues

1. **Frame allocation happens at the WRONG PROGRAM POINT, not just the
   wrong SIZE.** Retail's `addiu sp,sp,-0x10` lands immediately after the
   second (`/16129` unsigned) division, BEFORE the first read of
   `D_8008EA26`. This derivation's frame allocation lands several
   instructions LATER, after the raw `a1`-preserve and the `D_8008EA26`
   read/shift. This is a DIFFERENT shape of residue from `vmNoiseOn`'s
   (which has the right program POSITION but wrong SIZE, -0x10 vs retail's
   -8) -- here the eventual SIZE was not even reached before the position
   diverged, so D8E0's "mark the chained-division intermediates volatile"
   lever was tried directly (see Attempts) and made this function WORSE,
   not better, confirming the two functions' frame gaps are not the same
   lever despite superficially similar division chains.
2. **A register-identity swap at the `id != 0x21` join**, same unfixable
   class as this unit's other three stalls (`vmNoiseOn2`,
   `SePitchBend`, `vmNoiseOn`): retail's unconditional delay-slot copy
   is `move $a2,$a3` (i.e. `pan1`'s fallback gets a fresh register, `pan2`'s
   fallback quietly reuses whichever register already held `lvl1`); every
   derivation tried here either omits the copy entirely (when `pan1`/`pan2`
   are assigned in-place, in which case retail's explicit copy has no C
   counterpart) or emits `move $a3,$a2` -- the mirror-image swap -- when
   `pan1`/`pan2` are split into distinct `_pre` intermediates matching
   `vmNoiseOn`'s exact proven shape (see Attempts #3). Per HARD RULE 6
   and this unit's own established finding, not pursued further as an
   independent lever.
3. **`D_8008EA26`'s per-read signedness may not be uniform across the
   function** -- the very first read (feeding `chanIdx`) is confirmed `lh`,
   but a full per-call-site audit of the ~5 raw `D_8008EA26` references in
   `asm/nonmatchings/code_179d8_l/SpuVmKeyOnNow.s` (rather than trusting one
   blanket `volatile s16` declaration to cover all of them) was not
   completed this round -- next concrete step.

## Attempts

Within the 30-attempt cap (9 real builds used):
1. m2c-seeded skeleton (`tools/m2ctx.py code_179d8_l --sig 'void
   SpuVmKeyOnNow(s32 a0, s32 a1)' --run`) transcribed directly, entry-field-
   first multiply order (`_svm_vh->unk18 * 0x3FFF * D_8008EA10 / 16129`):
   compiled, but the very FIRST instruction loaded `D_8008EA10` before
   `_svm_vh`/`unk18` -- opposite of retail. **Root cause confirmed via
   GCC's own re-association, not a source-order artifact**: swapping the
   C-level operand order (`D_8008EA10 * (_svm_vh->unk18 * 0x3FFF)`)
   produced the IDENTICAL wrong load order, because GCC 2.6.3 re-associates
   a chain of multiplies to attach the constant-foldable `*0x3FFF` to
   whichever operand it prefers, REGARDLESS of source parenthesization, when
   both multiplies sit in one expression tree.
2. Split the constant multiply into its own statement
   (`prio = _svm_vh->unk18 * 0x3FFF; lvl0 = D_8008EA10 * prio / 16129;`):
   **fixed the load order** (confirmed via `objdump -dr` register-and-
   relocation comparison, not just visual diff) -- the statement boundary
   prevents GCC's re-association from reaching across it. This is the
   version kept.
3. Split `pan1`/`pan2`'s first assignment into `pan1pre`/`pan2pre`
   intermediates, mirroring `vmNoiseOn`'s exact proven three-stage-blend
   shape (`pan1 = pan1pre; pan2 = (pan2pre*byte)/63;` in each arm): compiled
   to 337/316 (WORSE than the 332 kept version) and swapped which register
   got the explicit `move` (retail: `a2,a3`; this attempt: `a3,a2`) rather
   than fixing it. **Reverted** -- residue 2 above is the same unfixable
   class already documented three times in this unit, not something this
   restructuring resolves, and it cost 5 extra words while not doing so.
4. `volatile` on `lvl0`/`lvl1` (the direct `vmNoiseOn` lever, which
   fixed THAT function's frame-size gap from 295 to 309/311): compiled to
   342/316, WORSE than both the non-volatile version and attempt #3.
   **Reverted.** This is worth recording plainly: **the same lever that
   closed `vmNoiseOn`'s frame gap makes this structurally-similar
   sibling function's gap WORSE**, confirming (as the two functions'
   residue-1 descriptions already show) that the frame issues are NOT the
   same underlying mechanism despite both functions chaining
   magic-multiply divisions through several intermediates.
5. `D_8008EA26` declared `extern volatile s16` (signed) instead of the
   other units' `volatile u16`: fixed the FIRST read (`chanIdx` computation)
   to `lh`, confirmed byte-identical through the first ~30 instructions
   against retail. Cost +6 words overall (326 -> 332) from knock-on effects
   further down the function -- kept anyway, since the alternative (`u16`)
   left the very first instructions wrong, which is a worse starting point
   for the next attempt even though the raw word count was lower.

### Proposed learning

**A lever that fixes one function's frame-allocation gap can make a
structurally-near-identical sibling's gap WORSE, even within the same unit
and even when both chain the same family of magic-multiply divisions.**
`vmNoiseOn` and `SpuVmKeyOnNow` share the two-level `_ss_score`
lookup, the exact same three-stage blend cascade, and the same
`D_8008E8C0` clamp shape, and both have unresolved frame-size/position
gaps -- but marking the chained-division intermediates `volatile` (which
took D8E0 from 295 to 309/311) makes D1B4 regress from 332 to 342. Do not
assume a documented frame-size fix generalizes to a "similar-looking"
function in the same unit without measuring; treat each function's frame
residue as its own lever search.

**Also worth carrying forward: GCC 2.6.3 re-associates constant multiplies
across an entire expression tree regardless of source parenthesization**,
so splitting a constant-foldable sub-multiply into its own statement (not
just reparenthesizing) is the actual lever for controlling load/evaluation
order in a chained-multiply expression -- a statement boundary, not a pair
of parens, is what stops the re-association.

## Disposition

**Restored to `INCLUDE_ASM`** (no score short of byte-exact stays in
`src/`). First-ever attempt on this function reached a genuine, verified
structural derivation: the opening ~30 instructions (both magic-multiply
divisions, the two-level entry lookup, the `id != 0x21` early-out, and the
full three-stage blend cascade reused verbatim from `vmNoiseOn`'s
already-proven shape) match retail byte-for-byte. The gap is a
frame-allocation-position residue (not yet closed by either of the two
levers tried) plus this unit's already-documented register-identity class.
The next attempt should (a) audit each of `D_8008EA26`'s ~5 raw read sites
in the `.s` individually for `lh` vs `lhu` rather than trusting one blanket
declaration, and (b) search for what makes retail allocate the frame
immediately after the second division rather than after the `D_8008EA26`
read+shift -- neither the D8E0-style `volatile` lever nor plain
restructuring reached it this round.

---

# (round 44, superseded) SpuVmKeyOnNow -- ASSIGNABLE, NEVER ATTEMPTED (316w, no figures existed before round 45)

> **Round 44, head.** No length, word-match or first-diff figure is quoted here
> because NONE HAS EVER BEEN MEASURED -- this function has never been compiled.
> It is cold ground, not a stall, and `nearmiss.py` should be read that way.
> The old title (`TOOLCHAIN BLOCKED (nop_mflo_mfhi), NOT ATTEMPTED`) is
> withdrawn: that blocker was RESOLVED in round 42 by `--no-nop-mflo-mfhi`.
>
> Its two unit-siblings were reopened on the same evidence and both moved a
> long way on FIRST contact in round 44 -- `ServiceSoundCueSet` 0 -> 110/132 and
> `vmNoiseOn` 0 -> 309/311 -- so treat this as promising rather than hard.
> Delta ran out of round budget before reaching it; nothing about it was tried
> and found difficult.

Unit: `src/code_179d8_l.c` (carved round 24, 2026-09-08) · Size: 316 words.

## Evidence (obsolete -- kept for history)

Screened with the canonical forward form (`grep -A2`, which prints the two
FOLLOWING lines -- the direction is load-bearing and every reimplementation so
far has inverted it):

```sh
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/code_179d8_l/SpuVmKeyOnNow.s \
  | grep -E '\b(mult|multu|div|divu)\b'
```

Hits: 2 pairs (mflo at 0x8002D2DC -> multu at 0x8002D2E0; mfhi at 0x8002D49C -> mult at 0x8002D4A0, both adjacent).

This screen is now moot: `--no-nop-mflo-mfhi` (round 42) makes the pipeline
absorb both pairs without a length penalty, which round 45's actual attempt
confirmed empirically -- neither of these two sites appears anywhere in the
list of residues above.

---

## Round 65 update (runner bravo): REVISIT — 332 words LONG down to **EXACT LENGTH 316/316**, word-match 33/316 -> 195/316

**REVISITED, round 65: MAJOR IMPROVEMENT (built length 334 -> 316 = EXACT;
raw word-match 33/316 -> 195/316; asm-differ score 9040 -> 3765; funcdiff
insertions 62/deletions 62 -> 45/45 with NO out-of-range drift); still a
STALL; names/types USED — the round-45 `volatile s16 D_8008EA26` model is
wrong and is retracted here, replaced by an incomplete-array `s16
D_8008EA26[]`.**

### Rebuilt-body measurement, taken BEFORE any change (mandatory bookkeeping)

Round 45's preserved body spliced back verbatim (including its
`extern volatile s16 D_8008EA26;`) and built through the real oracle:

- **334 words** (`0x538`), i.e. **18 words LONG**, not the 332 / 16-long the
  round-45 title records. The body is byte-identical to the report's; the two
  extra words are a TOOLCHAIN delta, not a transcription error — round 63
  added maspsx `--nop-at-expansion` (CLAUDE.md, "Open toolchain blockers"),
  which inserts load-delay nops round 45's pipeline did not. **A recorded
  LENGTH figure is only comparable within one toolchain generation; the
  word-match figure was unaffected (33/316 reproduced exactly).**
- `funcdiff`: `33/316 words match`,
  `insertions 62 / deletions 62 (... positional skeleton diffs 282)`,
  plus a `differs OUTSIDE this range (drift)` warning — so that 62/62 is
  length-contaminated and is NOT a Gate 3 check-3 signature.

### The finding that carried the round: `D_8008EA26` is an ORDINARY `s16`, and round 45's `volatile` is the reason the body was 18 words long

Round 45 declared it `extern volatile s16 D_8008EA26;` because retail reads
it more than once. That reasoning is sound and the conclusion is wrong, and
the way it is wrong is worth writing down.

**Measured: retail reads `D_8008EA26` exactly FIVE times, and each read is a
single `lh`.** Four of them use a base register (`lh $v1, 0($a0)` with
`$a0 = &D_8008EA26` materialised once), which is why a
`grep '%lo(D_8008EA26)'` over the `.s` finds only one and a first pass —
mine — concludes "read once". **Grep the `.s` for the symbol AND for a
register-based reload before concluding how many times a global is read.**

The five reads line up one-for-one with the intervening STORES:

| read | at | store that killed the previous cache |
| --- | --- | --- |
| 1 | 1DA34 | — (first) |
| 2 | 1DD10 | the three `D_8008D7F4[...]` stores |
| 3 | 1DD40 | `_svm_sreg_dirty[...]` |
| 4 | 1DD6C | `D_8008D98C + ...` |
| 5 | 1DD9C | `D_8008D9A3 + ...` |

Read 5 serves the `< 0x10` test AND both `sllv`s, with no store between —
i.e. CSE holds exactly where nothing kills it. **That is ordinary
non-volatile CSE-invalidation by stores through computed addresses, not
`volatile` semantics.**

And `volatile` is not merely unnecessary, it is expensive in this toolchain:
a `volatile s16` read used in an `s32` context compiles to
`lhu` + `sll 16` + `sra 16` (3 words), because combine will not fold a
sign-extension through a volatile MEM into `lh` (1 word). Four such reads is
8 wasted words. Dropping `volatile` and referencing the global directly at
each of the five sites — letting the stores do the invalidating — is what
took the body from 328 to 318 words.

### The second finding: the INCOMPLETE-ARRAY vs SCALAR declaration axis is NOT inert here, and it closed the last word

`extern s16 D_8008EA26;` with scalar references emits a fresh
`lui` + `lh $x, %lo(sym)($x)` pair at every read site (2 words each).
`extern s16 D_8008EA26[];` with `D_8008EA26[0]` references makes GCC 2.6.3
materialise the address ONCE into a GPR (`lui`/`addiu`) and spend one word
per read (`lh $v1, 0($a0)`) — which is retail, exactly.

**317 -> 316 words = EXACT LENGTH**, asm-differ 5310 -> 4240, word-match
181/316. The project's note that this axis "does not reproduce in isolation"
still stands — it did not reproduce on a reduced test here either — but it is
plainly not byte-inert in situ, and the mechanism is now named: it is an
ADDRESS-MATERIALISATION axis, and it only pays where the same global is read
several times in one function.

### Third: two more levers, one of them transferred straight from `vmNoiseOn2` this same round

- **`pan1sq` belongs AFTER the `D_8008E8C0` clamp, not before-and-again-inside.**
  Round 45's body computed `pan1sq = pan1*pan1;` before the `if`, then again
  inside it. Retail has two `mult $a2,$a2` and only ONE `mflo` — which is a
  single source-level multiply after the block, with GCC filling the `bne`
  delay slot with the `mult` on the fall-through path and recomputing it after
  the swap. Writing it once, after the block: 318 -> 317 words.
  **Two `mult`s and one `mflo` is a delay-slot duplication, not two source
  multiplies** — counting `mult`s to infer source statements is what produced
  the round-45 shape.
- **The `vmNoiseOn2` tail lever transfers verbatim** (see that report's
  round-65 section): write `G = x | G;` not `G |= x;` (retail's `or $v1,$a2,$v1`
  puts the loaded value SECOND), and order the four global updates by retail's
  LOAD order `E228, E22C, C60, C64` rather than `E228, C60, E22C, C64`. The
  same applies to the `_svm_orev1`/`_svm_orev2` pair in the `D_8008EA20 & 4`
  arm. 4240 -> 3765.

### Levers tried and their verdicts

| lever | result |
| --- | --- |
| drop `volatile`, read via a single local `ch` everywhere | 334 -> 301 words but 15 SHORT — the local defeats the reloads retail has. Diagnostic, not the answer |
| drop `volatile`, reference the global directly at all five sites | 328 -> 318 words; the correct model |
| `extern s16 D_8008EA26[];` + `D_8008EA26[0]` (INCOMPLETE-ARRAY axis) | **POSITIVE** — 317 -> **316, exact**; address materialised once, as retail |
| `pan1sq` moved after the clamp block | **POSITIVE** — one word |
| `G = x \| G` operand order + LOAD-order statement order in the two enable-bit tails | **POSITIVE** — 4240 -> 3765 |
| `chanIdx` statement placement, 4 positions (top / after `lvl0` / before `e=` / after `e=`) | all four **INERT** — identical 301 words and identical score. The scheduler normalises them |
| `u16 chanIdx` instead of `s32` + `(u16)` at use | **REGRESSION** — turns retail's `lh` into `lhu`, because a u16 destination makes the sign irrelevant |
| a separate `u16 idx` local for the three `D_8008D7F4` subscripts | **INERT** — still `andi 0xfff8`, identical 317/5310 |
| hoisting `(s16)D_8008EA22` into a local before the `e=` subscript | mixed: **fixes the FRAME to `-0x10`** (see below) but merges the read into a single `lh` where retail has `lhu`, net worse (4035 vs 3765) |
| splitting only `pan2` in blend stage 1 (to reach retail's `move $a1,$a3`) | **REGRESSION** — 316 -> 318, 4080. Consistent with round 45's attempt #3, which split both |

### What is left (45 insertions / 45 deletions at exact length, no drift)

1. **Frame SIZE, `addiu sp,sp,-8` against retail's `-0x10`.** Round 45
   recorded this as a frame-POSITION residue; at 316 words the position is
   now right and only the size differs. **It is reachable**: the
   `(s16)D_8008EA22`-hoist variant above produces `-0x10` — so one more
   long-lived local is all it takes. Finding the one that does it WITHOUT
   changing the `D_8008EA22` load from `lhu` to `lh` is the next concrete
   step, and it is a narrow, well-posed search rather than a stall.
2. **Head scheduling**: retail fills the `multu` latency with the
   `D_8008EA26` read, the `$a1` preserve and the `chanIdx` shift only;
   the built body also hoists the whole `D_8008EA22` decode in there.
3. **`andi $v1,$t1,0xffff` vs `andi $v1,$t1,0xfff8`** — one immediate.
   GCC simplifies our mask because it can see `chanIdx` is `x << 3`; retail's
   could not. Not reached by the `u16`-local axis.
4. **`move $a1,$a3` at blend stage 1** — the unit's register-identity class.
5. **`sll 16` sharing in the `D_8008EA22` decode**: retail derives BOTH the
   `(s16)` compare value (`sra 16`) and the high-byte subscript (`srl 24`)
   from one `sll 16`; the built body computes the subscript as `srl 8` from
   the unshifted value.

### Attempts

~16 build/oracle iterations. Inside the 30-attempt cap and nowhere near the
"30 consecutive builds with no improvement" stop rule — the last improvement
was two iterations before the stop.

### Preserved body (316/316 words EXACT, 195/316 matched — rebuild this FIRST next time)

Declarations to add to the unit alongside the ones already before this
function; note `D_8008EA26` is an INCOMPLETE ARRAY and is NOT volatile:

#if 0
/* Object holding a per-note "priority"-ish scale byte at +0x18; only field
 * this function needs. */
typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *_svm_vh;

extern u8 D_8008EA10;
/* NOT volatile, and declared as an incomplete ARRAY on purpose: the array
 * spelling is what makes GCC 2.6.3 materialise the address once into a GPR
 * and spend one word per read, which is retail.  Retail's five reloads come
 * from ordinary CSE invalidation by the stores between them. */
extern s16 D_8008EA26[];

/* Independent 0x10-byte-stride s16 array. */
extern s16 D_8008D7F4[];

void SpuVmKeyOnNow(s32 a0, s32 a1) {
    SsScore *e;
    s32 prio;
    s32 lvl0;
    u32 lvl1;
    u32 pan1;
    u32 pan2;
    u32 pan1sq;
    u32 pan2sq;
    s32 chanIdx;
    s32 lowBit;
    s32 highBit;

    prio = _svm_vh->unk18 * 0x3FFF;
    lvl0 = D_8008EA10 * prio / 16129;
    lvl1 = (u32)lvl0 * D_8008EA16 * D_8008EA19 / 16129;

    chanIdx = D_8008EA26[0] * 8;

    e = &_ss_score[D_8008EA22 & 0xFF][D_8008EA22 >> 8];
    pan1 = lvl1;
    pan2 = lvl1;
    if ((s16)D_8008EA22 != 0x21) {
        pan1 = lvl1 * e->unk74 / 127;
        pan2 = lvl1 * e->unk76 / 127;
    }

    if ((u8)D_8008EA1A < 0x40) {
        pan2 = (pan2 * D_8008EA1A) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA1A)) / 63;
    }

    if ((u8)D_8008EA17 < 0x40) {
        pan2 = (pan2 * D_8008EA17) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA17)) / 63;
    }

    if ((u8)D_8008EA11 < 0x40) {
        pan2 = (D_8008EA11 * pan2) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA11)) / 63;
    }

    if (D_8008E8C0 == 1) {
        if (pan1 < pan2) {
            pan1 = pan2;
        } else {
            pan2 = pan1;
        }
    }
    pan1sq = pan1 * pan1;
    pan2sq = pan2 * pan2;

    D_8008D7F4[(u16)chanIdx] = (s16)a1;
    D_8008D7F4[(u16)chanIdx - 2] = (s16)(pan1sq / 16383);
    D_8008D7F4[(u16)chanIdx - 1] = (s16)(pan2sq / 16383);

    _svm_sreg_dirty[D_8008EA26[0]] |= 7;
    *(u16 *)(D_8008D98C + D_8008EA26[0] * 0x34) = (s16)a1;
    *(u8 *)(D_8008D9A3 + D_8008EA26[0] * 0x34) = 1;

    if (D_8008EA26[0] < 0x10) {
        lowBit = 1 << D_8008EA26[0];
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (D_8008EA26[0] - 0x10);
    }

    if (D_8008EA20 & 4) {
        _svm_orev1 = lowBit | _svm_orev1;
        _svm_orev2 = highBit | _svm_orev2;
    } else {
        _svm_orev1 = _svm_orev1 & ~lowBit;
        _svm_orev2 = _svm_orev2 & ~highBit;
    }

    D_8008E228 = lowBit | D_8008E228;
    _svm_okon2 = highBit | _svm_okon2;
    _svm_okof1 = _svm_okof1 & ~D_8008E228;
    _svm_okof2 = _svm_okof2 & ~_svm_okon2;
}
#endif

### Proposed learnings (round 65)

**1. "Retail reads this global more than once" does NOT imply `volatile`, and
in this toolchain guessing `volatile` is expensive.** GCC 2.6.3's CSE drops a
cached global load at any store through a computed address, so an ordinary
global read on both sides of such a store is reloaded — five reloads here,
all from plain stores into `u8[]`/`s16[]` tables. The discriminator is the
INSTRUCTION, not the count: a non-volatile `s16` read in an `s32` context is
`lh`; the same read declared `volatile` is `lhu` + `sll 16` + `sra 16`,
because combine refuses to fold a sign-extension through a volatile MEM.
**So: count the reads to raise the question, then read ONE of them to answer
it.** Retail here is `lh`, three words cheaper per read, and 8 words of the
round-45 body's 18-word overshoot were that one declaration.

**2. Grep for the register-based reload, not only for `%lo(sym)`.** Four of
this function's five reads spell the symbol nowhere — they are
`lh $v1, 0($a0)` off a materialised base. A `grep %lo(D_8008EA26)` returns 1
and reads like a fact. It cost me a whole wrong model (a single local `ch`,
which compiled 15 words SHORT) before the tail diff showed the base register.

**3. The INCOMPLETE-ARRAY vs SCALAR axis is an ADDRESS-MATERIALISATION
lever.** `extern T sym;` + scalar refs costs `lui`+`l*` per read;
`extern T sym[];` + `sym[0]` refs materialises `&sym` once into a GPR and
costs one word per read. It therefore only bites where a global is read
several times in one function — which is exactly the situation created by
learning 1. The two levers are a PAIR: dropping `volatile` creates the
reloads, and the array spelling makes each reload cost what retail's costs.
Neither alone reaches exact length here.

**4. Two `mult`s and one `mflo` is ONE source multiply.** GCC fills a
conditional branch's delay slot with the multiply on the fall-through path
and recomputes it on the path that changed an operand, then takes a single
`mflo` at the join. Counting `mult` instructions to infer how many times the
source multiplies is what produced round 45's duplicated `pan1sq`.

## Round 65 addendum (runner bravo): the session's ONE permuter search, run on the length-exact body — a real lever, and it needed translating twice

### Gate 3, all three checks, run BEFORE the search (PARALLEL-RUNS.md §3.5)

- **Check 1 (correctness):** the scaffold compiles and scores.
  `tools/setup-permuter.sh SpuVmKeyOnNow <seed>` built cleanly; base score
  **4470**, `Stack Differences: 0 (1)`, `Branch Differences: 0 (1)`,
  `Register Differences: 42 (5)`, `Reorderings: 1 (60)`.
- **Check 2 (cost):** the scaffold's `--debug --stack-diffs`
  insertion/deletion count is **21 / 21**.
- **Check 3 (agreement):** the real-build `tools/funcdiff.py` line on the
  SAME body is **insertions 45 / deletions 45** (positional skeleton diffs
  114), with **no out-of-range drift** — the first time this function has had
  a check-3-eligible signature at all, because it is the first time it has
  been length-exact.

**Verdict recorded honestly: the two signatures are both non-degenerate but
they DISAGREE numerically (21/21 vs 45/45), roughly 2:1.** Neither of §3.5's
two decline rows applies — the scaffold is not *dirtier* than a zero-drift
real build, and it is not a perfect zero against a non-matching real build.
The scaffold's own printed diff visibly carries the SAME residues the real
build has (the `addiu sp,sp,0x10` / `addiu sp,sp,8` frame-size pair, the same
register swaps), which is the substantive thing check 3 exists to establish.
The factor-of-two is most likely alignment: this function contains four
near-identical magic-multiply division sequences, i.e. exactly the repeating
skeleton alphabet `funcdiff` warns can align falsely. **Searched on that
basis, and the outcome (below) supports it — the search found a real lever.**

### The search

`-j 6 --stop-on-zero --best-only`, bounded to 1500s, which it hit
(`timeout` exit 124). Zero `score = 0` hits. Best candidate **2065** from a
base of 4470/4325, at `permuter-work/SpuVmKeyOnNow/output-2065-1`.

### The candidate, and what translating it took

Its ONLY structural change (everything else in the file is the permuter's
reformatting) is three lines:

```c
    pan1sq = pan1 * pan1;
    lvl0 = (s16)(pan1sq / 16383);   /* <- the first division hoisted into a temp */
    pan2sq = pan2 * pan2;           /* <- and pan2sq moved AFTER it */
    ...
    D_8008D7F4[((u16)chanIdx) - 2] = lvl0;
```

**Verified by direct rebuild in the real tree: asm-differ 3765 -> 2060,
word-match 195/316 -> 201/316, length still exactly 316.** So the search
paid, on a body the permuter could not have reached on its own (it started
from a base four levers of hand work better than round 45's).

**The translation has a trap, and it is the interesting half.** The permuter
reused the function's own DEAD `lvl0` local to hold the value. Written the
obvious honest way — a fresh `s32 sq1;` declared for the purpose, everything
else identical — the same change scores **3265**, barely better than the
3765 base. Reusing one pseudo instead of introducing a second is the
load-bearing part; the hoist alone is worth about a third of the gain.
In the body below the variable is named `tmp` and serves both roles;
**renaming was confirmed byte-inert** (2060 unchanged), consistent with this
unit's standing finding that identifier choice does not reach the allocator.

That is the local-COUNT lever again, in the direction the round-65 brief
names — and it is the third instance this session across two functions
(`vmNoiseOn2`'s four deleted tail temps, this). **The consistent shape:
when a residue is register identity, the number of simultaneously-live
pseudos is the axis, and it is reachable from C by merging or deleting
locals, not by renaming or reordering them.**

### Post-search state

**316/316 words (exact), 201/316 raw word-match, asm-differ 2060 (from 9040
at the start of the round), funcdiff insertions 46 / deletions 46.** The
preserved body above is superseded by the one below. The residue list is
unchanged in kind: the frame SIZE (`-8` vs `-0x10`, shown reachable), the
head scheduling of the `D_8008EA22` decode, the `0xffff`/`0xfff8` mask
immediate, `move $a1,$a3` at blend stage 1, and the `sll 16` sharing.

### Preserved body, round 65 FINAL (316/316 exact, 201/316 matched — rebuild THIS one)

Same declarations as the previous preserved body (`D_8008EA26` an incomplete
array, NOT volatile).

#if 0
void SpuVmKeyOnNow(s32 a0, s32 a1) {
    SsScore *e;
    s32 prio;
    s32 tmp;
    u32 lvl1;
    u32 pan1;
    u32 pan2;
    u32 pan1sq;
    u32 pan2sq;
    s32 chanIdx;
    s32 lowBit;
    s32 highBit;

    prio = _svm_vh->unk18 * 0x3FFF;
    tmp = D_8008EA10 * prio / 16129;
    lvl1 = (u32)tmp * D_8008EA16 * D_8008EA19 / 16129;

    chanIdx = D_8008EA26[0] * 8;

    e = &_ss_score[D_8008EA22 & 0xFF][D_8008EA22 >> 8];
    pan1 = lvl1;
    pan2 = lvl1;
    if ((s16)D_8008EA22 != 0x21) {
        pan1 = lvl1 * e->unk74 / 127;
        pan2 = lvl1 * e->unk76 / 127;
    }

    if ((u8)D_8008EA1A < 0x40) {
        pan2 = (pan2 * D_8008EA1A) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA1A)) / 63;
    }

    if ((u8)D_8008EA17 < 0x40) {
        pan2 = (pan2 * D_8008EA17) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA17)) / 63;
    }

    if ((u8)D_8008EA11 < 0x40) {
        pan2 = (D_8008EA11 * pan2) / 63;
    } else {
        pan1 = (pan1 * (0x7F - D_8008EA11)) / 63;
    }

    if (D_8008E8C0 == 1) {
        if (pan1 < pan2) {
            pan1 = pan2;
        } else {
            pan2 = pan1;
        }
    }
    pan1sq = pan1 * pan1;

    tmp = (s16)(pan1sq / 16383);
    pan2sq = pan2 * pan2;

    D_8008D7F4[(u16)chanIdx] = (s16)a1;
    D_8008D7F4[(u16)chanIdx - 2] = tmp;
    D_8008D7F4[(u16)chanIdx - 1] = (s16)(pan2sq / 16383);

    _svm_sreg_dirty[D_8008EA26[0]] |= 7;
    *(u16 *)(D_8008D98C + D_8008EA26[0] * 0x34) = (s16)a1;
    *(u8 *)(D_8008D9A3 + D_8008EA26[0] * 0x34) = 1;

    if (D_8008EA26[0] < 0x10) {
        lowBit = 1 << D_8008EA26[0];
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (D_8008EA26[0] - 0x10);
    }

    if (D_8008EA20 & 4) {
        _svm_orev1 = lowBit | _svm_orev1;
        _svm_orev2 = highBit | _svm_orev2;
    } else {
        _svm_orev1 = _svm_orev1 & ~lowBit;
        _svm_orev2 = _svm_orev2 & ~highBit;
    }

    D_8008E228 = lowBit | D_8008E228;
    _svm_okon2 = highBit | _svm_okon2;
    _svm_okof1 = _svm_okof1 & ~D_8008E228;
    _svm_okof2 = _svm_okof2 & ~_svm_okon2;
}
#endif

## NON_MATCHING body promoted, round 69

Promoted the round-65 FINAL preserved body (316/316 exact, 201/316 matched,
the one this report's TITLE figures describe) into `src/code_179d8_l.c`
under `#ifdef NON_MATCHING` (verified build unchanged, keeps `INCLUDE_ASM`);
`./build-and-verify.sh` and `tools/check-nonmatching.sh` both green. One
cosmetic change from the literal preserved text: the permuter's reused `tmp`
local (serving two unrelated roles -- the `/16129` intermediate, then the
`pan1sq/16383` value -- because reusing one pseudo instead of introducing a
second was the load-bearing part of the byte-shaped candidate) is split back
into two plainly-named locals, `lvl0` and `pan1out`, for the promoted body.
The report's own round-65-addendum section already confirms renaming is
byte-inert, so this changes nothing about what was measured.

## Naming (round 75, runner alpha, FINISHING-PLAN track 3)

Already carries its real name: identified round 74 (track 2, runner bravo)
as `libsnd/vmanager SpuVmKeyOnNow` (shape 0.99 vs the disc-3.3 reference,
312w reference vs our 316w, position within the libsnd neighborhood). Sony
symbol; this pass does not rename it further. The function remains a STALL
(frame-size residue, see above).

## Proposed global names (not renamed this pass -- weak/thin evidence)

Left as `D_` per the tier rubric ("a wrong tier-A name is worse than a
placeholder"); recorded here rather than guessed into a rename:

- `_svm_vh` (the `ObjE970`/`unk18` "priority-ish scale byte" object) --
  single use (`prio = _svm_vh->unk18 * 0x3FFF`). Could be a
  currently-playing-note object, not established.
- `D_8008EA10` -- multiplies `prio` before the two chained `/16129`
  divisions (`lvl0 = D_8008EA10 * prio / 16129`). Candidate "master
  volume"-style scalar; not confirmed against a real Sony field name.
- `D_8008EA16` / `D_8008EA19` -- the two `/127` multipliers producing
  `lvl1`. Candidate main-volume/submix-volume pair (mirrors the same
  cascade in `vmNoiseOn`'s already-matched `pAttack`/`pDecay` -> `lvl1`
  chain), not confirmed.
- `D_8008EA1A` / `D_8008EA17` / `D_8008EA11` -- the three `< 0x40` blend
  control bytes (division by 63, `0x7F - byte` on the else branch).
  Candidate pan/balance-style controls; three of them cascaded suggests a
  main/aux/reverb-style stack, not confirmed.
- `D_8008EA20` -- single bit (`& 4`) selects which direction
  `_svm_orev1`/`_svm_orev2` gets updated. Candidate per-voice routing/output
  flag byte; not confirmed.

These recur identically in `vmNoiseOn` (same unit, matching cascade shape --
see that report's "What it computes"), so a future pass with stronger
evidence (e.g. a Sony reference for this shape, since `SpuVmKeyOnNow` is
already identified as `libsnd/vmanager SpuVmKeyOnNow`) should rename them
together in both functions at once.

**Head note, round 75: none of these gets a game name.** `D_8008EA10` to
`D_8008EA20` lie inside Sony's `_svm_cur` (pinned at 0x8008EA0C in
`config/psyq-objects.ld`, extending to `_svm_vab_used` at 0x8008EA2C): they
are FIELDS of one libsnd struct, not separate globals, and this is Sony data
touched by Sony code. The right spelling is `_svm_cur.<field>` once that
struct is typed, which is track 2 / track 4 work. The runner's first pass
renamed 24 such addresses (including `_svm_tn` as `gNoteTable` and
`_ss_score` under a voice-envelope game name); the head dropped those commits
(`docs/PROGRESS.md`, round 75).

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

The NON_MATCHING body now stores `_svm_voice[D_8008EA26[0]].unk04/unk1B`; normalized disassembly identical.

**_svm_sreg_buf / _svm_sreg_dirty (same round).** `D_8008D7F0` (0x180 bytes, 24 voices x 0x10, halfwords at +0x0..+0xA spelled `D_8008D7F0`..`D_8008D7FA` by splat) is Sony's `_svm_sreg_buf` and `D_8008D970` (24 bytes) is `_svm_sreg_dirty`: libsnd/vmanager.o bss +0x000 and +0x180, anchored at 0x8008D7F0. Both are in the symbols file; the record type is `SvmSreg` in `include/SvmData.h` (fields by offset). The NON_MATCHING body keeps its halfword indexing, now `((s16 *)_svm_sreg_buf)[(u16)chanIdx + 2]` / `[(u16)chanIdx]` / `[(u16)chanIdx + 1]` for the old `D_8008D7F4[(u16)chanIdx]` / `[-2]` / `[-1]`; normalized disassembly identical.

## Types (round 98, alpha)

This unit's `D8008E978Entry` is now `<libsnd.h>`'s `VagAtr` and `ObjE970` is `VabHdr` (unk4 -> `center`, unk5 -> `shift`, unk12 -> `pbmin`, unk13 -> `pbmax`, ObjE970.unk18 -> `mvol`); preserved bodies above keep the old spellings. See SpuVmAlloc.md, "Unit banner history".
