# TaskObjF__AttachChildB -- MATCH

> Renamed from `Class86E00_3bb8c_g__AttachChildB` on 2026-09-23 (tools/rename.py). Address 0x800505a8.

> Renamed from `func_800505A8` on 2026-09-23 (tools/rename.py). Address 0x800505a8.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__AttachChildB`: 50/50 words match.

## Source

```c
void TaskObjF__AttachChildB(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0) {
        if (self->unk7C == NULL) {
            self->unk7C = New_Class86F88(self->unk38, 1);
            self->unk74 = 1;
        }
        self->methods->slot10(self, self->unk7C);
        self->unk7C->methods->slot44(self->unk7C, self->unk68);
        self->unk7C->methods->slot4C(self->unk7C, self->unk60, self->unk64, self->unk6C);
    }
}
```

First attempt, byte-exact. The exact twin of `TaskObjF__AttachChildA` (same guard
shape, same lazy-init-then-attach sequence), operating on `unk7C` instead
of `unk78` and calling `New_Class86F88` instead of `New_Obj86ED0`. Unlike
`TaskObjF__AttachChildA`, `New_Class86F88`'s second argument (`1`) is materialized
right in the `jal`'s own delay slot -- an ordinary, unremarkable argument
setup, not the "surprise 2nd parameter hoisted several instructions
early" residue `TaskObjF__AttachChildA` needed to diagnose. Applying that
function's already-corrected `slot44` arity (`self, s32 arg1`) here
directly is what made this one match cold.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- every slot and the `New_Class86F88` extern were already
declared correctly (the `slot44` arity fix came from `TaskObjF__AttachChildA`'s
report, applied here without needing its own derivation).

### Proposed learning

None new. Worth noting as a case where reading a SIBLING function's match
report before starting paid off directly -- no repeat of the arity
mistake that cost `TaskObjF__AttachChildA` an extra attempt.

## Naming

`TaskObjF__AttachChildB` (was `func_800505A8`), tier B: the
"B" twin of `TaskObjF__AttachChildA` -- identical guard and
attach sequence, operating on `self->unk7C` via `New_Class86F88` instead of
`self->unk78` via `New_Obj86ED0`. See `AttachChildA`'s naming note: the
A/B suffixes are positional labels, not an established functional split.
