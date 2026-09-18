> Renamed from `func_800571A8` on 2026-09-18 (tools/rename.py). Address 0x800571a8.

# BaseObjO__ClearCompanions -- MATCHED (16/16 words)

Unit: `class_3bb8c_o` (round 17). `BaseObjOMethods::slot18` -- an
unconditional full teardown of both companion pointers (no tag check),
already named `unk18`/`BaseObjO__ClearCompanions` in `code_55dd4.h`'s
`D800878D4Methods`.

## Final source

```c
void BaseObjO__ClearCompanions(BaseObjO *self) {
    self->unk4C = NULL;
    self->unk50 = NULL;
    GetClass6B5CCMethods()->slot18(self);
}
```

## Derivation

`sw $zero,0x4C($s0)` / `sw $zero,0x50($s0)` (both unconditional, no tag
read at all) then chain to the fixed table's `slot18(self)`. Straight
transcription, matched first try -- no tag classification is present here
because, unlike `slot10`/`slot14`, this slot takes no companion-object
argument to classify; it simply clears both fields regardless of which one
(if either) was set.

### Proposed learning

None -- confirms rather than extends the link/unlink pair's field naming
already established by `BaseObjO__LinkCompanion`/`BaseObjO__UnlinkCompanion`.

## Naming

**`BaseObjO__ClearCompanions` -- tier A.** Mechanics ARE the purpose:
unconditionally clears BOTH companion fields (no tag check, unlike
`LinkCompanion`/`UnlinkCompanion`) and chains to the base table's own
`slot18`. `slot18` has no other established occupant name in any sibling
header (`code_55dd4.h` still calls the field `unk18`), so this name is
this unit's own contribution.
