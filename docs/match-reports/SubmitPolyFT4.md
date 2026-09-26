# SubmitPolyFT4 — MATCHED (88/88, round 75): lever = RETURN VALUE (next-packet pointer, arg0 + 0x28) + calls-arm-first branch order

> Renamed from `func_80019D84` on 2026-09-24 (tools/rename.py). Address 0x80019d84.

REVISITED, round 75: MATCHED; names/types used (return type `void *`, matching the caller's own `extern void *SubmitPolyFT4(void *prim, void *ctx)` in `code_8220_b.c`).

## ROUND 75 (bravo): MATCHED

Previous title: "SubmitPolyFT4 — STALL: length EXACT (88/88 words, no drift); 80/88 raw word-match; first real diff at in-range word 12 (file 0xA5B4 / vram 0x80019DB4), register-identity choice (`$a2` vs `$a1`, duplicated OT mask)".

**Preserved body rebuilt first** (the `#ifdef NON_MATCHING` body compiled live
in place of the `INCLUDE_ASM`): 80/88, `insertions 0 / deletions 0
(opcode-level, built vs retail; positional skeleton diffs 8)`. The eight diffs
are the `$a2`/`$a1` OT-mask register and the missing `addiu $v0,$s1,0x28`.

**Lever: the sibling lever from `SubmitPolyF3` / `SubmitPolyF4`, applied
unchanged.** The function returns the next packet pointer. `N = 0x28` is read
off this function's own disassembly (`addiu $v0, $s1, 0x28` at vram
0x80019DD8 in the splice arm) and agrees with sizeof(POLY_FT4) (tag, rgbc,
four xy/uv pairs with clut and tpage = 8 + 4*8). The earlier report's
"align_up_4(last touched self field + width)" reading was a coincidence of
the same number: 0x24 + 2 rounded up is 0x28 because the last field is the
last halfword of the primitive. The other arm falls into the epilogue after
`jal RCpolyFT4`, so its `$v0` is RCpolyFT4's own return.

| body | score |
| --- | --- |
| preserved void body | 80/88 |
| `void *`, `if (!= 0) { calls; stores; return RCpolyFT4(..); } splice; return arg0 + 0x28;` | **88/88** on the first build, whole image `OK: build matches retail` |

**Callers checked:** `func_80018464` in `src/code_8220_b.c` (two call sites,
`prim = (u8 *)SubmitPolyFT4(prim, ctx);`), declared there as
`extern void *SubmitPolyFT4(void *prim, void *ctx);`. The return type agrees.
`RCpolyFT4` stays declared `void` in `include/code_8220.h` and is called
through the local cast.

## Earlier history

# (previous title) # SubmitPolyFT4 — STALL: length EXACT (88/88 words, no drift); 80/88 raw word-match; first real diff at in-range word 12 (file 0xA5B4 / vram 0x80019DB4), register-identity choice (`$a2` vs `$a1`, duplicated OT mask)

NON_MATCHING body promoted, round 65.

## ROUND 65 (charlie): NON_MATCHING body promoted

Track 1b mechanical promotion. The standing `#if 0` snapshot in
`src/code_8220_c.c` (family-shared register-identity residue, hand-derived
per the round-13 HEAD PASS analysis — see `SubmitPolyF3.md`) is not a
permuter candidate. Wrapped it in `#ifdef NON_MATCHING ... #else
INCLUDE_ASM ... #endif` in place, no bytes changed. `./build-and-verify.sh`:
`build exit=0`, `OK: build matches retail`. `tools/check-nonmatching.sh
code_8220_c`: `OK`. No stale symbol references (`RCpolyFT4` is already the
current name).

## ROUND 48 (bravo): first REAL per-function permuter search — not closed, no new mechanism, third identical confirmation

Same provenance gap as `SubmitPolyG4`/`SubmitPolyGT3` (see those reports):
never had its own iteration-based search before this round.

**Check 3:** in-tree rebuild of the preserved body, full oracle: `build
exit=2`, zero compile-error hits, **80/88, identical diff to every prior
round**, `git status --porcelain` empty after revert. Scaffold `--debug
--stack-diffs`: **base score = 260**, identical decomposition to the root
and both siblings already searched this round. AGREE — search meaningful.

**Search: blind, no PERM macros, `-j 4`** (dropped from `-j 6` per this
round's contention note — up to four concurrent searches system-wide),
`timeout 900`.
```
permuter rc=124
```
**78126 iterations.** Floor held at 260. Minimum score reached: **215**
(two variants, same pointer-truncation shape) plus **240** (same
OT-pointer-caching shape) — byte-identical mechanism to `SubmitPolyG4`
and `SubmitPolyGT3`'s round-48 searches, confirmed by inspection of
`output-215-*/source.c` and `output-240-1/source.c`. Not translated.

No zero reached. **Not permuter-exhausted — not closed in 78126 iterations
under load (rc=124).** Third of three siblings searched this round, and
the third identical outcome: the same two shapes are the only sub-260
attractors, regardless of which sibling's own disassembly seeds the
search. This is now strong evidence the attractor pair is a property of
the RESIDUE CLASS itself (the `addPrim`-macro re-evaluation + duplicated
OT-mask shape shared by all 8 `code_8220_c` RCpoly siblings), not an
artifact of any one function's particular instruction stream. Reverted;
`git diff --stat` empty.

## ROUND 44 (alpha): rebuild-reconfirmed, not re-attempted

Flipped the live `#if 0` body to `#if 1`, ran the full oracle in isolation,
reverted: `build exit=2`, zero compile-error hits, **80/88 words, identical
diff to the recorded figure** (first diff word 12, `retail=00ff063c
built=00ff053c`; the "missing filler" residue at word 21 still reads
`retail=28002226 built=00000000` — `addiu $v0,$s1,0x28`). `git status
--porcelain` empty after revert. Checked the else-branch disassembly line by
line against this report's align-4-refined offset formula and against the
actual `FillRVectors4`/field-store arguments used (0x8/0x10/0x18/0x20 only) —
confirmed `self+0x28` is referenced NOWHERE in either branch of this
function's own `.s`, so the filler really is a value retail computes and
never uses anywhere, not a mistranscribed real argument. Same residue class
as `SubmitPolyF3`; not re-attempted beyond this verification.

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


## ROUND 40 (alpha): Job 1 rebuild-verify, third independent reproduction — 80/88, byte-identical

Re-ran the Job-1 rebuild independently of round 36 and round 39 (bravo): the
LIVE `#if 0` body in `src/code_8220_c.c` (round-36 symbol-corrected,
`RCpolyFT4` not `func_8001BAB4`) toggled over `INCLUDE_ASM`, full oracle in
isolation, reverted. `build exit=2`, zero compile-error/`undefined
reference` hits.

**Result: 80/88 words, byte-identical to round 36/39's figure. First diff at
word 12 (file 0xA5B4, vram 0x80019DB4): `retail=00ff063c built=00ff053c`** —
same `$a2`/`$a1` family residue as `SubmitPolyF3`. `git diff --stat` empty
after revert.

**Title rebuilt to the three-figure CLAUDE.md format.** Classification
unchanged.

## ROUND 39 (bravo): rebuilt-verified, re-derived against the new `gte.h` macro layer and the hoist-both-before-either lever -- both negative at the family root, unchanged here

Rebuilt this function's preserved body live this round (`#if 1`, `INCLUDE_ASM`
wrapped in `#if 0`), ran the full oracle, reverted: **80/88, byte-identical
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
`func_8001BAB4` -> `RCpolyFT4` (a real `libgte` symbol). `src/code_8220_c.c`'s
preserved `#if 0` snapshot was updated to the new name in that same commit
(`50fd52c`) but never rebuilt, so the `80/88` figure below was carried
forward UNVERIFIED. This round swapped the snapshot in over the
`INCLUDE_ASM`, built (`build exit=2`, no compile error, `RCpolyFT4` resolves
against the linked SDK object, no `undefined reference`), and re-ran
`funcdiff.py`.

**Result: 80/88 words, byte-identical to the figure already on record, no
drift warning.** The rename did not disturb the residue.

The corrected, LINKABLE snapshot (identical to what's live in
`src/code_8220_c.c`):

```c
#if 0
/* ROUND 36 measurement snapshot. Symbol-corrected (round 34 renamed the
 * Psy-Q callee func_8001BAB4 -> RCpolyFT4). Rebuilt LIVE and MEASURED:
 * 80/88 words, matching this report's own previously-recorded figure --
 * the rename did not disturb the residue.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Same
 * residue class: $a2 vs $a1 for the OT high-byte mask, plus one missing
 * `addiu $v0,$s1,0x28`. This is the first sibling where the raw
 * "last-field-offset + access-width" formula from SubmitPolyF3.md does
 * NOT land exactly on the filler: the highest arg0 field this function
 * touches is +0x24 (a u16 read, raw end 0x26), but the filler is 0x28.
 * 0x28 = align-up-to-4(0x26). Not cracked.
 */
void SubmitPolyFT4(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        FillDivPolygonHeader(gDivPolygon4, arg1, (u8 *)arg0 + 0x4, 1, *(u16 *)((u8 *)arg0 + 0xE), *(u16 *)((u8 *)arg0 + 0x16));
        FillRVectors4((u8 *)arg1 + 0x94, (u8 *)arg1 + 0xA4, (u8 *)arg0 + 0x8,
                      (u8 *)arg0 + 0x10, (u8 *)arg0 + 0x18, (u8 *)arg0 + 0x20);

        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x94) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x98) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x9C) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0xA0) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x94) + 0x8) = *(u16 *)((u8 *)arg0 + 0xC);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x98) + 0x8) = *(u16 *)((u8 *)arg0 + 0x14);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x9C) + 0x8) = *(u16 *)((u8 *)arg0 + 0x1C);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0xA0) + 0x8) = *(u16 *)((u8 *)arg0 + 0x24);

        RCpolyFT4(arg0, gDivPolygon4);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_8220_c", SubmitPolyFT4);
```

(`OtTag` and `RCpolyFT4`'s prototype come from `include/code_8220.h`,
already included by the unit.)

Unit: `src/code_8220_c.c`. Sixth sibling of the `SubmitPolyF3` OT-splice-
or-calls family. Calls-branch shape combines `SubmitPolyFT3`'s
`FillDivPolygonHeader(...,1,self->0xE,self->0x16)` argument pattern with
`SubmitPolyG4`'s quad (4-record) output and `FillRVectors4` call; tail
copies are two full passes of four `u16` widen-stores (into `+0xA` from a
shared `self+0x1E`, then into `+0x8` from four different `self` offsets).
Calls `func_8001BAB4` (Psy-Q SDK, `asm/psyq_rcpolyft3.s` — a different SDK
source file than the `rcpolyf3`/`rcpolyf4`/`rcpolyg3` siblings called so
far).

## Best body reached (80/88 words)

```c
> **ROUND 39 (head): THIS PRESERVED BODY WILL NOT LINK AS WRITTEN.**
> Rename(s) needed before it builds: `func_8001BAB4` -> `RCpolyFT4`.
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
void SubmitPolyFT4(void *arg0, void *arg1)
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
        FillDivPolygonHeader(gDivPolygon4, prim, self + 0x4, 1, *(u16 *)(self + 0xE), *(u16 *)(self + 0x16));
        FillRVectors4(prim + 0x94, prim + 0xA4, self + 0x8, self + 0x10, self + 0x18, self + 0x20);

        *(u16 *)(*(u8 **)(prim + 0x94) + 0xA) = *(u16 *)(self + 0x1E);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0xA) = *(u16 *)(self + 0x1E);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0xA) = *(u16 *)(self + 0x1E);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0xA) = *(u16 *)(self + 0x1E);
        *(u16 *)(*(u8 **)(prim + 0x94) + 0x8) = *(u16 *)(self + 0xC);
        *(u16 *)(*(u8 **)(prim + 0x98) + 0x8) = *(u16 *)(self + 0x14);
        *(u16 *)(*(u8 **)(prim + 0x9C) + 0x8) = *(u16 *)(self + 0x1C);
        *(u16 *)(*(u8 **)(prim + 0xA0) + 0x8) = *(u16 *)(self + 0x24);

        func_8001BAB4(self, gDivPolygon4);
    }
}
#endif
```

Byte-exact calls branch on the first attempt (nothing past word 27 of 88
differs) — no `$s1`/`$s2` swap.

## Residue

Identical class to the other five siblings, confined to the `if`
(list-splice) branch: top-byte mask constant in `$a1` instead of `$a2`,
one dead `addiu $v0,$s1,0x28` absent. Not re-investigated; see
`SubmitPolyF3.md`. Filed directly as a stall.

### Proposed learning

None new — this is the sixth confirmed instance of the exact same
list-branch residue, with the calls branch again matching byte-for-byte
regardless of which specific Psy-Q submission routine or argument shape it
ends in. At this point the pattern is thoroughly established: any
`prim->0x78`-gated OT-splice-or-submit function in this family should be
expected to match its calls branch on the first attempt and stall on the
identical list-branch residue, full stop.

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

## RUNNER PASS, round 13 continued: applied, instruction-exact, refined the offset formula

Applied both changes with this function's own offsets (gouraud-quad flavor:
`FillDivPolygonHeader` a3=1, `FillRVectors4`, `func_8001BAB4`). One attempt, 80/88
words, confirmed via `asm-differ` zero-inserted/zero-deleted. Calls branch
byte-exact. Remaining residue: `$a2`/`$a1` register identity plus one
missing `addiu $v0,$s1,0x28`.

**This sibling refines `SubmitPolyF3.md`'s cross-sibling formula.** The
highest `arg0`-relative field this function's calls branch touches is
`+0x24` (a `u16` read), whose raw "offset + width" is `0x26` — but the
actual filler is `0x28`, not `0x26`. Every other sibling checked so far
happened to have a raw sum that was ALREADY a multiple of 4, so this is the
first data point that distinguishes the two readings. The rule that
matches here is **`align_up_4(last_offset + width)`** — the filler points
at the START of the next 4-byte-aligned slot after the last field touched,
not merely one byte past it. Worth re-checking the remaining siblings (and
re-confirming the four already recorded) against this refined rule rather
than the raw one, though all four already measured are consistent with
both readings and don't distinguish them.

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
`permuter-work/SubmitPolyFT4/` (gitignored, session-local).

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
continuing) rather than trusting the title line. Result: **80/88, no
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

`func_80019D84` -> `SubmitPolyFT4`, parameters (`arg0`, `arg1`) -> (`prim`, `ctx`). **Tier
A.** code_8220_b's own comment (`src/code_8220_b.c`, the "eight submit
wrappers" block) already names this whole family collectively: each is a
tail call to Sony's `RCpolyFT4` (unrenamed, RCpoly* polygon-subdivision
family, `libgte`) or a direct OT splice, matching every sibling's shape.
This function's own discriminator: the splice arm returns
`prim + 0x28` = `sizeof(POLY_FT4)`, and the calls arm falls straight
into `jal RCpolyFT4`. `prim`/`ctx` match the parameter names code_8220_b's
own `extern void *SubmitPolyFT4(void *prim, void *ctx);` view already used.

## Round 91 polish (bravo)

Retyped with Sony's structs like the rest of the family; SubmitPolyF3's
report has the details (POLY_*, DIVPOLYGON3/4, RVECTOR, addPrim, Sony's
RCpoly* prototype, the renamed `gDivPolygon3`/`gDivPolygon4`). Byte-identical
on the first build. The field reads, for this primitive:

`clut`/`tpage` to the header; RVECTOR `pad` from `pad1` for all four; `uv`
from `u0v0`..`u3v3`. Returns `prim + 1` (0x28).
