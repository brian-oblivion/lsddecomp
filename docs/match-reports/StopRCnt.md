# StopRCnt -- MATCHED (14/14 words)

> Renamed from `ClearIrqMask` on 2026-09-23 (tools/rename.py). Address 0x80032c28.

> Renamed from `func_80032C28` on 2026-09-23 (tools/rename.py). Address 0x80032c28.

Unit: `libsnd_ssinit`. Round 23, runner bravo.

## What it does

Clears an IRQ mask bit in the same shadow interrupt-controller pair as
`StartRCnt` above (`D_8006DCAC` -> I_STAT/I_MASK pair, `+0x4` is I_MASK;
`D_8006DCB4[which]` selects the bit). Unlike its sibling, the return value is
an unconditional `1` (`ori $v0, $zero, 0x1` sitting in the `jr $ra` delay
slot) -- there is no `slti` bounds check computed here at all, dead or
otherwise.

## Final C

Reuses the `IrqRegs` type and `D_8006DCAC`/`D_8006DCB4` externs declared for
`StartRCnt` immediately above it in the unit (see that report for the
struct and the field's `volatile` rationale).

```c
s32 StopRCnt(u16 which)
{
    s32 idx = which;
    IrqRegs *reg = D_8006DCAC;

    reg->mask &= ~D_8006DCB4[idx];
    return 1;
}
```

## Residue and how it closed

None -- matched on the first attempt, once `StartRCnt`'s `IrqRegs` fields
were declared `volatile`. Without that (tried while closing the sibling),
this same idiom for `StartRCnt` scored 9/14 because GCC hoisted an
independent store into a branch delay slot instead of leaving the `nop`
retail has; `volatile` fields prevent that hoist project-wide, so this
function needed no extra work. Confirmed byte-exact, `build exit=0`.

## Proposed learning

See `StartRCnt.md` -- same idiom, same fix, filed once there to avoid
duplicating the explanation.

## Naming

Round 69 (delta). `StopRCnt` (was `func_80032C28`): the inverse of
`StartRCnt` (`D_8006DCAC->mask &= ~D_8006DCB4[idx]`). Tier A, same
reasoning as `StartRCnt.md`.

## Identification (round 69, head)

Sony's `StopRCnt`, `libapi/counter`: the five functions at 0x80032B18 are that module's exports in its own order (SetRCnt, GetRCnt, StartRCnt, StopRCnt, ResetRCnt), with the first three offsets exact against the 3.5/3.6 `counter.o` and the last two 4 and 8 bytes later (a library build the discs do not carry, so no object can be linked); `KERNEL.H` prototypes agree on arity. Two evidence kinds per FINISHING-PLAN track 2. Delta had named it `ClearIrqMask` (mechanically right); renamed at merge.
