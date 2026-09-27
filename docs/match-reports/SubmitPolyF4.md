# SubmitPolyF4 — MATCHED (56/56, round 75): lever = RETURN VALUE (next-packet pointer, arg0 + 0x18) + calls-arm-first branch order

> Renamed from `func_80019B24` on 2026-09-24 (tools/rename.py). Address 0x80019b24.

REVISITED, round 75: MATCHED; names/types used (return type `void *`, matching the caller's own `extern void *SubmitPolyF4(void *prim, void *ctx)` in `code_8220_b.c`).

## ROUND 75 (bravo): MATCHED

Previous title: "SubmitPolyF4 — STALL: length EXACT (56/56 words, no drift); 48/56 raw word-match; first real diff at in-range word 12 (file 0xA354 / vram 0x80019B54), register-identity choice (`$a2` vs `$a1`, duplicated OT mask)".

**Preserved body rebuilt first** (the `#ifdef NON_MATCHING` body compiled
live): 48/56, `insertions 0 / deletions 0 (positional skeleton diffs 8)`.

**Lever, carried unchanged from `SubmitPolyF3` (matched earlier this
round):** the function returns the next packet pointer. The "missing filler"
`addiu $v0, $s1, 0x18` is `return (u8 *)arg0 + 0x18` (0x18 = sizeof(POLY_F4))
in the OT-splice arm; the other arm is `return RCpolyF4(arg0, gDivPolygon4)`
(through a `void *(*)(void *, void *)` cast, because `RCpolyF4` stays `void`
in the shared header). The calls arm must come first:
`if (x != 0) { calls; return RCpolyF4(...); } splice; return arg0 + 0x18;`.
One build: 56/56, whole image `OK: build matches retail`. The `$a2`/`$a1`
mask-register residue went away with it; it was a side effect of the
missing live `$v0`.

See `SubmitPolyF3.md` for the proposed learning (an unread `$v0` write
before the epilogue is the return value).


NON_MATCHING body promoted, round 65.

## ROUND 65 (charlie): NON_MATCHING body promoted

Track 1b mechanical promotion. The standing `#if 0` snapshot in
`src/TmdRenderer.c` (family-shared register-identity residue, hand-derived
per the round-13 HEAD PASS analysis this family shares — see
`SubmitPolyF3.md`) is not a permuter candidate. Wrapped it in
`#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` in place, no bytes
changed. `./build-and-verify.sh`: `build exit=0`, `OK: build matches
retail`. `tools/check-nonmatching.sh TmdRenderer`: `OK`. No stale symbol
references (`RCpolyF4` is already the current name).

## ROUND 48 (bravo): first REAL per-function permuter search — not closed, fifth identical confirmation

Same provenance gap as the other four OT-mask siblings searched this round.
**Check 3:** in-tree rebuild, full oracle: `build exit=2`, zero
compile-error hits, **48/56, identical diff to every prior round**,
`git status --porcelain` empty after revert. Scaffold `--debug
--stack-diffs`: **base score = 260**, same decomposition as the root and
every sibling searched this round. AGREE.

**Search: blind, no PERM macros, `-j 4`, `timeout 600`** (shorter bound —
four prior siblings this round had already converged on the identical
floor+attractor pair by well under 600s of wall time each).
```
permuter rc=124
```
**80126 iterations.** Floor held at 260. Minimum score reached: **215**
(pointer-truncation, two variants) plus **240** (OT-pointer-caching) — the
same two shapes as every other sibling in this round's queue, confirmed
by inspecting `output-*/source.c`. Not translated.

No zero reached. **Not permuter-exhausted — not closed in 80126 iterations
under load (rc=124).** Fifth of six OT-mask-class siblings searched this
round (only `SubmitPolyGT4` remains); fifth identical outcome. Reverted;
`git diff --stat` empty.

## ROUND 44 (alpha): rebuild-reconfirmed, not re-attempted

Flipped the live `#if 0` body to `#if 1`, ran the full oracle in isolation,
reverted: `build exit=2`, zero compile-error hits, **48/56 words, identical
diff to the recorded figure** (first diff word 12, `retail=00ff063c
built=00ff053c`). `git status --porcelain` empty after revert. Same residue
class as `SubmitPolyF3` (see that report for the shared root analysis and
the two already-falsified fix attempts); not re-attempted this round.
`FillDivPolygonHeader` (this unit's OTHER `gp_rel`-reopened sibling, MATCHED 27/27
this round) shares no code path with this residue, so its fix does not
transfer here.

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


## ROUND 40 (alpha): Job 1 rebuild-verify, third independent reproduction — 48/56, byte-identical

Re-ran the Job-1 rebuild independently of round 36 and round 39 (bravo): the
LIVE `#if 0` body in `src/TmdRenderer.c` (round-36 symbol-corrected,
`RCpolyF4` not `func_8001A8D4`) toggled over `INCLUDE_ASM`, full oracle in
isolation, reverted. `build exit=2`, zero compile-error/`undefined
reference` hits.

**Result: 48/56 words, byte-identical to round 36/39's figure. First diff at
word 12 (file 0xA354, vram 0x80019B54): `retail=00ff063c built=00ff053c`** —
same `$a2`/`$a1` family residue as `SubmitPolyF3`. `git diff --stat` empty
after revert.

**Title rebuilt to the three-figure CLAUDE.md format.** Classification
unchanged.

## ROUND 39 (bravo): rebuilt-verified, re-derived against the new `gte.h` macro layer and the hoist-both-before-either lever -- both negative at the family root, unchanged here

Rebuilt this function's preserved body live this round (`#if 1`, `INCLUDE_ASM`
wrapped in `#if 0`), ran the full oracle, reverted: **48/56, byte-identical
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
`func_8001A8D4` -> `RCpolyF4` (a real `libgte` symbol). `src/TmdRenderer.c`'s
preserved `#if 0` snapshot was updated to the new name in that same commit
(`50fd52c`) but never rebuilt, so the `48/56` figure below was carried
forward UNVERIFIED. This round swapped the snapshot in over the
`INCLUDE_ASM`, built (`build exit=2`, no compile error, `RCpolyF4` resolves
against the linked SDK object, no `undefined reference`), and re-ran
`funcdiff.py`.

**Result: 48/56 words, byte-identical to the figure already on record, no
drift warning.** The rename did not disturb the residue.

The corrected, LINKABLE snapshot (identical to what's live in
`src/TmdRenderer.c`):

```c
#if 0
/* ROUND 36 measurement snapshot. Symbol-corrected (round 34 renamed the
 * Psy-Q callee func_8001A8D4 -> RCpolyF4). Rebuilt LIVE and MEASURED:
 * 48/56 words, matching this report's own previously-recorded figure --
 * the rename did not disturb the residue.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Same
 * residue class as SubmitPolyF3 in this unit: $a2 vs $a1 for the OT
 * high-byte mask (cascading register renames), plus one missing
 * load-delay-slot filler `addiu $v0,$s1,0x18` = arg0 + 0x18, which is
 * exactly one byte past uv3 (arg0+0x14, a PolyUV4, the last arg0 field
 * this function's calls branch touches) -- see SubmitPolyF3.md for the
 * verified cross-sibling formula. Not cracked.
 */
void SubmitPolyF4(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        FillDivPolygonHeader(gDivPolygon4, arg1, (u8 *)arg0 + 0x4, 0, 0, 0);
        FillRVectors4((u8 *)arg1 + 0x94, (u8 *)arg1 + 0xA4, (u8 *)arg0 + 0x8,
                      (u8 *)arg0 + 0xC, (u8 *)arg0 + 0x10, (u8 *)arg0 + 0x14);
        RCpolyF4(arg0, gDivPolygon4);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/TmdRenderer", SubmitPolyF4);
```

(`OtTag` and `RCpolyF4`'s prototype come from `include/BMemPMgr.h`, already
included by the unit.)

Unit: `src/TmdRenderer.c`. Quad-flavored sibling of `SubmitPolyF3` (this
unit, also stalled at the identical residue) — same OT-splice-or-calls
structure, `gDivPolygon4` instead of `gDivPolygon3`, `FillRVectors4` (also
stalled this unit, 6-arg quad-flavored copy) instead of `FillRVectors3`,
and `func_8001A8D4` (Psy-Q SDK, `asm/psyq_rcpolyf4.s`, quad-flavored
sibling of `func_8001A564`) instead of `func_8001A564`.

## Best body reached (48/56 words)

```c
> **ROUND 39 (head): THIS PRESERVED BODY WILL NOT LINK AS WRITTEN.**
> Rename(s) needed before it builds: `func_8001A8D4` -> `RCpolyF4`.
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
void SubmitPolyF4(void *arg0, void *arg1)
{
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        *(u32 *)arg0 = (*(u32 *)arg0 & 0xFF000000) | (*(*(u32 **)((u8 *)arg1 + 0x30)) & 0xFFFFFF);

        {
            u32 *head1 = *(u32 **)((u8 *)arg1 + 0x30);

            *head1 = (*head1 & 0xFF000000) | ((u32)arg0 & 0xFFFFFF);
        }
    } else {
        FillDivPolygonHeader(gDivPolygon4, arg1, (u8 *)arg0 + 0x4, 0, 0, 0);
        FillRVectors4((u8 *)arg1 + 0x94, (u8 *)arg1 + 0xA4, (u8 *)arg0 + 0x8, (u8 *)arg0 + 0xC, (u8 *)arg0 + 0x10, (u8 *)arg0 + 0x14);
        func_8001A8D4(arg0, gDivPolygon4);
    }
}
#endif
```

Build compiles clean, no drift (in-range, matches retail's declared `0xE0`
= 56 words). Went in directly using `SubmitPolyF3`'s already-inverted
`if(cond==0){list}else{calls}` layout (no re-derivation needed) and scored
48/56 on the FIRST attempt — the `else` (calls) branch is entirely
byte-exact, confirmed via objdump; every diff is confined to the `if`
(list-splice) branch, and is bit-for-bit the SAME shape as
`SubmitPolyF3`'s residue: the top-byte mask constant (`0xFF000000`) lands
in `$a1` where retail uses `$a2`, and one `addiu $v0,$s1,0x18` (this
function's own dead address computation, analogous to `SubmitPolyF3`'s
`addiu $v0,$s1,0x14`) is absent.

## Why no further attempts were made

This is the identical residue class already exhaustively explored in
`docs/match-reports/SubmitPolyF3.md` (6 attempts there: shared-vs-split
reload locals, named-vs-literal mask constants, inlined-vs-cached
double-dereference — all producing either no change or the same diff).
Since the underlying mechanism (a register-allocator choice apparently
driven by cross-branch pressure from the OTHER arm's call arguments, per
that report's analysis) is structural to the SHAPE of this OT-splice
pattern rather than to this specific function's own text, re-running the
same six experiments here would not produce new information. Filed
directly as a stall to preserve budget for the unit's remaining fresh
functions.

### Proposed learning

**Confirms `SubmitPolyF3`'s residue is a reusable SHAPE, not a one-off**:
the identical `if(flag==0){small OT splice}else{calls}` construct produces
the identical register-identity residue (dead address computation +
misplaced mask-constant register) in a second, independently-written sibling
function. When a future runner meets a third instance of this shape (a
`prim->0x78`-gated OT splice vs. calls-to-quad/triangle-submission-helpers
pattern) elsewhere in this codebase, treat it as PRE-CLASSIFIED: invert the
`if`/`else` layout for the free ~85% match, then stop — don't re-spend the
six-attempt budget `SubmitPolyF3` already paid to establish that nothing
else moves it.

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

`OtTag` is already declared in `include/BMemPMgr.h`.

**So this function is not known to be unreachable, and the analogy that
retired it no longer holds.** The residue's SHAPE was correctly identified as
shared across the family; its reachability was not. Re-attempt with the two
changes above before spending anything on register-level reshaping.

## RUNNER PASS, round 13 continued: applied, instruction-exact, same residue class

Applied both changes with this function's own offsets (quad flavor:
`FillDivPolygonHeader`/`FillRVectors4`/`func_8001A8D4`, table `gDivPolygon4`). One
attempt, 48/56 words, confirmed via `asm-differ` zero-inserted/zero-deleted.
The `else` (calls) branch is byte-exact. Remaining residue, identical class
to `SubmitPolyF3`:

- `$a2` vs `$a1` for the OT high-byte mask (and the cascading renames that
  follow from it).
- One missing `addiu $v0,$s1,0x18` in a load-delay slot. Per
  `SubmitPolyF3.md`'s verified cross-sibling formula, `0x18` = `0x14 + 4`
  = one byte past `uv3` (`arg0 + 0x14`, a `PolyUV4`, `FillRVectors4`'s last
  argument and the last `arg0` field this function's calls branch touches).
  Consistent with the formula on the fourth data point now measured
  independently in this function's own disassembly, not by analogy.

Not re-attempted beyond confirming the formula holds — see
`SubmitPolyF3.md`'s "RUNNER PASS" section for the two hypotheses already
ruled out (cross-branch local forces a 4th saved register and drifts;
branch-local unused local is eliminated by `-O2`) rather than re-running
them here.

## RUNNER PASS, permuter round (alpha, TmdRenderer): base confirmed, cross-reference only

Framing correction: HARD RULE 6 bans the asm/operand-constraint MECHANISM
for register identity, not the outcome of a register differing -- see
`SubmitPolyF3.md`'s new section for the full reasoning. Not "unreachable,
full stop."

`--debug` confirms this function's scaffold scores identically to the
family root: base = **260** (100 insertion + 100 deletion + 12x5 register
diffs). Not independently full-searched this pass; time budget went to a
deep single search on `SubmitPolyF3` (40000 iterations, floor held at
260, two false leads found and falsified against the real oracle) plus
reading `TmdRenderer`'s `SortTmdObject` for `self`'s real type -- see that
report's new section for the trace, and `SubmitPolyGT4.md` for the one
sibling where the caller's own field writes independently confirm the
size formula. Permuter scaffold left provisioned at
`permuter-work/SubmitPolyF4/` (gitignored, session-local).

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
continuing) rather than trusting the title line. Result: **48/56, no
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

`func_80019B24` -> `SubmitPolyF4`, parameters (`arg0`, `arg1`) -> (`prim`, `ctx`). **Tier
A.** TmdRenderer's own comment (`src/TmdRenderer.c`, the "eight submit
wrappers" block) already names this whole family collectively: each is a
tail call to Sony's `RCpolyF4` (unrenamed, RCpoly* polygon-subdivision
family, `libgte`) or a direct OT splice, matching every sibling's shape.
This function's own discriminator: the splice arm returns
`prim + 0x18` = `sizeof(POLY_F4)`, and the calls arm falls straight
into `jal RCpolyF4`. `prim`/`ctx` match the parameter names TmdRenderer's
own `extern void *SubmitPolyF4(void *prim, void *ctx);` view already used.

## Round 91 polish (bravo)

Retyped with Sony's structs like the rest of the family; SubmitPolyF3's
report has the details (POLY_*, DIVPOLYGON3/4, RVECTOR, addPrim, Sony's
RCpoly* prototype, the renamed `gDivPolygon3`/`gDivPolygon4`). Byte-identical
on the first build. The field reads, for this primitive:

`FillDivPolygonHeader(gDivPolygon4, ...)` and `FillRVectors4` over
`ctx->quadVtx`; nothing else. Returns `prim + 1` (0x18).
