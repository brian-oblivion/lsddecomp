# SetIrqMask -- MATCHED (14/14 words)

> Renamed from `func_80032BF0` on 2026-09-23 (tools/rename.py). Address 0x80032bf0.

Unit: `code_179d8_c`. Round 23, runner bravo.

## What it does

Sets an IRQ mask bit in the shadow interrupt-controller pair pointed to by
`D_8006DCAC` (base `0x1F801070` = I_STAT; `+0x4` = I_MASK). `which` selects
which bit out of the `D_8006DCB4` table (`{0x10, 0x20, 0x40, 0x1}` --
Tmr0/Tmr1/Tmr2 IRQ bits for indices 0-2, VBLANK for index 3; this table is
also the one `SetRCnt` a few functions up implicitly matches against, since
that function manages root counters 0-2).

The store is unconditional -- there is no bounds check on `which` gating the
OR. The `slti $v0, $v0, 0x3` result (index < 3) is not discarded: nothing
overwrites `$v0` before `jr $ra`, so it IS the function's return value. This
reads like a boolean "was this a valid root-counter index" result computed
purely as a side observation, with the masking happening regardless of it.

## Final C

```c
typedef struct {
    volatile u32 stat; /* 0x0, I_STAT */
    volatile u32 mask; /* 0x4, I_MASK */
} IrqRegs;

extern IrqRegs *D_8006DCAC;
extern u32 D_8006DCB4[4];

s32 SetIrqMask(u16 which)
{
    s32 idx = which;
    IrqRegs *reg = D_8006DCAC;

    reg->mask |= D_8006DCB4[idx];
    return idx < 3;
}
```

## Residue and how it closed

First pass (plain `u32 mask`, not `volatile`) built clean but scored 9/14 with
a large "differs outside range" warning. `objdump -dr` on the built `.o`
showed the correct 13 *distinct* instructions with correct relocations, but
GCC's delay-slot filler had hoisted the independent `sw $v1, 0x4($a1)` store
into the `jr $ra` delay slot, dropping the explicit trailing `nop` retail has.
That makes the compiled function 13 instructions (0x34 bytes) instead of
retail's 14 (0x38) -- one word of address drift, which shifted every
following byte in the image and made the following function (`ClearIrqMask`)
compare against nothing meaningful.

Retail does NOT hoist that store. The fix: mark the hardware-register struct
fields `volatile`. `reg->mask` is memory-mapped I/O (I_MASK), so this is the
correct type regardless of the byte match, and it also stops GCC's reorg pass
from moving the store past the return-value computation into the delay slot,
reproducing retail's `sw` / `jr` / `nop` order exactly. Confirmed byte-exact,
`build exit=0`.

## Proposed learning

A `volatile` struct field is not just a modelling nicety for a hardware
register -- it can be the difference between GCC filling a branch delay slot
with the field's store and leaving the `nop` retail has. When a body compiles
one instruction short with a plausible-looking delay-slot hoist, and the
field being stored is memory-mapped I/O, try `volatile` before treating it as
a scheduling residue to chase with a barrier.
