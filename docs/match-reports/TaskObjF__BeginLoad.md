# TaskObjF__BeginLoad

> Renamed from `TaskObjF__func_8004F638` on 2026-09-26 (tools/rename.py). Address 0x8004f638.

> Renamed from `func_8004F638` on 2026-09-20 (tools/rename.py). Address 0x8004f638.

**Unit:** title_menu · **Size:** 51 words (0xCC) · **Status:** MATCH

`void TaskObjF__BeginLoad(TaskObjF *self, s32 a1, s32 a2, s32 a3, s32 a4)`.
Stores the four params into `self->unk40`/`unk44`/`unk54`/`unk58`, sets
the state tag `self->unk24 = 1`, then — only if `TaskObjF__Validate(self)`
(this unit's own validation gate, matched separately) succeeds — tears
down and reallocates the two buffers via `TaskObjF__FreeBuffers`/`TaskObjF__AllocBuffers`
(also this unit, matched), dispatches `self->methods->slot5C(self,
self->unk38, self->unk3C, self->unk30, self->unk34)` (a 5-argument custom
slot, this class's own, not inherited), stores its result into
`self->unk2C`, and on success calls `TaskObjF__FreeUnusedBuffers` (walks the buffer
array, matched) before picking an error code (`0xF` if
`self->unk28 == 0xE`, else `0x12`) or, on failure, forcing `self->unk2C =
0xF` and code `0xD` — finally dispatching `self->methods->slot7C(self,
code)`.

Matched on first structurally-faithful transcription (explicit `if/else`
for the two-way error-code choice, matching retail's own delay-slot-filled
branch shape without needing any manual reshaping).

## Naming (round 60, track 3)

`func_8004F638` -> `TaskObjF__BeginLoad`. **Tier C placeholder.** The
CLASS is established (`TaskObjF`, per `TaskObjF__ForEachEvent`'s report),
so bare `func_8004F638` would be a regression under FINISHING-PLAN track
3's convention -- but the function's own concrete purpose is NOT
established: it sets `self->opMode = 1`, gates on `TaskObjF__Validate`,
tears down and reallocates the buffer pool, then dispatches
`self->methods->slot5C(...)`, a vtable slot whose actual implementation
lives in a subclass outside this unit (not carved/named here). Naming it
e.g. "StartWrite" or "Submit" would assert a purpose no evidence in this
unit actually supports -- `self->opMode`'s two values (1 here, 2 in
`TaskObjF__BeginSave`) distinguish *some* two operations, but which
is which is not derivable from this unit alone. Left as the documented
tier-C form rather than guessing.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__func_8004F638`. Round 60 left it tier C because slot +0x05C looked like a subclass's; `tools/classtable.py gTaskObjFMethods` shows every slot is this class's own, so the chain is readable: +0x05C is TaskObjF__CollectExistingMemcardFiles (fills `titles`/`foundSuffixes` with the save files that exist, returns the count), code 0x12 opens the item list of those titles (TaskObjF__AttachItemList via SetState), the list's result 2 stores the choice and sets state 0xE, TaskObjF__AdvanceState builds `fileName`/`title` from the chosen entry and calls this slot (+0x074) again, which now takes code 0xF: TaskObjF__TickStateDelay turns 0xF into 0x15 and SetState(0x15) calls +0x064 TaskObjF__ReadMemcardFile(fileName, data, dataSize). opMode 1 marks this operation. It starts a load; the name says no more than that.

## Round 95 (track 7, charlie)

### Naming and constants

`result` -> `found` (collectExistingMemcardFiles' count), `code` -> `state`.
States: 0xE `TASKOBJF_STATE_LOAD_WARNING` ("LOADWAR", the state
onItemListResult sets after a pick; advanceState re-runs beginLoad from it),
0xF `LOADING`, 0x12 `CHOOSE_FILE`, 0xD `LOAD_NOT_FOUND`; opMode 1
`TASKOBJF_OP_LOAD`; `bufCount = 0xF` is `TASKOBJF_MAX_FILES`, so
FreeBuffers frees all fifteen title buffers AllocBuffers made. Evidence for
each state name is its comment in include/task_objf.h. Zero bytes.
