# Class6B5CC__GetSetUnk10Flag8 -- MATCHED (12/12 words)

> Renamed from `func_8001D4AC` on 2026-09-18 (tools/rename.py). Address 0x8001d4ac.

Round 12, runner delta. `code_d294_b`.

## Summary

Same family as `Class6B5CC__GetSetUnk10Flag7` (see that report) -- double-inversion
(`a1 == 0` in, `== 0` on the result) wrapper around `GetSetBitField`, shift 8
width 1, `s32` return type.

```c
s32 Class6B5CC__GetSetUnk10Flag8(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 8, 1, a1 == 0) == 0;
}
```

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/Class6B5CC__GetSetUnk10Flag8.s`):
```
sltiu $a3, $a1, 0x1       # a3 (value) = (a1 == 0)
addiu $a0, $a0, 0x10      # a0 = &self->unk10
ori   $a1, $zero, 0x8     # a1 (shift) = 8
jal   GetSetBitField
 ori  $a2, $zero, 0x1     # a2 (width) = 1
sltiu $v0, $v0, 0x1       # result = (raw == 0)
```

Note: `include/class_3ac78.h` documents an UNRELATED cross-unit call that
also names this symbol `Class6B5CC__GetSetUnk10Flag8` but through a different table
(`Class86668::unk34`) with a 4-argument `(self, arg1, arg2, arg3)` shape at
its own local slot `+0x080`. That's the same "same code address, different
argument count per call site" precedent already established for
`GetClass6B5CCMethods` elsewhere in this unit -- it does not affect this unit's own
implementation, which is typed only to this unit's own call sites
(`GetSetBitField` and the vtable slot `Class6B5CCMethods::+0x080`, confirmed
via `tools/classtable.py D_8006B5CC`).

### Proposed learning

None new -- extends the family census.

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `Class6B5CC__GetSetUnk10Flag8`
(tier A: pure bitfield accessor, shift 8 width 1, double-inverted
boolean -- same shape as the renamed `Class6B5CC__GetSetUnk10Flag7`).
Held back because this symbol is name-checked (in comments, not calls)
from `src/class_3bb8c_o.c:186` and `include/class_3ac78.h:173` -- two
different units' own vtable-slot census comments, both discussing a
coincidental address match in an unrelated table (`D_800876FC`'s own
slot80, a different class entirely). Renaming would edit those files
too, out of this round's scope. Posted to the broadcast.
