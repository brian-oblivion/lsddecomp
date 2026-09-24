# Obj86ED0__AdvanceCountdown -- MATCHED (26/26 words)

> Renamed from `func_80051784` on 2026-09-24 (tools/rename.py). Address 0x80051784.

Unit: `src/class_3bb8c_j.c` (class_3bb8c_j, newly carved round 15).

## Class identity

**ROUND 75 CORRECTION.** This section originally claimed `self` is
`Obj866E8` (the class established across class_3bb8c_b/c/etc, vtable
`D_800866E8`) and that the header additions below went onto that type.
That was wrong, caught by `tools/classtable.py D_800866E8` (which does
NOT contain this function's address at any of its 80 slots) versus
`tools/classtable.py D_80086ED0` (which places this function, and all
five siblings named below, at its +0x094..+0x0A8) -- `D_80086ED0` is
`Obj86ED0`, a DIFFERENT, ALREADY shared and fully-typed class in
`include/class_3bb8c.h`, established independently by class_3bb8c_i,
whose own struct already carries unk10/unk14/unk18/unk1C/unk20/unk48 and
whose own comments already tie slotA4/slotA8 to this unit's
`Obj86ED0__DispatchIndexValue`/`Obj86ED0__DispatchLookupValue` ("outside
this unit's slice"). See `src/class_3bb8c_j.c`'s file header comment for
the full evidence. Nothing about the matched BYTES was ever affected (a
type name is not codegen) -- only the class attribution and the `self`
type were wrong, both now fixed to the already-shared `Obj86ED0`/
`Obj86ED0Methods` with NO header edit needed (the type and every field
below already existed there).

`self` is `Obj86ED0`. This function only touches offsets 0x10-0x48, all
of which were previously unnamed padding there. Established this as a
"countdown/flush" subsystem alongside three siblings in this same unit
(`Obj86ED0__ToggleFlag20`, `Obj86ED0__ResetCountdown`,
`Obj86ED0__ResetAllAndFinish`) and two gp_rel-blocked siblings that touch the identical
fields (`Obj86ED0__DispatchIndexValue`, `Obj86ED0__DispatchLookupValue`).

## Header additions

None (round 75): the fields and slots below were already present on the
shared `Obj86ED0`/`Obj86ED0Methods` in `include/class_3bb8c.h`, established
by class_3bb8c_i. (The paragraph that used to describe additive edits to
`Obj866E8`/`Obj866E8Methods` here described edits to the WRONG type --
see the correction above. `Obj866E8`/`Obj866E8Methods` are untouched,
correctly used elsewhere by class_3bb8c_b/etc.)

`unk10`/`unk14`/`unk18`/`unk1C`/`unk20`/`unk48` -- already named on
`Obj86ED0`.

`slotA4`/`slotA8` -- already named on `Obj86ED0Methods`. `slotA8` is this
function's own dispatch target: `(self, arg1, arg2, arg3)`, called here
as `(self, self->unk18, count, 1)`.

## Body

```c
void Obj86ED0__AdvanceCountdown(Obj86ED0 *self)
{
    s32 count;

    if (self->unk48) {
        count = self->unk1C - 1;
        self->unk1C = count;
        if (count > 0) {
            self->methods->slotA8(self, self->unk18, count, 1);
        } else {
            self->unk1C = self->unk14;
        }
    }
}
```

## Notes

- `self->unk1C` is decremented and stored UNCONDITIONALLY (retail's
  `sw $a2,0x1C($a0)` sits in the `blez` branch's delay slot, which always
  executes) -- writing this as one statement (`count = ...; self->unk1C =
  count;`) before the branch reproduces that.
- **Branch-arm order was load-bearing.** The natural reading --
  `if (count <= 0) { reset } else { call }` -- compiled 2 words SHORT
  (24/26): GCC put the reset arm as the fall-through and needed an extra
  `j` to skip the call arm, while retail's fall-through IS the call arm
  and the reset arm is the branch target with no trailing jump. Swapping
  to `if (count > 0) { call } else { reset }` reproduced retail's exact
  branch polarity and layout byte-for-byte on the first retry. Same
  family as DECOMPILATION_LEARNINGS' "a two-armed if/else's LAYOUT and
  VALUE-PER-ARM are independently wrong-able."

### Proposed learning

Same as above -- another confirmed instance of if/else arm order being
load-bearing independent of the condition's logical polarity; worth a
line in the "two-armed if/else" entry if not already covered by this
exact shape (early-exit-style reset vs. a call as the "main" path).
