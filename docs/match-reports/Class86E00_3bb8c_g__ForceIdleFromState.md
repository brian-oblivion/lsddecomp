# Class86E00_3bb8c_g__ForceIdleFromState -- MATCH

> Renamed from `func_800501F0` on 2026-09-23 (tools/rename.py). Address 0x800501f0.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86E00_3bb8c_g__ForceIdleFromState`: 36/36 words match.

## Source

```c
void Class86E00_3bb8c_g__ForceIdleFromState(Class86E00_3bb8c_g *self)
{
    switch (self->unk28) {
    case 4:
    case 6:
    case 0xA:
    case 0xE:
        self->methods->slot8C(self, 0x10);
        self->methods->slot7C(self, 0x17);
        break;
    default:
        break;
    }
}
```

First attempt, byte-exact. A plain `switch` over the sparse 4-value set
`{4, 6, 0xA, 0xE}` reproduced GCC 2.6.3's own binary-search-style
comparison tree (retail checks `==6` first, then `<7` to split, then
`==4` on one side and `==0xA`/`==0xE` on the other) with no hand-written
`if`/`else` chain needed. Confirms the existing "GCC 2.6.3 lays out case
bodies in textual source order but picks its own comparison order" rule
generalises cleanly to a 4-value fall-through group, not just single-value
cases.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- `slot8C`/`slot7C` were both already declared while surveying
the unit (`Class86E00_3bb8c_g__TickCardIcon`'s report).

### Proposed learning

None new.
