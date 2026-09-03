# func_8004F5DC

**Unit:** class_3bb8c_f · **Size:** 23 words (0x5C) · **Status:** MATCH

`void func_8004F5DC(TaskObjF *self)`. Clears `self->unk68`/`unk6C` to 0,
then unregisters the two children `func_8004F55C` had registered, via the
inherited `BasicClass::removeChild` slot:
`self->methods->removeChild(self, (void *)self->unk60)` then the same for
`self->unk64`.

## Lever

**Field write order matters even when the two stores are otherwise
independent.** The first attempt wrote `unk68` then `unk6C` (matching
their numeric/offset order); retail stores `unk6C` first. Simply matching
retail's own store order (`unk6C = 0; unk68 = 0;`) fixed the last 2/23
words with no other change — a pure statement-order residue, not a
register or type issue.
