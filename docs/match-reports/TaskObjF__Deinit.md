# TaskObjF__Deinit

> Renamed from `func_8004F5DC` on 2026-09-20 (tools/rename.py). Address 0x8004f5dc.

**Unit:** class_3bb8c_c · **Size:** 23 words (0x5C) · **Status:** MATCH

`void TaskObjF__Deinit(TaskObjF *self)`. Clears `self->unk68`/`unk6C` to 0,
then unregisters the two children `TaskObjF__Init` had registered, via the
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

## Naming (round 60, track 3)

`func_8004F5DC` -> `TaskObjF__Deinit`. **Tier B.** The mirror teardown of
`TaskObjF__Init` (see that report): clears two fields and unregisters two
children via the inherited `BasicClass::removeChild` slot. Same caveat as
`TaskObjF__Init`: the children's own purpose is not established from
this unit alone.

## Round 95 (track 7, charlie)

`sound`/`spriteParent` cleared with `NULL`. Zero bytes.
