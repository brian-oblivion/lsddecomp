# Class86E00_3bb8c_g__AttachChildB -- MATCH

> Renamed from `func_800505A8` on 2026-09-23 (tools/rename.py). Address 0x800505a8.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86E00_3bb8c_g__AttachChildB`: 50/50 words match.

## Source

```c
void Class86E00_3bb8c_g__AttachChildB(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk7C == NULL) {
            self->unk7C = func_80051A5C(self->unk38, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk7C);
        self->unk7C->methods->slot44(self->unk7C, self->unk68);
        self->unk7C->methods->slot4C(self->unk7C, self->unk60, self->unk64, self->unk6C);
    }
}
```

First attempt, byte-exact. The exact twin of `Class86E00_3bb8c_g__AttachChildA` (same guard
shape, same lazy-init-then-attach sequence), operating on `unk7C` instead
of `unk78` and calling `func_80051A5C` instead of `func_80050BA8`. Unlike
`Class86E00_3bb8c_g__AttachChildA`, `func_80051A5C`'s second argument (`1`) is materialized
right in the `jal`'s own delay slot -- an ordinary, unremarkable argument
setup, not the "surprise 2nd parameter hoisted several instructions
early" residue `Class86E00_3bb8c_g__AttachChildA` needed to diagnose. Applying that
function's already-corrected `slot44` arity (`self, s32 arg1`) here
directly is what made this one match cold.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- every slot and the `func_80051A5C` extern were already
declared correctly (the `slot44` arity fix came from `Class86E00_3bb8c_g__AttachChildA`'s
report, applied here without needing its own derivation).

### Proposed learning

None new. Worth noting as a case where reading a SIBLING function's match
report before starting paid off directly -- no repeat of the arity
mistake that cost `Class86E00_3bb8c_g__AttachChildA` an extra attempt.
