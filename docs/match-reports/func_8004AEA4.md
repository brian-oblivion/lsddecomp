# func_8004AEA4 — MATCH

**Unit:** class_3ac78 · **Size:** 79 instructions · **Result:** 79/79 words

## What it does

`Class866E8Methods` slot `+0x12C`, dispatched from `func_8004ADD8`'s
`slotD0` chain. Computes a "gate" argument (`0` normally, or
`(u8*)arg1->unk14 + 0x38` when `arg1->unk0C != 0`) and passes it to
`self->methods->slot110(self, &buf, gate)`; if that call returns nonzero,
returns immediately. Otherwise: saves `self->unk88` and the whole 0x30-byte
`self->unk8C` block; calls either `func_8004AFE0(self, &buf, 3)` or
`func_8004B030(self, &buf, 3)` depending on whether
`self->unk68->unk4 == 0`; calls `func_8004B100(self, arg1, arg2)`; then
restores `self->unk88` and `self->unk8C` from the saved copies.

## Final source

```c
extern void func_8004AFE0(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2);
extern void func_8004B030(Class866E8 *self, UnkArgObj_3ac78 *arg1, s32 arg2);
extern void func_8004B100(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2);

void func_8004AEA4(Class866E8 *self, UnkListObj_3ac78 *arg1, s32 arg2)
{
    s32 gateArg;
    HistoryBlock_3ac78 saved;
    UnkArgObj_3ac78 buf;
    s32 savedUnk88;

    if (arg1->unk0C != 0) {
        gateArg = (s32)((u8 *)arg1->unk14 + 0x38);
    } else {
        gateArg = 0;
    }

    if (self->methods->slot110(self, &buf, gateArg) != 0) {
        return;
    }

    savedUnk88 = self->unk88;
    saved = self->unk8C;

    if (self->unk68->unk4 == 0) {
        func_8004AFE0(self, &buf, 3);
    } else {
        func_8004B030(self, &buf, 3);
    }

    func_8004B100(self, arg1, arg2);

    self->unk88 = savedUnk88;
    self->unk8C = saved;
}
```

`func_8004AFE0` and `func_8004B030` are both defined later in this same
unit/file (matched and STALLED respectively) — the `extern` prototypes
above are ordinary forward declarations, needed only because ROM address
order puts `func_8004AEA4` before both of them in `src/class_3ac78.c`.
`func_8004B100` (117 instructions, out of this round's scope) gets the
same treatment per DECOMPILATION_LEARNINGS' "calling into a function
still `INCLUDE_ASM` elsewhere is fine" precedent.

## Corrections this function forced to EARLIER struct knowledge

This function's evidence is stronger than `func_8004B030`'s (which never
closed — see its own STALL report), and it directly contradicts two
fields that stalled attempt tentatively introduced. Both are now fixed in
`include/class_3ac78.h`, and `func_8004B030.md` carries a matching
"Update" note per the round's instructions:

1. **`self->unk8C` is a 3-element, 0x30-byte block (`HistoryBlock_3ac78`
   wrapping `HistoryEntry_3ac78 e[3]`), not a standalone 4-byte pointer
   field.** `func_8004B030`'s stalled attempt modeled `self->unk8C` as a
   single `void *` because that's the only field IT writes there. This
   function copies the WHOLE 0x30-byte region (`self+0x8C` to `self+0xBC`)
   with a batched 4-word-per-iteration loop on both the save and the
   restore side — the classic "whole-struct assignment, not an indexed
   loop" block-move signature already established by `func_80025E1C`
   (see that report for the precedent). `func_8004B030`'s single-pointer
   write is consistent with this as `unk8C.e[0].unk0` — the STALLED
   report's inline body is left as-is (it's preserved code, not doctrine)
   but the header field it depends on has moved.
2. **`self->unk68` is a pointer (`UnkPtr68Obj_3ac78 *`), not a plain
   `s32`.** This function dereferences it and reads a field at `+0x4`.
   `func_8004B344` (already matched) only ever STORES a raw word there and
   never dereferences it, so retyping its own parameter to the pointer
   type changes nothing about its compiled bytes — confirmed by rebuilding
   after the retype: still byte-exact, whole-image SHA1 still green.

Also extended `UnkListObj_3ac78` with `unk0C` (an `s32` gate flag) — safe,
it was previously undifferentiated padding that `func_8004AA6C` (the
type's only other user) never touches.

## Residue and the fixes that closed it (70/79 -> 79/79 in one more attempt)

Two independent issues on the first attempt:

1. **The two stack locals landed in the wrong SLOTS relative to each
   other.** `buf` (`UnkArgObj_3ac78`, passed to `slot110`/`func_8004AFE0`/
   `func_8004B030`) needs to sit at `sp+0x40`; the save/restore
   `HistoryBlock_3ac78` needs `sp+0x10`. Declaring `buf` textually FIRST
   put it at `sp+0x10` instead. Swapping the declaration order (history
   block first, `buf` second) flipped the stack assignment to match.
2. **Branch polarity on the `unk68->unk4` dispatch.** Retail's actual
   instruction is `bnez $v0,ALT` where the FALLTHROUGH (untaken) path
   calls `func_8004AFE0` and the branch target calls `func_8004B030` — the
   fallthrough is the `!= 0`-is-false case. Writing the natural-reading
   `if (self->unk68->unk4 != 0) { func_8004B030(...); } else {
   func_8004AFE0(...); }` compiles to the OPPOSITE polarity (`beqz`,
   `func_8004B030` as fallthrough) because GCC's default `if`/`else`
   lowering treats the `if` clause as the fallthrough branch. Inverting
   the source comparison (`== 0` calling `func_8004AFE0` first) reproduced
   retail's actual branch sense.

### Proposed learning

**Local-variable stack SLOT ASSIGNMENT is sensitive to declaration order
in a way that isn't obviously the same as "GCC allocates biggest-first"
or any other simple rule** — here, reversing the order of two same-scope
locals (a small struct vs. a bigger one) flipped which one landed at the
lower stack offset. When two stack buffers both get passed to different
calls and their absolute SP offsets don't match retail, try reordering
their declarations before reaching for anything more invasive.

**`if (cond) A; else B;` and `if (!cond) B; else A;` are not
interchangeable at the byte level even though they're logically
identical** — GCC's default `if`/`else` codegen makes the FIRST clause the
fallthrough and the `else` the branch target, so which physical branch
instruction (`beqz` vs `bnez`) and which block is JUMPED TO vs. FALLS
THROUGH is entirely a function of how the condition is spelled, not just
what it evaluates to. When retail's branch-taken target and your
source's `if`-body don't match up, try inverting the comparison before
suspecting anything deeper.

## Provenance

round 2026-09-02 (head-requested extension), runner ALPHA, unit
class_3ac78. First attempt 70/79 (two independent, both diagnosed and
fixed); second attempt 79/79.
