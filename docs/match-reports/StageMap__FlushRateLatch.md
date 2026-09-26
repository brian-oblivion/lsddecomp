# StageMap__FlushRateLatch — MATCHED (18/18 words)

> Renamed from `Class866E8__FlushRateLatch` on 2026-09-26 (tools/rename.py). Address 0x8004d088.

> Renamed from `func_8004D088` on 2026-09-24 (tools/rename.py). Address 0x8004d088.

Not a vtable slot in this unit's own dispatch (`class_3ac78.h` documents
it as its OWN independent view's `StageMapMethods::slot140`, called
from that unit's `StageMap__ResetAllElements` — not this unit's concern; here it is
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
lui   $a1, %hi(StageMap__ResetChildRate)
addiu $a1, $a1, %lo(StageMap__ResetChildRate)
jal   StageMap__ForEachElem
 move $a2, $zero
sw    $zero, 0x1E0($s0)    ; self->unk1E0 = 0 (unconditional on this path)
.skip:
...epilogue
```

## Final C

```c
void StageMap__FlushRateLatch(Obj866E8 *self) {
    if (self->unk1E0 != 0) {
        StageMap__ForEachElem(self, StageMap__ResetChildRate, 0);
        self->unk1E0 = 0;
    }
}
```

`StageMap__ForEachElem` is still `INCLUDE_ASM` in this unit; forward-declared per
the established "calling into a still-INCLUDE_ASM function is fine"
convention. Its own signature was derived from THIS call site plus
`StageMap__StepScaleRamp`'s (own report): `(Obj866E8 *self, void
(*itemCallback)(Obj866E8*, Unk10ChildObj_3bb8c_b*), void
(*perArrCallback)(Obj866E8*, Elem*))` — both known callers pass 0 for the
third argument, so its true type is inferred from `StageMap__ForEachElem`'s own
body (still unmatched) rather than confirmed live.

## New struct/global knowledge

- `Obj866E8::unk1E0` (`s32`, +0x1E0) — a gate/countdown value, also used
  by the sibling `StageMap__StepScaleRamp` (own report).
- `extern void StageMap__ForEachElem(...)` added (still raw asm in this unit).

## Attempts

1 (matched on first attempt).

### Proposed learning

None new.

## Naming

**Tier B.** Not a vtable slot. One-shot sibling of
`StageMap__StepScaleRamp`: if `self->rateCountdown != 0`, resets
every child's rate (`ForEachElem(self, ResetChildRate, 0)`) and clears the
latch to 0 in a single call, no per-tick decrement. "Latch" distinguishes
it from the countdown sibling -- it fires once and clears, rather than
ticking down.
