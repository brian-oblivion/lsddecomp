# func_8005CBC8 -- STALL (extremely close: 1 word / 4 bytes short)

Unit `code_4cd08` ("DreamAux"). Restored to `INCLUDE_ASM`; no C left in
`src/`. Owns the `0x206C` rodata slot's jump table (`jtbl_8001188C`, 20
entries) -- untouched, stays embedded in `asm/nonmatchings/code_4cd08/func_8005CBC8.s`
since the function reverted to `INCLUDE_ASM`.

Every branch target and every case body in this ~100-word, 20-case switch
function matches retail byte-for-byte **except one instruction**, in the
three-way preamble that computes the switch index. The function is
consistently 4 bytes (one word) shorter than retail across every source
shape tried, confirmed via `./build-and-verify.sh`'s whole-image diff
staying at a constant byte count regardless of which equivalent C was used.

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

## Near-miss body (compiles, builds green, 1 word short)

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
        idx = -sel;
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

Every C shape tried here compiles to the same, SHORTER sequence:

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

Both compute the identical value in `a1`/`idx` and reach the identical
`<join>` point with identical live registers -- this is a real, provably
correct compilation of the same algorithm, just more efficient than
retail's (retail keeps an extra, avoidable `j`+delay-slot pair that a
"smarter" codegen would drop). GCC 2.6.3 apparently always prefers the
efficient form for this exact shape; no C-level reshaping tried made it
choose the less-efficient one.

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

## Proposed learning

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
