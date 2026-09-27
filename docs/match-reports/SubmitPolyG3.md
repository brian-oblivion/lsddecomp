# SubmitPolyG3 — MATCHED (84/84, round 75): lever = RETURN VALUE (next-packet pointer, arg0 + 0x1C) + calls-arm-first branch order

> Renamed from `func_8001989C` on 2026-09-24 (tools/rename.py). Address 0x8001989c.

REVISITED, round 75: MATCHED; names/types used (return type `void *`, matching the caller's own `extern void *SubmitPolyG3(void *prim, void *ctx)` in `TmdRenderer.c`).

## ROUND 75 (bravo): MATCHED

Previous title: "SubmitPolyG3 — STALL: length EXACT (84/84 words, no drift); 76/84 raw word-match (up from 48/84, round 48); first real diff at in-range word 12 (file 0xA0CC / vram 0x800198CC), the family-shared register-identity residue -- the function's OWN self/prim register-swap residue is CLOSED".

**Preserved body rebuilt first** (the `#ifdef NON_MATCHING` body compiled live
in place of the `INCLUDE_ASM`): 76/84, `insertions 0 / deletions 0
(opcode-level, built vs retail; positional skeleton diffs 8)`. The eight diffs
are the family-shared `$a2`/`$a1` OT-mask register and the missing
`addiu $v0,$s1,0x1c`.

**Lever: the sibling lever from `SubmitPolyF3` / `SubmitPolyF4`, applied
unchanged.** The function returns the next packet pointer. `N = 0x1C` is read
off this function's own disassembly (`addiu $v0, $s1, 0x1C` at vram
0x800198F0 in the splice arm) and agrees with sizeof(POLY_G3) (tag, then
three rgb+xy pairs = 4 + 3*8). The other arm falls into the epilogue after
`jal RCpolyG3`, so its `$v0` is RCpolyG3's own return.

| body | score |
| --- | --- |
| preserved void body (with the permuter-found `do/while(0)` wrap and `vtx0` caching) | 76/84 |
| `void *`, calls arm first returning `RCpolyG3(..)`, splice then `return self + 0x1C`, `vtx0` caching KEPT, `do/while(0)` dropped | **84/84**, `OK: build matches retail` |
| same, `vtx0` caching ALSO dropped (all six stores written uniformly) | **84/84**, `OK: build matches retail` — committed |

Two builds. **The earlier permuter lever (`do/while(0)` wrap + partial
`vtx0` caching) was compensating for the missing return value and is not
needed once it exists**: neither survives into the committed body. That
lever's "self/prim register swap sub-residue" was the same missing-`$v0`
artefact seen from a different angle.

**Callers checked:** `SortTmdObject` in `src/TmdRenderer.c` (two call sites,
`prim = (u8 *)SubmitPolyG3(prim, ctx);`), declared there as
`extern void *SubmitPolyG3(void *prim, void *ctx);`. The return type agrees.
`RCpolyG3` stays declared `void` in `include/code_8220.h` and is called
through the local cast.

### Proposed learning

A permuter-found lever on a body that is missing a semantic feature (here the
return value) can be a compensation, not a fact about the source. When the
real feature is added, re-test whether the permuter's contortions are still
needed; here both became unnecessary.

## Earlier history

# (previous title) # SubmitPolyG3 — STALL: length EXACT (84/84 words, no drift); 76/84 raw word-match (up from 48/84, round 48); first real diff at in-range word 12 (file 0xA0CC / vram 0x800198CC), the family-shared register-identity residue -- the function's OWN self/prim register-swap residue is CLOSED

NON_MATCHING body promoted, round 65.

## ROUND 65 (charlie): NON_MATCHING body promoted

Track 1b mechanical promotion. This function's standing `#if 0` body
combines a hand-derived family-shared residue with one permuter-found
lever (the `do { ... } while (0)` wrap plus partial `vtx0` caching, round
48). Per track 1b's rule ("a permuter candidate is promoted only after a
human-style review that its semantics are what the disassembly does"):
round 48's own report already did that review — it verified the lever
against the live oracle (76/84, no drift), confirmed the transformation is
a scheduling-only reshaping (wrap + cache), not scorer-exploiting UB (it
explicitly distinguishes this lead from the same search's `output-230-1`,
which it rejected on inspection as a dead-branch/masking trick), and
confirmed the remaining 8-word gap is the identical family-shared class
documented elsewhere in this unit. Treating this as reviewed and eligible.
Wrapped the function in `#ifdef NON_MATCHING ... #else INCLUDE_ASM
... #endif` in place (the `Vec2s16_98` typedef stays outside the ifdef,
unconditional, as it already was). No bytes changed.
`./build-and-verify.sh`: `build exit=0`, `OK: build matches retail`.
`tools/check-nonmatching.sh TmdRenderer`: `OK`. No stale symbol references
(`RCpolyG3` is already the current name).

## ROUND 48 (bravo): first REAL permuter search — CLOSED the self/prim-swap sub-residue (48/84 -> 76/84), the family-shared residue remains

**This function had never had a real iteration-based permuter search** — only
a single `--debug` type-axis hand test (base score 405/415 depending on
struct typing). Round 48's family-wide sweep (see `SubmitPolyGT4.md` and the
broadcast) covered the other 6 `TmdRenderer` OT-mask siblings; this was the
last function in the unit's stall queue to get a real search of its own.

**Check 3:** in-tree rebuild of the preserved body, full oracle: `build
exit=2`, zero compile-error hits, **48/84, identical diff to every prior
round**, `git status --porcelain` empty after revert. Scaffold `--debug
--stack-diffs`: **base score = 413** (1 insertion + 1 deletion + 43
register diffs, stack diffs 8) — this function's OWN residue (the
self/prim swap) stacks on TOP of the family-shared 260, unlike the other
6 siblings' scaffolds which score exactly 260. AGREE (nonzero on both
sides, and the decomposition matches this report's own prior `--debug`
figure of 405/415 within the same family). Search meaningful.

**Search: blind, no PERM macros, `-j 6`, `timeout 900`.**
```
permuter rc=124
```
**156329 iterations.**

**A genuine, non-degenerate lead was found and ORACLE-VERIFIED —
this is a real improvement, not a false lead like the family's usual
attractors.** `output-260-1` (found early, ~iteration 18780): wraps the
`if` branch's OtTag-splice pair in a `do { ... } while (0)`, and caches
`*(u8 **)(prim + 0x88)` into a new local (`vtx0`) used for the FIRST of
the three `+0xA` stores only:

```c
if (*(s32 *)(prim + 0x78) == 0) {
    do {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } while (0);
} else {
    ...
    vtx0 = *(u8 **)(prim + 0x88);
    *(u16 *)(vtx0 + 0xA) = *(u8 *)(self + 0xF);
    *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u8 *)(self + 0xF);
    ...
```

Applied to `src/TmdRenderer.c` in place of `INCLUDE_ASM`, full oracle:
`build exit=2`, zero compile-error hits, **76/84 words match, NO drift
warning**. This is a genuine, verified improvement of **28 words** — it
closes this function's OWN self/prim-swap residue outright. The remaining
8-word residue is now the EXACT family-shared shape documented across
this round's other 6 reports: word 12 is the `$a2`/`$a1` duplicated OT
mask, and the diff positions/count match the family's usual 8-diff
pattern for this residue class exactly.

**Why the `do`/`while(0)` wrapper is what moved it, read against
`DECOMPILATION_LEARNINGS.md`'s own entry for that lever:** the guide's
"`do { } while (0)` wrapping is a SMALL-BODY lever" entry says it is load-
bearing for delay-slot scheduling in small bodies — this function's own
prior round-1 attempts had tried swapping the `self`/`prim` DECLARATION
order (not wrapping the branch body) and found the same address-drift
regression every time. The wrapper is a different move: it does not touch
which local is declared first, it changes how GCC schedules the two-
statement OtTag-splice block relative to the branch, which is exactly the
axis every previous attempt on this function's OWN residue never tried.

**Why the partial `vtx0` caching (first store only, not all three) is not
an accident worth "fixing" for symmetry:** the other two `+0xA` stores
still address `prim+0x8C`/`prim+0x90` directly. Tried making all three
use cached locals (`vtx0`/`vtx1`/`vtx2`) for symmetry — **regressed**, back
down to 48/84 with a different diff shape (extra register pressure, one
more callee-saved register live). The permuter's asymmetric shape is the
one that matches; a hand-written "cleaner" symmetric version does not.
This is now the standing best body — see below.

The search continued past this lead (per `--best-only`, not
`--stop-on-zero`-satisfied since no zero was found): floor mostly held at
260 for the rest of the run (post-fix scaffold matches the family's own
260 base exactly, consistent with the residue now being identical to the
6 siblings'). One further candidate, `output-230-1`, scored below 260 but
is a semantic non-candidate on inspection — it introduces
`if (gDivPolygon3) { X } else { X }` with byte-identical bodies on both
arms (branching on a rodata table POINTER as a boolean condition that
appears nowhere in retail's control flow) plus several dead-value/masking
tricks (`0xF & 0xFFFFFFFFFFFFFFFF`). Not oracle-tested — the shape itself
disqualifies it, same class as the family's own `output-215-*`
truncating-cast false leads (a permuter score that coincidentally
overlaps retail bytes without being a real candidate).

No zero reached. **Not permuter-exhausted — not closed in 156329
iterations under load (rc=124).** But this search is NOT a pure negative:
it produced a real, oracle-verified 28-word improvement, now the standing
best body in `src/TmdRenderer.c`'s `#if 0` block and below. The remaining
residue is the family-shared one — see `SubmitPolyGT4.md`'s round 48
entry for that residue class's own exhaustive characterization (6/6
siblings searched this round, no fix found; treat this function's
remaining 8-word gap the same way, not as an open lead).

## Best body reached (76/84 words — CLOSES the self/prim-swap residue; family-shared residue remains)

```c
#if 0
/* A 2-s16 pair (alignment 2, not 4) -- see FlagLargePolyForDivide's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_98;

/* STALL snapshot round 48 -- see docs/match-reports/SubmitPolyG3.md.
 * 76/84 words, no drift. This function's OWN residue (a pre-existing
 * self/prim register swap) is CLOSED by a permuter-found lead: wrapping
 * the OtTag-splice branch in `do { ... } while (0)` and caching
 * *(u8 **)(prim + 0x88) into `vtx0` for the FIRST +0xA store only (NOT
 * all three -- making it symmetric regresses to 48/84). Remaining residue
 * is the family-shared class documented in SubmitPolyGT4.md: register-
 * identity ($a2 vs $a1, duplicated OT mask) plus a missing delay-slot
 * filler `addiu $v0,$s1,0x1c`. That residue was searched exhaustively
 * (6/6 siblings, 471873 combined iterations, round 48) with no fix found
 * -- do not re-search this function's remaining gap without a genuinely
 * new axis not already tried on the other six.
 */
void SubmitPolyG3(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;
    u8 *vtx0;

    if (*(s32 *)(prim + 0x78) == 0) {
        do {
            ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
            (*(OtTag **)(prim + 0x30))->addr = (u32)self;
        } while (0);
    } else {
        FillDivPolygonHeader(gDivPolygon3, prim, self + 0x4, 0, 0, 0);
        FillRVectors3((PolyVtx **)(prim + 0x88), (PolyVtx **)(prim + 0xA4),
                      (PolyUV4 *)(self + 0x8), (PolyUV4 *)(self + 0x10),
                      (PolyUV4 *)(self + 0x18));

        vtx0 = *(u8 **)(prim + 0x88);
        *(u16 *)(vtx0 + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u8 *)(self + 0x17);

        *(Vec2s16_98 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_98 *)(self + 0x4);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_98 *)(self + 0xC);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_98 *)(self + 0x14);

        RCpolyG3(self, gDivPolygon3);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/TmdRenderer", SubmitPolyG3);
```

### Proposed learning

**A `do { } while (0)` wrapper around a TWO-STATEMENT branch body is a
distinct move from reordering the branch's own declarations, and this
function's own history had only tried the latter.** Four prior rounds
(round 1, round 39, the permuter-round `--debug` type-axis test) all
targeted the self/prim swap by changing declaration order or struct
typing — never by wrapping the CONSUMING branch's statements. The
`DECOMPILATION_LEARNINGS.md` entry for this lever already documents it as
a small-body scheduling instrument; this is a concrete instance of it
resolving what four rounds had filed as a "pre-existing register swap,
investigated exhaustively" residue, because the exhaustive investigation
had only explored the declaration axis, not the statement-grouping axis.
**When a report says a residue was "investigated exhaustively," check
which AXES were actually tried before accepting that framing** — this is
the same lesson `DECOMPILATION_LEARNINGS.md` already draws from round 38's
hoist-both lever, applied to a different move (statement wrapping vs.
value hoisting).

## ROUND 40 (alpha): Job 1 rebuild-verify, third independent reproduction — 48/84, byte-identical

Re-ran the Job-1 rebuild independently of round 36 and round 39 (bravo): the
LIVE `#if 0` body in `src/TmdRenderer.c` (round-36 symbol-corrected,
`RCpolyG3` not `func_8001AD54`) toggled over `INCLUDE_ASM`, full oracle in
isolation, reverted. `build exit=2`, zero compile-error/`undefined
reference` hits.

**Result: 48/84 words, byte-identical to round 36/39's figure. First diff at
word 1 (file 0xA0A0, vram 0x800198A0): `retail=2000b2af built=1c00b1af`** —
this function's own pre-existing `$s1`=prim/`$s2`=self swap (opposite of
retail and every other sibling), already investigated exhaustively per the
round-2/round-1 attempts below. `git diff --stat` empty after revert.

**Title rebuilt to the three-figure CLAUDE.md format.** Classification
unchanged.

## ROUND 39 (bravo): rebuilt-verified, re-derived against the new `gte.h` macro layer and the hoist-both-before-either lever -- both negative at the family root, unchanged here

Rebuilt this function's preserved body live this round (`#if 1`, `INCLUDE_ASM`
wrapped in `#if 0`), ran the full oracle, reverted: **48/84, byte-identical
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
`func_8001AD54` -> `RCpolyG3` (a real `libgte` symbol, in
`config/symbols.slps01556.lsdde.txt`). `src/TmdRenderer.c`'s own preserved
`#if 0` snapshot was updated to the new name as part of that same commit
(`50fd52c`), but the body was never swapped back in and rebuilt — so the
`48/84` figure below was carried forward UNVERIFIED against the renamed
symbol. This round did exactly that: swapped the snapshot in over the
`INCLUDE_ASM`, ran `./build-and-verify.sh` (build exit=2, no compile error,
no `undefined reference` — `RCpolyG3` resolves against the linked SDK
object), and re-ran `funcdiff.py`.

**Result: 48/84 words, byte-identical to the figure this report already
carried, and `funcdiff` raised no drift warning** (in-range comparison only,
confirming the function's compiled length still matches retail exactly).
The rename did not disturb the residue in any way — it only made the
existing figure trustworthy instead of assumed.

The corrected, LINKABLE snapshot (identical to what's live in
`src/TmdRenderer.c` right now, wrapped back in `#if 0`/`INCLUDE_ASM` per the
match-report convention):

```c
#if 0
/* A 2-s16 pair (alignment 2, not 4) -- see FlagLargePolyForDivide's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_98;

/* ROUND 36 measurement snapshot. Symbol-corrected (round 34 renamed the
 * Psy-Q callee func_8001AD54 -> RCpolyG3). Rebuilt LIVE and MEASURED:
 * 48/84 words, matching this report's own previously-recorded figure --
 * the rename did not disturb the residue.
 * Instruction-exact (asm-differ: zero inserted, zero deleted -- ONE
 * differing line, the delay-slot filler below). TWO residues stack here,
 * both already documented: (a) the shared family residue ($a2 vs $a1 for
 * the OT mask, plus missing `addiu $v0,$s1,0x1c` = self+0x1c, matching
 * the cross-sibling formula -- last touched self field is +0x18, a
 * PolyUV4, width 4, end 0x1c); (b) this function's OWN pre-existing
 * self/prim register swap ($s1=prim, $s2=self, opposite of retail and
 * every OTHER sibling), already investigated exhaustively in this
 * report's round-1 attempts. Not cracked.
 */
void SubmitPolyG3(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } else {
        FillDivPolygonHeader(gDivPolygon3, prim, self + 0x4, 0, 0, 0);
        FillRVectors3((PolyVtx **)(prim + 0x88), (PolyVtx **)(prim + 0xA4),
                      (PolyUV4 *)(self + 0x8), (PolyUV4 *)(self + 0x10),
                      (PolyUV4 *)(self + 0x18));

        *(u16 *)(*(u8 **)(prim + 0x88) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u8 *)(self + 0x17);

        *(Vec2s16_98 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_98 *)(self + 0x4);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_98 *)(self + 0xC);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_98 *)(self + 0x14);

        RCpolyG3(self, gDivPolygon3);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/TmdRenderer", SubmitPolyG3);
```

(`OtTag`, `PolyVtx`, `PolyUV4` and `RCpolyG3`'s prototype come from
`include/code_8220.h`, already included by the unit.)

Unit: `src/TmdRenderer.c`. Third sibling of `SubmitPolyF3`/`SubmitPolyF4`
(this unit, both stalled) — same `prim->0x78`-gated OT-splice-or-calls
shape, this time with MORE post-call work: after
`FillDivPolygonHeader`(gp_rel-blocked)/`FillRVectors3` (matched), it copies a byte
from `self` (widened to `s16`) into three output records' `+0xA` field,
then an unaligned 4-byte value from three different `self` offsets into
the same three records' `+0xC` field, then calls `func_8001AD54` (Psy-Q
SDK, `asm/psyq_rcpolyg3.s`, Gouraud-flavored sibling of `func_8001A564`/
`func_8001A8D4`).

## Best body reached (48/84 words)

```c
> **ROUND 39 (head): THIS PRESERVED BODY WILL NOT LINK AS WRITTEN.**
> Rename(s) needed before it builds: `func_8001AD54` -> `RCpolyG3`.
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
} Vec2s16_98;

void SubmitPolyG3(void *arg0, void *arg1)
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
        FillDivPolygonHeader(gDivPolygon3, prim, self + 0x4, 0, 0, 0);
        FillRVectors3(prim + 0x88, prim + 0xA4, self + 0x8, self + 0x10, self + 0x18);

        *(u16 *)(*(u8 **)(prim + 0x88) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u8 *)(self + 0x17);

        *(Vec2s16_98 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_98 *)(self + 0x4);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_98 *)(self + 0xC);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_98 *)(self + 0x14);

        func_8001AD54(self, gDivPolygon3);
    }
}
#endif
```

Build compiles clean, no address drift. `FlagLargePolyForDivide`'s `Vec2s16`
alignment-2-struct-copy idiom reproduced the three unaligned 4-byte copies
correctly on the first try (confirmed structurally correct against the
disassembly — the residue below is entirely register CHOICE, never a wrong
field, wrong offset, or missing/extra instruction).

## Two distinct residues, not one

1. **The already-documented mask-constant/dead-address residue** in the
   `if` (list-splice) branch — identical to `SubmitPolyF3`/`SubmitPolyF4`
   (this unit): top-byte mask lands in `$a1` where retail wants `$a2`, plus
   one missing dead `addiu`. Not re-investigated here; see
   `SubmitPolyF3.md` for the six attempts already spent establishing this
   is not reachable from C.
2. **A NEW residue, specific to this function's size**: `self`/`prim` land
   in `$s1`/`$s2` SWAPPED from retail and from BOTH shorter siblings.
   Retail (and `SubmitPolyF3`/`SubmitPolyF4`) use `$s2`=`arg1`(prim),
   `$s1`=`arg0`(self) throughout, matching the source order `move
   s2,a1` (unconditional, at entry) then `move s1,a0` (the `bnez`'s delay
   slot). My build assigns the OPPOSITE: `$s1`=prim, `$s2`=self — confirmed
   via objdump on EVERY attempt regardless of local-variable naming,
   declaration order (`self` before `prim` or vice versa — the reverse
   order also drifted the function's total LENGTH, to 2/84 with drift,
   made things categorically worse), or whether `self`/`prim` were named
   locals at all vs. repeated `(u8 *)argN` casts. This swap then
   propagates: because `self` (needed far more often in this function's
   longer "calls" branch — 8 uses vs. `prim`'s 5) ends up in the
   HIGHER-numbered register, register choices for EVERY subsequent
   temporary in the calls branch differ from retail's too (confirmed:
   the `FillRVectors3` call's own argument setup, otherwise byte-identical
   in the two shorter siblings, differs here in exactly the ADDIU
   destination registers, not the values).

## Attempts (6)

1. First cut, matching `SubmitPolyF3`'s already-proven structure with this
   function's own offsets and the extra tail copies inline via repeated
   `(u8 *)argN` casts — 35/84, WORSE than the siblings' analogous first
   attempts. Root cause turned out to be BOTH residues stacking, not one.
2. Removed three redundant `p0`/`p1`/`p2` locals that were re-declared
   per-statement (caching `prim->0x88` etc. across the byte-copy and
   word-copy groups) so each field access re-dereferences fresh — 48/84.
   This alone fixed a chunk of the calls-branch mismatches (the `func_
   8001A3EC` call itself, and the field-address `addiu`s' IMMEDIATES,
   though not their destination REGISTERS), confirming reload-not-cache
   applies here too, but did not touch the `$s1`/`$s2` swap.
3. Introduced explicit `self`/`prim` named locals (declared `self` first)
   to see if naming affected the swap — no change, still 48/84, identical
   diff.
4. Swapped the declaration order (`prim` first, matching retail's
   PROCESSING order even though `self`/`arg0` is textually the first C
   parameter) — regressed badly: 2/84 with ADDRESS DRIFT (the function's
   own compiled length changed). Reverted immediately.
5. Rewrote the six tail copy statements (three `u16` byte-widen stores,
   three `Vec2s16_98` word copies) as one literal `__asm__` block with `"r"`
   operands for `prim`/`self` and raw `$2`/`$3` scratch, mirroring retail's
   own register choices for the SCRATCH registers exactly (this technique
   is what made `FillRVectors3` byte-exact) — regressed sharply to 4/84
   with drift. The difference from `FillRVectors3`'s success: there, the
   whole function's parameters arrived directly in `$a0`-`$a3` (natural
   ABI registers, no prior C code to disturb them); here, `prim`/`self` are
   MID-FUNCTION values already living in whatever callee-saved registers
   the earlier code picked, so binding them through `"r"` operands forced
   GCC to reconcile the asm block's operand requirements against an
   allocation it had already committed to elsewhere in the function, and it
   did so badly. Reverted to attempt 2/3's body.
6. Confirmed attempt 2's body (`self` declared before `prim`, no operand-
   bound asm) is reproducibly 48/84 with no drift; stopped there.

### Proposed learning

**The `$s1`/`$s2` role assignment for a "self"/"prim" pair is not fixed by
the SHAPE of the opening `if(prim->0x78==0){...}else{...}` guard alone — it
depends on the RELATIVE FREQUENCY of the two variables' use across the
WHOLE function, and gets it right in short functions (`SubmitPolyF3`:
prim used 5x, self 3x -- wait, both under 10 total mentions) but wrong once
one side dominates (`SubmitPolyG3`: self used 8x vs. prim's 5x, total
mentions past a size threshold somewhere between these two). Declaration
order of local aliases does not override this (attempts 3-4), and once the
swap happens it cascades into register-identity mismatches for every
OTHER temporary in the same branch, not just the two swapped variables —
meaning a single wrong high-level register choice can manufacture what
looks like a dozen unrelated residues. When the "calls" branch of this OT-
splice-or-calls family scores much worse than `SubmitPolyF3`'s clean
100%-match precedent, check `$s1`/`$s2` role assignment via objdump FIRST,
before assuming the extra tail logic itself is wrong — it may be
byte-correct already (as `Vec2s16`'s alignment-2 idiom was here) and simply
inheriting a bad register choice made upstream.

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

## RUNNER PASS, round 13 continued: applied, instruction-exact aside from the pre-existing self/prim swap

Applied both family-wide changes with this function's own offsets. One
attempt with `self`/`prim` locals (matching round 1's winning declaration
order), 48/84 words -- `asm-differ` shows exactly ONE inserted/deleted
line (the delay-slot filler), confirming this is ALSO instruction-exact.
Re-tried swapping the declaration order (`prim` before `self`) to see if
the bitfield rewrite changed the earlier self/prim-swap finding — it did
not: same regression as round 1 (address drift). Reverted to `self`-first.

Two residues stack in this specific sibling, both already fully documented
elsewhere:

1. The shared family residue: `$a2`/`$a1` on the OT mask, plus one missing
   `addiu $v0,$s1,0x1c` — matches `SubmitPolyF3.md`'s formula exactly
   (last touched `self` field is `+0x18`, a `PolyUV4`, width 4, raw end
   `0x1c`, already 4-aligned so the `SubmitPolyFT4`-derived align-4
   refinement makes no difference here).
2. This function's OWN pre-existing residue (documented in this report's
   round-1 section): `$s1`/`$s2` land as `prim`/`self`, the OPPOSITE of
   retail and every other sibling. Not reachable via declaration-order
   changes under either the old hand-masked form or the new bitfield form.

Not re-attempted beyond confirming (2) is unchanged by the bitfield
rewrite — see `SubmitPolyF3.md` for (1)'s ruled-out hypotheses and this
report's round-1 section for (2)'s.

## RUNNER PASS, permuter round (alpha): type/declaration-axis test on the self/prim swap -- negative

Per the head's round-wide broadcast (type/declaration axes, not expression
shape, closed two other stalls this round), tested whether concrete struct
typing of `self`/`prim` (instead of `void *` + casts) affects THIS
function's own residue -- the `$s1`/`$s2` self/prim register swap unique
to this sibling, stacked on top of the family's shared OT-mask/filler
residue (see `SubmitPolyF3.md` for that half, also independently
re-tested type/declaration axes there with the same negative result).

Declared `arg0`/`arg1` as concrete struct pointers (`Self98T`/`Prim98T`,
matching this function's own field layout: `OtTag tag`, `xy0`, three
`PolyUV4` UV records for `self`; `head`/`flag`/`dst[3]` for `prim`), all
field access via `->` instead of raw offset casts. `--debug`: base score
**415** (1 insertion + 1 deletion + 43 register diffs) -- marginally
WORSE than this report's existing 405 (41 register diffs), not better.
Concrete typing does not fix the self/prim swap here; if anything it adds
two more register mismatches elsewhere in the function.

**Clean negative, not exhaustion** -- one concrete typing scheme tried, not
every possible declaration shape. Combined with `SubmitPolyF3.md`'s four
negative type/declaration tests, this sibling's TWO stacked residues both
remain open (not disproven-reachable) after this round's broadened
axis, and neither closed.

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
continuing) rather than trusting the title line. Result: **48/84, no
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

`func_8001989C` -> `SubmitPolyG3`, parameters (`arg0`, `arg1`) -> (`prim`, `ctx`). **Tier
A.** TmdRenderer's own comment (`src/TmdRenderer.c`, the "eight submit
wrappers" block) already names this whole family collectively: each is a
tail call to Sony's `RCpolyG3` (unrenamed, RCpoly* polygon-subdivision
family, `libgte`) or a direct OT splice, matching every sibling's shape.
This function's own discriminator: the splice arm returns
`prim + 0x1c` = `sizeof(POLY_G3)`, and the calls arm falls straight
into `jal RCpolyG3`. `prim`/`ctx` match the parameter names TmdRenderer's
own `extern void *SubmitPolyG3(void *prim, void *ctx);` view already used.

## Round 91 polish (bravo)

Retyped with Sony's structs like the rest of the family; SubmitPolyF3's
report has the details (POLY_*, DIVPOLYGON3/4, RVECTOR, addPrim, Sony's
RCpoly* prototype, the renamed `gDivPolygon3`/`gDivPolygon4`). Byte-identical
on the first build. The field reads, for this primitive:

`FillDivPolygonHeader(gDivPolygon3, ctx, (CVECTOR *)&prim->r0, 0, 0, 0)`;
RVECTOR `pad` from `prim->pad1`, `pad1`, `pad2` (0xF, 0xF, 0x17: vertex 1
reuses vertex 0's byte, as retail does); RVECTOR `c` from the colour words
`r0`, `r1`, `r2` (0x4, 0xC, 0x14). Returns `prim + 1` (0x1C).

**Vec2s16 is retired.** The colour copies were `*(Vec2s16 *)` whole-struct
assignments, using a unit-local `{ s16 x, y; }` type whose only job was
alignment 2, so that GCC emits lwl/lwr + swl/swr. Its comment's history:
five separately typedef'd copies (`Vec2s16_98/_C04/_EE4/_A64/_268`, each
named for its file offset) were merged into one unit-local type in round 77
(alpha); DECOMPILATION_LEARNINGS, "A struct whose members are all s8/s16 has
alignment 2", is the idiom. Sony's `CVECTOR` (four `u_char`, alignment 1)
copies with the same lwl/lwr + swl/swr, measured: the build stayed
byte-identical with every Vec2s16 copy replaced by a CVECTOR or DVECTOR
one.
