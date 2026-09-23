# Class86E00_3bb8c_g__OnItemSelected -- MATCH

> Renamed from `func_80050730` on 2026-09-23 (tools/rename.py). Address 0x80050730.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86E00_3bb8c_g__OnItemSelected`: 46/46 words match.

## Source

```c
void Class86E00_3bb8c_g__OnItemSelected(Class86E00_3bb8c_g *self, GenericSlot9CObj_3bb8c_g *arg1, s32 arg2)
{
    switch (arg2) {
    case 2:
        self->unk80 = arg1->methods->slot9C(arg1);
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0xE);
        break;
    case 3:
        self->methods->slotAC(self);
        self->methods->slot7C(self, 0x17);
        break;
    }
}
```

First attempt, byte-exact. Same two-value `switch` layout lesson as
`Class86E00_3bb8c_g__OnCommand` (last one written, applied directly here without
re-deriving): out-of-line case bodies reached by forward `beq`s, which a
plain `switch` reproduces and an `if`/`else if` chain would not.

This is this unit's last fresh function -- all 12 of `class_3bb8c_g`'s
non-blocked functions are now matched.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- `slotAC` and `GenericSlot9CObj_3bb8c_g::slot9C` were both
already declared while surveying the unit (`Class86E00_3bb8c_g__TickCardIcon`'s report);
this is the function that exercises both.

### Proposed learning

None new.

## Naming

`Class86E00_3bb8c_g__OnItemSelected` (was `func_80050730`), tier B: same
external `arg2 in {2, 3}` dispatch shape as `Class86E00_3bb8c_g__OnCommand`,
but its `2` case additionally reads a return value off its own `arg1`
(a `GenericSlot9CObj_3bb8c_g *`) into `self->unk80` before transitioning --
read as "an item was chosen, and here it is" rather than a plain command,
though which UI concept `arg1` represents is not established.
