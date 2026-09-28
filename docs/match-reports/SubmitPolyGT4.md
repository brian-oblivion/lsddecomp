# SubmitPolyGT4 — MATCHED (112/112, round 75): lever = RETURN VALUE (next-packet pointer, arg0 + 0x34) + calls-arm-first branch order

> Renamed from `func_8001A064` on 2026-09-24 (tools/rename.py). Address 0x8001a064.

REVISITED, round 75: MATCHED; names/types used (return type `void *`, matching the caller's own `extern void *SubmitPolyGT4(void *prim, void *ctx)` in `code_8220_b.c`).

## ROUND 75 (bravo): MATCHED

Previous title: "SubmitPolyGT4 — STALL: length EXACT (112/112 words, no drift); 104/112 raw word-match; first real diff at in-range word 12 (file 0xA894 / vram 0x8001A094), register-identity choice (`$a2` vs `$a1`, duplicated OT mask), plus a SEPARATE absent-instruction residue at word 21 (`retail=34002226 built=00000000`, `addiu $v0,$s1,0x34`)".

**Preserved body rebuilt first** (the `#ifdef NON_MATCHING` body compiled live
in place of the `INCLUDE_ASM`): 104/112, `insertions 0 / deletions 0
(opcode-level, built vs retail; positional skeleton diffs 8)`, first diff at
word 12 (`lui $a2` built as `lui $a1`), exactly as the old title says. The eight
diffs are the `$a2`/`$a1` OT-mask register and the missing `addiu $v0,$s1,0x34`.

**Lever: the sibling lever from `SubmitPolyF3` / `SubmitPolyF4` /
`SubmitPolyFT3` / `SubmitPolyG3` / `SubmitPolyFT4`, applied unchanged.** The
function returns the next packet pointer. `N = 0x34` is read off this
function's own disassembly (`addiu $v0, $s1, 0x34` at vram 0x8001A0B8 in the splice
arm) and agrees with sizeof(POLY_GT4). (tag, then four rgb/xy/uv-clut-tpage triples = 4 + 4*12.) The old title's "SEPARATE absent-instruction residue at word 21" was this same instruction: it is the return value, not a separate residue, and the earlier "align_up_4(last touched self field + width)" reading was a coincidence of the same number. The other arm falls into the
epilogue after `jal RCpolyGT4`, so its `$v0` is RCpolyGT4's own return.

| body | score |
| --- | --- |
| preserved void body | 104/112 |
| `void *`, `if (!= 0) { calls; stores; return RCpolyGT4(..); } splice; return (u8 *)arg0 + 0x34;` | **112/112** on the first build, whole image `OK: build matches retail` |

The `$a2`/`$a1` residue disappeared on its own once the return existed, as
it did for the five siblings. No search was run; no other variants were
needed. The preserved `self`/`prim` locals and the `Vec2s16` whole-struct
copies (`lwl`/`lwr` + `swl`/`swr`) are kept unchanged from the old body.

**Callers checked:** `SortTmdObject` in `src/graphics/TmdRenderer.c` (one call site,
`prim = (u8 *)SubmitPolyGT4(prim, ctx);`), declared there as
`extern void *SubmitPolyGT4(void *prim, void *ctx);`. No other reference in `src/`,
`asm/` or `config/`. The return type agrees. `RCpolyGT4` stays declared `void` in
`include/code_8220.h` and is called through the local cast.

## Earlier history

# (previous title) SubmitPolyGT4 — STALL: length EXACT (112/112 words, no drift); 104/112 raw word-match; first real diff at in-range word 12 (file 0xA894 / vram 0x8001A094), register-identity choice (`$a2` vs `$a1`, duplicated OT mask), plus a SEPARATE absent-instruction residue at word 21 (`retail=34002226 built=00000000`, `addiu $v0,$s1,0x34`)

NON_MATCHING body promoted, round 65.

## ROUND 65 (charlie): NON_MATCHING body promoted

Track 1b mechanical promotion. The standing `#if 0` snapshot in
`src/graphics/TmdRenderer.c` (family-shared register-identity residue plus the
missing delay-slot filler, hand-derived per the round-13 HEAD PASS
analysis — see `SubmitPolyF3.md`) is not a permuter candidate. Wrapped it
in `#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` in place, no
bytes changed. `./build-and-verify.sh`: `build exit=0`, `OK: build matches
retail`. `tools/check-nonmatching.sh TmdRenderer`: `OK`. No stale symbol
references (`RCpolyGT4` is already the current name). This was the last of
the nine functions in this round's track 1b assignment for this unit — all
nine now have a NON_MATCHING body.

## ROUND 48 (bravo): first REAL per-function permuter search — not closed, sixth-of-six identical confirmation, family search complete

My brief listed this function's prior search as "—" (unmeasured); the
actual figure is 104/112, and like its five `TmdRenderer` OT-mask siblings
it had never had its own iteration-based permuter search — only the
family root's cross-referenced base score.

**Check 3:** in-tree rebuild of the preserved body, full oracle: `build
exit=2`, zero compile-error hits, **104/112, identical diff to every
prior round**, `git status --porcelain` empty after revert. Scaffold
`--debug --stack-diffs`: **base score = 260**, identical decomposition to
the root and every other sibling searched this round. AGREE.

**Search: blind, no PERM macros, `-j 4`, `timeout 600`.**
```
permuter rc=124
```
**59969 iterations.** Floor held at 260. Minimum score reached: **215**
(pointer-truncation, two variants) plus **240** (OT-pointer-caching) — the
same two shapes every other sibling produced this round. Not translated.

No zero reached. **Not permuter-exhausted — not closed in 59969 iterations
under load (rc=124).**

**This closes the family-wide search this round.** All six OT-mask
siblings that had never had their own bounded permuter search
(`SubmitPolyG4` 74830 iters, `SubmitPolyGT3` 102364 iters, `SubmitPolyFT4`
78126 iters, `SubmitPolyFT3` 76458 iters, `SubmitPolyF4` 80126 iters,
this function 59969 iters — **471873 iterations combined, all rc=124, all
under load**) now do, and every single one converged on the identical pair
of sub-260 attractors and nothing else. Combined with the family root's
own 40000-iteration search (round 13, same attractors at a coarser
resolution) and `SubmitPolyG3`'s round 48 search (see that report — a
different, additionally-stacked residue), this is as close to an
exhaustive characterization of this residue class's local search space as
blind permuter search gets: **the OT-pointer-caching (240) and
pointer-truncation (215) shapes are the only local minima below the
260-point register-identity/code-motion-filler floor, for every one of the
eight `TmdRenderer` RCpoly siblings that carries this residue, regardless
of which one's own disassembly seeds the search.** Both are semantically
wrong (a real reload elided, or a genuine byte truncated) and both were
independently oracle-falsified when hand-applied (`SubmitPolyF3.md`,
round 13/permuter-round sections). This is a stall by HARD RULE 6 (a
register-identity mismatch), not a source-shape gap — see that rule's own
`SubmitPolyF3.md`-cited discussion for why permuter search on this
residue class is still legitimate (it targets the OUTCOME via ordinary C
reshaping, not the banned MECHANISM), and why it is now spent for this
family absent a genuinely new structural hypothesis.

This function's own SECOND residue (the absent `addiu $v0,$s1,0x34`
filler at word 21) is unaffected by any of the above — see
`docs/DECOMPILATION_LEARNINGS.md`'s "an absent-instruction residue needs a
source-shape change that makes the compiler COMPUTE the missing value;
there is no scheduling lever for it" entry (measured directly on this
function, round 21). No barrier or permuter mutation over this seed moved
that residue either, consistent with that finding. Reverted;
`git diff --stat` empty.

### Proposed learning

**A residue class's local-search-space characterization generalises across
siblings that share the class, and doing so is CHEAPER than searching each
in isolation would suggest.** Six independent 60-100k-iteration searches
across six different functions (different sizes, different instruction
counts, different surrounding code) all converged on the exact same two
named C-level shapes. That is not six separate negative results — it is
one result, replicated six times, which is strong evidence the attractor
pair is intrinsic to the shared residue (the `addPrim` macro's OT-mask
re-evaluation + code-motion filler), not an accident of any one function's
own compiled bytes. For the next runner who finds a residue matching this
shape (`$aX`/`$aY` duplicated-mask substitution, base score decomposing as
100+100+12×5), the two shapes documented here are worth checking against
FIRST, by inspection, before spending a fresh 900s search — a permuter run
on a structurally identical residue is very likely to rediscover the same
two false leads rather than something new.

## ROUND 44 (alpha): rebuild-reconfirmed, not re-attempted

Flipped the live `#if 0` body to `#if 1`, ran the full oracle in isolation,
reverted: `build exit=2`, zero compile-error hits, **104/112 words, identical
diff to the recorded figure** (first diff word 12, `retail=00ff063c
built=00ff053c`; missing filler still word 21, `retail=34002226
built=00000000` = `addiu $v0,$s1,0x34`). `git status --porcelain` empty
after revert. Same residue class as `SubmitPolyF3`; not re-attempted this
round.

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
> when it measured vocabulary instead of runs). Six of the nine `TmdRenderer`
> RCpoly siblings are in this position; one search covers the family.


## ROUND 40 (alpha): Job 1 rebuild-verify, third independent reproduction — 104/112, byte-identical

Re-ran the Job-1 rebuild independently of round 36 and round 39 (bravo): the
LIVE `#if 0` body in `src/graphics/TmdRenderer.c` (round-36 symbol-corrected,
`RCpolyGT4` not `func_8001C474`) toggled over `INCLUDE_ASM`, full oracle in
isolation, reverted. `build exit=2`, zero compile-error/`undefined
reference` hits.

**Result: 104/112 words, byte-identical to round 36/39's figure. First diff
at word 12 (file 0xA894, vram 0x8001A094): `retail=00ff063c built=00ff053c`**
— same `$a2`/`$a1` family residue as `SubmitPolyF3`. `git diff --stat`
empty after revert. Per round 21's `DECOMPILATION_LEARNINGS.md` measurement
on this exact function, a SECOND, distinct residue sits at word 21 — an
ABSENT instruction (`retail=34002226 built=00000000`, retail's
`addiu $v0,$s1,0x34`), which the discriminator table there marks as NOT a
barrier candidate (no scheduling lever creates an instruction from nothing;
it needs a source-level computation of the missing value).

**Title rebuilt to the three-figure CLAUDE.md format**, naming both residues
since the "first diff" figure alone would otherwise hide the second,
already-characterized one. Classification unchanged.

## ROUND 39 (bravo): rebuilt-verified, re-derived against the new `gte.h` macro layer and the hoist-both-before-either lever -- both negative at the family root, unchanged here

Rebuilt this function's preserved body live this round (`#if 1`, `INCLUDE_ASM`
wrapped in `#if 0`), ran the full oracle, reverted: **104/112, byte-identical
diff to every prior round's report, no drift.**

This round's assignment asked whether `include/gte.h` (new, round 38 --
`TmdRenderer`'s `TransformAndCullPoly` closed 58/58 by replacing a whole-function
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
`func_8001C474` -> `RCpolyGT4` (a real `libgte` symbol). `src/graphics/TmdRenderer.c`'s
preserved `#if 0` snapshot was updated to the new name in that same commit
(`50fd52c`) but never rebuilt, so the `104/112` figure below was carried
forward UNVERIFIED. This round swapped the snapshot in over the
`INCLUDE_ASM`, built (`build exit=2`, no compile error, `RCpolyGT4` resolves
against the linked SDK object, no `undefined reference`), and re-ran
`funcdiff.py`.

**Result: 104/112 words, byte-identical to the figure already on record, no
drift warning.** The rename did not disturb the residue.

The corrected, LINKABLE snapshot (identical to what's live in
`src/graphics/TmdRenderer.c`):

```c
#if 0
/* A 2-s16 pair (alignment 2, not 4) -- see FlagLargePolyForDivide's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_A64;

/* ROUND 36 measurement snapshot. Symbol-corrected (round 34 renamed the
 * Psy-Q callee func_8001C474 -> RCpolyGT4). Rebuilt LIVE and MEASURED:
 * 104/112 words, matching this report's own previously-recorded figure --
 * the rename did not disturb the residue.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Same
 * residue class: $a2 vs $a1 for the OT mask, plus one missing
 * `addiu $v0,$s1,0x34`. Confirms the align-4-refined cross-sibling
 * formula a third time: last touched self field is +0x30 (a u16), raw
 * end 0x32, align_up_4(0x32) = 0x34. Not cracked.
 */
void SubmitPolyGT4(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } else {
        FillDivPolygonHeader(gDivPolygon4, prim, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        FillRVectors4(prim + 0x94, prim + 0xA4, self + 0x8, self + 0x14,
                      self + 0x20, self + 0x2C);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0xA) = *(u16 *)(self + 0x32);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0xA) = *(u16 *)(self + 0x32);

        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x94) + 0xC) = *(Vec2s16_A64 *)(self + 0x4);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x98) + 0xC) = *(Vec2s16_A64 *)(self + 0x10);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x9C) + 0xC) = *(Vec2s16_A64 *)(self + 0x1C);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0xA0) + 0xC) = *(Vec2s16_A64 *)(self + 0x28);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0x8) = *(u16 *)(self + 0x24);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0x8) = *(u16 *)(self + 0x30);

        RCpolyGT4(self, gDivPolygon4);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/TmdRenderer", SubmitPolyGT4);
```

(`OtTag` and `RCpolyGT4`'s prototype come from `include/code_8220.h`,
already included by the unit.)

Unit: `src/graphics/TmdRenderer.c`. Eighth and final sibling of the
`SubmitPolyF3` OT-splice-or-calls family, and the largest (Gouraud-quad
flavor: `FillDivPolygonHeader` with `a3=1`, `FillRVectors4` quad copy, 4 output
records, two full passes of `u16` widen-stores). Calls `func_8001C474`
(Psy-Q SDK, same `asm/psyq_rcpolygt3.s` file as `func_8001BFD4`).

## Best body reached (104/112 words)

```c
> **ROUND 39 (head): THIS PRESERVED BODY WILL NOT LINK AS WRITTEN.**
> Rename(s) needed before it builds: `func_8001C474` -> `RCpolyGT4`.
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
/* A 2-s16 pair (alignment 2, not 4) -- see FlagLargePolyForDivide's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_A64;

void SubmitPolyGT4(void *arg0, void *arg1)
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
        FillDivPolygonHeader(gDivPolygon4, prim, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x1A));
        FillRVectors4(prim + 0x94, prim + 0xA4, self + 0x8, self + 0x14, self + 0x20, self + 0x2C);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0xA) = *(u16 *)(self + 0x26);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0xA) = *(u16 *)(self + 0x32);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0xA) = *(u16 *)(self + 0x32);

        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x94) + 0xC) = *(Vec2s16_A64 *)(self + 0x4);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x98) + 0xC) = *(Vec2s16_A64 *)(self + 0x10);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0x9C) + 0xC) = *(Vec2s16_A64 *)(self + 0x1C);
        *(Vec2s16_A64 *)(*(u8 **)(prim + 0xA0) + 0xC) = *(Vec2s16_A64 *)(self + 0x28);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0x8) = *(u16 *)(self + 0x18);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0x8) = *(u16 *)(self + 0x24);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0x8) = *(u16 *)(self + 0x30);

        func_8001C474(self, gDivPolygon4);
    }
}
#endif
```

Byte-exact calls branch on the first attempt, at the family's largest
scale yet — the self/prim swap `SubmitPolyG3` hit never recurred across
any of the six other siblings including this one, confirming it is a
narrow, not-yet-precisely-characterized condition rather than a simple
function-size or self-usage-count threshold.

## Residue

Identical class to all seven earlier siblings, confined to the `if`
(list-splice) branch: top-byte mask constant in `$a1` instead of `$a2`,
one dead `addiu $v0,$s1,0x34` absent. Not re-investigated; see
`SubmitPolyF3.md`. Filed directly as a stall.

### Proposed learning

Eighth and final confirmed instance in this unit's queue. This closes out
the `SubmitPolyF3` OT-splice-or-calls family for `TmdRenderer`: all eight
siblings (`SubmitPolyF3`, `SubmitPolyF4`, `SubmitPolyG3`,
`SubmitPolyFT3`, `SubmitPolyG4`, `SubmitPolyFT4`, `SubmitPolyGT3`,
`SubmitPolyGT4`) share the identical list-branch register-identity
residue and none of the calls-branch variation (triangle/quad output count,
which Psy-Q submission routine, flat vs. Gouraud argument shape) affects
it. A future runner who finds a NINTH such function elsewhere in the
executable should expect the same outcome without re-deriving it.

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

## RUNNER PASS, round 13 continued: applied, instruction-exact, confirms the align-4 formula a third time

Applied both changes with this function's own offsets (largest sibling,
Gouraud quad). One attempt, 104/112 words, confirmed via `asm-differ`
zero-inserted/zero-deleted. No self/prim swap. Remaining residue:
`$a2`/`$a1` on the OT mask plus one missing `addiu $v0,$s1,0x34` — matches
`SubmitPolyFT4.md`'s align-4-refined formula exactly (last touched `self`
field is `+0x30`, a `u16`, raw end `0x32`, `align_up_4(0x32) = 0x34`). This
is now the THIRD independent confirmation of the align-4 refinement
(`SubmitPolyFT4` itself, `SubmitPolyGT3`, and this function). Not
independently re-attempted.

## RUNNER PASS, permuter round (alpha, TmdRenderer): base confirmed, cross-reference only

**Framing correction (applies here too):** HARD RULE 6 bans the
`register T v asm("$N")`/operand-constraint MECHANISM for fixing register
identity, not the OUTCOME of a register differing. Ordinary C reshaping that
happens to land the allocator on retail's register is not a rule
violation — see `SubmitPolyF3.md`'s equivalent correction for the full
reasoning. This function's residue is not "unreachable, full stop"; it is
"not yet closed by the reshapings tried so far."

`--debug` confirms this function's own scaffold scores identically to the
family root: base = **260** (100 insertion + 100 deletion + 12x5 register
diffs), matching `SubmitPolyF3`'s decomposition exactly with this
function's own offsets. Not independently full-searched this pass -- time
budget went to a deep single search on the root case
(`SubmitPolyF3`, 40000 iterations, floor held at 260, two false leads
found and falsified against the real oracle -- see that report) plus reading
`TmdRenderer`'s `SortTmdObject` for `self`'s real type.

**This function is the one `SortTmdObject` independently confirms the size
formula on.** Immediately before the `jal SubmitPolyGT4` call site,
`SortTmdObject` GTE-stores four vertex-color records into `self` at
`+0x4`, `+0x10`, `+0x1C`, `+0x28` (stride `0xC`, 4 records) -- one past the
last record is `0x28+0xC=0x34`, exactly this function's filler
(`addiu $v0,$s1,0x34`). This is a real, independent confirmation of the
`align_up_4(...)` formula from the CALLER's own field writes, not a
curve-fit from this function's own reads -- see `SubmitPolyF3.md`'s new
section for the full trace and for why it does NOT yet explain the missing
C-level mention (no `self+1`/`&self[1]`-shaped expression exists anywhere
in `SortTmdObject` either; it loops over its own cursors, not over `self`).

Permuter scaffold left provisioned at `permuter-work/SubmitPolyGT4/`
(gitignored, session-local) for a future targeted (PERM-macro) run.

## ROUND 20 note

COP2/GTE-clobber-trap hypothesis tested against this function directly (its
own disassembly grepped for every GTE/COP2 mnemonic: zero hits) as part of
a whole-family screen — falsified for all ten functions in this unit's
work list, not just this one. Full method and family-wide result in
`SubmitPolyF3.md`'s "ROUND 20" section. This function's own residue
remains the register-identity + code-motion-filler class already
documented above, unaffected by this screen.

**Also: `FillRVectors4` (this function's own `FillRVectors4` call, described
above/in this report's earlier sections as "also stalled this unit") is now
MATCHED (35/35), round 20 — see `FillRVectors4.md`. Its stall was a wrong
SOURCE SHAPE (an unnecessary whole-function raw-register `__asm__`
transcription of what turned out to be ordinary struct-copy C, one array
element past what `FillRVectors3` already handles), not a residue of this
family's own register-identity/delay-slot-filler class. This function's own
remaining residue is unaffected — `FillRVectors4` is called via ordinary
`jal`, and the whole-image build is `build exit=0` after the retype, so
nothing about this function's own call site needed to change.**

## ROUND 21 note

Independently re-derived this function's 104/112 from scratch (not by
trusting this title line or the round-20 spot-check): flipped this
function's preserved body to `#if 1`, commented out its own `INCLUDE_ASM`,
ran the full oracle. Result: `SubmitPolyGT4: 104/112 words match`, no
stale-build or drift warning, identical diff shape (word 12: `$a2`→`$a1`
mask swap; word 21: missing `addiu $v0,$s1,0x34`). Confirmed clean, not
drifted. Reverted both edits and re-confirmed `build exit=0` before
continuing. This function was chosen as the family's largest and the round's
designated "closest-to-passing" figure to rebuild before trusting any other
sibling's title line — it holds exactly.

Full family-wide cross-sibling grep (confirming `$a2` at the identical
disassembly line in all eight siblings, and the delay-slot filler formula
holding on all eight) and the `__asm__("")` barrier-lever evaluation (ruled
out on both of this residue's components, for reasons specific to each) are
recorded once, at the family root, in `SubmitPolyF3.md`'s "ROUND 21"
section. No new lead for this function specifically; residue and score
unchanged.

## ROUND 32 note (runner charlie)

Re-verified this function's preserved body directly against the live oracle
this round (flipped to `#if 1`/inserted the report's own preserved source in
place of `INCLUDE_ASM`, ran `./build-and-verify.sh` + `funcdiff.py`, then
reverted and re-confirmed `git diff --stat` clean and `build exit=0` before
continuing) rather than trusting the title line. Result: **104/112, no
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
(`_card_clear`, `libcard_card.c`) instead, which surfaced a genuinely new
finding (a `li`-expansion ADDIU-vs-ORI encoding invisible to `asm-differ`/the
permuter's own scorer, see that function's own report) — checked whether
that class of hidden residue could explain any of this family's own diff
words: it does not, since every diff word in this family's own funcdiff
output changes a REGISTER FIELD (e.g. `$a1` vs `$a2` in an otherwise-identical
`lui`/`ori` instruction), not just an opcode's top bits with the same
register operand -- these are the genuine, already-diagnosed register-identity
residue, not a second hidden instance of the ADDIU/ORI blind spot.

## Naming (round 77, alpha)

`func_8001A064` -> `SubmitPolyGT4`, parameters (`arg0`, `arg1`) -> (`prim`, `ctx`). **Tier
A.** TmdRenderer's own comment (`src/graphics/TmdRenderer.c`, the "eight submit
wrappers" block) already names this whole family collectively: each is a
tail call to Sony's `RCpolyGT4` (unrenamed, RCpoly* polygon-subdivision
family, `libgte`) or a direct OT splice, matching every sibling's shape.
This function's own discriminator: the splice arm returns
`prim + 0x34` = `sizeof(POLY_GT4)`, and the calls arm falls straight
into `jal RCpolyGT4`. `prim`/`ctx` match the parameter names TmdRenderer's
own `extern void *SubmitPolyGT4(void *prim, void *ctx);` view already used.

## Round 91 polish (bravo)

Retyped with Sony's structs like the rest of the family; SubmitPolyF3's
report has the details (POLY_*, DIVPOLYGON3/4, RVECTOR, addPrim, Sony's
RCpoly* prototype, the renamed `gDivPolygon3`/`gDivPolygon4`). Byte-identical
on the first build. The field reads, for this primitive:

`clut`/`tpage` to the header; RVECTOR `pad` from `pad2`, `pad2`, `pad3`,
`pad3`; `c` from `r0`..`r3`; `uv` from `u0v0`..`u3v3`. Returns `prim + 1`
(0x34).
