# Class866E8__AdvanceRateCountdown — MATCHED (24/24 words)

> Renamed from `func_8004D028` on 2026-09-24 (tools/rename.py). Address 0x8004d028.

Sibling of `Class866E8__FlushRateLatch` (own report) — same shape, one instruction
longer because the gate here is a genuine countdown rather than a
one-shot latch.

## Disassembly

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
move  $s0, $a0
sw    $ra, 0x14($sp)
lw    $v0, 0x1E0($s0)        ; v0 = self->unk1E0
blez  $v0, .skip
 nop
lui   $a1, %hi(Class866E8__ApplyRateToChild)
addiu $a1, $a1, %lo(Class866E8__ApplyRateToChild)
jal   Class866E8__ForEachElem
 move $a2, $zero
lw    $v0, 0x1E0($s0)         ; reload
addiu $v0, $v0, -1
bnez  $v0, .skip
 sw   $v0, 0x1E0($s0)          ; delay slot: self->unk1E0 = v0 (ALWAYS runs)
addiu $v0, $zero, -1
sw    $v0, 0x1E0($s0)           ; only reached when the decrement hit exactly 0
.skip:
...epilogue
```

The classic "default value (decrement), then conditionally overwritten by
an `if` with no `else`" idiom already documented in
`docs/DECOMPILATION_LEARNINGS.md`: the decrement's delay slot stores the
new value unconditionally, and the branch-not-taken path (decrement hit
0) overwrites it again with `-1`.

## Final C

```c
void Class866E8__AdvanceRateCountdown(Obj866E8 *self) {
    if (self->unk1E0 > 0) {
        Class866E8__ForEachElem(self, Class866E8__ApplyRateToChild, 0);
        self->unk1E0 -= 1;
        if (self->unk1E0 == 0) {
            self->unk1E0 = -1;
        }
    }
}
```

`Class866E8__ApplyRateToChild` is defined later in this file (ROM order), so it needs a
forward declaration here — same pattern already used for
`Class866E8__ResetChildRate` in `Class866E8__FlushRateLatch`.

## New struct knowledge

None new (reuses `Obj866E8::unk1E0`, established by `Class866E8__FlushRateLatch`).

## Attempts

1 (matched on first attempt — same "default, conditionally overwritten"
idiom already proven elsewhere in this project made the shape
recognizable immediately).

### Proposed learning

None new — confirms the existing idiom, does not extend it.
