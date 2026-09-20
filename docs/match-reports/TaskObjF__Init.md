> Renamed from `func_8004F55C` on 2026-09-20 (tools/rename.py). Address 0x8004f55c.

# TaskObjF__Init

**Unit:** class_3bb8c_f · **Size:** 32 words (0x80) · **Status:** MATCH

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
