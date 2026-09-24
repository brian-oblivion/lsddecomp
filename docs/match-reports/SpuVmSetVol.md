# SpuVmSetVol -- STALL (round 62 REVISIT: 315/324 built words -- 9 words SHORT, 10/324 raw word-match, **insertions 76 / deletions 76**, first real diff at the very first instruction, vram 0x80030980)

> Renamed from `func_80030980` on 2026-09-24 (tools/rename.py). Address 0x80030980.

> **Head note, round 76: not staffed as a REVISIT-2, pending an ownership decision.**
> The body reads Sony libsnd vmanager state pinned in `config/psyq-objects.ld`:
> `_svm_pg` (0x8008E968), `_svm_vh` (0x8008E970), `_svm_tn` (0x8008E978), a
> field inside `_svm_cur` (0x8008EA22 = `_svm_cur`+0x16) and `_ss_score`
> (0x800902E8). Its only call is `SpuVmVSetUp`, and its only caller is the
> code_179d8_k sequencer, which also calls `SpuVmKeyOn`. Position: right after
> placed `libsnd/vm_prog` (3.6), followed in this unit by Sony's `SsUtKeyOn`
> and `SsUtKeyOnV`. `sdkname.py` finds no fingerprint (best shape 0.51
> `SpuVmSetVol`, below the 0.90 lead bar), so the track 2 two-evidence rule is
> NOT met, and no `NOT GAME CODE` marker was placed. Escalated to the operator.

**REVISITED, round 62: STALL, improved and RE-CLASSIFIED -- round 50's
"whole-function register-identity cascade" verdict is FALSIFIED (its body
measures insertions 101 / deletions 101, and a register-identity residue is
ins 0 / del 0 by definition); two new source-level fixes found and kept, the
residue is now localised to ONE CSE decision plus two loop-invariant hoists;
names/types not relevant (this is a shape problem, and every name this body
uses was already in the unit).**

> This function has now had its one revisit. Round 50's `volatile` lever is
> retired as a *wrong* lever for this function, with a measured reason, not a
> preference: **retail's frame is `addiu sp,sp,-0x38` = 0x18 (outgoing-arg
> area for five arguments) + 0x20 (eight callee-saved slots: s0,s1,s2,s3,s4,
> s5,s6,ra) and therefore has ZERO bytes of spill/local space.** Every
> `volatile` local forces a stack slot. A body with seven `volatile` scalars
> cannot be the shape that produced retail's bytes, whatever its total word
> count says.

Unit: `src/code_179d8_j_b.c`. Size: 324 words (0x510 bytes), file offset
`0x21180`, vram `0x80030980`.

## Step (a): the inherited body, rebuilt before anything was changed

Round 50's preserved body was reproduced verbatim and built first, as the
revisit rule requires. Every FIGURE it records is exactly right:

| measurement | round 50 recorded | round 62 measured |
| --- | --- | --- |
| built words | 324/324 (length exact) | 324 (confirmed) |
| raw word-match | 7/324 | 7/324 |
| drift outside 0x21180-0x21690 | zero | zero (`cmp -l` over the whole image: 1014 differing bytes, every one in range) |
| **insertions / deletions** | *not recorded* | **101 / 101** |

**The VERDICT built on those figures is wrong.** Residue 1 of the round-50
section reads "Whole-function register-allocation cascade ... register-only
('r') diffs dominate almost the entire function body", and the round-50
disposition treats the function as a register-identity stall of the kind HARD
RULE 6 forecloses. A register-identity residue is `insertions 0 / deletions 0`:
same instructions, different registers. **101 inserted and 101 deleted
instructions is a structural difference in 101 places.**

The two readings were never distinguishable from what round 50 measured,
because "length exact" was inferred from *equal total word count* -- and equal
total word count is exactly what 101 insertions balanced against 101 deletions
produces. This is the pre-round-58 Gate-3-check-3 trap, now measured on a live
report rather than argued: **an equal word count is not an ins/del verdict, and
"zero drift" does not upgrade it into one.** Zero drift proves the *window* is
trustworthy; it says nothing about what is inside the window.

For comparison, round 45's body (the same computation, no `volatile` at all)
measures **insertions 85 / deletions 85** at 292/324. So round 50's `volatile`
sweep, measured on the honest metric, made the function **structurally worse**
(85/85 -> 101/101) while making its word count look perfect. That is the whole
lesson of this revisit in one line.

## Step (b): fresh re-read of the disassembly, and what it gives

Read cold from `asm/nonmatchings/code_179d8_j_b/SpuVmSetVol.s`, not from the
previous attempts' line. Two source-level facts fell straight out, both kept:

### Fix 1 -- the loop index is `(u8)i`, not `i`. (`u8 i`)

Retail keeps the counter full-width in `$t1` (`addiu $t1,$t1,0x1`, unmasked)
and re-derives `andi $vX,$t1,0xFF` at **every** use: the 0x34-stride record
index at the loop head (0x80030A50 in the preheader, 0x80030E5C in the
backedge delay slot), again at 0x80030C0C, again at 0x80030CB4, again at
0x80030DE0 for the `D_8008D7F0` store offset and the `D_8008D970` index.
Rounds 45 and 50 both declared `s32 i` and indexed with plain `i`, masking only
in the loop condition. Declaring `u8 i` and dropping the cast from the
condition reproduces retail's mask-at-every-use shape.

Measured alone, on round 45's non-`volatile` body:

| body | built words | raw | ins/del |
| --- | --- | --- | --- |
| round 50 (seven `volatile`) | 324 | 7/324 | 101/101 |
| round 45 (no `volatile`) | 292 | 3/324 | 85/85 |
| **round 62 (`u8 i`, no `volatile`)** | **315** | **10/324** | **76/76** |

### Fix 2 -- the high byte of `a0` is `(a0 & 0xFF00) >> 8`, not `(u8)(a0 >> 8)`

Retail: `andi $a0,$a0,0xFF00` then `sra $a0,$a0,8`. `(u8)(a0 >> 8)` compiles to
`srl` then `andi 0xff` -- the two operations in the other order, and a `srl`
where retail has `sra`. `(a0 & 0xFF00) >> 8` on a signed `a0` compiles to
exactly retail's pair (the mask makes the value non-negative, so GCC's `sra`
for a signed `>>` is correct). Confirmed word-for-word in the built object.
Score-neutral on its own (both words were already register-diffs) and kept
because it is the correct shape.

## Where the remaining 9 words are, exactly

Built 315 vs retail 324. Localised by an opcode-level alignment of the built
object against the `.s` (registers ignored), then read directly. It is **not**
spread across the function -- it is one region:

**The `D_8008E978[D_8008D99C[i].unk0]` address is computed ONCE in our build
and TWICE in retail.**

- `.unk2` read (`e978c`): retail re-uses `$a3`, the `52*(u8)i` byte offset
  computed at the loop head, and spends 9 words (0x80030B7C-0x80030BA0).
  Our build re-derives the index and spends 15.
- `.unk3` read (`e978d`): retail re-derives the ENTIRE chain -- `andi` for
  `(u8)i`, the five-instruction `*0x34`, `lui/addiu/addu/lh` for
  `D_8008D99C[i]`, `lui/lw` for the `D_8008E978` pointer, `sll 5`, `addu`,
  `lbu` -- 15 words at 0x80030C0C-0x80030C54. Our build reuses the `.unk2`
  address register and spends **1** (`lbu v1,3(a2)`).

Net -14 there, +6 for the `.unk2` index re-derivation, +1 in the store
addressing (below) = the 9-word shortfall, near enough.

**The sibling case proves our body's SHAPE is right and the difference is a
compiler decision, not a missing construct.** The structurally identical
`D_8008E968[D_8008D998[i].unk0]` pair -- `.unk1` early, `.unk4` before the
second blend -- is re-derived in full by BOTH compilers: retail 0x80030CB4..
0x80030CEC is 15 words and our build's is 15 words, instruction for
instruction. The only difference between the two cases is that the `.unk4`
read sits after the first blend's control-flow JOIN (`.L80030CB4`), and the
`.unk3` read does not: it is in the same straight-line basic block as the
`.unk2` read (0x80030B7C through the `beqz` at 0x80030C60, no label between).
So GCC 2.6.3's CSE merges the two `.unk3`/`.unk2` addresses in our build and
did not in retail's, **inside a single basic block, with no store and no call
between them to invalidate the table**. Round 45's residue 3 believed writing
the two accesses as textually independent expressions was enough; it is not,
and the preserved body has always had them textually independent.

No idiomatic source spelling was found that separates them: they are the same
expression on the same index, and every rewrite that leaves them semantically
identical leaves them CSE-identical too. This is the one place a lever is still
missing, and it is worth **14 of the 9 net words**, so it is where the next
attempt should go -- not at the frame, not at the registers.

Two further differences, both already-known classes, both small:

- **Loop-invariant hoisting retail did not do (round 45/50 residue 2, still
  live).** Our build hoists `andi t2,s0,0xff` (`(u8)a4`) and
  `subu t6,t5,t2` (`0x7F - (u8)a4`) into the loop preheader; retail computes
  both inside the loop (0x80030D44, 0x80030D74). Word-count-neutral (two words
  either way) but it changes `a4`'s allocno weight, which is one plausible
  input to the register-assignment difference below. `volatile` on the
  parameter is not spellable usefully and retail keeps `a4` in `$s3`, a
  register, so the spill lever is ruled out by retail's own bytes.
- **The two `D_8008D7F0` store addresses.** Retail hoists `&D_8008D7F0` into
  `$t6` and `&D_8008D7F0 + 2` into `$t7` (3 preheader words) and spends 2 words
  in the loop (`addu $v0,$v1,$t6`, `addu $v1,$v1,$t7`); our build spends 8 in
  the loop as two `lui/addiu/addu` sequences and hoists nothing. Net +1 for us.

## Register assignment (what residue 1 should have said)

Retail: `$s0`=result, `$s1`=e, `$s2`=a0, `$s3`=a4, `$s4`=a2, `$s5`=a3,
`$s6`=a1. Ours: `$s0`=a4, `$s1`=result, `$s2`=a0, `$s3`=e, `$s4`=a1,
`$s5`=a2, `$s6`=a3. Both use seven callee-saved registers plus `$ra`; retail's
frame is `-0x38`, ours `-0x30` because ours reserves 0x10 of outgoing-arg area
where retail reserves 0x18. Declaration order was tested as a lever
(`result` moved to first in the declaration list) and is **INERT** here: the
build was byte-identical, 315/324, 76/76. Per HARD RULE 6 nothing further is
attemptable on register identity directly; the point of writing it down is that
it is *downstream* of the shape difference above, not the thing to attack.

## Best-derived body (315/324 words, 10/324 raw, ins 76 / del 76; preserved for the next attempt)

Declarations used, all of them already present in `src/code_179d8_j_b.c`
before this round (`Rec34D994`, `SlotE968`, `RecordE978`, `D800902E8Entry`,
`ObjE970`, `D_8008E9D0`, `D_8008EA22`, `D_8008E8C0`, `D_8008D7F0`,
`D_8008D970`, `SpuVmVSetUp`).

```c
#if 0
s32 SpuVmSetVol(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4) {
    D800902E8Entry *e;
    u8 i;
    s32 result;
    u32 pan1;
    u32 pan2;

    result = 0;
    e = &D_800902E8[a0 & 0xFF][(a0 & 0xFF00) >> 8];
    SpuVmVSetUp((s16)a1, (s16)a2);
    D_8008EA22 = (u16)a0;

    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            if (D_8008D996[i].unk0 == (s16)a0) {
                s32 t0 = D_8008D99A[i].unk0;
                if (t0 == (s16)a2 && D_8008D99E[i].unk0 == (s16)a1) {
                    u8 e968FromD998 = D_8008E968[D_8008D998[i].unk0].unk1;
                    u8 e968FromT0 = D_8008E968[t0].unk1;
                    s32 lvl0;
                    s32 prio;
                    s32 lvl1;
                    u32 lvl1b;
                    u32 lvl1c;
                    u32 lvl2;
                    u8 e978c;
                    u8 e978d;
                    u8 e968d;
                    u32 pan1sq;
                    u32 pan2sq;
                    s32 off16;

                    lvl0 = D_8008D990[i].unk0 * (u16)a3 / 127;
                    prio = lvl0 * 0x3FFF;
                    lvl1 = D_8008E970->unk18 * prio / 16129;

                    if (e968FromD998 != e968FromT0) {
                        lvl1b = lvl1 * e968FromT0;
                    } else {
                        lvl1b = lvl1 * e968FromD998;
                    }

                    e978c = D_8008E978[D_8008D99C[i].unk0].unk2;
                    lvl1c = lvl1b * e978c;
                    lvl2 = lvl1c / 16129;

                    pan1 = (lvl2 * e->unk74) / 127;
                    pan2 = (lvl2 * e->unk76) / 127;

                    e978d = D_8008E978[D_8008D99C[i].unk0].unk3;
                    if (e978d < 0x40) {
                        pan2 = (pan2 * e978d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e978d)) / 63;
                    }

                    e968d = D_8008E968[D_8008D998[i].unk0].unk4;
                    if (e968d < 0x40) {
                        pan2 = (pan2 * e968d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e968d)) / 63;
                    }

                    if ((u8)a4 < 0x40) {
                        pan2 = (pan2 * (u8)a4) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - (u8)a4)) / 63;
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

                    off16 = i << 4;
                    *(u16 *)(D_8008D7F0 + off16) = (u16)(pan1sq / 16383);
                    *(u16 *)(D_8008D7F0 + off16 + 2) = (u16)(pan2sq / 16383);

                    result++;
                    D_8008D970[i] |= 3;
                }
            }
            i++;
        } while (i < D_8008E9D0);
    }
    return result;
}
#endif
```

Everything the round-45/50 sections say about the COMPUTATION (the division
chain and its magics, the blend cascade, the clamp, the two-level
`D_800902E8` indexing) is confirmed again this round and unchanged. The whole
control-flow shape is confirmed correct against retail instruction by
instruction: every branch target lines up, the `!=`-arm of the `e968` select is
the jumping arm and is written first (`beq` skips to the fallthrough `else`),
each blend's `< 0x40` arm is the jumping arm and is written first, and the
`pan1 < pan2` arm jumps. Those are all cases of the block-order rule and they
are all already right.

## Attempts (round 62 -- 6 real builds, no scaffold)

1. Round 50's seven-`volatile` body verbatim: 324/324 words, 7/324 raw, zero
   drift, **ins 101 / del 101**. Every figure reproduced; verdict falsified.
2. Same body with all seven `volatile` qualifiers removed (= round 45's
   computation): 292/324 words confirmed, 3/324 raw, **ins 85 / del 85**.
3. `u8 i` with `while (i < D_8008E9D0)`: 315/324, 10/324 raw, **ins 76 / del 76**.
   The round's single largest improvement, and the only one so far that moves
   the honest metric.
4. `(a0 & 0xFF00) >> 8` for the entry high byte: word-for-word match on those
   two instructions; score unchanged (they were already register-diffs). Kept.
5. `result` declared first in the local list (targeting retail's `$s0`=result):
   **byte-identical output**, 315/324, 76/76. INERT. Reverted.
6. Final confirmation build of the reported body; `INCLUDE_ASM` restored and the
   whole-image oracle re-run green.

No permuter search was spent here. A 76/76 residue is not the class the
permuter is for (one insertion + one deletion), and this session's one search
was better spent on `SsUtKeyOffV`.

### Proposed learning

**`funcdiff.py`'s `insertions N / deletions M` line is the only thing that
distinguishes a register-identity residue from a structural one, and equal
total word count actively disguises the difference.** Round 50 measured this
function at exactly 324 built words against retail's 324, with zero drift
anywhere in the image, and concluded from that pair of facts that what remained
was register identity -- a class HARD RULE 6 forecloses, i.e. a reason to stop.
The body is ins 101 / del 101: one hundred and one instructions inserted and one
hundred and one deleted, which is *why* the totals agreed. Two consequences
worth carrying:

- **A "length-exact" claim in a report title is not evidence of structural
  agreement and must never be used to classify a residue.** Only ins/del does
  that. Where a title quotes length, it should quote ins/del beside it.
- **`volatile` as an anti-optimisation lever should be screened against the
  target's FRAME SIZE before it is swept at all.** Every `volatile` local costs
  a stack slot. Decompose retail's frame first -- outgoing-arg area (0x10, or
  more when there are more than four arguments) plus 4 bytes per callee-saved
  register in the prologue -- and if the remainder is zero, retail spilled
  nothing and no amount of `volatile` can be the shape that produced it. Here
  that is a two-minute check (`-0x38` = 0x18 + 0x20, remainder 0) that would
  have redirected a 13-step sweep. The sibling report this lever was borrowed
  from (`vmNoiseOn`) should be re-read the same way before it is trusted
  again: its own recorded residue is a frame-size DISAGREEMENT
  ("`addiu sp,sp,-0x10`; retail has `addiu sp,sp,-0x8`"), which is the same
  screen failing in the same direction.

## Disposition

**Restored to `INCLUDE_ASM`**; whole-image oracle green. The revisit is spent.
The function is better characterised than it has ever been and the next
attempt has exactly one place to go: the `D_8008E978[D_8008D99C[i].unk0]`
CSE, worth 14 words, with a proven-correct sibling (the `D_8008E968`/`.unk4`
chain) matching instruction-for-instruction two blocks later as the control.
Do **not** re-sweep `volatile`, and do not re-open the register-identity
reading.

---

# SpuVmSetVol -- STALL (round 50: LENGTH-EXACT, 292/324 -> 324/324 built words -- 0 words off, 7/324 raw word-match, ZERO drift outside the function, first real diff at the very first instruction, vram 0x80030980)

> Round 45 reached a complete structural derivation (every computational
> piece already proven in the SPU-voice-level family) but landed 32 words
> SHORT with drift-affected scoring. Round 50 found the lever that closes
> the LENGTH gap exactly -- `volatile` on the seven scalar intermediates
> listed below -- without changing a single line of the actual computation.
> The residue that remains is the same whole-function register-identity
> cascade round 45 already characterised (residue 1, below), now isolated
> with a clean, driftless measurement instead of a drift-affected one.

Unit: `src/code_179d8_j_b.c`. Size: 324 words (0x510 bytes), file offset
`0x21180`, vram `0x80030980`.

## What it computes

Unchanged from round 45 -- see that section below for the full
computation writeup (division chain, blend cascade, clamp, magic
constants). Round 50 touched only which locals are `volatile`, not the
arithmetic or control flow.

## Round 50: the `volatile` lever that closes the length gap

Round 45's report proposed, as the next attempt's most promising lever,
investigating why retail does not hoist `(s16)a0` out of the loop
(residue 2). That specific lever was NOT what closed the gap -- it
remains unresolved (see residue 2, carried forward unchanged). What
closed it instead is the SAME `volatile`-on-divisor-chain-intermediates
lever this unit's sibling `vmNoiseOn`'s report already documents
(`docs/match-reports/vmNoiseOn.md`, residue 2: "Marking
`lvl1`/`lvl2`/`lvl1b`/`lvl2b` all `volatile` forces all four to spill to
memory and reproduces every correction sequence"), extended to this
function's own six-stage division chain PLUS one variable the sibling
report never had occasion to try: the store-address offset.

**The exact set, found by systematic sweep (see Attempts below):**

```c
volatile s32 lvl0;
volatile s32 prio;
volatile s32 lvl1;
volatile u32 lvl1b;
volatile u32 lvl1c;
volatile u32 lvl2;
/* ... */
volatile s32 off16;
```

All seven, together, and ONLY this combination among everything tried,
compiles to exactly 324/324 words -- byte-for-byte the same length as
retail, with **zero drift outside the function's own 0x21180-0x21690
range** (confirmed both by `funcdiff.py`'s own drift check reporting no
warning, and independently by `cmp -l build/SLPS_015.56 disk/SLPS_015.56`:
every differing byte across the WHOLE IMAGE falls inside this function's
own byte range). This is a qualitatively different, and much more
trustworthy, measurement than round 45's 292/324 (drift-affected): every
remaining diff is now a real, isolated fact about this function, not an
artifact of a downstream address shift.

**`off16` is a NEW instance of the sibling's lever, not previously tried
on any of the three SPU-voice-level functions.** It is the byte offset
(`i << 4`) used twice, to compute the two `D_8008D7F0` store addresses.
Marking it `volatile` forces the offset to be recomputed from memory
rather than kept live across both stores in a register, which is what
retail's own bytes show happening structurally at that point (retail
computes the two store addresses through a DIFFERENT register-reuse
pattern than a naive register-only allocation produces -- see the
`sll v1,a1,0x4` shared-then-added-to-two-different-bases shape in the
diff).

**The seven-variable set is not decomposable, and not extensible.**
Systematically swept (see Attempts): dropping any ONE of the seven
regresses the length by 2-5 words (never stays at 324); every one of nine
additional candidate `volatile` qualifiers tried on top of the full seven
(on `e978c`, `e978d`, `e968d`, `pan1sq`, `pan2sq`, `e968FromD998`,
`e968FromT0`, `t0`, `result`, `pan1`, and the `e` pointer -- see Attempts)
overshoots past 324 rather than improving the raw match. This matches
`vmNoiseOn`'s own finding almost exactly: "`volatile` as an
anti-optimization lever does not compose linearly or monotonically... the
right set is a discrete, non-obvious combination."

## Struct/global model

Unchanged from round 45 (all declarations already existed in this unit
before round 45, or were added by round 45 and are unaffected by round
50's `volatile` lever):

```c
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;
extern D800902E8Entry *D_800902E8[];

typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;

extern s16 D_8008E8C0;
extern u8 D_8008D7F0[];
extern u8 D_8008D970[];
extern Rec34D994 D_8008D990[];   /* added round 45, in this unit's existing Rec34D994 block */
```

## Best-derived body (324/324 words, ZERO drift; 7/324 raw word-match; preserved for the next attempt)

```c
s32 SpuVmSetVol(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4) {
    D800902E8Entry *e;
    s32 i;
    s32 result;
    u32 pan1;
    u32 pan2;

    result = 0;
    e = &D_800902E8[a0 & 0xFF][(u8)(a0 >> 8)];
    SpuVmVSetUp((s16)a1, (s16)a2);
    D_8008EA22 = (u16)a0;

    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            if (D_8008D996[i].unk0 == (s16)a0) {
                s32 t0 = D_8008D99A[i].unk0;
                if (t0 == (s16)a2 && D_8008D99E[i].unk0 == (s16)a1) {
                    u8 e968FromD998 = D_8008E968[D_8008D998[i].unk0].unk1;
                    u8 e968FromT0 = D_8008E968[t0].unk1;
                    volatile s32 lvl0;
                    volatile s32 prio;
                    volatile s32 lvl1;
                    volatile u32 lvl1b;
                    volatile u32 lvl1c;
                    volatile u32 lvl2;
                    u8 e978c;
                    u8 e978d;
                    u8 e968d;
                    u32 pan1sq;
                    u32 pan2sq;
                    volatile s32 off16;

                    lvl0 = D_8008D990[i].unk0 * (u16)a3 / 127;
                    prio = lvl0 * 0x3FFF;
                    lvl1 = D_8008E970->unk18 * prio / 16129;

                    if (e968FromD998 != e968FromT0) {
                        lvl1b = lvl1 * e968FromT0;
                    } else {
                        lvl1b = lvl1 * e968FromD998;
                    }

                    e978c = D_8008E978[D_8008D99C[i].unk0].unk2;
                    lvl1c = lvl1b * e978c;
                    lvl2 = lvl1c / 16129;

                    pan1 = (lvl2 * e->unk74) / 127;
                    pan2 = (lvl2 * e->unk76) / 127;

                    e978d = D_8008E978[D_8008D99C[i].unk0].unk3;
                    if (e978d < 0x40) {
                        pan2 = (pan2 * e978d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e978d)) / 63;
                    }

                    e968d = D_8008E968[D_8008D998[i].unk0].unk4;
                    if (e968d < 0x40) {
                        pan2 = (pan2 * e968d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e968d)) / 63;
                    }

                    if ((u8)a4 < 0x40) {
                        pan2 = (pan2 * (u8)a4) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - (u8)a4)) / 63;
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

                    off16 = i << 4;
                    *(u16 *)(D_8008D7F0 + off16) = (u16)(pan1sq / 16383);
                    *(u16 *)(D_8008D7F0 + off16 + 2) = (u16)(pan2sq / 16383);

                    result++;
                    D_8008D970[i] |= 3;
                }
            }
            i++;
        } while ((u8)i < D_8008E9D0);
    }
    return result;
}
```

(Locals are block-scoped, declared at the tightest enclosing compound
block rather than function top -- round 50's FIRST attempt, before the
`volatile` lever, confirmed this alone is INERT here: byte-identical
output to round 45's function-top layout, 292/324 both times. Kept
because it does no harm and documents that DECOMPILATION_LEARNINGS 3d's
"declare inside the block where the value must survive one call" lever
does not apply to this function -- nothing here survives a CALL
boundary, only loop iterations, which is a different precondition.)

## Residues

1. **Whole-function register-allocation cascade, visible from the very
   first instruction -- UNCHANGED from round 45, now measured driftless.**
   Retail's frame is `addiu sp,sp,-0x38` (56 bytes); this derivation's
   frame is `-0x48` (72 bytes, larger this time -- the `volatile` lever
   trades frame size for length, the same tradeoff `vmNoiseOn`'s
   report documents: "`addiu sp,sp,-0x10` (16-byte) frame; retail has
   `addiu sp,sp,-0x8`"). Retail initializes `$s0 = 0` (the `result`
   accumulator) as literally its SECOND instruction, before even touching
   `a0`; every derivation attempted (round 45's, and every `volatile`
   variant round 50 tried) instead preserves `a0` into a callee-saved
   register first and defers `result`'s zero-init to later (a `move
   $s1,zero` in a branch delay slot, well into the function). This is
   THE dominant residue: register-only ('r') diffs dominate almost the
   entire function body once past the setup sequence (confirmed by
   reading `tools/asm-differ/diff.py SpuVmSetVol` directly on the new
   324/324 build). Per HARD RULE 6 this is not fixable by pinning a
   register identity, and per `vmNoiseOn`'s own extensively-searched
   history (12+ builds across `volatile` subsets, all documented as
   non-monotonic), it is very unlikely to be a small remaining lever
   rather than a structural property of GCC 2.6.3's RTL scheduling order
   for this shape of function.
2. **Retail does NOT hoist `(s16)a0` out of the search loop; this
   derivation's compiler does -- UNCHANGED, confirmed still present after
   the `volatile` lever.** Retail recomputes the sign-extension of `a0`
   fresh every loop iteration; every derivation here computes it once and
   reuses it. Per DECOMPILATION_LEARNINGS 3h ("The permitted `__asm__("")`
   barrier is inert against every pass that is not the scheduler...
   measured on... a GCSE/value-availability hoist AT ANY PLACEMENT"),
   a barrier is proven NOT to reach this class of residue on this
   pipeline, so it was not spent as a build this round. `volatile` on the
   parameter itself is not attemptable (parameters cannot be usefully
   `volatile`-qualified without changing the function's declared type in
   a way no caller matches). Left unresolved, as round 45 left it.

   **A SECOND instance of the same phenomenon was found this round, on a
   DIFFERENT loop-invariant expression.** Before the `off16`/`lvl*`
   `volatile` lever was applied, an intermediate build showed GCC hoisting
   `(0x7F - (u8)a4)` (the else-branch blend value for the function's own
   5th, stack-passed parameter) OUT of the loop entirely, computing it
   ONCE before the loop into a dedicated register and reusing it every
   iteration, even though the source computes it inside a per-iteration
   `if`/`else`. This is the SAME class of residue as `(s16)a0`'s hoisting
   (a pure register computation over a value that does not change across
   iterations, CSE'd/LICM'd by GCC regardless of C-level scope), just on
   a second, independent expression. Neither instance has a known source
   lever on this pipeline; both are left as the same residue class.
3. **A duplicate-load pattern, partially addressed (round 45, unchanged
   this round)**: retail recomputes the `D_8008D99C[i]` index chain
   independently for its two field reads (`.unk2` and `.unk3`) rather than
   caching it; the preserved body already reflects this (round 45's
   attempt 2). Confirmed still present and still NOT the dominant residue.

## Attempts

Within the 30-build cap (all real builds -- the project's shared-build
oracle, no scaffold used):

1. **Verbatim reproduction of round 45's derivation** (function-top
   declarations, no `volatile`): confirmed 292/324 exactly, 3/324 raw,
   drift-affected -- matches round 45's recorded figures exactly, single
   build.
2. **Block-scoped locals** (declared at the tightest enclosing compound
   block per DECOMPILATION_LEARNINGS 3d), no other change: byte-identical
   to attempt 1 (292/324, same drift). INERT -- this function's locals
   never survive a CALL boundary (only loop iterations), which is a
   different precondition from the lever's documented success case
   (`Snd_setVabAttr`). Kept in the body since it is harmless and slightly
   clearer.
3. **`volatile` on `lvl1`/`lvl2` only** (the two names `vmNoiseOn`'s
   own report names): 292 -> **310/324** (14 short). Large, immediate
   win -- first confirmation this sibling lever transfers to
   `SpuVmSetVol`'s own (differently-shaped) divisor chain.
4. **`volatile` on ALL SIX divisor-chain scalars** (`lvl0`, `prio`,
   `lvl1`, `lvl1b`, `lvl1c`, `lvl2`): 310 -> **321/324** (3 short). Beat
   every subset tried (see step 5).
5. **Subset sweep around the six-variable set**, each a separate build:
   - Drop `lvl0`+`prio` (keep the other four): 292 -> 317/324 (worse than
     six-together's 321).
   - Drop `prio` only (keep five): 318/324.
   - Drop `lvl0` only (keep five): 317/324.
   All worse than the full six (321). Reverted to all six.
6. **`volatile` on `pan1sq`/`pan2sq` in addition to the six**: regressed
   to 328/324 (4 words OVER -- the first sign this direction of guessing
   overshoots). Reverted.
7. **`volatile` on `e978c`/`e978d`/`e968d` in addition to the six**:
   regressed hard, to 337/324. Reverted -- matches `vmNoiseOn`'s own
   finding that more `volatile` is not monotonically better.
8. **`volatile` on `off16` in addition to the six**: **324/324 EXACT**,
   zero drift. This round's key find -- the combination of the six
   divisor-chain scalars PLUS the store-address offset is the unique
   length-exact set found. Raw word-match 7/324.
9. **Systematic "drop one of the seven" sweep** (7 separate builds, one
   per variable): dropping `lvl0` -> 322; `prio` -> 321; `lvl1` -> 319;
   `lvl1b` -> 322; `lvl1c` -> 322; `lvl2` -> 320; `off16` -> 321. **None
   reach 324** -- every one of the seven is necessary for the exact
   length.
10. **Systematic "add one more volatile to the seven" sweep** (8 separate
    builds): `e978c` -> 327; `e978d` -> 329; `e968d` -> 332; `pan1sq` ->
    327; `pan2sq` -> 326; `e968FromD998` -> 326; `e968FromT0` -> 326;
    `t0` -> 330. **All overshoot past 324** -- the seven-variable set is
    not extensible without regressing length.
11. **`volatile` on `result`** (function-scope, not just the loop-body
    scalars): regressed to 327/324 with drift. Reverted.
12. **`volatile` on `pan1`** (one of the two function-scope accumulators):
    regressed hard to 343/324. Reverted -- matches `vmNoiseOn`'s own
    finding that `volatile` on `pan1`/`pan2`-equivalent values ("regressed
    sharply, to 340/311") is the wrong direction on this shape too.
13. **`D800902E8Entry * volatile e`** (the entry pointer, function-scope):
    regressed to 327/324. Reverted.

None of steps 6, 7, 11, 12, 13 improved on step 8's 324/324; the
seven-variable set from step 8 is the final, reported best.

### Proposed learning

**The sibling `vmNoiseOn`'s "`volatile` on the divisor-chain
intermediates" lever generalises across the SPU-voice-level family, but
the specific SET of variables it needs is per-function, not
transferable verbatim, and grows to include address-offset locals, not
just arithmetic ones.** `vmNoiseOn`'s own report names `lvl1`/`lvl2`/
`lvl1b`/`lvl2b` as its closing set; `SpuVmSetVol`'s closing set is
`lvl0`/`prio`/`lvl1`/`lvl1b`/`lvl1c`/`lvl2` (six, not four, reflecting
this function's own longer division chain) PLUS `off16`, a pure address
arithmetic local with no division in it at all -- a NEW instance of the
lever applied to a construct (`*(u16*)(base+offset)` store addressing)
neither sibling report tried. The generalisable process is: (1) start
from the sibling's own named set as a hypothesis, (2) sweep OUTWARD one
variable at a time rather than guessing a full set, (3) use whole-image
byte-count (not just funcdiff's window) as the acceptance test once a
sweep gets close, because EXACT LENGTH (zero drift) is a qualitatively
different, more trustworthy state than "closest word count so far" --
every reported figure after that point measures the SAME real residue,
not an address-shifted approximation of it. And (4), confirmed by two
independent 7-8-build sweeps here: neither the "drop one" nor the "add
one more" direction found ANY improvement on the discovered set --
for a whole-function `volatile`-spill lever, once a length-exact set is
found by outward sweep, further single-variable perturbation in either
direction is not worth budgeting on the same function again without a
new, specific hypothesis.

## Disposition

**Restored to `INCLUDE_ASM`** (no score short of byte-exact stays in
`src/`). Round 50 did not close this function, but converted a
drift-affected 32-words-short measurement into a driftless, length-exact
one -- every remaining difference is now a real, isolated fact about this
function's own register allocation, not an artifact of a downstream
address shift. The dominant residue (1, the whole-function
register-identity cascade from the very first instruction) is the same
class `vmNoiseOn`'s own extensively-searched report treats as a
structural property of this pipeline's RTL scheduling rather than a
findable source lever; residue 2 (loop-invariant hoisting GCC performs
that retail's binary does not) is proven immune to the one barrier
construct this project sanctions (DECOMPILATION_LEARNINGS 3h). The next
attempt's most promising avenue, not tried this round, is a permuter
search seeded from this round's 324/324 body -- unlike round 45's
32-words-short body, a length-exact seed gives the permuter a
byte-for-byte-comparable target from its first candidate rather than a
drift-corrupted one, which is a substantially better starting position
than either prior attempt had.

---

# (round 45, superseded by round 50's `volatile` lever above) SpuVmSetVol -- STALL (first-ever attempt, 0 -> 292/324 built words -- 32 words SHORT, 3/324 raw word-match (drift-affected), first real diff at the very first instruction, vram 0x80030980)

> Both blockers this report previously cited (`addiu_at`, `nop_mflo_mfhi`)
> are RESOLVED (rounds 21 and 42 respectively; see CLAUDE.md's "Open
> toolchain blockers"). Round 45 is this function's first-ever attempt, and
> it reached a genuine, complete structural derivation reusing every idiom
> this project has already established for the game's SPU voice-level
> family (`vmNoiseOn`, `SpuVmKeyOnNow` in `code_179d8_l.c`). The
> residue is a whole-function-scale register-allocation-cascade difference,
> not a missing construct.

Unit: `src/code_179d8_j_b.c`. Size: 324 words (0x510 bytes), file offset
`0x21180`, vram `0x80030980`.

## What it computes

The largest function in this unit, and the most elaborate member of the
SPU-voice-level family this project has now derived three times over. Scans
a fixed-count table (`D_8008E9D0` entries, 0x34-byte-stride `Rec34D994`
records already declared in this unit -- `D_8008D996`/`98`/`9A`/`9C`/`9E`)
for the entry whose `D_8008D996[i]`/`D_8008D99A[i]`/`D_8008D99E[i]` fields
match the function's three ID-ish parameters (`a0`, `a2`, `a1`, compared as
signed 16-bit values). On a match:

- Calls `SpuVmVSetUp((s16)a1, (s16)a2)` and writes `D_8008EA22 = (u16)a0`
  (write-only here, matching this unit's existing `D_8008EA22` comment
  about which functions write it).
- Looks up a `D800902E8Entry` via the SAME two-level `D_800902E8[lowbyte]
  [highbyte]` indexing `vmNoiseOn`/`SpuVmKeyOnNow` already established
  (declared locally here, not shared, per this project's convention).
- Computes a base level through a THREE-STAGE division chain that is a new
  combination of pieces each individually already proven correct
  elsewhere: `D_8008D990[i].unk0 * (u16)a3 / 127` (signed, matches
  `SpuVmSetVol`'s sibling `SpuVmKeyOnNow`'s first-stage magic exactly,
  reproducer-verified there), times `0x3FFF`, times `D_8008E970->unk18`,
  divided by `16129` (signed, same magic `0x82061029` as `SpuVmKeyOnNow`'s
  second stage) -- **this exact `*0x3FFF /16129` combination, with the
  SAME operand-order re-association hazard `SpuVmKeyOnNow`'s report
  documents, is reused verbatim here** (an explicit `prio` intermediate
  statement, not a parenthesized expression, is what keeps GCC from
  re-attaching the constant multiply to the wrong operand).
- A conditional multiply by one of two `SlotE968` byte fields (already
  declared in this unit, `.unk1`) depending on whether two differently-
  indexed reads of the same field agree, then a multiply by a
  `RecordE978` byte field (already declared in this unit, `.unk2`), then
  ANOTHER division by `16129` -- this one UNSIGNED (`multu`, magic
  `0x040C2051`, the same unsigned-16129 magic `SpuVmKeyOnNow`'s second
  division uses).
- The result feeds the entry's `unk74`/`unk76` fields through a SINGLE
  `/127` division each (unsigned) -- structurally distinct from both
  `vmNoiseOn` (double-chained `/127`) and `SpuVmKeyOnNow` (further
  `/16129`-combined); this function's own shape is the simplest of the
  three, a direct single division.
- A THREE-STAGE `<0x40`-branch blend cascade, IDENTICAL IN SHAPE to
  `vmNoiseOn`/`SpuVmKeyOnNow`'s (this is now confirmed a THIRD time
  in this project, always the same "if byte<0x40: scale one side by byte;
  else: scale the other side by 127-byte" pattern, divisor 63) -- but
  fed by three DIFFERENT byte sources this time: `RecordE978.unk3`,
  `SlotE968.unk4`, and the function's own 5th parameter (`a4`, passed on
  the STACK per MIPS o32 register-argument exhaustion, loaded via `lhu` at
  `sp+0x48`).
- The same `D_8008E8C0`-gated clamp shape as both siblings.
- Squares both final pan values and divides each by `16383` (same magic
  `0x00040011` `SpuVmKeyOnNow`'s tail already established), writing the
  results into `D_8008D7F0`'s 16-byte-stride slots (`vmNoiseOn`'s own
  table, offset `i<<4` and `i<<4 + 2`) and incrementing a result counter.
  Also sets `D_8008D970[i] |= 3` (byte array, direct `i`-indexed, no
  scaling -- matches `vmNoiseOn`'s own `D_8008D970` usage exactly).

**No new struct or divisor family was needed anywhere in this function** --
every single computational piece is a direct reuse of something
`vmNoiseOn` or `SpuVmKeyOnNow` already proved compiles correctly
through the pinned pipeline. The residue is purely in how GCC allocates
registers across this much larger, five-parameter, stack-arg,
loop-plus-nested-branches function, not in any individual construct.

## Struct/global model

All of `Rec34D994`, `SlotE968`, `RecordE978`, `D_8008E9D0`, `D_8008EA22`,
`D_80090C60`/`64`, `D_8008E228`/`22C` were ALREADY declared in this unit
(`src/code_179d8_j_b.c`) before this round, for `SsUtKeyOn`'s existing
stall. Added this round, following `vmNoiseOn`/`SpuVmKeyOnNow`'s
already-proven declarations verbatim:

```c
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;
extern D800902E8Entry *D_800902E8[];

typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;

extern s16 D_8008E8C0;
extern u8 D_8008D7F0[];
extern u8 D_8008D970[];
extern Rec34D994 D_8008D990[];   /* added to this unit's existing Rec34D994 block */
```

## Best-derived body (292/324 words, 32 words SHORT, preserved for the next attempt)

```c
s32 SpuVmSetVol(s32 a0, s32 a1, s32 a2, s32 a3, u16 a4) {
    D800902E8Entry *e;
    s32 i;
    s32 t0;
    u8 e968FromD998;
    u8 e968FromT0;
    u8 e968d;
    u8 e978c;
    u8 e978d;
    s32 lvl0;
    s32 prio;
    s32 lvl1;
    u32 lvl1b;
    u32 lvl1c;
    u32 lvl2;
    u32 pan1;
    u32 pan2;
    u32 pan1sq;
    u32 pan2sq;
    s32 off16;
    s32 result;

    result = 0;
    e = &D_800902E8[a0 & 0xFF][(u8)(a0 >> 8)];
    SpuVmVSetUp((s16)a1, (s16)a2);
    D_8008EA22 = (u16)a0;

    if (D_8008E9D0 != 0) {
        i = 0;
        do {
            if (D_8008D996[i].unk0 == (s16)a0) {
                t0 = D_8008D99A[i].unk0;
                if (t0 == (s16)a2 && D_8008D99E[i].unk0 == (s16)a1) {
                    e968FromD998 = D_8008E968[D_8008D998[i].unk0].unk1;
                    e968FromT0 = D_8008E968[t0].unk1;

                    lvl0 = D_8008D990[i].unk0 * (u16)a3 / 127;
                    prio = lvl0 * 0x3FFF;
                    lvl1 = D_8008E970->unk18 * prio / 16129;

                    if (e968FromD998 != e968FromT0) {
                        lvl1b = lvl1 * e968FromT0;
                    } else {
                        lvl1b = lvl1 * e968FromD998;
                    }

                    e978c = D_8008E978[D_8008D99C[i].unk0].unk2;
                    lvl1c = lvl1b * e978c;
                    lvl2 = lvl1c / 16129;

                    pan1 = (lvl2 * e->unk74) / 127;
                    pan2 = (lvl2 * e->unk76) / 127;

                    e978d = D_8008E978[D_8008D99C[i].unk0].unk3;
                    if (e978d < 0x40) {
                        pan2 = (pan2 * e978d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e978d)) / 63;
                    }

                    e968d = D_8008E968[D_8008D998[i].unk0].unk4;
                    if (e968d < 0x40) {
                        pan2 = (pan2 * e968d) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - e968d)) / 63;
                    }

                    if ((u8)a4 < 0x40) {
                        pan2 = (pan2 * (u8)a4) / 63;
                    } else {
                        pan1 = (pan1 * (0x7F - (u8)a4)) / 63;
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

                    off16 = i << 4;
                    *(u16 *)(D_8008D7F0 + off16) = (u16)(pan1sq / 16383);
                    *(u16 *)(D_8008D7F0 + off16 + 2) = (u16)(pan2sq / 16383);

                    result++;
                    D_8008D970[i] |= 3;
                }
            }
            i++;
        } while ((u8)i < D_8008E9D0);
    }
    return result;
}
```

## Residues

1. **Whole-function register-allocation cascade, visible from the very
   first instruction.** Retail's frame is `addiu sp,sp,-0x38` (56 bytes,
   saving `s0`/`s1`/`s2`/`s3`/`s4`/`s5`/`s6`/`ra`); this derivation's frame
   is `-0x30` (48 bytes), one callee-saved register fewer. Retail
   initializes `$s0 = 0` (the `result` accumulator) as literally its
   SECOND instruction, before even touching `a0`; this derivation's
   register allocator instead uses that register slot to preserve `a0`
   first. This is a DIFFERENT register-allocation choice from the very
   start of the function, and it cascades: nearly every subsequent
   instruction in the realigned diff shows the SAME two operands, doing
   the SAME operation, in a DIFFERENT register (confirmed by reading
   `tools/asm-differ/diff.py SpuVmSetVol` directly -- register-only
   diffs (`r`) vastly outnumber any structural insertion/deletion for
   long stretches of the function).
2. **Retail does NOT hoist `(s16)a0` out of the search loop; this
   derivation's compiler does.** Retail recomputes `sll v0,s2,0x10; sra
   v0,v0,0x10` (sign-extending `a0`) FRESH inside the loop body, at the
   point of the `D_8008D996[i].unk0 == (s16)a0` comparison, every
   iteration. This derivation's build computes it ONCE, before the loop,
   into a dedicated register (`t8`) and reuses it every iteration --
   ordinary, expected loop-invariant code motion, but NOT what retail's
   own binary shows. Since `a0` does not change across iterations, both
   are valid C-level behaviors for the identical source; which one GCC
   2.6.3 picks appears to depend on register-pressure/scheduling
   decisions elsewhere in the function rather than on anything directly
   controllable in this expression. Not resolved this round.
3. **A duplicate-load pattern was found and partially addressed**: an
   early attempt cached `D_8008D99C[i].unk0` (the `RecordE978` index) into
   a local variable and reused it for both the `.unk2` and `.unk3` reads;
   retail instead RECOMPUTES the entire index chain (the `i*0x34`
   multiply-by-52 shift-add sequence, the `D_8008D99C` load, AND a FRESH
   reload of the `D_8008E978` pointer) independently for each of the two
   accesses. Removing the cached local and writing both accesses as
   textually independent expressions (see the preserved body above, which
   already reflects this) is closer to retail's shape but did not close
   the length gap on its own (293 -> 292 words, i.e. negligible) --
   the dominant residue is (1) above, not this.

## Attempts

Within the 30-attempt cap (3 real builds used):
1. Direct transcription from an m2c-seeded skeleton
   (`tools/m2ctx.py code_179d8_j_b --sig 's32 SpuVmSetVol(s32 a0, s32 a1,
   s32 a2, s32 a3, u16 a4)' --run`), with EVERY divisor/magic-constant
   claim cross-checked against the pinned-pipeline reproducer before
   writing it down (the `/127` signed magic `0x81020409`/shift-6 was
   verified fresh this round: `int probes127(int a){return a/127;}`
   reproduces it exactly; the `/16129` and `/16383` magics were confirmed
   against `SpuVmKeyOnNow`'s own already-published constants for the same
   divisors). Compiled clean on the first try: 293/324 words (31 short).
2. Removed the `idxE968`/`idxE978` local-variable caching in favor of
   repeating the raw `D_8008D998[i].unk0`/`D_8008D99C[i].unk0` expressions
   at each use site (matching residue 3's observation that retail
   recomputes rather than reuses): 293 -> 292/324, a 1-word improvement,
   not the dominant lever.
3. No further structural changes attempted -- residue 1 (whole-function
   register-allocation cascade) is not something either of the two levers
   tried so far reaches, and per this unit's siblings' own experience
   (`SpuVmKeyOnNow`'s report: "the same lever that closed
   `vmNoiseOn`'s frame gap makes this structurally-similar sibling's
   gap WORSE"), guessing at `volatile`/reordering levers without a
   specific hypothesis is unlikely to be productive use of the remaining
   budget for a function this large. Left as a genuine, well-characterized
   stall rather than spending further attempts on unguided search.

### Proposed learning

**A function that reuses every individual construct another function in
the same family has already proven (same divisors, same
struct shapes, same blend cascade, same clamp) can still fail to match
purely on whole-function register allocation, scaled up with function
size.** `SpuVmSetVol` is the largest and most parameter-heavy member of
the SPU-voice-level family this project has now derived three times
(`vmNoiseOn`, `SpuVmKeyOnNow`, this function).

> **HEAD CORRECTION, round 50.** This section as written called
> `vmNoiseOn` an "already-closed function" and read the family as three
> closures. **Neither sibling is matched: both are STALLS**
> (`vmNoiseOn` 309/311, 2 words short; `SpuVmKeyOnNow` 332/316, 16 words
> long), and the head's own assignment brief asserted they were MATCHED --
> the error is the head's, not the runner's. What is true, and is what the
> paragraph's argument actually rests on, is that the family's individual
> CONSTRUCTS are each separately proven; the FUNCTIONS are not closed. This
> strengthens rather than weakens the point below, and adds one: TWO of the
> three (`vmNoiseOn` and this function) now stall with the first real
> diff at the very first instruction. `SpuVmKeyOnNow`'s is at word 30, so
> the pattern is two of three, not three of three -- but two independent
> length-near-exact bodies whose FIRST instruction is already wrong is a
> family-level residue worth attacking as one problem rather than two.

Every one of its
individual pieces was ALREADY proven correct in isolation before this
function was ever attempted, and it still landed 32 words short on first
contact, dominated by register-allocation cascade rather than any
unverified construct. This suggests the family's remaining difficulty
scales with function SIZE (more live values competing for callee-saved
registers) rather than with any per-construct novelty -- worth keeping in
mind when staffing the next attempt at this function or a similarly large
sibling: budget for register-allocation archaeology, not construct
research. **Round 50 addendum: this held for the LENGTH gap specifically,
but not for whether that gap is closeable -- it closed exactly, from a
lever (`volatile`) already on file for a sibling in the same family.**

## Disposition (superseded by round 50 above)

Restored to `INCLUDE_ASM`. See round 50's section at the top of this file
for the current disposition and the next attempt's most promising avenue.

---

# (round 21, superseded) SpuVmSetVol -- STALL (addiu_at/nop_mflo_mfhi blocker, not attempted)

> **REOPENED by round 42, AND SINCE WORKED -- marker spent (head, round 51).**
> This is a DOCUMENTED STALL, not fresh ground: rounds 45 and 50 both worked
> it after the reopening, and the three-figure verdict is the title line at
> the top of this file. Both blockers this section cites are RESOLVED
> (`addiu_at` round 21, `nop_mflo_mfhi` round 42). Everything below is
> evidence from before the fix: kept for history only.

Unit `code_179d8_j (round 21, 2026-09-06)`. **Not attempted at the time.**

## Classification (obsolete -- kept for history)

### `addiu_at`

```sh
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/code_179d8_j/SpuVmSetVol.s
```

Hit, on `D_8008D970`, `D_8008D990`, `D_8008D996`. RESOLVED round 21
(`--addiu-at`).

### `nop_mflo_mfhi`

```sh
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/code_179d8_j/SpuVmSetVol.s \
  | grep -E '\b(mult|multu|div|divu)\b'
```

Hit. RESOLVED round 42 (`--no-nop-mflo-mfhi`). Round 45's actual attempt
confirms empirically -- neither construct appears anywhere in this
report's residue list above.

## Round 70 re-measure (runner charlie): unaffected by `--nop-at-expansion`;
NON_MATCHING promoted, score unchanged

Promotion job (`docs/FINISHING-PLAN.md` track 1b), not a matching session:
measure once, do not iterate. Round 62's body (the "Best-derived body,"
315/324 words) was put LIVE (replacing `INCLUDE_ASM`, no `#ifdef`, both
sibling stalls in this unit left at `INCLUDE_ASM` so their own bytes do
not contaminate this window) and rebuilt through the current toolchain,
which now includes the round-63 `--nop-at-expansion` flag (this function's
title predates that flag but was never attributed to it).

**Reproduces round 62's figures exactly: 315/324 built (9 words short,
`build/lsdde.map`: `SsUtKeyOn` at `0x80030e6c` against retail's
`0x80030e90`), raw word-match 10/324, insertions 76 / deletions 76.** The
flag has no effect here, which is expected: this function's residue is
the GCC 2.6.3 CSE decision on the `D_8008E978[D_8008D99C[i].unk0]` address
(round 62's "Where the remaining 9 words are, exactly" section), not the
below-cc1 load-delay-nop construct the flag targets. Confirms the title is
still current -- no correction needed here, unlike this unit's other two
stalls.

**Disposition: promoted to `#ifdef NON_MATCHING ... #else INCLUDE_ASM
... #endif`** in `src/code_179d8_j_b.c`, using round 62's body (not round
50's -- round 50's 324/324 "length-exact" body is FALSIFIED at ins101/del101,
a worse structural match despite the matching word count; see round 62's
"Step (a)" above). Comment names the current score, verified build
unchanged (`./build-and-verify.sh` green, `tools/check-nonmatching.sh`
green).

NON_MATCHING body promoted, round 70
