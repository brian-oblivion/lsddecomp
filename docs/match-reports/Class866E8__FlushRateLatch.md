# Class866E8__FlushRateLatch — MATCHED (18/18 words)

> Renamed from `func_8004D088` on 2026-09-24 (tools/rename.py). Address 0x8004d088.

Not a vtable slot in this unit's own dispatch (`class_3ac78.h` documents
it as its OWN independent view's `Class866E8Methods::slot140`, called
from that unit's `Class866E8__ResetAllElements` — not this unit's concern; here it is
just a plain function with a real body).

## Disassembly

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
move  $s0, $a0
sw    $ra, 0x14($sp)
lw    $v0, 0x1E0($s0)     ; v0 = self->unk1E0
beqz  $v0, .skip
 nop
lui   $a1, %hi(Class866E8__ResetChildRate)
addiu $a1, $a1, %lo(Class866E8__ResetChildRate)
jal   Class866E8__ForEachElem
 move $a2, $zero
sw    $zero, 0x1E0($s0)    ; self->unk1E0 = 0 (unconditional on this path)
.skip:
...epilogue
```

## Final C

```c
void Class866E8__FlushRateLatch(Obj866E8 *self) {
    if (self->unk1E0 != 0) {
        Class866E8__ForEachElem(self, Class866E8__ResetChildRate, 0);
        self->unk1E0 = 0;
    }
}
```

`Class866E8__ForEachElem` is still `INCLUDE_ASM` in this unit; forward-declared per
the established "calling into a still-INCLUDE_ASM function is fine"
convention. Its own signature was derived from THIS call site plus
`Class866E8__AdvanceRateCountdown`'s (own report): `(Obj866E8 *self, void
(*itemCallback)(Obj866E8*, Unk10ChildObj_3bb8c_b*), void
(*perArrCallback)(Obj866E8*, Elem*))` — both known callers pass 0 for the
third argument, so its true type is inferred from `Class866E8__ForEachElem`'s own
body (still unmatched) rather than confirmed live.

## New struct/global knowledge

- `Obj866E8::unk1E0` (`s32`, +0x1E0) — a gate/countdown value, also used
  by the sibling `Class866E8__AdvanceRateCountdown` (own report).
- `extern void Class866E8__ForEachElem(...)` added (still raw asm in this unit).

## Attempts

1 (matched on first attempt).

### Proposed learning

None new.
