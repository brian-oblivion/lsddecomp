# func_8005CBC8 -- STALL (ONE word short; 99/100). Blocker GONE as of round 21.

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
> `grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/code_4cd08/func_8005CBC8.s`
> hits at line 66. CLAUDE.md's screen says in terms that a `%lo(jtbl_*)` hit
> counts; the screen was not applied to this function because it had already
> been reasoned about as a preamble problem. **The blocker screen is a
> precondition for a stall classification, not just for assignment.**
>
> One correction to the report's own accounting, though, went the other way:
> the preamble residue IS partly reachable, and the fix is below.

Unit `code_4cd08` ("DreamAux"). Restored to `INCLUDE_ASM`; no C left in
`src/`. Owns the `0x206C` rodata slot's jump table (`jtbl_8001188C`, 20
entries) -- untouched, stays embedded in `asm/nonmatchings/code_4cd08/func_8005CBC8.s`
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

`func_8005CBC8(value, record)`: reads `record->sel` (offset `0x1`,
signed byte). If it's `1`, succeeds immediately. Otherwise derives a switch
index `idx`: if `sel >= 0`, `idx = sel`; if `sel < 0`, fails immediately
when `record->unk0` (offset `0x0`, signed byte) is nonzero, else
`idx = -sel`. Dispatches on `idx - 2` through a 20-entry jump table:

- 0,1,2: delegate to `func_8005CDA8(value, idx - 1)`.
- 3: succeed iff `value % 3 == 0`.
- 4: succeed iff `value % 3 != 0`.
- 5: succeed iff `func_8005630C()` (no args) is truthy.
- 6: succeed iff `value % 3 == 1`.
- 7: succeed iff `value % 3 == 2`.
- 18: succeed iff `value` is even.
- 19: succeed iff `value` is odd.
- default (indices 8-17, and anything outside the table's `idx` range of
  roughly `[2, 22)`): succeed unconditionally if `idx < 10`; otherwise
  delegate to `func_8005CD58(idx)`.

On any success path, `record->unk0` is set to `1` before returning `true`.

## Best body (compiles, builds green, 2 words short -- one of them the blocker)

```c
#include "common.h"
#include "code_4cd08.h"

bool func_8005CBC8(s32 value, TriggerRecord *record)
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
        if (!func_8005CDA8(value, idx - 1)) {
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
        if (!func_8005630C()) {
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
            if (!func_8005CD58(idx)) {
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

Needs `TriggerRecord`, `func_8005CDA8`, `func_8005630C`, `func_8005CD58` from
`include/code_4cd08.h` (already added this round -- `func_8005CDA8` is
matched, see its own report; `func_8005630C`/`func_8005CD58` are still
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
  jump* moved, never whether one exists. Contrast with `func_8005CAB4`
  (same round, same unit) where a genuinely analogous-looking "extra
  jump" residue WAS reachable, by switching `do-while` to a pre-test
  `while` -- the difference there was a real CFG change (confirmed via
  branch-target alignment per the head's broadcast), not an instruction-
  selection preference. Always do the branch-target check before assuming
  either "this is fixable" or "this is a scheduling stall" -- both
  conclusions need the same evidence, and this function is the
  counter-example to file next to `func_8005CAB4`'s.
