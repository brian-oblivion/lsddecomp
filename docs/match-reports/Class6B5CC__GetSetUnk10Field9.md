> Renamed from `func_8001D480` on 2026-09-18 (tools/rename.py). Address 0x8001d480.

# Class6B5CC__GetSetUnk10Field9 -- MATCHED (11/11 words)

Round 12, runner delta. `code_d294_b`.

## Summary

Same family as `func_8001D424` (see that report) -- raw pass-through wrapper
around `GetSetBitField(&self->unk10, shift, width, value)`, shift 9, width 3.

```c
u32 Class6B5CC__GetSetUnk10Field9(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 9, 3, a1);
}
```

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/Class6B5CC__GetSetUnk10Field9.s`):
```
addu $a3, $a1, $zero      # a3 (value) = a1
addiu $a0, $a0, 0x10      # a0 = &self->unk10
ori  $a1, $zero, 0x9      # a1 (shift) = 9
jal  GetSetBitField
 ori $a2, $zero, 0x3      # a2 (width) = 3
```
No post-processing of `$v0`.

### Proposed learning

None beyond the family census already noted in `func_8001D424.md`.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D480` via `tools/rename.py`. **Tier A** -- same
reasoning as `Class6B5CC__GetSetUnk10Flag7`: a pure bitfield accessor
(shift 9, width 3, raw pass-through), mechanics fully known, field's
real purpose not established. Purely local to this unit + its header.
