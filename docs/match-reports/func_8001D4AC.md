# func_8001D4AC -- MATCHED (12/12 words)

Round 12, runner delta. `code_d294_b`.

## Summary

Same family as `Class6B5CC__GetSetUnk10Flag7` (see that report) -- double-inversion
(`a1 == 0` in, `== 0` on the result) wrapper around `GetSetBitField`, shift 8
width 1, `s32` return type.

```c
s32 func_8001D4AC(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 8, 1, a1 == 0) == 0;
}
```

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/func_8001D4AC.s`):
```
sltiu $a3, $a1, 0x1       # a3 (value) = (a1 == 0)
addiu $a0, $a0, 0x10      # a0 = &self->unk10
ori   $a1, $zero, 0x8     # a1 (shift) = 8
jal   GetSetBitField
 ori  $a2, $zero, 0x1     # a2 (width) = 1
sltiu $v0, $v0, 0x1       # result = (raw == 0)
```

Note: `include/class_3ac78.h` documents an UNRELATED cross-unit call that
also names this symbol `func_8001D4AC` but through a different table
(`Class86668::unk34`) with a 4-argument `(self, arg1, arg2, arg3)` shape at
its own local slot `+0x080`. That's the same "same code address, different
argument count per call site" precedent already established for
`func_8001E57C` elsewhere in this unit -- it does not affect this unit's own
implementation, which is typed only to this unit's own call sites
(`GetSetBitField` and the vtable slot `Class6B5CCMethods::+0x080`, confirmed
via `tools/classtable.py D_8006B5CC`).

### Proposed learning

None new -- extends the family census.
