# CheckDreamAuxTriggerCondition -- MATCHED (byte-exact, 100/100 words). Round 25, head.

> Renamed from `func_8005CBC8` on 2026-09-21 (tools/rename.py). Address 0x8005cbc8.

> **ROUND 25 (2026-09-08), head. CLOSED. The last word was BASIC-BLOCK ORDER,
> and every expression in round 24's 99/100 body was already right.**
>
> The residue three rounds of work had characterised as "retail's redundant
> `j` over the switch-index join" was exactly that, and it is reachable from
> C. It is not a scheduling artifact and it needed neither a barrier nor the
> permuter: it is a consequence of WHICH BASIC BLOCK GCC PLACES WHERE, and
> the source controls that.
>
> Retail's layout, read off `asm/nonmatchings/code_4cd08/CheckDreamAuxTriggerCondition.s`:
>
> ```
>         bgez  $a0, .L8005CC0C     # sel >= 0 -> the "idx = sel" arm
>          nop
>         lb    $v0, 0x0($s0)       # record->unk0
>         nop
>         beqz  $v0, .L8005CC14     # unk0 == 0 -> the "~sel + 1" tail
>          nor  $v0, $zero, $a0
>         j     .L8005CD44          # return false
>          addu $v0, $zero, $zero
>   .L8005CC0C:
>         j     .L8005CC18          # <- THE WORD. jumps over the tail below.
>          addu $a1, $a0, $zero
>   .L8005CC14:
>         addiu $a1, $v0, 0x1
>   .L8005CC18:
> ```
>
> So retail's block order is: [`sel < 0` test] [`return false`] [`idx = sel`
> arm, which must jump] [`idx = ~sel + 1` tail] [join]. The `idx = sel` arm
> sits BETWEEN the fail path and the negate tail, which is the only reason it
> needs a jump at all.
>
> Round 24's body wrote the preamble the obvious way:
>
> ```c
>     if (sel < 0) {
>         if (record->unk0 != 0) {
>             return false;
>         }
>         idx = ~sel + 1;
>         goto have_idx;
>     }
>     idx = sel;
> have_idx:
> ```
>
> which puts the `idx = sel` arm LAST, where it falls through into the join
> and needs no jump. That body is one word short **for any expression-level
> reshape**, which is why thirteen hand attempts and 77k permuter iterations
> all sat at 99/100: they were all searching the wrong axis.
>
> **The fix is to place the blocks in retail's order, in the source.** Invert
> the inner test so the negate arm becomes a forward `goto` and `return false`
> becomes the fallthrough, then put the `idx = sel` block textually BEFORE the
> label it jumps to:
>
> ```c
>     if (sel < 0) {
>         if (record->unk0 == 0) {
>             goto negate;
>         }
>         return false;
>     }
>     idx = sel;
>     goto have_idx;
>
> negate:
>     idx = ~sel + 1;
>
> have_idx:
> ```
>
> GCC 2.6.3 then emits exactly retail's five blocks in exactly retail's
> order, `j` included, and `~sel` still lands in the `beqz` delay slot as
> `nor $v0, $zero, $a0` without being asked to. **100/100, whole-image SHA1
> green, no `__asm__`, no barrier, no operand constraint.**
>
> ### Proposed learning
>
> **A one-word residue that survives every expression reshape and every
> permuter run is evidence about BLOCK ORDER, not about scheduling.** The
> permuter mutates expressions, declarations and statement order within a
> block; it does not restructure control flow into a different basic-block
> LAYOUT, so a layout residue is invisible to it and reads as an exhausted
> plateau. The tell in the disassembly is a `j` (not a conditional branch)
> whose target is the next join and whose delay slot carries real work: that
> jump exists because retail's compiler had a block after it, and the source
> has to put a block there too.
>
> The lever is a forward `goto` plus TEXTUAL PLACEMENT: an if/else gives GCC
> the choice of which arm falls through, and it always picks the last one. To
> force an arm to jump, make it not-last -- invert the guarding test so the
> other arm becomes the `goto`, and write the jumping arm above the label.
> This is the block-order sibling of the already-documented "case order
> follows the jump table" idiom: both are cases where retail's own layout is
> readable off the binary and has to be reproduced rather than reasoned about.
>
> The round-24 note below (the `case 6`/`case 7` jump-table merge) stands
> unchanged and was a necessary prerequisite -- without it the body is 8
> words TOO LONG and this residue is invisible. Everything below is history,
> kept because the derivation is worth reading and because the sequence of
> corrections is itself the record of how a 100-word function took four
> rounds.
>
> **Both scaffold bugs the round-24 note reports are already FIXED on `main`,
> in commit `434f396` — do not re-fix them.** `tools/setup-permuter.sh:75`
> carries `--addiu-at`, matching `Makefile:44`; and `target.s` generation now
> deletes splat's `.set noat`/`.set noreorder` **by content** rather than by
> the old `sed -e '1,4d'` position, so a function owning embedded rodata keeps
> its `.section .rodata` directive. Trap 6 in that script's header comment
> records it. (This paragraph originally claimed the second bug was still
> live; that was wrong, and it is corrected here rather than deleted because
> a stale "still broken" note costs the next round a wasted fix.)


> **ROUND 24 (2026-09-08), runner delta. Round 23's "99/100" figure was measured
> against an INCOMPLETE body and was itself wrong -- rebuilding it (as this
> round's assignment explicitly required) found a real, previously-undiscovered
> structural bug that round 23 missed, not just a stale number.**
>
> Restoring round 23's exact preserved body and building it gave **108/100
> words -- 8 WORDS TOO LONG**, not 99/100 (confirmed via `nm` on the linked
> ELF: `IsCurrentDreamColor` landed at `+0x1B0` from this function's start, not
> retail's `+0x190`). Round 23 never re-derived this number from a build; it
> inherited round 13's asm-differ read and treated the dispatch fix as the
> whole story.
>
> **The actual cause: retail MERGES switch cases 6 and 7 into ONE shared
> handler, and the preserved C body had them as two separate `case` blocks.**
> Proof is in the function's own jump table, still on disk in
> `asm/nonmatchings/code_4cd08/CheckDreamAuxTriggerCondition.s`:
>
> ```
>     /* 20A4 800118A4 C8CC0580 */ .word .L8005CCC8   <- index 6
>     /* 20A8 800118A8 C8CC0580 */ .word .L8005CCC8   <- index 7, SAME label
> ```
>
> Both slots point at the identical handler, which computes the modulo-3
> remainder ONCE and compares it against `idx - 7` (`idx==8` for case 6 gives
> `idx-7==1`; `idx==9` for case 7 gives `idx-7==2` -- exactly the two
> constants the old separate-case bodies hardcoded). Writing this as
> `case 6: case 7: if (value % 3 != idx - 7) return false; break;` collapses
> the redundant second modulo-3 computation GCC otherwise emits for the two
> separate arms, and drops the function straight to **99/100** -- confirmed
> both via `nm` (built size now exactly `0x18C` vs retail's `0x190`, i.e. one
> word short) and via `asm-differ` realigned on the function's own stream
> (every instruction in the body matches retail except the one preamble word
> below; no other divergence anywhere, including the case 6/7 block itself
> which now reproduces byte-for-byte).
>
> This generalizes the existing "case order follows the jump table"
> idiom (`docs/DECOMPILATION_LEARNINGS.md`, "case order recoverable from the
> binary") one step further: **two (or more) jump-table SLOTS can point at
> the SAME handler label**, meaning the source must write those case values
> as a single shared `case A: case B:` arm with a computation that is
> deliberately made to depend on which value dispatched into it (here,
> `idx`), not as separate arms with the constant baked in. The tell is
> identical jump-table words for different indices -- grep the `.s` file's
> `.word` column for a repeated label before assuming every index has its
> own arm.
>
> **Permuter, run against the corrected (99/100) body. Two runs, 34,180 +
> 43,084 = 77,264 iterations total, NOT CLOSED under this load** (`-j 6
> --stack-diffs --stop-on-zero --best-only`, 280s wall-clock budget each).
> Phrasing deliberately per this round's instruction: "not closed in N
> iterations under load", not "permuter-exhausted" -- the machine had four
> other runners on it (`ps` showed concurrent searches from worktrees `echo`
> and others throughout), so these counts are weak evidence about the search
> space and say nothing about whether a longer or uncontended run would
> differ. **Run 1's exit status was not captured** -- it was launched
> `nohup timeout 280 ... &` without recording `$?` on completion, a real gap
> in this round's own discipline (CLAUDE.md's `timeout`/124-vs-137 recipe is
> for exactly this and was not followed). Circumstantial evidence (continuous
> iteration counts to 34,180 with no error/abort message, ending at
> approximately the 280s mark) is consistent with the wall-clock bound firing
> rather than an external kill, but this is inference, not a recorded exit
> code. Run 2 was launched the same way and same gap applies; recommend
> whoever resumes this function fix the launch to capture `$?` immediately.
>
> Two improving candidates found across the two runs, both verified against
> the real oracle and both REJECTED -- neither is committable, and both
> illustrate the same caution from opposite directions:
>
> **Run 1's candidate (permuter score 330 vs base 420)** reorders the
> `idx = ~sel + 1;` computation to BEFORE the
> `if (record->unk0 != 0) return false;` check instead of after:
>
> ```c
>     if (sel < 0) {
>         idx = ~sel + 1;              /* moved up, was after the check */
>         if (record->unk0 != 0) {
>             return false;
>         }
>         goto have_idx;
>     }
> ```
>
> **Translated to real C and re-verified against the actual oracle, this
> REGRESSES: 98/100 (2 words short), not an improvement** -- confirmed via
> `nm` (`IsCurrentDreamColor` moved to `+0x188`, not `+0x190`). This is exactly the
> documented caution that a permuter score is not the project's oracle
> (MATCHING-GUIDE.md's "Permuter" section): the permuter's own weighted
> penalty improved (fewer visible mismatched instructions in ITS diff) while
> the real word-count got worse. The kept near-miss body reverts this
> reorder.
>
> **Run 2's candidate (permuter score 202 vs base 420)** is a `volatile`
> dead-store trick, not an idiomatic reshape:
>
> ```c
> bool CheckDreamAuxTriggerCondition(s32 value, TriggerRecord *record)
> {
>     volatile unsigned int new_var;
>     s8 sel = record->sel;
>     s32 idx;
>     if (sel == 1) {
>         goto success;
>     }
>     if (sel < 0) {
>         if (record->unk0 != 0) {
>             return new_var = false;   /* forces a spurious store */
>         }
>         idx = (~sel) + 1;
>         goto have_idx;
>     }
>     /* ... rest unchanged ... */
> ```
>
> Compiled directly through this run's `permuter-work/CheckDreamAuxTriggerCondition/compile.sh`
> and objdumped (not just read as permuter-internal score): it grows the
> stack frame from retail's `addiu sp,sp,-0x18` to `addiu sp,sp,-0x20` and
> inserts a real `sw zero,0x10(sp)` for the `volatile` write -- a frame-size
> regression, worse in a more obvious way than run 1's candidate, and
> exactly the "UB/duplicate-arm form reaches a low score, no idiomatic
> translation matches it" case MATCHING-GUIDE.md's permuter section names as
> the stopping condition for this class, not a lead worth iterating on
> further by hand.
>
> **A permuter setup bug found and fixed LOCALLY** (not in the shared script,
> per parallel-mode rules) -- worth escalating. `tools/setup-permuter.sh`
> generates `compile.sh` with
> `MASPSX_FLAGS="--aspsx-version=2.34 --dont-force-G0 --expand-div"`, missing
> the Makefile's `--addiu-at` (`Makefile:44`). For a function that does not
> touch the `addiu_at` construct this is invisible; for THIS function (whose
> whole point is a jump-table dispatch) it means the permuter's own compiled
> candidates never get retail's unfolded dispatch, so no candidate could ever
> reach zero on it -- the run would still have measured the preamble residue
> correctly (dispatch mismatch dominates the score in one place, not both),
> but the scaffold itself was stale relative to the pinned toolchain. Also
> found: `sed -e '1,4d' "$asm"` (dropping the retail `.s`'s first four lines
> to build `target.s`) assumes those four lines are always just the two
> `.set` directives, a blank line, and `.section .text` -- true for an
> ordinary function, but **false for a function that owns embedded rodata
> ahead of itself** (this one: `.section .rodata` / the jump table / blank /
> THEN `.section .text`). Dropping line 4 here deletes the `.section .rodata`
> directive itself, so the jump table's bytes land in `.text` (prelude.inc's
> default section) instead, which corrupted `target.o` enough that
> `permuter.py`'s objdump-output parser crashed outright
> (`IndexError: list index out of range` in `simplify_objdump`). Both were
> patched only inside this run's gitignored `permuter-work/CheckDreamAuxTriggerCondition/`
> (never in the shared `tools/setup-permuter.sh`, per this round's parallel-
> mode constraint) -- fix: keep `--addiu-at` in `compile.sh`'s
> `MASPSX_FLAGS`, and for `target.s` generation, drop only the TRUE
> boilerplate lines (here, `sed -e '1,3d'`, keeping `.section .rodata`)
> rather than a hardcoded `1,4d`. Whoever owns `tools/setup-permuter.sh`
> should fix this for every future embedded-rodata function, not just this
> one -- `InitDreamAux` in this same unit shares the "indexed-global folds
> through the same maspsx mechanism" trait, though it has no embedded rodata
> of its own so it did not hit the second bug.
>
> **Verdict stands as STALL, corrected to a clean 99/100.** The kept
> near-miss body below now has the case 6/7 merge; restored to `INCLUDE_ASM`,
> no C left in `src/`.

> **VERDICT CORRECTED, round 23 (2026-09-07), head. This report's own title
> and its round-13 correction both said this function "cannot be matched as C
> under the pinned toolchain". THAT IS NO LONGER TRUE.** The blocker it named
> was `addiu_at`, resolved in round 21 (maspsx `--addiu-at`,
> `docs/research/addiu-at-blocker.md`). The preserved body below was spliced
> back in and re-measured with the fix live:
>
> - **The switch dispatch now reproduces EXACTLY** -- `lui $at` /
>   `addiu $at, $at, %lo(jtbl_8001188C)` / `addu $at, $at, $v0` /
>   `lw $v0, 0x0($at)`, all four words, byte for byte. That was one of the two
>   residue words and it is gone.
> - **The residue is ONE word**, on a 100-word function: 99/100. It is retail's
>   redundant `j` over the switch-index join, described precisely below.
>
> So this is **the closest open near-miss in the corpus and a prime permuter
> target**, not an unmatchable function. Two shapes were re-tried this round
> with the blocker gone and both reproduce the round-13 result unchanged:
>
> | variant | result |
> | --- | --- |
> | plain `if/else` on `sel < 0`, no `goto` on the `sel >= 0` path | byte-identical to the `goto` form; **1 word short** |
> | `sel >= 0` tested FIRST (retail's own block order) | **3 words short** -- GCC hoists `move $a1, $a0` into the `bgez` delay slot and drops both the `j` and its `nop`, exactly as round 13 measured |
>
> **Why this went unnoticed for two rounds, which is the transferable part.**
> Round 22 swept the `addiu_at` reports and marked five with
> `REOPENED -- ASSIGNABLE`. This one was missed because it does not present as
> a stub: it is a long, detailed, twice-corrected report with a real attempt
> history, and its verdict sentence ("cannot be matched ... regardless of
> source shape") reads as a considered plateau rather than as a blocker
> citation. **A blocker's death invalidates every report that RELIED on it, not
> only the ones that look like stubs.** The cheap detector is
> `python3 tools/nearmiss.py`, which lists this function in its
> blocker-CLEAN section while the report says otherwise -- a report contradicting
> the live screen is the signal.
>
> It is deliberately NOT marked `REOPENED -- ASSIGNABLE`: it is genuinely
> worked ground with a characterised one-word residue, and `progress.py` should
> keep counting it as a documented stall rather than returning it to `fresh`.


> **HEAD CORRECTION, round 13 (2026-09-03).** This report was filed as
> "extremely close: 1 word / 4 bytes short", cause attributed to an
> instruction-selection preference in the `sel<0` preamble. Re-measured with
> asm-differ: the body is **TWO** words short, and one of the two is
> `addiu $at, $at, %lo(jtbl_8001188C)` -- the **`addiu-at` jump-table folding
> blocker** (`docs/research/addiu-at-blocker.md`). Retail's switch dispatch is
> the UNFOLDED four (`lui $at` / `addiu $at` / `addu $at` / `lw 0($at)`); the
> pinned pipeline emits the FOLDED three (`lui $at` / `addu $at` /
> `lw %lo(...)($at)`). **This function cannot be matched as C under the pinned
> toolchain regardless of source shape**, so it is a blocked function, not a
> near-miss, and it should not be staffed again until that blocker is resolved.
>
> The tell was in this report's own file all along and was never run:
> `grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/code_4cd08/CheckDreamAuxTriggerCondition.s`
> hits at line 66. CLAUDE.md's screen says in terms that a `%lo(jtbl_*)` hit
> counts; the screen was not applied to this function because it had already
> been reasoned about as a preamble problem. **The blocker screen is a
> precondition for a stall classification, not just for assignment.**
>
> One correction to the report's own accounting, though, went the other way:
> the preamble residue IS partly reachable, and the fix is below.

Unit `code_4cd08` ("DreamAux"). Restored to `INCLUDE_ASM`; no C left in
`src/`. Owns the `0x206C` rodata slot's jump table (`jtbl_8001188C`, 20
entries) -- untouched, stays embedded in `asm/nonmatchings/code_4cd08/CheckDreamAuxTriggerCondition.s`
since the function reverted to `INCLUDE_ASM`.

Every branch target and every case body in this ~100-word, 20-case switch
function matches retail byte-for-byte. The residue is **two instructions**:
one in the `sel<0` preamble that computes the switch index, and one in the
`switch` DISPATCH itself -- the latter being the `addiu-at` folding blocker,
which is what makes the function unmatchable. The body is 8 bytes / two
words shorter than retail.

The original text here read "except one instruction ... consistently 4
bytes (one word) shorter ... confirmed via `build-and-verify.sh`'s
whole-image diff staying at a constant byte count". That method is what
produced the wrong count: compiling this `switch` from C replaces the `.s`
file's embedded `jtbl_8001188C` with GCC's own copy, relocating rodata and
shifting the entire image, so the whole-image byte count is dominated by
drift and cannot size a per-function residue at all. asm-differ's
inserted/deleted markers can, and they say two.

## What it does

`CheckDreamAuxTriggerCondition(value, record)`: reads `record->sel` (offset `0x1`,
signed byte). If it's `1`, succeeds immediately. Otherwise derives a switch
index `idx`: if `sel >= 0`, `idx = sel`; if `sel < 0`, fails immediately
when `record->triggered` (offset `0x0`, signed byte; named round 63 --
see `## Naming` below) is nonzero, else `idx = -sel`. Dispatches on
`idx - 2` through a 20-entry jump table:

- 0,1,2: delegate to `IsDayInPeriodPhase(value, idx - 1)`.
- 3: succeed iff `value % 3 == 0`.
- 4: succeed iff `value % 3 != 0`.
- 5: succeed iff `IsStyleVariantEven()` (no args) is truthy.
- 6: succeed iff `value % 3 == 1`.
- 7: succeed iff `value % 3 == 2`.
- 18: succeed iff `value` is even.
- 19: succeed iff `value` is odd.
- default (indices 8-17, and anything outside the table's `idx` range of
  roughly `[2, 22)`): succeed unconditionally if `idx < 10`; otherwise
  delegate to `IsCurrentDreamColor(idx)`.

On any success path, `record->triggered` is set to `1` before returning `true`.

## Best body, ROUND 24 (compiles, builds green, 99/100 -- ONE word short, no blocker)

Supersedes the "Best body" block immediately below it, which is 8 WORDS TOO
LONG as written (missing the case 6/7 merge -- see the round-24 note at the
top of this report). Kept for history since its per-case bodies and overall
shape are otherwise identical and it is what round 13/23's attempt history
below refers to.

```c
#include "common.h"
#include "code_4cd08.h"

bool CheckDreamAuxTriggerCondition(s32 value, TriggerRecord *record)
{
    s8 sel = record->sel;
    s32 idx;

    if (sel == 1) {
        goto success;
    }

    if (sel < 0) {
        if (record->unk0 != 0) {
            return false;
        }
        idx = ~sel + 1;
        goto have_idx;
    }
    idx = sel;

have_idx:

    switch (idx - 2) {
    case 0:
    case 1:
    case 2:
        if (!IsDayInPeriodPhase(value, idx - 1)) {
            return false;
        }
        break;
    case 3:
        if (value % 3 != 0) {
            return false;
        }
        break;
    case 4:
        if (value % 3 == 0) {
            return false;
        }
        break;
    case 5:
        if (!IsStyleVariantEven()) {
            return false;
        }
        break;
    case 6:
    case 7:
        if (value % 3 != idx - 7) {
            return false;
        }
        break;
    case 18:
        if ((value & 1) != 0) {
            return false;
        }
        break;
    case 19:
        if ((value & 1) == 0) {
            return false;
        }
        break;
    default:
        if (idx >= 10) {
            if (!IsCurrentDreamColor(idx)) {
                return false;
            }
        }
        break;
    }

success:
    record->unk0 = 1;
    return true;
}
```

The one remaining word is the same "redundant `j` over the switch-index
join" residue documented below (retail places the `sel >= 0` arm BEFORE the
`~sel + 1` arm and needs an explicit jump over it; GCC places `sel >= 0` last
and falls through instead) -- unchanged by the case 6/7 fix, and still not
closed by any hand attempt or by the round-24 permuter runs (see the note at
the top of this report).

## Best body (compiles, builds green, 2 words short -- one of them the blocker)

```c
#include "common.h"
#include "code_4cd08.h"

bool CheckDreamAuxTriggerCondition(s32 value, TriggerRecord *record)
{
    s8 sel = record->sel;
    s32 idx;

    if (sel == 1) {
        goto success;
    }

    if (sel < 0) {
        if (record->unk0 != 0) {
            return false;
        }
        idx = ~sel + 1;
        goto have_idx;
    }
    idx = sel;

have_idx:

    switch (idx - 2) {
    case 0:
    case 1:
    case 2:
        if (!IsDayInPeriodPhase(value, idx - 1)) {
            return false;
        }
        break;
    case 3:
        if (value % 3 != 0) {
            return false;
        }
        break;
    case 4:
        if (value % 3 == 0) {
            return false;
        }
        break;
    case 5:
        if (!IsStyleVariantEven()) {
            return false;
        }
        break;
    case 6:
        if (value % 3 != 1) {
            return false;
        }
        break;
    case 7:
        if (value % 3 != 2) {
            return false;
        }
        break;
    case 18:
        if ((value & 1) != 0) {
            return false;
        }
        break;
    case 19:
        if ((value & 1) == 0) {
            return false;
        }
        break;
    default:
        if (idx >= 10) {
            if (!IsCurrentDreamColor(idx)) {
                return false;
            }
        }
        break;
    }

success:
    record->unk0 = 1;
    return true;
}
```

Needs `TriggerRecord`, `IsDayInPeriodPhase`, `IsStyleVariantEven`, `IsCurrentDreamColor` from
`include/code_4cd08.h` (already added this round -- `IsDayInPeriodPhase` is
matched, see its own report; `IsStyleVariantEven`/`IsCurrentDreamColor` are still
`INCLUDE_ASM` elsewhere in this unit and off-limits this round, gp-relative-
blocked per `docs/research/gp-relative-blocker.md`).

## The residue, precisely

Retail's sel-derivation preamble (`sel<0` path):

```
lb   a0, 1(s0)          ; a0 = sel
li   v0, 1
beq  a0, v0, <success>
bgez a0, <sel>=0 path>
lb   v0, 0(s0)           ; v0 = record->unk0   (only reached when sel<0)
beqz v0, <continue: idx = -sel>
 nor v0, zero, a0         ; delay slot (ALWAYS runs): v0 = ~sel
j    <epilogue>            ; unk0 != 0 -> return false
 move v0, zero
<sel>=0 path>:
 j    <join>
  move a1, a0               ; idx = sel
<continue>:
 addiu a1, v0, 1             ; idx = ~sel + 1 = -sel
<join>:
 addiu v1, a1, -2             ; switch index = idx - 2
 ...
```

Every C shape tried in the ORIGINAL round compiled to the same, SHORTER
sequence (this is the pre-correction baseline; see the round-13 note below
for what the `negu` line should have been):

```
lb   a0, 1(s0)
li   v0, 1
beq  a0, v0, <success>
bgez a0, <join>              ; sel>=0 skips straight to <join> with idx=sel
                              ; already in a1 (hoisted into this branch's
                              ; own delay slot, no separate jump needed)
lb   v0, 0(s0)
bnez v0, <return false>      ; INVERTED polarity vs retail's beqz
 move v0, zero
negu a1, a0                  ; single instruction, not nor+addiu
<join>:
 addiu v1, a1, -2
 ...
```

**Round 13 correction: half of this residue WAS reachable.** The `negu`
-vs- `nor`+`addiu` half is a source-shape difference, not a codegen
preference. Retail computes the negation as `nor $v0, $zero, $a0` followed
by `addiu $a1, $v0, 1` -- literally `~sel + 1`, in two instructions.
Writing `idx = -sel;` gets GCC 2.6.3's single `negu`; writing
`idx = ~sel + 1;` gets retail's `nor` + `addiu` exactly. All seven attempts
below used `-sel`, so none of them could reach it. With `~sel + 1` the
preamble residue collapses to a single word.

What is left after that, and is NOT reachable, is retail's redundant `j`
over the switch-index join. Retail lays the `sel>=0` block FIRST (so it
must jump over the `sel<0` continue block to reach the join); GCC lays the
`sel<0` continue block first and lets `sel>=0` fall through. Two further
round-13 attempts on exactly this:

- **Polarity** (`if (unk0 == 0) { idx = ~sel + 1; goto have_idx; } return
  false;`, matching retail's `beqz`-to-continue) -- byte-identical output to
  the `!= 0` early-return form. Confirms this report's attempt (4): GCC
  canonicalizes the branch direction and the polarity is not a lever.
- **Block order** (`if (sel >= 0) { idx = sel; goto have_idx; }` first, so
  the `~sel + 1` block sits adjacent to the join, which is retail's own
  layout) -- **worse, 3 words short.** GCC hoisted `move $a1, $a0` into the
  `bgez` delay slot and dropped both the `j` and its `nop`. Reproducing
  retail's block ORDER in the source made the compile MORE compact, not
  less.

So the standing residue is exactly two words: one blocker, one
block-ordering preference. Since the blocker alone makes the function
unmatchable, the remaining word is not worth further attempts.

## What was tried (all produced the IDENTICAL compiled result unless noted)

Checked against CLAUDE.md's rule 6 test and the head's branch-target
broadcast before writing this off -- every branch TARGET, once matched for
the size offset, lands on the logically-equivalent code on both sides; this
is genuinely an instruction-selection/scheduling choice, not a wrong CFG.

1. `sel >= 0` tested first, `else { if (unk0 == 0) ... else return false; }`
   (nested if/else) -- baseline, 1 word short.
2. Same, with `goto have_idx;` instead of falling out of the if/else (a
   literal one-to-one translation of retail's own `j` structure) --
   identical compiled output to (1).
3. `record->sel` accessed directly at each use vs. hoisted once into a
   local `s8 sel` -- no difference.
4. `if (record->unk0 == 0) { idx = -sel; ... } else { return false; }`
   (positive-first polarity) vs. `if (record->unk0 != 0) { return false; }
   idx = -sel;` (negative-first / early-return polarity) -- no difference;
   GCC canonicalizes both to the same branch direction.
5. Outer condition inverted: `if (sel < 0) { ...unk0 check...; idx = -sel;
   } idx = sel;` (tested `sel < 0` first instead of `sel >= 0` first) --
   the ONLY variant that measurably changed the output (the `bgez` target
   now matches retail exactly, and the redundant jump moved into the
   *continue* path instead of the *return* path), but the total instruction
   count and the "1 word short" residue were unchanged -- still a
   compact-vs-redundant mismatch, just relocated. This is the shape kept
   above since it lines up the most branch targets.
6. A bare `__asm__("");` as the very first statement, and again
   immediately before the `sel < 0` test -- no effect on word count either
   time; one placement (immediately after the `bgez`-equivalent branch,
   tried transiently) changed which register held `sel` (`a0` -> `a1`), a
   real register-identity change, so per CLAUDE.md rule 6 that placement is
   banned outright regardless of score. The placements that didn't change
   identity also didn't change the score, so no barrier placement found
   here is both permitted and useful.
7. A ternary (`idx = (sel >= 0) ? sel : -sel;`) computed unconditionally
   before a combined `if (sel < 0 && record->unk0 != 0) return false;` --
   **worse** (this function's own residue grew by roughly another word;
   confirmed via the whole-image byte-drift count going from 85827 to
   88204 bytes).

## Proposed learnings

- **`~x + 1` and `-x` are NOT interchangeable source shapes for GCC 2.6.3.**
  `-x` compiles to a single `negu`; `~x + 1` compiles to `nor` + `addiu`.
  Where retail shows `nor $vN, $zero, $rX` followed by `addiu $rY, $vN, 1`,
  the source said `~x + 1` and no amount of reshaping around a `-x` will
  reach it. Generally: **read retail's CHOICE OF INSTRUCTIONS as evidence
  about the source expression**, not only its control flow. A two-instruction
  encoding of something the compiler can do in one is usually the source
  spelling it out.

- **Run the blocker screen before accepting a stall CLASSIFICATION, not just
  before assigning work.** This report attributed a residue to instruction
  selection while an `addiu $at, $at, %lo(jtbl_*)` hit sat unexamined in the
  same `.s` file. A stall report's cause is the thing the next round acts on,
  so a misattributed cause is more expensive than a wrong score: it turns a
  blocked function into a permanent near-miss that keeps attracting attempts.

- **A residue count is a measurement, and "1 word short" was wrong by one.**
  The whole-image byte-drift number this report used to size the residue
  counts rodata relocation too -- compiling a `switch` from C replaces the
  `.s` file's embedded jump table with GCC's own, which shifts the whole
  image and swamps the signal. Size a per-function residue with asm-differ's
  inserted/deleted instruction markers, never with the whole-image count.

- **Not every "retail keeps a redundant jump that a smarter compile would
  drop" residue is reachable by reshaping the surrounding conditional.**
  Six distinct if/else/goto/ternary encodings of the exact same 3-way
  branch (`sel==1` / `sel>=0` / `sel<0 && unk0`) all compiled to the more
  efficient form; only the *choice of which branch carries the extra
  jump* moved, never whether one exists. Contrast with `ProcessDreamAuxTriggerRecord`
  (same round, same unit) where a genuinely analogous-looking "extra
  jump" residue WAS reachable, by switching `do-while` to a pre-test
  `while` -- the difference there was a real CFG change (confirmed via
  branch-target alignment per the head's broadcast), not an instruction-
  selection preference. Always do the branch-target check before assuming
  either "this is fixable" or "this is a scheduling stall" -- both
  conclusions need the same evidence, and this function is the
  counter-example to file next to `ProcessDreamAuxTriggerRecord`'s.

## Proposed learnings, round 24

- **A jump table can point the SAME slot at two (or more) different case
  values.** `switch` case-order recovery (documented above and in
  `docs/DECOMPILATION_LEARNINGS.md`) assumed one arm per index; this
  function's table has indices 6 and 7 both pointing at `.L8005CCC8`. The
  tell is identical `.word` labels at different jump-table offsets in the
  `.s` file -- when seen, the source must merge those case values into one
  `case A: case B:` arm with a condition that depends on the dispatched
  value (here `idx - 7`), not duplicate the arm with a hardcoded constant.
  Failing to notice this cost 8 words (a wrong "108/100", read as "99/100"
  by trusting an inherited figure instead of rebuilding it).

- **Rebuild every inherited figure before trusting it, even ones with a
  detailed derivation.** This report's round-23 "99/100, corrected" verdict
  read as settled -- it had a derivation, a table of re-tried variants, and
  an explicit instruction not to re-derive the old (worse) verdict. It was
  still wrong, because the body it measured was incomplete. A confident
  write-up is not evidence the underlying number was re-measured against
  the CURRENT source; only an `nm`/`asm-differ` run against a freshly built
  object is.

- **The permuter setup script (`tools/setup-permuter.sh`) is stale against
  the pinned Makefile for `addiu_at`-touching functions, and has a second,
  independent bug for functions owning embedded rodata.** See the round-24
  note at the top of this report for both fixes (applied locally to this
  run's `permuter-work/`, not to the shared script, per parallel-mode
  rules). Any function whose report recommends "reach for the permuter"
  and which involves a jump table, or any indexed-global fold, should have
  its `permuter-work/<func>/compile.sh` checked for `--addiu-at` before
  trusting a "no improvement found" result -- an absent flag means the
  scaffold was never capable of finding retail's unfolded form regardless
  of how long the search runs.

- **A permuter score improvement is not evidence of a smaller residue --
  confirmed again.** The one improving candidate found (permuter score 330
  vs base 420) reorders two independent statements; translated to real C it
  is 98/100, two words worse than the 99/100 it started from. Consistent
  with MATCHING-GUIDE.md's existing caution (three prior false leads in
  round 18) -- this is the fourth measured instance, on a different
  function, in a different round.

## Naming

**CheckDreamAuxTriggerCondition** — tier B. A multi-branch predicate: derives
a switch index from `record->sel` (short-circuiting to `false` via the new
`triggered` field when `sel < 0` and the record already fired once), then
dispatches through a ~20-entry jump table of small, heterogeneous checks
(mod-3 residues, parity, a day/range-bucket delegate, a world-state
delegate, an unconditional pass for a middle band of indices). The mechanics
are exactly what the name says -- gate on a per-record condition -- but the
game-level meaning of `sel`'s cases is not established from this unit alone,
hence B not A. Renamed `record->unk0` to `triggered` in the struct
definition as part of this pass (see the commit renaming struct fields);
every accessor was confined to this unit, confirmed by rebuild.

## Round 100 (alpha): track 7, moved from src/code_4cd08.c and include/code_4cd08.h

`value` -> `day`, `sel` -> `condition`, `idx` -> `id`. The ids are
enum TriggerCondition (include/code_4cd08.h), and `switch (idx - 2)` with
cases 0..19 is `switch (id)` with the enum's cases: GCC subtracts the lowest
case itself, byte-identical. Two MATCHING lines stand for the derivation
below and above: the `id = condition; goto have_idx;` arm placed before
`negate:`, and `~condition + 1` (nor + addiu, where -condition is one negu).

The function comment, as it stood:

```c
/* CheckDreamAuxTriggerCondition -- MATCHED round 25.  The last word came from BASIC-BLOCK
 * ORDER, not from the expression shapes.  Retail lays the `sel >= 0` arm
 * out BETWEEN the `return false` path and the `~sel + 1` tail, so it needs
 * an explicit `j` over the join; the obvious spelling
 * (`if (sel < 0) { ...; idx = ~sel + 1; goto have_idx; } idx = sel;`) lets
 * the `sel >= 0` arm fall through into the join instead and is one word
 * short forever.  Writing the inner test as `if (triggered == 0) goto negate;
 * return false;`, with the `idx = sel; goto have_idx;` block placed
 * textually BEFORE the `negate:` label, reproduces retail's block order
 * exactly.  See docs/match-reports/CheckDreamAuxTriggerCondition.md.
 */
```
