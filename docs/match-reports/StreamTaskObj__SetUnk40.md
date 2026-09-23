# StreamTaskObj__SetUnk40

> Renamed from `func_8003BCF4` on 2026-09-23 (tools/rename.py). Address 0x8003bcf4.

**Unit:** code_2c054 · **Size:** 7 instructions (0x1C bytes) · **Status:** MATCHED (7/7 words, whole-image SHA1 green), first attempt

## What it does

Sets `self->unk40 = a1`, then, only when `a1 >= 0`, overwrites it with
`a1 * 15`. Textbook instance of the project's already-confirmed "default
value, then conditionally overwritten by an `if` with no `else`" idiom
(`docs/DECOMPILATION_LEARNINGS.md`, `CheckTriggerParity`): 2.6.3 slides the
unconditional store into the guarding branch's delay slot for free.

## Derivation

```
bltz  $a1, .L8003BD08
 sw   $a1, 0x40($a0)
sll   $v0, $a1, 4
subu  $v0, $v0, $a1        ; a1*16 - a1 == a1*15
sw    $v0, 0x40($a0)
.L8003BD08:
jr    $ra
 nop
```

```c
void StreamTaskObj__SetUnk40(StreamTaskObj *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}
```

Matched first attempt. `a1 * 15` reproduces retail's shift-subtract
multiply-by-constant expansion directly; no need to write the shift/subtract
by hand.

## New struct/header knowledge

Named `StreamTaskObj::unk40` in `include/code_2c054.h`.

## Proposed learning

Another confirmed instance of the "default value, conditionally overwritten,
no `else`" idiom from `docs/DECOMPILATION_LEARNINGS.md` -- worth keeping on
the shortlist of shapes to try first when the residue is "one extra
instruction" or a delay-slot store that looks unconditional at a glance.
