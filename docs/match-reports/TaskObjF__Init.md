# TaskObjF__Init

> Renamed from `func_8004F55C` on 2026-09-20 (tools/rename.py). Address 0x8004f55c.

**Unit:** title_menu · **Size:** 32 words (0x80) · **Status:** MATCH

`void TaskObjF__Init(TaskObjF *self, s32 a1, s32 a2, s32 a3, s32 a5, s32
a6, s32 a7)`. Stores `a1`/`a2` into `self->unk30`/`unk34`, clears
`self->unk38`, stores the two stack args `a6`/`a7` into
`self->unk68`/`unk6C`, registers TWO children via the class's own
**inherited `BasicClass::addChild`** slot (`self->methods->addChild(self,
a3)` then `self->methods->addChild(self, a5)`), then clears
`self->unk70`/`unk28`/`unk24`.

See `TaskObjF__ForEachEvent`'s report for how `addChild`/`removeChild` were
identified as inherited `BasicClass` slots at +0x010/+0x014 rather than
this class's own. Matched on first transcription once that slot typing was
in place — no register or ordering lever needed.

`TaskObjF__Deinit` (this unit, matched separately) is this function's mirror:
it unregisters the same two children via `removeChild`.

## Naming (round 60, track 3)

`func_8004F55C` -> `TaskObjF__Init`. **Tier B.** Mechanics clear: stores
two context values, resets the buffer-pool pointer and status/mode
fields to a fresh state, and registers two children via the inherited
`BasicClass::addChild` slot. Kept generic ("Init", not e.g. "AttachChildren")
because the two children's own purpose is not established here -- and,
per this function's own report, `self->unk60`/`unk64` (the fields
`TaskObjF__Deinit` later removes as children) are never written inside
this function at all, so whatever the real "child" relationship is, it
is set up by a caller outside this unit, not fully visible from
`TaskObjF__Init` alone.

## Round 95 (track 7, charlie)

Pointer fields cleared with `NULL`, `state`/`opMode` with
`TASKOBJF_STATE_IDLE`/`TASKOBJF_OP_NONE` (include/task_objf.h, added this
round). Zero bytes.
