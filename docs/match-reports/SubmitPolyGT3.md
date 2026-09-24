# SubmitPolyGT3 — MATCHED (96/96, round 75): lever = RETURN VALUE (next-packet pointer, arg0 + 0x28) + calls-arm-first branch order

> Renamed from `func_80019EE4` on 2026-09-24 (tools/rename.py). Address 0x80019ee4.

REVISITED, round 75: MATCHED; names/types used (return type `void *`, matching the caller's own `extern void *SubmitPolyGT3(void *prim, void *ctx)` in `code_8220_b.c`).

## ROUND 75 (bravo): MATCHED

Previous title: "SubmitPolyGT3 — STALL: length EXACT (96/96 words, no drift); 88/96 raw word-match; first real diff at in-range word 12 (file 0xA714 / vram 0x80019F14), register-identity choice (`$a2` vs `$a1`, duplicated OT mask)".

**Preserved body rebuilt first** (the `#ifdef NON_MATCHING` body compiled live
in place of the `INCLUDE_ASM`): 88/96, `insertions 0 / deletions 0
(opcode-level, built vs retail; positional skeleton diffs 8)`, first diff at
word 12 (`lui $a2` built as `lui $a1`), exactly as the old title says. The eight
diffs are the `$a2`/`$a1` OT-mask register and the missing `addiu $v0,$s1,0x28`.

**Lever: the sibling lever from `SubmitPolyF3` / `SubmitPolyF4` /
`SubmitPolyFT3` / `SubmitPolyG3` / `SubmitPolyFT4`, applied unchanged.** The
function returns the next packet pointer. `N = 0x28` is read off this
function's own disassembly (`addiu $v0, $s1, 0x28` at vram 0x80019F38 in the splice
arm) and agrees with sizeof(POLY_GT3). (tag, then three rgb/xy/uv-clut-tpage triples = 4 + 3*12.) The earlier report's "align_up_4(last touched self field + width)" reading was a coincidence of the same number: the last field touched is the last halfword of the primitive. The other arm falls into the
epilogue after `jal RCpolyGT3`, so its `$v0` is RCpolyGT3's own return.

| body | score |
| --- | --- |
| preserved void body | 88/96 |
| `void *`, `if (!= 0) { calls; stores; return RCpolyGT3(..); } splice; return (u8 *)arg0 + 0x28;` | **96/96** on the first build, whole image `OK: build matches retail` |

The `$a2`/`$a1` residue disappeared on its own once the return existed, as
it did for the five siblings. No search was run; no other variants were
needed. The preserved `self`/`prim` locals and the `Vec2s16` whole-struct
copies (`lwl`/`lwr` + `swl`/`swr`) are kept unchanged from the old body.

**Callers checked:** `func_80018464` in `src/code_8220_b.c` (one call site,
`prim = (u8 *)SubmitPolyGT3(prim, ctx);`), declared there as
`extern void *SubmitPolyGT3(void *prim, void *ctx);`. No other reference in `src/`,
`asm/` or `config/`. The return type agrees. `RCpolyGT3` stays declared `void` in
`include/code_8220.h` and is called through the local cast.

## Earlier history

# (previous title) SubmitPolyGT3 — STALL: length EXACT (96/96 words, no drift); 88/96 raw word-match; first real diff at in-range word 12 (file 0xA714 / vram 0x80019F14), register-identity choice (`$a2` vs `$a1`, duplicated OT mask)

NON_MATCHING body promoted, round 65.

## ROUND 65 (charlie): NON_MATCHING body promoted

Track 1b mechanical promotion. The standing `#if 0` snapshot in
`src/code_8220_c.c` (family-shared register-identity residue, hand-derived
per the round-13 HEAD PASS analysis — see `SubmitPolyF3.md`) is not a
permuter candidate. Wrapped it in `#ifdef NON_MATCHING ... #else
INCLUDE_ASM ... #endif` in place, no bytes changed. `./build-and-verify.sh`:
`build exit=0`, `OK: build matches retail`. `tools/check-nonmatching.sh
code_8220_c`: `OK`. No stale symbol references (`RCpolyGT3` is already the
current name).

## ROUND 48 (bravo): first REAL per-function permuter search — not closed, no new mechanism

Same provenance gap as `SubmitPolyG4` (see that report and the round-48
broadcast/head-audit): this function had never had its own iteration-based
search before this round, only a cross-referenced base-score check against
the family root.

**Check 3:** in-tree rebuild of the preserved body, full oracle: `build
exit=2`, zero compile-error hits, **88/96, identical diff to every prior
round**, `git status --porcelain` empty after revert. Scaffold `--debug
--stack-diffs`: **base score = 260** (1 insertion + 1 deletion + 12×5
register diffs), identical decomposition to the root and to
`SubmitPolyG4`. AGREE — search meaningful.

**Search: blind, no PERM macros, `-j 6`, `timeout 900`.**
```
permuter rc=124
```
(124 read directly off the task's own captured `$?`, not inferred.)
**102364 iterations.** Floor held at 260 for the large majority. Minimum
score reached: **215**, in two independent variants (`output-215-1`:
`char new_var = (u32)self` as the store's own assignment expression;
`output-215-2`: an inline `(unsigned short)((u32)self)` cast), plus the
now-familiar `output-240-1` (caching `*(OtTag**)(prim+0x30)` in one local
for both the read and write). All three are the SAME attractor family
`SubmitPolyG4`'s round-48 search and the root's round-40 search already
identified and oracle-falsified: the 215s truncate the stored pointer
value (wrong VALUE, not just a different register — U.B./coincidental
byte overlap), and the 240 removes a genuine reload and drifts the whole
image. Not translated; not a new mechanism.

No zero reached. **Not permuter-exhausted — not closed in 102364
iterations under load (rc=124).** Third independent confirmation (root,
`SubmitPolyG4`, this function) that these two shapes are the search
space's only local attractors below 260 for this entire residue class.
Reverted; `git diff --stat` empty.

## ROUND 44 (alpha): rebuild-reconfirmed, not re-attempted

Flipped the live `#if 0` body to `#if 1`, ran the full oracle in isolation,
reverted: `build exit=2`, zero compile-error hits, **88/96 words, identical
diff to the recorded figure** (first diff word 12, `retail=00ff063c
built=00ff053c`; missing filler at word 21, `retail=28002226 built=00000000`
= `addiu $v0,$s1,0x28`). `git status --porcelain` empty after revert. Same
residue class as `SubmitPolyF3`; not re-attempted this round.

> **SEARCH PROVENANCE CORRECTED (round 41, head): the permuter figure in this
> report is a SIBLING'S, not this function's.** The "40000 iterations" here is
> a faithful cross-reference to the one deep search round 40 ran on
> `SubmitPolyF3`, the family's root case -- round 40's alpha labelled its own
> section "base confirmed, cross-reference only" and was explicit about it.
> What was validated against THIS function is its SCAFFOLD (`--debug` scores
> it identically to the root), which is a different claim from a search having
> been run on it. **This function has never had a permuter search of its own.**
>
> Why this needed saying: Gate 1b's sixth screen keys on evidence that a search
> RAN -- iteration counts, `rc=`, `permuter-exhausted` -- and an honest
> cross-reference quotes exactly that evidence. So a faithfully-attributed
> sibling figure makes this function read as SEARCHED and spent, which is the
> expensive direction of error (round 37 corrected the same screen once already,
> when it measured vocabulary instead of runs). Six of the nine `code_8220_c`
> RCpoly siblings are in this position; one search covers the family.


## ROUND 40 (alpha): Job 1 rebuild-verify, third independent reproduction — 88/96, byte-identical

Re-ran the Job-1 rebuild independently of round 36 and round 39 (bravo): the
LIVE `#if 0` body in `src/code_8220_c.c` (round-36 symbol-corrected,
`RCpolyGT3` not `func_8001BFD4`) toggled over `INCLUDE_ASM`, full oracle in
isolation, reverted. `build exit=2`, zero compile-error/`undefined
reference` hits.

**Result: 88/96 words, byte-identical to round 36/39's figure. First diff at
word 12 (file 0xA714, vram 0x80019F14): `retail=00ff063c built=00ff053c`** —
same `$a2`/`$a1` family residue as `SubmitPolyF3`. `git diff --stat` empty
after revert.

**Title rebuilt to the three-figure CLAUDE.md format.** Classification
unchanged.

## ROUND 39 (bravo): rebuilt-verified, re-derived against the new `gte.h` macro layer and the hoist-both-before-either lever -- both negative at the family root, unchanged here

Rebuilt this function's preserved body live this round (`#if 1`, `INCLUDE_ASM`
wrapped in `#if 0`), ran the full oracle, reverted: **88/96, byte-identical
diff to every prior round's report, no drift.**

This round's assignment asked whether `include/gte.h` (new, round 38 --
`code_8220_b`'s `TransformAndCullPoly` closed 58/58 by replacing a whole-function
`__asm__` with C over its macros) or the "hoist both values before either is
consumed" lever moved this family. Both were tested concretely on the family
ROOT case (`SubmitPolyF3.md`, "ROUND 39" section) rather than repeated
per-sibling, per the project's own "record a family-wide screen once"
convention (round 20): **neither applies.** The GTE macro layer's domain
(COP2 register transfer) is disjoint from this family's (a GPR-only 24-bit
bitfield write from `LIBGPU.H`'s `addPrim`, confirmed zero GTE/COP2
mnemonics in this function's own disassembly, same as all eight siblings).
The hoist lever's two concrete readings (a fresh local for the reused
pointer value; a hoisted hand-masked form matching retail's own mask-load
order) scored byte-identical-negative and a real regression respectively on
the root case -- see that report for both bodies and scores. This function's
own residue and score are unaffected; no new attempt made here specifically,
since the residue is confirmed structurally identical in position across all
eight siblings (round 21) and both new levers were already falsified at that
shared position.

## ROUND 36: symbol rename verified, rebuilt LIVE, MEASURED (confirms the figure below)

Round 34's SDK-object conversion renamed this function's Psy-Q callee
`func_8001BFD4` -> `RCpolyGT3` (a real `libgte` symbol). `src/code_8220_c.c`'s
preserved `#if 0` snapshot was updated to the new name in that same commit
(`50fd52c`) but never rebuilt, so the `88/96` figure below was carried
forward UNVERIFIED. This round swapped the snapshot in over the
`INCLUDE_ASM`, built (`build exit=2`, no compile error, `RCpolyGT3` resolves
against the linked SDK object, no `undefined reference`), and re-ran
`funcdiff.py`.

**Result: 88/96 words, byte-identical to the figure already on record, no
drift warning.** The rename did not disturb the residue.

The corrected, LINKABLE snapshot (identical to what's live in
`src/code_8220_c.c`):

```c
#if 0
/* A 2-s16 pair (alignment 2, not 4) -- see UpdatePolyBBoxAndCull's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_EE4;

/* ROUND 36 measurement snapshot. Symbol-corrected (round 34 renamed the
 * Psy-Q callee func_8001BFD4 -> RCpolyGT3). Rebuilt LIVE and MEASURED:
 * 88/96 words, matching this report's own previously-recorded figure --
 * the rename did not disturb the residue.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Same
 * residue class as SubmitPolyF3: $a2 vs $a1 for the OT mask, plus one
 * missing `addiu $v0,$s1,0x28`. Matches the align-4-refined cross-sibling
 * formula from SubmitPolyFT4.md: last touched self field is +0x24 (a
 * u16), raw end 0x26, align_up_4(0x26) = 0x28. Not cracked.
 */
void SubmitPolyGT3(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } else {
        FillRCPolyHeader(gPolySubmitTableTri, prim, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        CopyPolyVtx3((PolyVtx **)(prim + 0x88), (PolyVtx **)(prim + 0xA4),
                      (PolyUV4 *)(self + 0x8), (PolyUV4 *)(self + 0x14),
                      (PolyUV4 *)(self + 0x20));

        *(u16 *)(*(u8 **)(prim + 0x88) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u16 *)(self + 0x26);

        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_EE4 *)(self + 0x4);
        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_EE4 *)(self + 0x10);
        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_EE4 *)(self + 0x1C);

        *(u16 *)(*(u8 **)(prim + 0x88) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0x8) = *(u16 *)(self + 0x24);

        RCpolyGT3(self, gPolySubmitTableTri);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_8220_c", SubmitPolyGT3);
```

(`OtTag`, `PolyVtx`, `PolyUV4` and `RCpolyGT3`'s prototype come from
`include/code_8220.h`, already included by the unit.)

Unit: `src/code_8220_c.c`. Seventh sibling of the `SubmitPolyF3` OT-splice-
or-calls family — Gouraud-triangle flavor combining `SubmitPolyFT3`'s
`FillRCPolyHeader` argument shape (`a3=1`, two `u16` stack args) with
`CopyPolyVtx3` (triangle, 3-record output). Calls `func_8001BFD4` (Psy-Q
SDK, `asm/psyq_rcpolygt3.s`).

## Best body reached (88/96 words)

```c
> **ROUND 39 (head): THIS PRESERVED BODY WILL NOT LINK AS WRITTEN.**
> Rename(s) needed before it builds: `func_8001BFD4` -> `RCpolyGT3`.
> The symbol was retargeted when that function became a linked Psy-Q
> SDK object, so the old name no longer exists in this tree. The NAME
> is stale; the residue this body demonstrates usually is not. Correct
> the name and REBUILD before trusting any figure attached to this
> block -- including one quoted in its own heading.
>
> Found by `python3 tools/stalesyms.py`. Note this warning is placed only
> where the stale name appears in CODE: a block whose prose merely
> discusses the rename is fine and is deliberately not marked.

#if 0
/* A 2-s16 pair (alignment 2, not 4) -- see UpdatePolyBBoxAndCull's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_EE4;

void SubmitPolyGT3(void *arg0, void *arg1)
{
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        *(u32 *)self = (*(u32 *)self & 0xFF000000) | (*(*(u32 **)(prim + 0x30)) & 0xFFFFFF);

        {
            u32 *head1 = *(u32 **)(prim + 0x30);

            *head1 = (*head1 & 0xFF000000) | ((u32)self & 0xFFFFFF);
        }
    } else {
        FillRCPolyHeader(gPolySubmitTableTri, prim, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        CopyPolyVtx3(prim + 0x88, prim + 0xA4, self + 0x8, self + 0x14, self + 0x20);

        *(u16 *)(*(u8 **)(prim + 0x88) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u16 *)(self + 0x26);

        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_EE4 *)(self + 0x4);
        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_EE4 *)(self + 0x10);
        *(Vec2s16_EE4 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_EE4 *)(self + 0x1C);

        *(u16 *)(*(u8 **)(prim + 0x88) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0x8) = *(u16 *)(self + 0x24);

        func_8001BFD4(self, gPolySubmitTableTri);
    }
}
#endif
```

Byte-exact calls branch on the first attempt.

## Residue

Identical class to the six earlier siblings, confined to the `if`
(list-splice) branch: top-byte mask constant in `$a1` instead of `$a2`,
one dead `addiu $v0,$s1,0x28` absent. Not re-investigated; see
`SubmitPolyF3.md`. Filed directly as a stall.

### Proposed learning

Seventh confirmed instance; no new information.

## HEAD NOTE, round 13: this function's inherited cause is SUPERSEDED

This report classified its residue by analogy to `SubmitPolyF3`, the family's
root case, which was filed as an unreachable register-identity residue. **That
root classification has been superseded** — see
`docs/match-reports/SubmitPolyF3.md`, "HEAD PASS". The head reached the
correct instruction count and length there by changing two things, both of
which apply to this function too:

1. The OT splice is a **24-bit bitfield write** (`OtTag.addr`, a local minimal
   view of Psy-Q's `P_TAG`), not hand-written `& 0xFF000000` / `& 0x00FFFFFF`
   masking. Same value, different register allocation — which is exactly what
   made the residue look like an unreachable register choice.
2. The OT expression must be **re-evaluated, not cached in a local**. Retail
   re-reads it before the second store, because `addPrim(ot, p)` expands `ot`
   twice.

`OtTag` is already declared in `include/code_8220.h`.

**So this function is not known to be unreachable, and the analogy that
retired it no longer holds.** The residue's SHAPE was correctly identified as
shared across the family; its reachability was not. Re-attempt with the two
changes above before spending anything on register-level reshaping.

## RUNNER PASS, round 13 continued: applied, instruction-exact, confirms the align-4 formula

Applied both changes with this function's own offsets. One attempt, 88/96
words, confirmed via `asm-differ` zero-inserted/zero-deleted. No self/prim
swap here. Remaining residue: `$a2`/`$a1` on the OT mask plus one missing
`addiu $v0,$s1,0x28`, which independently confirms `SubmitPolyFT4.md`'s
align-4 refinement: last touched `self` field is `+0x24` (`u16`, raw end
`0x26`), and `align_up_4(0x26) = 0x28` matches exactly. Not independently
re-attempted; see `SubmitPolyF3.md`/`SubmitPolyFT4.md` for the ruled-out
hypotheses.

## RUNNER PASS, permuter round (alpha, code_8220_c): base confirmed, cross-reference only

Framing correction: HARD RULE 6 bans the asm/operand-constraint MECHANISM
for register identity, not the outcome of a register differing -- see
`SubmitPolyF3.md`'s new section for the full reasoning. Not "unreachable,
full stop."

`--debug` confirms this function's scaffold scores identically to the
family root: base = **260** (100 insertion + 100 deletion + 12x5 register
diffs). Not independently full-searched this pass; time budget went to a
deep single search on `SubmitPolyF3` (40000 iterations, floor held at
260, two false leads found and falsified against the real oracle) plus
reading `code_8220_b`'s `func_80018464` for `self`'s real type -- see that
report's new section for the trace, and `SubmitPolyGT4.md` for the one
sibling where the caller's own field writes independently confirm the
size formula. Permuter scaffold left provisioned at
`permuter-work/SubmitPolyGT3/` (gitignored, session-local).

## ROUND 20 note

COP2/GTE-clobber-trap hypothesis tested against this function directly (its
own disassembly grepped for every GTE/COP2 mnemonic: zero hits) as part of
a whole-family screen — falsified for all ten functions in this unit's
work list, not just this one. Full method and family-wide result in
`SubmitPolyF3.md`'s "ROUND 20" section. This function's own residue
remains the register-identity + code-motion-filler class already
documented above, unaffected by this screen.

## ROUND 21 note

Cross-sibling grep (`SubmitPolyF3.md`, "ROUND 21" section) confirms this
function's mask-constant `lui $a2` sits at the identical disassembly line
(19) as all seven other siblings, and its own delay-slot filler satisfies the
same align-4 formula. The `__asm__("")` barrier lever was evaluated against
round 20's own discriminator and ruled out for both components of this
residue class (register choice; absent, not misordered, instruction) — see
that section for the full reasoning, not repeated per sibling. No new lead
for this function specifically; residue and score unchanged.

## ROUND 32 note (runner charlie)

Re-verified this function's preserved body directly against the live oracle
this round (flipped to `#if 1`/inserted the report's own preserved source in
place of `INCLUDE_ASM`, ran `./build-and-verify.sh` + `funcdiff.py`, then
reverted and re-confirmed `git diff --stat` clean and `build exit=0` before
continuing) rather than trusting the title line. Result: **88/96, no
stale-build or drift warning, identical diff shape to every prior round's
report.** Confirmed clean, not drifted.

Read this family's full round-13/19/20/21 history (see `SubmitPolyF3.md`
for the shared root analysis) before attempting further reshaping this
round: the register-identity + code-motion-filler residue has already been
tested against six-plus independent axes (masking expression, reload/cache
count, per-reload local scoping, constant naming, signed/unsigned bitfield,
widened-copy intermediate, concrete struct typing for both operands, the
aggregate-assignment lever, the COP2/GTE-clobber-trap hypothesis, and a
40000-iteration blind permuter search on the root case) and confirmed a
genuine wall each time, most recently re-confirmed by the head in round 31.
Per CLAUDE.md's explicit instruction this round, the GTE/COP2-exception
hypothesis was NOT re-run a third time. No new mechanism found or attempted
against this specific residue this round; time went to the LEAD TASK
(`_card_clear`, `class_3bb8c_v.c`) instead, which surfaced a genuinely new
finding (a `li`-expansion ADDIU-vs-ORI encoding invisible to `asm-differ`/the
permuter's own scorer, see that function's own report) — checked whether
that class of hidden residue could explain any of this family's own diff
words: it does not, since every diff word in this family's own funcdiff
output changes a REGISTER FIELD (e.g. `$a1` vs `$a2` in an otherwise-identical
`lui`/`ori` instruction), not just an opcode's top bits with the same
register operand -- these are the genuine, already-diagnosed register-identity
residue, not a second hidden instance of the ADDIU/ORI blind spot.
