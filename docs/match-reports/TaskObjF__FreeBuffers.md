# TaskObjF__FreeBuffers

> Renamed from `func_8004F810` on 2026-09-20 (tools/rename.py). Address 0x8004f810.

**Unit:** title_menu · **Size:** 37 words (0x94) · **Status:** MATCH

`void TaskObjF__FreeBuffers(TaskObjF *self)`. The mirror teardown of
`TaskObjF__AllocBuffers`: if `self->unk38` is allocated, frees `self->unk3C`, frees
every `self->unk38[i]` for `i` in `[0, self->unk2C)`, frees `self->unk38`
itself, and clears `self->unk38` to 0.

Matched on first transcription; `self->unk2C` is re-read fresh each loop
iteration (`for (i = 0; i < self->unk2C; i++)`, no local cache), matching
retail's own repeated `lw` of that field.

## Naming (round 60, track 3)

`func_8004F810` -> `TaskObjF__FreeBuffers`. **Tier A.** The mirror
teardown of `TaskObjF__AllocBuffers`: if `self->bufArray` is allocated,
frees `self->scratchBuf`, frees every `self->bufArray[i]` for `i` in
`[0, self->bufCount)`, frees `self->bufArray` itself, and clears it to 0
-- a full teardown, unlike `TaskObjF__FreeUnusedBuffers`'s partial trim.

## Round 95 (track 7, charlie)

`titles` compared with and cleared to `NULL`. Zero bytes.
