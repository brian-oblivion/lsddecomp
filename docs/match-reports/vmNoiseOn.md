# vmNoiseOn -- STALL (round 45: permuter lever tried, NEGATIVE -- still 309/311, 2 words SHORT; scaffold confirmed not representative of the real build; first real diff still the opening instruction, vram 0x8002D8E0)

> Renamed from `func_8002D8E0` on 2026-09-24 (tools/rename.py). Address 0x8002d8e0.

> **REOPENED by round 44, AND SINCE WORKED -- marker spent (head, round 45).**
> This is now a DOCUMENTED STALL with a real, measured residue (309/311, 2 words
> short, gap isolated to the stack-frame size), not fresh ground. It is a
> two-word near-miss and wants a CHANGED state, not another cold pass, so
> `progress.py` counts it as a stall rather than returning it to `fresh`.
> The round-44 reopening text is kept for history: this report's sole cited
> cause was `nop_mflo_mfhi`, RESOLVED in round 42 by the maspsx flag
> `--no-nop-mflo-mfhi` (live in the Makefile's MASPSX_FLAGS; whole image
> byte-exact). The MECHANISM it documents is real and was re-confirmed by the
> round-44 head with the canonical forward grep -- what expired is the PREMISE
> that it is unfixable. Round 44 attempted the function for the first time ever
> and reached 309/311; everything below the divider is that work.

Unit: `src/code_179d8_l.c` (carved round 24, 2026-09-08) · Size: 311 words
(0x4DC bytes), file offset `0x1E0E0`, vram `0x8002D8E0`.

**2 words short** (best derivation compiles to 309 words against retail's
311). Raw word-match is not meaningful under that drift (funcdiff reports
5/311 windowed -- drift ripple, not real residue count). **First real diff is
the very first instruction**, vram `0x8002D8E0`: the derived body inserts an
extra `move $t2,$a0` (preserving the raw channel parameter) one instruction
earlier than retail schedules it -- confirmed with
`tools/asm-differ/diff.py vmNoiseOn`, not inferred. This is the SAME
"preserve raw copy before narrowing in place" register-identity residue
already documented for `vmNoiseOn2` and `SePitchBend` in this unit
(both also stalled on it, across three prior rounds each) -- a fourth
instance of the same class, in the same unit.

## What it computes

SPU voice envelope/pan setup, called with `a0` = channel number (0-255,
narrowed to a byte in two places: `chanRaw` for two 52-byte-stride table
index computations, `chan` for the 16-byte-stride pan tables and the
enable-mask bit). Looks up a per-note entry in a two-level table
(`_ss_score`: an array of pointers, each pointing to an array of
0xAC-byte-stride records -- indexed by `D_8008EA22`'s low byte for which
pointer, high byte for the record within it) to read two `u16` fields
(`unk74`, `unk76` -- envelope attack/decay levels, each multiplied by 0x81
== 129). Those feed FOUR CASCADED divisions by 127 (each an unsigned
`x/127`, confirmed to compile to retail's exact magic-multiply sequence --
see below), producing `lvl1`/`lvl2` and then `lvl1b`/`lvl2b` from those.
Three successive `if (byte < 0x40) ... else ...` blends (division by 63,
using either the raw control byte or `0x7F` minus it) combine `lvl1b`/`lvl2b`
into a final `pan1`/`pan2` pair, clamped so neither exceeds the other when a
global flag (`D_8008E8C0 == 1`) is set. The rest is the tail this unit's
`vmNoiseOn2` already established byte-for-byte: write `pan1`/`pan2` into
the 16-byte-stride `_svm_sreg_buf`/`D_8008D7F2` tables, OR `3` into
`_svm_sreg_dirty[chan]`, compute a 32-bit voice-enable bit split across
`lowBit`/`highBit` by `chan<16`, reset the whole `D_8008D9A3` 52-byte-stride
table's low bit for every live voice (`D_8008E9D0` of them) then mark this
channel's own slot `2`, OR the enable bits into `D_8008E228`/`_svm_okon2`
and AND-NOT them out of `_svm_okof1`/`D_80090C64`, conditionally OR/AND-NOT
them into a second enable pair (`_svm_orev1`/`_svm_orev2`, gated on
`D_8008EA20 & 4` -- new globals, not touched by `vmNoiseOn2`), and
finally write the bits to the SPU key-on registers via `D_8006DAD4`. Also
patches one field of `D_8006DAD4[0xD5]` (byte offset `0x1AA`) with a 6-bit
delta between `D_8008EA0E` and `D_8008EA1C`, shifted into the high byte.

## The `x/127` and `x/63` magic-multiply idiom -- verified against the pinned pipeline before writing any C

Retail's four `mult`/`mflo`/`lui`/`ori`/`multu`/`mfhi`/`subu`/`srl`/`addu`/`srl`
sequences (divisor 127) and three shorter ones (divisor 63, `srl` by 5
instead of 6) are NOT a hand-rolled division -- confirmed with the
CLAUDE.md-mandated reproducer pipeline, run BEFORE writing the function body:

```c
unsigned int probe(unsigned int a) { return a / 127; }
```

compiles (through `tools/gcc263/cpp` | `cc1 -O2` | `maspsx` with this
project's pinned `MASPSX_FLAGS` | `mipsel-linux-gnu-as`) to exactly

```
lui v0,0x204; ori v0,v0,0x811; multu a0,v0; mfhi v0
subu a0,a0,v0; srl a0,a0,0x1; addu v0,v0,a0; srl v0,v0,0x6
```

byte-for-byte the magic constant (`0x02040811`) and full 8-instruction
correction sequence retail uses. `a / 63` reproduces the same shape with
constant `0x04104105` and a final `srl` by 5. **Plain `x / 127` and
`x / 63` in C, on `u32` operands, is therefore the correct and ONLY
spelling** -- no hand-written `(hi + ((x-hi)>>1))>>N` reconstruction is
needed or wanted; that shape IS what the division compiles to, and writing
the two steps out by hand (tried first, see Attempts) does not reproduce the
same register scheduling.

**Retail interleaves consecutive divisions' instruction streams for
latency-hiding** -- e.g. division 2's `mult`/`mflo`/`multu`/`mfhi` begins
before division 1's `subu`/`srl`/`addu`/`srl` correction finishes, since the
two are data-independent. Writing four independent, natural `x/127` /
`x/63` expressions in source order let GCC 2.6.3's own scheduler do this
interleaving on its own; no manual reordering of the C was needed to get it.

## Struct/global model (all local to this unit)

```c
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} SsScore;
extern SsScore *_ss_score[];   /* array of pointers, indexed by D_8008EA22's low byte;
                                           each points to a 0xAC-stride array indexed by the high byte */

extern u8 D_8008EA16;
extern u8 D_8008EA19;
extern u8 D_8008EA17;
extern u8 D_8008EA11;
extern u8 D_8008EA1A;
extern s16 D_8008E8C0;
extern u16 D_8008EA22;
extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u16 *D_8006DAD4;
extern u8 _svm_sreg_buf[];
extern u8 D_8008D7F2[];
extern u8 _svm_sreg_dirty[];
extern u8 D_8008D98C[];
extern u8 D_8008D9A3[];
extern u8 D_8008E9D0;
extern u16 D_8008E228;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 D_80090C64;
extern u16 _svm_orev1;   /* new -- second enable/mask pair, gated on D_8008EA20 & 4 */
extern u16 _svm_orev2;   /* new */
extern u8 D_8008EA20;    /* new -- flag byte, bit 2 selects _svm_orev1/234 direction */
```

`_ss_score`'s two-level indexing was derived from the raw address
arithmetic, not guessed: `lowbyte*4` (pointer-array stride) selects which
pointer via `_ss_score[lowbyte]`, and the `highbyte*3*4*4 = highbyte*0xAC`
shift-add sequence retail computes by hand (`v1*3 -> <<2 -> -v1 -> <<2 ->
-v1 -> <<2`, i.e. `((v1*3-1)*4-1)*4 = v1*43*4 = v1*172 = v1*0xAC`) is exactly
what indexing an array of a 0xAC-byte struct by `highbyte` produces --
confirmed the shift-add sequence reproduces unchanged when written as plain
`_ss_score[lowbyte][highbyte]` array indexing (no need to hand-code the
multiply).

## Best-derived body (309/311 words, preserved for the next attempt)

```c
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
extern u8 D_8008EA0E;
extern u8 D_8008EA1C;
extern u16 *D_8006DAD4;
extern u8 _svm_sreg_buf[];
extern u8 D_8008D7F2[];
extern u8 _svm_sreg_dirty[];
extern u8 D_8008D98C[];
extern u8 D_8008D9A3[];
extern u8 D_8008E9D0;
extern u16 D_8008E228;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 D_80090C64;
extern u16 _svm_orev1;
extern u16 _svm_orev2;
extern u8 D_8008EA20;

void vmNoiseOn(s32 a0) {
    s32 chanRaw;
    SsScore *e;
    u32 pAttack;
    u32 pDecay;
    volatile u32 lvl1;
    volatile u32 lvl2;
    volatile u32 lvl1b;
    volatile u32 lvl2b;
    u32 pan1;
    u32 pan2;
    s32 chan;
    s32 off16;
    s32 idx52;
    s32 i;
    s32 lowBit;
    s32 highBit;
    u16 e228;
    u16 e22c;
    u16 c60;
    u16 c64;

    chanRaw = a0;
    e = &_ss_score[D_8008EA22 & 0xFF][D_8008EA22 >> 8];

    pAttack = e->unk74 * 0x81;
    pDecay = e->unk76 * 0x81;
    lvl1 = (pAttack * D_8008EA16) / 127;
    lvl2 = (pDecay * D_8008EA16) / 127;
    lvl1b = (lvl1 * D_8008EA19) / 127;
    lvl2b = (lvl2 * D_8008EA19) / 127;

    if ((u8)D_8008EA1A < 0x40) {
        pan1 = lvl1b;
        pan2 = (lvl2b * D_8008EA1A) / 63;
    } else {
        pan2 = lvl2b;
        pan1 = (lvl1b * (0x7F - D_8008EA1A)) / 63;
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

    chan = (u8)chanRaw;
    D_8006DAD4[0xD5] = (u16)((D_8006DAD4[0xD5] & 0xC0FF) | (((D_8008EA0E - D_8008EA1C) & 0x3F) << 8));

    off16 = chan << 4;
    *(u16 *)(D_8008D7F2 + off16) = pan2;
    *(u16 *)(_svm_sreg_buf + off16) = pan1;
    _svm_sreg_dirty[chan] |= 3;

    if (chan < 16) {
        lowBit = 1 << chan;
        highBit = 0;
    } else {
        lowBit = 0;
        highBit = 1 << (chan - 16);
    }

    idx52 = (u8)chanRaw * 52;
    *(u16 *)(D_8008D98C + idx52) = 10;
    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            idx52 = i * 52;
            *(u8 *)(D_8008D9A3 + idx52) = *(u8 *)(D_8008D9A3 + idx52) & 1;
            i++;
        } while (i < D_8008E9D0);
    }
    idx52 = (u8)chanRaw * 52;
    *(u8 *)(D_8008D9A3 + idx52) = 2;

    e228 = D_8008E228;
    e22c = _svm_okon2;
    c60 = _svm_okof1;
    e228 = lowBit | e228;
    e22c = highBit | e22c;
    D_8008E228 = e228;
    c60 = c60 & ~e228;
    _svm_okon2 = e22c;
    c64 = D_80090C64;
    _svm_okof1 = c60;
    c64 = c64 & ~e22c;
    D_80090C64 = c64;

    if (D_8008EA20 & 4) {
        _svm_orev1 |= lowBit;
        _svm_orev2 |= highBit;
    } else {
        _svm_orev1 &= ~lowBit;
        _svm_orev2 &= ~highBit;
    }

    D_8006DAD4[0xCA] = lowBit;
    D_8006DAD4[0xCB] = highBit;
}
```

## Residues

1. **The opening register-identity swap (unfixable class, per this unit's
   three prior reports on the same shape).** Retail computes the masked
   channel byte directly into its working register and defers the raw-value
   preserve by one instruction; every derivation here (and in
   `vmNoiseOn2`/`SePitchBend`) puts the preserve first. Per HARD RULE 6
   and this unit's own established finding ("variable identifier choice has
   no influence on register assignment" -- `vmNoiseOn2`'s report), not
   re-attempted as a distinct lever here; it is the same wall.
2. **A 2-word stack-frame-size gap, now well isolated but not closed.**
   Without ANY `volatile`, the function compiles to 295/311 (16 short) and
   drops the correction (`subu`/`srl`/`addu`) sequence entirely from ONE of
   the four `/127` divisions (GCC's range analysis apparently proves the
   simple one-step magic-multiply is already exact for that specific
   dividend's provable range, since it's the product of two earlier-divided,
   narrower values) -- a REAL codegen difference, not a rename. Marking
   `lvl1`/`lvl2`/`lvl1b`/`lvl2b` all `volatile` forces all four to spill to
   memory and reproduces every correction sequence, landing at 309/311 with
   an `addiu sp,sp,-0x10` (16-byte) frame; retail has `addiu sp,sp,-0x8`
   (8-byte). **Every 2-variable subset tried (lvl1+lvl2 only: 304/311;
   lvl1b+lvl2b only: 303/311; lvl1+lvl1b: 303/311; lvl2+lvl2b: 305/311;
   pAttack+pDecay: 304/311) scored WORSE than all four together (309/311)**
   -- retail's actual 8-byte frame almost certainly spills a DIFFERENT pair
   than any tried here, or spills something not yet identified as a
   candidate at all (e.g. an intermediate product, or `pan1`/`pan2`
   themselves under a different qualifier -- `volatile` on `pan1`/`pan2`
   was tried and regressed hard, to 340/311, so it is not simply "more
   volatiles = better"). Untried: `register`-adjacent tricks are banned by
   HARD RULE 6 if they'd pin identity, but a plain non-volatile local that
   GCC chooses to spill on its own (rather than being forced to) has not
   been searched for by varying which EXPRESSION computes each value (e.g.
   introducing the `SsScore` read as two separate loads instead of
   one `e->unk74`/`e->unk76` pair, on the chance that changes which two
   temporaries end up register-starved enough to spill naturally).

## Attempts

Within the 30-attempt cap (roughly 12 real builds used):
1. Full transcription per m2c's decoded expression tree (m2c's own
   `/ 127`-recognizing pattern matcher was a useful STARTING hint but its
   literal two-step `(hi + ((x-hi)>>1))>>6` phrasing, transcribed verbatim,
   is NOT needed -- verified in isolation via the pinned-pipeline reproducer
   above that plain `x/127`/`x/63` alone produces the identical instructions;
   the hand-written two-step form was tried first and is unnecessary
   complexity, simplified away).
2. A scheduling barrier (`__asm__("")`) between the third and fourth
   division: **regressed** (295 -> 294 words). Reverted.
3. Swapping computation order of `lvl1b`/`lvl2b`: **no change** (still 295).
   Reverted to natural order.
4. `volatile` on `lvl1`/`lvl2` only: 295 -> **304/311**.
5. `volatile` added to `lvl1b`/`lvl2b` as well (all four): 304 -> **309/311**,
   this report's best. `addiu sp,sp,-0x10` vs retail's `-0x8`.
6. The "narrow-cast defeats loop-strength-reduction" idiom (already
   validated in this unit for `vmNoiseOn2`/`SpuVmAlloc`) applied to
   this function's OWN `D_8008D9A3`-clearing loop (`idx52 = (s16)i * 52`,
   `while ((s16)i < D_8008E9D0)`): **regressed hard**, to 320/311 -- this
   loop is NOT the same shape as `vmNoiseOn2`'s (retail already recomputes
   the shift-add fresh per iteration WITHOUT any narrowing cast needed here,
   confirmed by re-reading the raw `.s`: `sll v1,a1,0x10`/`sra v1,v1,0x10` in
   retail is sign-extending the LOOP COUNTER for the COMPARISON only, not
   gating strength-reduction of the multiply) -- reverted. **Proposed
   learning below.**
7. Various 2-of-4 `volatile` subsets on `lvl1`/`lvl2`/`lvl1b`/`lvl2b`
   (6 combinations): all scored between 303 and 305, all worse than "all
   four" (309). Reverted to all four.
8. `volatile` on `pan1`/`pan2` (in addition to all four `lvl*` already
   volatile): **regressed sharply**, to 340/311. Reverted -- `volatile` is
   not a "the more the better" lever; it must match the SPECIFIC pair
   retail's own register allocator chose to spill, and guessing wrong costs
   more than not applying it at all.
9. `volatile` on `pAttack`/`pDecay` only (not the `lvl*` values): 295 ->
   **304/311**, worse than the `lvl*`-based 309. Reverted.

### Proposed learnings

- **The unit's own established "narrow-cast defeats strength-reduction"
  idiom is NOT a blanket fix for every `D_8008E9D0`-bounded loop over
  52-byte-stride tables in this unit** -- it fixed `vmNoiseOn2`'s loop
  (10-word swing) but actively regressed this function's structurally
  similar-looking loop by 25 words. The two loops differ in whether the
  loop-carried index is ALSO used for anything besides the multiply
  (`vmNoiseOn2`'s is not; this one's `i` is compared with a narrower type
  than it's declared, changing which sign-extension the compiler already
  emits for a DIFFERENT reason). Check the raw `.s` for what the narrowing
  instructions are actually FOR (comparison vs. multiply-strength-reduction)
  before assuming the idiom transfers.
- **`volatile` as an anti-optimization lever does not compose linearly or
  monotonically** -- four correctly-chosen `volatile`s (309) beat every
  2-of-4 subset (303-305) AND beat zero `volatile`s (295), but adding TWO
  MORE `volatile`s beyond those four (`pan1`/`pan2`) made it dramatically
  worse (340). The right set is a discrete, non-obvious combination tied to
  retail's own specific 8-byte frame, not a monotonic dial.
- **GCC 2.6.3 can prove one of several structurally-identical constant
  divisions doesn't need its own general correction sequence, based on the
  provable value range of a narrower-than-worst-case dividend** -- this is a
  new instance of "the compiler proves a cheaper equivalent" (the family
  CLAUDE.md's `note2pitch2` sra/srl lever and this unit's
  loop-strength-reduction lever both belong to), on CONSTANT DIVISION rather
  than a loop induction variable or a right-shift. Worth watching for
  elsewhere in this unit/project: any `/N` division whose dividend is itself
  the output of an earlier division by the same or a related constant is a
  candidate for this range-based correction-elision.

## Round 45 update: the permuter lever round 44 proposed was run -- NEGATIVE result, and the isolated scaffold is not trustworthy for this function

Round 44's disposition proposed a permuter run targeting the frame-size/
spill-pair question as the next concrete lever. It was run this round;
the result is a clean negative, plus a diagnostic finding about why:

**Before searching, `--debug --stack-diffs` was checked per this round's own
runner instructions** (`tools/setup-permuter.sh vmNoiseOn <seed with the
309/311 body>`, then
`PATH=permuter-work/bin:$PATH .venv/bin/python3 tools/decomp-permuter/permuter.py --debug --stack-diffs permuter-work/vmNoiseOn`).
The penalty list it printed:

```
Stack Differences:             0  (1)
Branch Differences:            0  (1)
Register Differences:          156  (5)
Reorderings:                   12  (60)
Insertions:                    49  (100)
Deletions:                     51  (100)
```

**This does not match the real oracle's 309/311 figure at all.** Insertions
and deletions near 50 each (out of ~155 instructions) describe a function
roughly HALF structurally different, not a 2-word-short near-miss. The
isolated scaffold's own `--debug` dump shows large blocks of the function's
TAIL (the `D_8008E228`/`22C`/`80090C60`/`64`/`8008E230`/`234` enable-bit
section) diverging in ways the real in-unit build does not -- confirmed by
rebuilding the exact same 309/311 body in `src/code_179d8_l.c` and reading
`tools/asm-differ/diff.py vmNoiseOn` directly: the realigned diff shows
the SAME single 2-word gap (isolated to the opening register swap plus the
frame size, exactly as this report already documents) with NO large
tail-section divergence anywhere. **The isolated permuter scaffold and the
real translation-unit build produce measurably different codegen for the
IDENTICAL source function** -- most likely because GCC's register allocator
sees a different set of live symbols/declarations in the single-function
scaffold file than in the real multi-function unit, which changes register
pressure and therefore allocation choices even though the target function's
own text is unchanged.

**A bounded search was still run (600s, `-j 4`, `--stop-on-zero`) to test
whether it would move despite the scaffold mismatch**: base score 11355 (NOT
a small number -- consistent with the scaffold's own bad `--debug` reading,
not with a 2-word-short function), 51435 iterations in the bound, best score
reached 8465. **No zero, not remotely close, confirming the assignment's own
prediction from the debug penalty counts** (high insertion/deletion counts
predict a long, fruitless search). `permuter rc=124` (bound fired, not a
crash). Posted to the broadcast channel as a negative result.

**Disposition of the lever**: do not re-run the permuter on this function
again without first fixing the scaffold-context mismatch (e.g. by including
more of the real unit's surrounding declarations in the seed file, or
comparing register pressure directly) -- searching against a base that
scores 11355 when the real function is 2 words from byte-exact is
searching the wrong landscape. This is itself worth recording as a general
caution: **`--debug --stack-diffs`' base score is only informative when it
is checked AGAINST the real in-unit build's own score first** (which this
round did do, per the runner instructions, and which is what caught the
mismatch before burning a longer search on it).

## Disposition

**Restored to `INCLUDE_ASM`** (no score short of byte-exact stays in
`src/`). Round 44 reached real, measured, FIRST-EVER progress on this
function -- 0 -> 309/311, isolated to a single, well-characterized 2-word
stack-frame gap (plus the unit's already-known unmovable opening register
swap). Round 45 ran the permuter lever round 44 proposed and got a clean
NEGATIVE, plus a concrete diagnosis: the isolated scaffold does not
reproduce the real build's codegen for this function, so any permuter score
measured against it is not informative until that mismatch is fixed. The
next attempt should NOT re-run the permuter as-is; either (a) diagnose and
fix the scaffold/real-build codegen mismatch first (compare declaration
order/content between the seed and the real unit), or (b) go back to
searching for the actual retail-matching spill pair by hand (varying which
EXPRESSION computes each intermediate, per residue 2's still-untried
"introduce the `SsScore` read as two separate loads" idea), which
remains the most concrete un-tried lever that doesn't depend on the
permuter at all.

## Naming (round 75, runner alpha, FINISHING-PLAN track 3)

Already carries its real name: identified round 74 (track 2, runner bravo)
as `libsnd/vmanager vmNoiseOn` (shape 0.98 vs the disc-3.3 reference, 308w
reference vs our 311w; also cross-checked against the public symbol
`SpuVmNoiseOn` at 0x8002F368 elsewhere in the image). Sony symbol; this
pass does not rename it further. The function remains a STALL (2 words
short, see above).

## Track 2 (round 86, 2026-09-26, alpha)

This function is still `INCLUDE_ASM` and its C was not touched, but the per-field symbols this report uses (`D_8008D988`..`D_8008D9BA` at a 0x34 stride) are ONE Sony table: libsnd/vmanager.o's `_svm_voice` (0x8008D988, 24 x 0x34 = 0x4E0 bytes), typed in `include/SvmData.h` with fields by offset (`D_8008D98C` is `_svm_voice[i].unk04`, `D_8008D9A3` is `unk1B`, and so on: address minus 0x8008D988). The next attempt should write `_svm_voice[i].unkNN`: in every converted accessor (libsnd_vm_vol_ut_key_ut_keyv/j_c/l/m/p) the struct spelling compiled byte-identically to the separate symbols, and two NON_MATCHING bodies moved closer to retail. The other `D_` spellings in preserved bodies below still link (splat keeps them as auto-symbols).
