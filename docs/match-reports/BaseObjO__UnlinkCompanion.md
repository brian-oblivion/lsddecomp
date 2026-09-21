# BaseObjO__UnlinkCompanion -- MATCHED (30/30 words)

> Renamed from `func_80057130` on 2026-09-18 (tools/rename.py). Address 0x80057130.

Unit: `class_3bb8c_o` (round 17). `BaseObjOMethods::slot14` -- the
"unlink" companion of `BaseObjO__LinkCompanion` (`slot10`), already named by both
`code_55dd4.h` and `DreamSys.h` at the identical offset in sibling classes.

## Final source

```c
void BaseObjO__UnlinkCompanion(BaseObjO *self, TagWordObjO *arg) {
    s32 tag = arg->methods->header;

    if ((tag & 0xFFF) == 0x114) {
        self->unk4C = NULL;
    } else if ((tag & 0xF) == 5) {
        self->unk50 = NULL;
    }
    GetClass6B5CCMethods()->slot14(self, arg);
}
```

## Derivation

Mirror image of `BaseObjO__LinkCompanion`'s order: here the tag check runs FIRST
(clearing `self->unk4C`/`self->unk50` to `NULL` rather than setting them),
and the chain to the fixed table's `slot14` runs LAST. Confirmed directly
against the disassembly's own instruction order -- no iteration needed.

`GetClass6B5CCMethods()`'s call site here passes `$a0` = leftover garbage from the
preceding tag-check code (`arg->methods->header`, not `self`) -- retail
itself sets up no explicit `$a0` before this `jal` either, which is exactly
the established "declare it 0-argument and let whatever is already resident
in `$a0` go along for the ride, because the callee ignores it regardless"
precedent. Matching this required NOT adding any argument-setup code before
the call, i.e. leaving `GetClass6B5CCMethods()` as a plain 0-argument call at every
site, not just the ones where `self` happens to still be resident.

### Proposed learning

- **The same fixed-table getter can be called with visibly different
  garbage in its ignored argument register across different call sites in
  the SAME function's sibling, and both reproduce retail exactly.**
  `BaseObjO__LinkCompanion` calls `GetClass6B5CCMethods()` with `self` still resident;
  `BaseObjO__UnlinkCompanion` calls it with the tag-check's leftover header word
  resident instead. Both are correct because the callee provably ignores
  its input -- reinforces (rather than extends) the existing
  `GetClass6B5CCMethods` precedent, but from the "it doesn't matter what's
  there" side rather than the "self happens to be there" side.

## Naming

**`BaseObjO__UnlinkCompanion` -- tier A.** Mirror image of
`BaseObjO__LinkCompanion` (see that report): classifies `arg` by the same
tag rule and CLEARS the matching companion field instead of setting it --
the "unlink" half of the pair, matched in ROM order right after `slot10`'s
occupant.
