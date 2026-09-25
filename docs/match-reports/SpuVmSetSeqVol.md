# SpuVmSetSeqVol -- STALL (length EXACT at 96/96 words; 89/96 raw word-match; first real diff at word 18 / retail 0x8003044C -- the `entry` pointer's register, RECLASSIFIED round 57: reachable from source shape, but the correct program is then 97 words because the table base takes `$a1` from parameter `p1`)

> Renamed from `func_80030404` on 2026-09-23 (tools/rename.py). Address 0x80030404.

**ROUND 57 CORRECTION -- READ THIS BEFORE THE PARAGRAPH BELOW.** The
"measurement note" that follows is a ROUND 23 paragraph describing the
round-23 body (91 words, 5 short, 21/96 raw, first real diff at word 0).
Round 56 replaced that body and the TITLE above is the current figure;
round 57 rebuilt round 56's body in-tree and reproduced it exactly --
**length EXACT at 96 words** (`build/lsdde.map` puts `SpuVmGetSeqVol` at
its retail `0x80030584`, zero drift), **89/96 raw**, first real diff at
word 18 / `0x8003044C`. Every figure in the next paragraph and in the
three sections after it is superseded; they are kept because their
NEGATIVES are still valid against the body they were measured on. Do not
quote them as this function's state.

**Measurement note (round 23 body; superseded -- see the correction above):** compiled LENGTH is 91 words against retail's 96 words
computed word-for-word length. **This is 5 words short, not 3** --
verified directly: `objdump` on the built `.o` shows this function
spanning file offsets `0x128`-`0x290` inclusive of its trailing `nop`,
i.e. `(0x294 - 0x128) / 4 = 91` words, against retail's `96` (`0x180`
bytes / 4). The raw hex word-match count is 21/96. `tools/asm-differ/diff.py`
confirms the first REAL divergence is at word 0 -- retail's very first
instruction is `andi v0,a0,0xff`, this build's is an inserted `move
t0,a0` -- so unlike this unit's other three stalls, this one is NOT
"mostly a shift from one later defect": the early section genuinely
diverges from the start, and the missing-length gap is a separate
symptom of the same early register-allocation difference, not an
independent thing to hunt for later in the function.

Unit `code_179d8_j`, round 23 (2026-09-07). Not a class method. Looks up
`D_800902E8[screen][slot]` (screen = low byte of `p0`, slot = high byte),
writes/clamps two fields of the found `Entry90902E8` record, computes two
`0x81`-scaled values from `p1`/`p2`, and -- gated on `p3 == 1` -- runs this
unit's now-familiar "scan `D_8008D996` for a 16-bit key, set two fields of a
16-byte-stride table plus an OR'd flag byte" loop (same idiom as
`SpuVmSeqKeyOff`, including the `(u8)` index-mask lever that function's
report documents for defeating strength reduction, applied here too).

## What it is (best-reached body: 91/96 words compiled -- 5 words short -- 21/96 raw match, first real diff at word 0)

```c
#if 0
s32 SpuVmSetSeqVol(s32 p0, s16 p1, s16 p2, s16 p3)
{
    s32 dead;
    Entry90902E8 *tbl = D_800902E8[(u8) p0];
    Entry90902E8 *entry;
    u8 recIdx;
    s16 key;
    u16 t1v, t0v;
    u32 i;

    D_8008EA22 = p0;
    recIdx = (p0 & 0xFF00) >> 8;
    entry = &tbl[recIdx];
    key = (s16) p0;

    entry->unk74 = p1;
    if ((u16) entry->unk74 >= 0x80)
        entry->unk74 = 0x7F;
    entry->unk76 = p2;
    if ((u16) entry->unk76 >= 0x80)
        entry->unk76 = 0x7F;

    t1v = (u16) p1 * 0x81;
    t0v = (u16) p2 * 0x81;

    if ((s16) p3 == 1) {
        for (i = 0; i < D_8008E9D0; i++) {
            if (D_8008D996[(u8) i].unk0 == key) {
                D_8008D7F0[i].unk0 = t1v;
                D_8008D7F0[i].unk2 = t0v;
                D_8008D970[i] |= 3;
            }
        }
    }
    if (0) {
        dead = 1;
    }
    return (s16) D_8008EA22;
}
#endif
```

Reuses `Entry90902E8` (this unit's top-of-file typedef), and the
shared globals/typedefs `D_8008EA22`, `D_8008E9D0`, `D_8008D996`
(`Rec34D994`), `D_8008D7F0` (`Rec16D7F0`), `D_8008D970`.

## One CLOSED finding

**The empty stack frame is the SAME dead-local idiom as `SsUtSetDetVVol`,
and a SCALAR dead local (not an array) is enough here.** This function
makes no calls, so retail's `addiu $sp,$sp,-8`/`+8` around the whole body,
with nothing ever read from or written to that frame, needed the
established "genuinely unreachable write to a dead local" trick
(`if (0) { dead = 1; }`). A plain scalar `s32 dead;` reproduced the exact
8-byte frame; the two-element array form used elsewhere in this unit
(`s32 dead[2];`) was tried first, produced the SAME result (no
difference from the scalar), so either spelling works for an 8-byte
target -- worth knowing that the array form isn't load-bearing beyond
being "at least one word".

## The unresolved residue: early-section register handling

The mismatch (21/96 raw match, compiled length 5 words short even after
the frame is fixed, first real diff at word 0 per `asm-differ`) starts at
the FIRST instruction and runs through the `D_800902E8` lookup, the
`D_8008EA22 = p0` store, and the `recIdx`/`key` extraction from `p0`.
Retail's actual shape, read directly off the ROM:

```
andi  v0, a0, 0xff        ; byte index for the table-of-pointers lookup
sll   v0, v0, 2
lui   at, %hi(D_800902E8)
addiu at, at, %lo(D_800902E8)
addu  at, at, v0
lw    t0, 0(at)           ; t0 = tbl (a0/p0 untouched throughout this)
addiu sp, sp, -8          ; frame instruction scheduled HERE, mid-sequence
lui   at, %hi(D_8008EA22)
sh    a0, %lo(D_8008EA22)(at)   ; D_8008EA22 = p0, still from the ORIGINAL a0
sll   a0, a0, 0x10        ; a0 = p0 << 16 (destroys a0, now safe -- last real use of p0 was the line above)
srl   v1, a0, 0x18        ; recIdx = (shifted a0) >>> 24  == (p0>>8)&0xff
...
sra   a0, a0, 0x10        ; key = (shifted a0) >> 16 (arithmetic)  -- REUSES the same shifted a0
```

Retail computes `recIdx` and `key` from a SINGLE shared intermediate
(`p0 << 16`, computed once), extracting one via `srl`/24 and the other via
a later `sra`/16 on that same value -- and it does this only AFTER the
table lookup and the `D_8008EA22` store, keeping the ORIGINAL, unshifted
`p0` alive in `$a0` across both of those (no register copy needed, since
nothing else needs `$a0` in the meantime).

This build's compiled output instead inserts an extra `move $t0, $a0` at
the very top (copying `p0` before doing anything else), computes `recIdx`
and `key` as two textually-independent expressions rather than from one
shared shifted value, and schedules the `D_8008EA22` store to a
DIFFERENT point in the sequence (after the table lookup, not
interleaved with it the same way). Net effect: the same VALUES are
computed, but through a different register-allocation strategy, 3
instructions longer.

Tried and rejected, all zero-effect (byte-identical compiled output to
the baseline above):

- Reordering `key = (s16) p0;` to sit immediately after `recIdx = ...;`
  (before building the `entry` pointer), matching retail's textual
  adjacency of the two extractions -- no effect.
- Collapsing the separate `tbl`/`recIdx` locals into one expression,
  `entry = &D_800902E8[(u8) p0][(p0 & 0xFF00) >> 8];`, computed before
  the `D_8008EA22` store -- no effect on the residue's shape (same
  extra `move`, same reordered store), though it is arguably cleaner
  source.

Both of these are the SAME class of finding as `SpuVmSetSeqVol`'s siblings
in this unit (`SsUtKeyOffV`, `SpuVmSeqKeyOff`): a value provably
derivable from an already-live register gets recomputed via a fresh
extraction/copy instead of reusing the exact bit-manipulation sequence
retail's compiler happened to choose, and no legal source reshaping tried
this round changed which register-allocation strategy GCC picked.

## Axes tried (for the next attempt)

- Dead-local frame idiom: scalar vs. 2-element array -- both close the
  frame gap identically; not the source of the remaining residue.
  **Closed.**
- Statement order for `recIdx`/`key`/`entry` (three orderings tried,
  including collapsing into one expression) -- **no effect** on the
  early-section register strategy in any case.

**Tried and REJECTED (regressed sharply):** spelling out retail's bit
trick explicitly, `s32 shifted = p0 << 16; recIdx = (u32) shifted >> 24;
key = shifted >> 16;`, in place of the more natural `(p0 & 0xFF00) >> 8` /
`(s16) p0` forms. This was expected to persuade GCC to reuse one shared
intermediate the way retail's compiler did; instead it collapsed the
score to 2/96 -- introducing an explicit 32-bit shifted local evidently
changes register pressure/liveness for the WHOLE early section far more
than it fixes the one targeted extraction, making this the single worst
variant tried. **Do not retry this spelling without a materially
different supporting change** -- it is not a small perturbation on this
function, it is a large one.

**Untested axis:** given the explicit-shift spelling backfired, the
remaining plausible lever is register-pressure-side rather than
expression-side -- e.g. deliberately extending or shortening `p0`'s live
range with an unrelated dummy use, the same class of lever
`SsUtSetDetVVol`'s report flagged as untested for its own delay-slot
residue. Not attempted this round for lack of remaining budget.

### Proposed learning

**A 5-word-short function with widely scattered (not clustered)
mismatches in its FIRST ~25% is often one register-allocation decision
in the very first few statements, not several independent residues** --
but that does not mean the fix is cheap. Every diff word in this
function's first quarter traces back to the single "compute `recIdx` and
`key` from one shared shifted intermediate, computed after the table
lookup and the global store, reusing `$a0` throughout" shape, yet BOTH
directions tried -- reordering/collapsing the natural expressions
(no effect) and writing out the shared intermediate literally
(sharply worse) -- failed to reach it. Read as one data point alongside
this unit's other stalls: GCC 2.6.3's choice of which shifted/masked
form of a value to materialize, and how long to keep it live, resists
being steered by the SHAPE of the source expression computing the same
value, in both the "too implicit" and "too explicit" directions.

**Second: "N words short" (a length count) and "M/N words match" (a raw
hex-equality count) measure different things, and this report's own first
draft got the FIRST one wrong (said 3, was actually 5) by eyeballing the
funcdiff hex dump instead of directly counting the built object's
instructions with `objdump`. Since this function's residue starts at word
0 (confirmed with `tools/asm-differ/diff.py`), the length gap here is not
even a separate question from the raw-match count the way it is for this
unit's other three stalls -- but the LENGTH number itself still needs to
be counted from the object file, not inferred from where funcdiff's hex
diff happens to stop being interesting.

## ROUND 31 (runner delta): rebuild confirms both title figures exactly

Rebuilt the preserved body verbatim through the current pinned pipeline.
`build/lsdde.map` puts the next function, `SpuVmGetSeqVol`, at built
address `0x80030570` against its own retail address `0x80030584` -- a
20-byte (5-word) deficit, confirming **91/96 words, 5 SHORT**, exactly as
this report's (already-corrected) title states. `funcdiff.py`'s raw count
reproduces **21/96** with the drift warning firing as expected for a
length-mismatched function. No new reshape attempted this round; the
register-pressure axis this report flags as the only remaining untested
lever was not reached given the per-function time budget, and the
explicit-shift spelling that regressed sharply to 2/96 in round 23 is not
worth re-trying without a materially different supporting change (per this
report's own warning). Restored to `INCLUDE_ASM`; still a STALL.

## ROUND 32 (runner alpha): rebuild confirms both figures exactly; sibling-derived barrier axis tried and rejected

Rebuilt the preserved body verbatim through the current pinned pipeline.
`build/lsdde.map` confirms `SpuVmGetSeqVol` still lands 20 bytes (5 words)
after its own retail address, and `tools/funcdiff.py` reproduces **21/96
raw match with the expected out-of-range drift warning** -- both figures
match this report's title exactly.

Noticed this unit's ALREADY-MATCHED siblings `SpuVmGetSeqLVol` and
`SpuVmGetSeqRVol` (both accessing the same `D_800902E8` table, both storing
to `D_8008EA22` from their own `p0`) place a bare `__asm__("");` barrier
immediately before their `D_8008EA22 = ...` store, after computing their
table pointer/index locals -- and this function's preserved body has that
same store in the same relative position (after the `tbl` lookup) but with
no barrier. Evidence-backed (from a matched sibling in the same file), so
tried adding the identical barrier immediately before `D_8008EA22 = p0;`.

**Result: regressed to 16/96 (from 21/96), with MORE out-of-range drift**
(confirmed via `tools/funcdiff.py`'s drift warning firing both before and
after, size unchanged at ~286KB but the in-range match got worse, not
better) -- rejected and reverted immediately. Unlike `SpuVmGetSeqLVol`/
`SpuVmGetSeqRVol` (both simple single-field getters/setters with nothing
else live across the barrier), this function has FIVE more live locals
(`entry`, `recIdx`, `key`, plus the two `t1v`/`t0v` products computed later)
whose liveness the barrier's scheduling boundary interacts with -- the
sibling idiom does not transfer to a function with substantially higher
register pressure at that point, the same lesson `SsUtKeyOffV`'s round-32
addendum draws about `SsUtKeyOff`'s type-asymmetry idiom not
transferring either.

Restored to the preserved body (no barrier) exactly as documented;
`git status --porcelain` empty; whole-image build re-verified green.
Still a STALL, same early-register-allocation-strategy classification
as before; the "register-pressure-side" axis this report already flags as
untested remains untested (a barrier insertion is a scheduling lever, not
a register-pressure one, so this round's negative result does not close
that axis, it just rules out one more specific placement).

### Proposed learning

**A scheduling barrier idiom that is load-bearing for a matched sibling
with LOW register pressure at the barrier's point does not transfer to a
sibling with HIGHER register pressure at the analogous point, even when
the store being guarded is textually identical (`D_8008EA22 = p0;`).**
This is the third round-32 instance of "a fix that works on one function in
this unit does not transfer to a structurally similar one" (see
`SsUtKeyOffV`'s and this same addendum's own sibling-derived-axis
findings) -- worth treating as a standing caution for this unit
specifically: shared idioms need re-verification per function, not
adoption by analogy, and checking the DONOR function's register pressure
(how many locals are live across the barrier) before assuming its barrier
placement will transfer.

## Round 47 (2026-09-16), runner delta -- rebuilt in-tree (third confirmation), then permuter DECLINED on check (b)

**Rebuild-before-trusting-the-score, third time.** Spliced the preserved
body into `src/code_179d8_j.c` (local reduced-view declarations for
`D_8008E9D0`, `D_8008D996`, `D_8008D7F0`, `D_8008D970`, reusing this file's
own already-declared `Entry90902E8`/`D_8008EA22`) and ran the real oracle:
`build exit=2`, no compile-error grep hits, `funcdiff.py` reproduces
**21/96 raw word-match with the expected out-of-range drift warning**.
`build/lsdde.map` (`SpuVmGetSeqVol - SpuVmSetSeqVol = 0x16C` = 91 words)
confirms **91/96, 5 words short**, exactly this report's title. Restored
to `INCLUDE_ASM` immediately after (diff against the pre-splice copy:
byte-identical); `./build-and-verify.sh` confirms `OK: build matches
retail SLPS_015.56`.

**Permuter pre-checks -- (a) and (b) run, (c) satisfied by the rebuild
above:**

- **(a) scaffold compiles and scores:** yes.
- **(b) insertion/deletion penalties, `--debug --stack-diffs`:** **NOT**
  near 0/0 -- the FARTHEST from it of all four of this round's assigned
  near-misses. Measured: `Insertions: 17 (100)`, `Deletions: 22 (100)`,
  `Reorderings: 4 (60)`, `Register Differences: 42 (5)`, `Stack
  Differences: 0 (1)`, **base score = 4350**. Zero stack differences (the
  dead-local frame trick this report already closed is holding), but an
  insertion/deletion sum of 39 on a 96-word function -- roughly 40% of the
  function's own length -- is not a residue a source-mutation search
  operates on; it reflects the SAME single early-section
  register-allocation-strategy difference this report's own body already
  identifies (retail derives `recIdx` and `key` from one shared `p0<<16`
  intermediate computed after the table lookup; every attempt here
  computes them as two independent expressions), which the permuter's own
  debug diff shows rippling through nearly the whole function rather than
  staying local to the first few words.
- **(c) scaffold-vs-real-build agreement:** the in-tree rebuild above
  (91/96, 21/96 raw, matching this report's title exactly) and the
  permuter's debug diff read the identical scaffold body, so there is no
  scaffold/real-build disagreement to flag.

**Verdict: search DECLINED.** This report already tried the two natural
directions on the early-section residue (reordering/collapsing the
extractions: no effect; writing out the shared `p0<<16` intermediate
explicitly: regressed sharply to 2/96) and concluded the lever is
register-pressure-side, not expression-side. The permuter's insertion/
deletion counts here (17/22, by far the largest of this round's four
near-misses) confirm that reading quantitatively: closing this gap needs
a different REGISTER-ALLOCATION OUTCOME for the whole early section, which
a source-mutation search over the current seed's expression tree is not
positioned to discover by trying textual variants of the same few
statements. Recorded as NOT SEARCHED (declined on evidence), not as a
spent, failed search. The "register-pressure-side" axis (extending/
shortening `p0`'s live range with a deliberate dummy use) remains the
right untested lever for a future hand attempt.


## Round 56 (2026-09-19), runner bravo -- the loop counter was TYPED WRONG for 33 rounds: 5-short/21-96 -> length EXACT/89-96

**The inherited body's biggest single error is its loop counter, and no
amount of reshaping the early section was ever going to fix it.** Every
previous round (23, 31, 32, 47) treated this function's scan loop as
`u32 i` with `(u8) i` index masking -- copied by analogy from its sibling
`SpuVmSeqKeyOff`, whose loop really is byte-masked. Read off the ROM, THIS
loop's counter is a **`s16`**:

```
    /* 20CD4 */  sll   $v0, $a2, 16        ; the raw 32-bit counter in a2 ...
  .L800304D8:
    /* 20CD8 */  sra   $a1, $v0, 16        ; ... sign-extended to 16 bits, every iteration
    ...
    /* 20D54 */  sll   $v0, $v0, 16
    /* 20D60 */  sra   $v0, $v0, 16
    /* 20D64 */  slt   $v0, $v0, $v1       ; SIGNED compare against D_8008E9D0
```

`sll 16`/`sra 16` is a sign-extension, and `slt` is the SIGNED compare -- a
`(u8)`-masked counter gives `andi 0xff` and `sltu`, which is exactly what
`SpuVmSeqKeyOff` has four hundred bytes later and what this function does
NOT. With `s16 i` and a plain `for (i = 0; i < D_8008E9D0; i++)`, GCC 2.6.3's
loop inversion emits retail's guard (`lbu`/`beqz`) and loop test verbatim.

That one correction plus six levers took the function from **5 words short /
21 of 96 matching** to **exactly 96 words with 89 of them byte-identical**.
The whole remaining residue is a single register-identity difference.

### The six levers, in the order they were found (each measured on the real oracle)

1. **`s16` loop counter** (above). Reproduces the guard, the loop test, the
   0x34-stride index and the `D_8008D970` index.
2. **The dead-local frame idiom needs the ARRAY spelling here.** This
   report's round-23 section closed the empty `addiu sp,sp,-8` frame with
   `s32 dead; if (0) { dead = 1; }` and recorded that the scalar and the
   two-element array forms were interchangeable. **On the round-56 body they
   are not**: the scalar produces NO frame at all, `s32 dead[2];` (with
   `dead[0] = 1;`) produces retail's exact 8-byte frame, and so does
   `volatile s32 dead;`. The array form is load-bearing, not cosmetic.
3. **The explicit shared `p0 << 16` intermediate, which round 23 BLACKLISTED.**
   That section says the spelling `s32 shifted = p0 << 16; recIdx = (u32)
   shifted >> 24; key = shifted >> 16;` "collapsed the score to 2/96" and
   "do NOT retry this spelling without a materially different supporting
   change". With levers 1 and 2 in place it is the single best spelling
   measured -- it reproduces retail's `sll a0,a0,0x10` / `srl v1,a0,0x18` /
   ... / `sra a0,a0,0x10` block byte-for-byte, where `(u8)(p0 >> 8)`,
   `(p0 & 0xFF00) >> 8` and `((u16) p0) >> 8` all fail. The round-23
   warning was honoured (the supporting change is material) and is now
   RETRACTED.
4. **A bare `__asm__("")` immediately AFTER the `D_8008EA22 = p0;` store**
   stops GCC sinking the `D_800902E8` table load past the shift block, giving
   retail's "lookup first, store second, shift third" order. Round 32 tried
   this barrier BEFORE that store (copying matched siblings
   `SpuVmGetSeqLVol`/`SpuVmGetSeqRVol`) and regressed 21/96 -> 16/96; after is
   right and before is still wrong (2145 vs 2085 permuter units on the
   round-56 body). Dropping the `tbl` local and writing
   `entry = D_800902E8[(u8) p0]; ... entry += (u32) shifted >> 24;` is worth
   a further step -- a separate `tbl` local costs a `move` and regresses.
5. **Two `u16 *` pointer locals for `D_8008D7F0`, declared INSIDE the loop
   body.** Retail hoists `&D_8008D7F0` into `a3` and `&D_8008D7F0 + 2` into
   `t2` before the loop and adds one shared byte offset to both
   (`addu v1,v0,a3` / `addu v0,v0,t2`); the struct-array spelling
   `D_8008D7F0[i].unk0`/`.unk2` instead re-materialises `lui`/`%lo` per
   access with a single base and a `2(...)` displacement. Two pointers fix
   the two-base shape -- **and WHERE they are declared decides where the
   preheader ends up**: assigned before the `for`, the `lui`/`addiu` pair is
   emitted BEFORE the `D_8008E9D0 != 0` guard; declared inside the loop body
   (or inside the `if`), loop-invariant motion hoists it into the preheader
   AFTER the guard, which is retail. That placement alone is worth 1085 ->
   610 permuter units. This is the same pointer-hoist lever round 56 found on
   `SpuVmSeqKeyOff`, applied to a second function in the same unit.
6. **The `D_8008D7F0` index is a `s16`, not an `int`.** Retail's byte offset
   is `sll v0,a1,0x13` / `sra v0,v0,0xf` -- a sign-extension from 16 bits
   FUSED with the scale, i.e. `(s16)(i * 8)` used as a `u16 *` index, not
   `(int) i * 8` (which is the single `sll v0,a1,0x4` the build emitted).
   Writing `s16 off; off = i * 8; slotLo[off] = ...;` closes it. **This is
   the largest single step in the whole round: 420 -> 110 permuter units,
   44/96 -> 86/96 raw**, and it is what makes the length exact.
7. **A second bare `__asm__("")` before the two `0x81` products** puts
   `andi v1,a1,0xffff` after the second clamp's join instead of into its
   branch-delay slot: 86/96 -> **89/96**.

(Also: `entry->unk76 = p2;` belongs immediately after `entry->unk74 = p1;`,
before BOTH clamps, not between them -- that is what frees the first clamp's
delay slot for the `sh a2,0x76` the way retail has it, and it moved 610 ->
420 units.)

### The best body (89/96, length exactly 96 words, zero drift)

```c
#if 0
/* ---- SpuVmSetSeqVol local reduced view (round 56) ---- */
extern u8 D_8008E9D0;

/* 0x34-byte-stride channel-configuration record; only the leading s16 this
 * function reads is named (same family as code_179d8_j_b.c's Rec34D994). */
typedef struct {
    s16 unk0;
    u8 pad2[0x34 - 0x2];
} Rec34D994;
extern Rec34D994 D_8008D996[];

/* 0x10-byte-stride slot record: two u16 fields at +0 and +2. */
typedef struct {
    u16 unk0;
    u16 unk2;
    u8 pad4[0x10 - 0x4];
} Rec16D7F0;
extern Rec16D7F0 D_8008D7F0[];

extern u8 D_8008D970[];

s32 SpuVmSetSeqVol(s32 p0, s16 p1, s16 p2, s16 p3)
{
    s32 dead[2];
    s32 shifted;
    Entry90902E8 *entry;
    s16 key;
    u16 t1v, t0v;
    s16 i;
    u16 *slotLo;
    u16 *slotHi;

    entry = D_800902E8[(u8) p0];
    D_8008EA22 = p0;
    __asm__("");
    shifted = p0 << 16;
    entry += (u32) shifted >> 24;
    key = shifted >> 16;

    entry->unk74 = p1;
    entry->unk76 = p2;
    if ((u16) entry->unk74 >= 0x80) {
        entry->unk74 = 0x7F;
    }
    if ((u16) entry->unk76 >= 0x80) {
        entry->unk76 = 0x7F;
    }

    __asm__("");
    t1v = (u16) p1 * 0x81;
    t0v = (u16) p2 * 0x81;

    if ((s16) p3 == 1) {
        for (i = 0; i < D_8008E9D0; i++) {
            if (D_8008D996[i].unk0 == key) {
                s16 off;

                off = i * 8;
                slotLo = &D_8008D7F0[0].unk0;
                slotHi = &D_8008D7F0[0].unk2;
                slotLo[off] = t1v;
                slotHi[off] = t0v;
                D_8008D970[i] |= 3;
            }
        }
    }
    if (0) {
        dead[0] = 1;
    }
    return (s16) D_8008EA22;
}
#endif
```

`Entry90902E8` and `D_800902E8`/`D_8008EA22` are `src/code_179d8_j.c`'s own
existing top-of-file declarations; only the block above is new.

### The ONE residue that remains

**The `entry` pointer's register, and nothing else.** Retail computes
`addu $v1, $v0, $t0` -- keeping the loaded table base in `t0` and putting the
element pointer in a SECOND register, `v1` -- and every subsequent access is
`0x74($v1)` / `0x76($v1)`. The build emits `addu $t0, $t0, $v0`, coalescing
the two into one pseudo, and every access is `0x74($t0)` / `0x76($t0)`.
Seven words, same instruction, same operands, one register apart:

| retail | build |
| --- | --- |
| `addu v1,v0,t0` | `addu t0,t0,v0` |
| `sh a1,0x74(v1)` | `sh a1,0x74(t0)` |
| `lhu v0,0x74(v1)` | `lhu v0,0x74(t0)` |
| `sh a2,0x76(v1)` | `sh a2,0x76(t0)` |
| `sh v0,0x74(v1)` | `sh v0,0x74(t0)` |
| `lhu v0,0x76(v1)` | `lhu v0,0x76(t0)` |
| `sh v0,0x76(v1)` | `sh v0,0x76(t0)` |

This is a **register-identity** stall by the project's own definition, so it
is not something to force: CLAUDE.md HARD RULE 6 bans `register T v asm("$N")`
and operand constraints for exactly this, and this function has a C form so
no exception applies.

### Axes tried against the `entry` register and REJECTED (all measured on the final body)

- A separate `tbl` local with `entry = &tbl[n]`, `entry = tbl + n`, and an
  offset-first cast `(Entry90902E8 *)(n * sizeof(...) + (u8 *) tbl)`: all
  three give 530 permuter units against the final body's 45 -- GCC either
  coalesces them anyway or adds a `move` at the top. Keeping ONE pseudo
  (`entry += n`) is strictly better even though retail has two.
- Writing the pointer sum offset-first to try to flip the `addu`'s operand
  order: byte-identical. GCC canonicalises commutative operands.
- **Declaration order: 120 orderings measured (of the 5040 permutations of
  the body's 7 declaration groups), every one byte-identical.** Declaration
  order does not steer allocation in this function.
- A barrier between the shift and the `entry +=`, and moving the
  `shifted = p0 << 16` before the `D_8008EA22` store: 45 -> 45 and 45 -> 560.
- Reordering the two `0x81` products, swapping the two `entry->` stores,
  declaring `p1`/`p2` as `u16`: no improvement.

### Proposed learning

**1. When a stalled function's loop shares an idiom with a sibling, VERIFY
the counter's type against the compare instruction before inheriting it.**
`andi 0xff` + `sltu` is a byte-masked unsigned counter; `sll 16`/`sra 16` +
`slt` is a `short`. Those are two instructions to look at, they are in the
loop's last three words, and getting it wrong here cost four rounds of
reshaping an early section that was never the main problem. The sibling
analogy was the trap: `SpuVmSeqKeyOff`, 0x298 bytes later in the same unit,
really IS byte-masked, and this report inherited its `(u8) i` wholesale.

**2. A blacklisted spelling is blacklisted RELATIVE TO A BODY.** Round 23
measured the explicit `p0 << 16` intermediate at 2/96 and wrote "do NOT
retry without a materially different supporting change". That warning was
correct, useful, and correctly conditional -- and the condition came true:
with the counter type fixed and the frame present, the same spelling is the
best of five. The lesson is not "ignore blacklists" but that a blacklist
entry needs its supporting body named, so a later round can tell whether the
condition still holds. Round 23's did name it, which is why it could be
discharged rather than just disobeyed.

**3. The same round-56 pointer-hoist lever now has two independent
confirmations in this unit** (`SpuVmSeqKeyOff`'s `&D_8008EA26`, this
function's `&D_8008D7F0[0].unk0`/`.unk2`), plus a new rider: **for a global
whose address must be hoisted into the PREHEADER of a `for` loop -- i.e.
after the loop-inversion guard, not before it -- the pointer local must be
assigned INSIDE the loop body and left to loop-invariant motion.** Assigning
it before the `for` puts the `lui`/`addiu` ahead of the guard, which is a
different program and 4 words of difference.

## Round 57 (2026-09-19), runner bravo -- title CONFIRMED by rebuild; the local-reuse lever does NOT transfer; permuter search SPENT

**Rebuild-before-trusting-the-score, fourth time, and this one had a specific
job.** Round 56 rewrote this function from 21/96 to 89/96 but left the
round-23 "measurement note" standing at the top of the report, where it still
said 91 words / 5 short / 21-of-96 / first real diff at word 0 -- figures that
flatly contradict the title two lines above them. PARALLEL-RUNS §3.3 screen 4
is exactly this case, so the figure was rebuilt before anything else was done
with it.

Spliced round 56's preserved body in verbatim: `build exit=2`, no
compile-error grep hits.

- **Length EXACT at 96 words.** `funcdiff.py` gives the range
  `0x20C04-0x20D84` (`0x180` bytes / 4 = 96) with **no out-of-range drift
  warning**, and `build/lsdde.map` puts the next function, `SpuVmGetSeqVol`,
  at its retail `0x80030584`.
- **89/96 raw word-match**, and the seven differing words are exactly the
  seven this report's round-56 section tabulates.
- **First real diff at word 18 / `0x8003044C`**, per `tools/asm-differ/diff.py`.

**The TITLE is right and the measurement note was stale.** It has been marked
as superseded in place rather than deleted, because the negatives it records
are still valid against the round-23 body they were measured on -- what was
wrong was its position, at the top of the file where it reads as the current
state.

### Gate 3 (PARALLEL-RUNS §3.5), all three checks run -- and this function passes all of them cleanly

Round 47 DECLINED a search here on check (b) at base score 4350, with
`Insertions: 17` / `Deletions: 22` on a 96-word function. Round 56's rewrite
changed that completely, and the decline does not survive it.

- **(a) scaffold compiles and scores:** yes.
- **(b) `--debug --stack-diffs`:** `Stack Differences: 0 (1)`,
  `Branch Differences: 0 (1)`, `Register Differences: 9 (5)`,
  `Reorderings: 0 (60)`, **`Insertions: 0 (100)`**, **`Deletions: 0 (100)`**,
  **base score = 45**. Zero of everything except register differences. This is
  the cleanest check (b) recorded anywhere in this corpus: round 47 declined
  this same function at 4350 and declined `SpuVmSeqKeyOff` at 1210; round 56
  searched `SpuVmSeqKeyOff` at 870.
- **(c) scaffold-vs-real-build agreement: AGREE.** The scaffold's debug diff
  shows the `addu` and the six `0x74`/`0x76` accesses differing by `$v1`
  vs `$t0` and nothing else -- the same seven words, in the same places, as
  the in-tree `funcdiff.py`/`asm-differ` run above. No scaffold artifact.

**Verdict: SEARCH.** A pure register-identity residue with zero
insertions, deletions and reorderings is precisely the shape a source-mutation
search can move -- round 56 proved that on this unit's `SpuVmSeqKeyOff`, where
the permuter found a `u16` intermediate that hand work had declared
unreachable. Note this is not a HARD RULE 6 problem: the ban is on fixing
register identity with `register T v asm("$N")` or an operand constraint, and
a source-shape mutation is neither.

### The round-57 hand attempt: `SpuVmSeqKeyOff`'s round-57 lever does NOT transfer here

Round 57 found on this unit's `SpuVmSeqKeyOff` that **which existing local you
reuse as a carrier is an allocation-priority lever** -- reusing `hiBit`, the
local assigned in both arms of a later if/else, extended that pseudo's live
range backwards and moved three registers into retail's assignment, worth ten
words. This function's residue is the same KIND of thing (one pseudo,
`entry`, in `$t0` where retail has `$v1`), so the lever was tried here before
the search was spent. It does not transfer. All measured on the real oracle
against the 89/96 base:

| variant | raw match |
| --- | --- |
| base (round 56 body) | **89/96** |
| name `recIdx` as its own `u32` local, then `entry += recIdx` | 89/96, byte-identical |
| carry the table base in `slotHi` (a `u16 *` assigned later, inside the loop) and cast | 74/96 |
| carry the table base in `slotLo`, same shape | length changed; funcdiff refused the score |
| carry the table base in `shifted` (`s32`, reassigned two statements later) | length changed; funcdiff refused the score |
| a dedicated `Entry90902E8 *tbl` local with `entry = tbl + n` | length changed; funcdiff refused the score |

The last one reproduces round 56's own finding that a separate `tbl` local
regresses; the new information is the first row and the third. **Naming the
index pseudo that retail's `$v1` was holding just before the `addu` changes
nothing at all** (byte-identical output), and **reusing a later-assigned local
as the carrier, which is exactly the `hiBit` mechanism, makes it worse here.**

The difference between the two functions is worth stating because it bounds
the new lever: on `SpuVmSeqKeyOff` the reused local (`hiBit`) is a **value**
that genuinely is assigned later in the same block, so the extended live range
is real and costs no instruction. Here the "reuse" has to go through a
pointer cast of an unrelated type, which is a different program, and the
allocator pays for it. **The lever is "reuse a local the block already writes
later", not "reuse any local in scope".**

### Round 57 permuter SEARCH OUTCOME -- the residue is RECLASSIFIED: reachable from source shape, and it costs exactly one word

**Search: 438,478 iterations, `-j 6 --stop-on-zero --best-only`, base score
45, wall clock capped with `timeout 2400`; ended on that cap, `rc=124`** (so
it stopped on the clock, not on a zero -- `--stop-on-zero` never fired). Four
candidates were kept: two at 45 (ties with base) and **two at 5**. No orphan
workers left behind (swept after the exit; `pgrep -af 'permuter.py|decomp-permuter'`
clean).

**Both score-5 candidates share one structural mutation, and it is the
answer to this report's oldest question.** Drop `entry += (u32) shifted >> 24;`
and spell the sum at every one of the six accesses instead:

```c
    (entry + ((u32) shifted >> 24))->unk74 = p1;
    (entry + ((u32) shifted >> 24))->unk76 = p2;
    if ((u16) (entry + ((u32) shifted >> 24))->unk74 >= 0x80) { ... }
    if ((u16) (entry + ((u32) shifted >> 24))->unk76 >= 0x80) { ... }
```

Translated to correct C and built on the real oracle, this **produces
retail's `addu $v1, $v0, <base>` and all six `0x74($v1)` / `0x76($v1)`
accesses**. Verified by `objdump` on `build/src/code_179d8_j.c.o`:
`addu v1,v0,a1` followed by `sh t0,116(v1)` / `lhu v0,116(v1)` /
`sh a2,118(v1)` / `sh v0,116(v1)` / `lhu v0,118(v1)` / `sh v0,118(v1)`.
**The seven-word register-identity residue that this report's round-56
section tabulates is GONE.** Keeping ONE pseudo (`entry += n`) -- which
round 56 concluded was "strictly better even though retail has two" -- was
the thing preventing it.

**It costs exactly one word, and the mechanism is specific.** With two
pseudos instead of one, the table base is live at the same time as the
element pointer, the allocator gives the base `$a1`, and `$a1` is where the
incoming parameter `p1` arrives -- so GCC emits `move t0,a1` at the very
first instruction of the function and the whole body then uses `$t0` for
`p1` where retail uses `$a1`. 97 words against retail's 96.

**Nine spellings measured in-tree, all 97 words** (length read from the
object's own symbols, since `funcdiff.py` correctly refuses to score a
length-mismatched function):

| spelling | words |
| --- | --- |
| `(entry + n)->f` inline at all six accesses | 97 |
| `entry[n].f` inline at all six | 97 |
| a named `u32 recIdx` local, `entry[recIdx].f` | 97 |
| a separate `Entry90902E8 *tbl` with `entry = tbl + n` | 97 |
| the above with no `__asm__("")` barrier | 97 |
| barrier moved after the shift instead of after the store | 97 |
| `entry` declared last in the declaration list | 97 |
| the `D_8008EA22` store moved before the table lookup | 97 |
| `key` computed after the four accesses instead of before | 98 |

**The strongest evidence in this whole section is what the permuter had to
do to get rid of that word.** Both score-5 candidates removed it the same
way: by making exactly ONE of the two `unk76` accesses address the table
base instead of element `n`.

- `output-5-1` breaks the **read**: `new_var = entry->unk76;` where the
  clamp must test `(entry + n)->unk76`.
- `output-5-2` breaks the **write**: `new_var = entry; ... new_var->unk76 = 0x7F;`
  where the store must go to `(entry + n)->unk76`.

They are mirror images of one semantic error, found independently, and the
error IS the missing word -- an access that does not need the element pointer
is one the allocator does not have to keep a second register for. Both are
**REJECTED** under Gate 3's "reject UB candidates" rule (these are not UB but
are plainly wrong programs: they read and write the wrong array element
whenever the high byte of `p0` is non-zero, which is the normal case).

**Verdict: the search is SPENT, and it paid in classification rather than in
words.** The title figure is unchanged at 89/96 -- the 97-word shape is not a
better body, it is a better-understood one. What changed is what the next
attempt is looking for:

> **Old reading (rounds 56 and earlier):** "one register-identity residue, the
> `entry` pointer, and nothing else" -- a HARD RULE 6 stall, nothing to reach
> for from source.
>
> **New reading:** the register identity is fully reachable from source shape
> and costs nothing. What costs one word is that the correct program needs the
> table base and the element pointer live at the same time, the base then takes
> `$a1`, and `$a1` is parameter `p1`'s incoming register. **The open question is
> how to keep `p1` in `$a1` while the base is live**, not which register
> `entry` gets.

The `SpuVmSeqKeyOff` carrier-local lever was already tried against the OLD
reading and failed (table above); against the NEW reading the untried axes are
the ones that touch `p1`'s liveness -- the same "register-pressure-side" axis
this report has flagged as untested since round 23, but now with a named
target instead of a hunch.

### Proposed learning

**1. A permuter search that returns no zero can still be the most valuable
thing that happens to a function, because a REJECTED candidate is a
measurement.** This search ran out its clock at 438,478 iterations without a
zero, which by the usual accounting is a negative. What it actually delivered
was the reclassification above: the shape in its two score-5 candidates,
stripped of the semantic error they both carry, moves this function's residue
from "register identity, unreachable by rule" to "one instruction, with a
named cause". **Read the candidates the search kept even when none of them
scored zero, and read what they had to BREAK** -- two candidates breaking the
same access in mirror-image ways is a measurement that no single hand variant
would have produced.

**2. "Keeping one pseudo is strictly better than retail's two" was a
score-driven conclusion, and score-driven conclusions invert when the score
is dominated by something else.** Round 56 measured the separate-`tbl` forms
at 530 permuter units against the merged form's 45 and concluded the merged
form was right. It was right about the SCORE and wrong about the SHAPE: the
merged form is what made the register residue unfixable, and the separate
form's 530 was one extra instruction plus the ripple, not a worse structure.
When a report says "X regressed, so retail did not do X", check whether X is
visible in retail's own instructions first -- here `addu v1,v0,t0` with a
surviving base register says plainly that retail had two pseudos.

**3. When the correct program is one instruction longer than retail, look for
an access that does not need the long-lived value.** That is what both
permuter candidates found by accident. It is also the shape of a real
possibility worth testing before anything else next round: retail may reach
one of the six accesses through a base-plus-constant that this reading of the
struct does not expose.

## Track 2 (round 86, 2026-09-26, alpha)

This function is still `INCLUDE_ASM` and its C was not touched, but the per-field symbols this report uses (`D_8008D988`..`D_8008D9BA` at a 0x34 stride) are ONE Sony table: libsnd/vmanager.o's `_svm_voice` (0x8008D988, 24 x 0x34 = 0x4E0 bytes), typed in `include/SvmVoice.h` with fields by offset (`D_8008D98C` is `_svm_voice[i].unk04`, `D_8008D9A3` is `unk1B`, and so on: address minus 0x8008D988). The next attempt should write `_svm_voice[i].unkNN`: in every converted accessor (code_179d8_j_b/j_c/l/m/p) the struct spelling compiled byte-identically to the separate symbols, and two NON_MATCHING bodies moved closer to retail. The other `D_` spellings in preserved bodies below still link (splat keeps them as auto-symbols).
