# TaskObjF__AllocBuffers

> Renamed from `func_8004F704` on 2026-09-20 (tools/rename.py). Address 0x8004f704.

**Unit:** class_3bb8c_c · **Size:** 32 words (0x80) · **Status:** MATCH

`void TaskObjF__AllocBuffers(TaskObjF *self)`. If `self->unk38` (a `void **`, a
16-entry pointer array) is unallocated, allocates it (`BMemPMgrAlloc
(0x40)`, 16 words), fills the first 15 entries with individually-allocated
0x41-byte buffers, then allocates `self->unk3C` (a single 0x40-byte
buffer). Entry 15 of `unk38` is deliberately left uninitialized here — it
is only ever written by `TaskObjF__FreeUnusedBuffers` (this unit, matched), which treats
it as a sentinel/terminator slot.

Matched on first transcription. See `TaskObjF__FreeBuffers`'s report for the
mirror teardown and `TaskObjF__ForEachEvent`'s report for the `TaskObjF` class.

## Naming (round 60, track 3)

`func_8004F704` -> `TaskObjF__AllocBuffers`. **Tier A.** Mechanics fully
evident from the body: if `self->bufArray` is unallocated, allocates it
(16 entries), fills the first 15 with individually-allocated 0x41-byte
buffers, and allocates `self->scratchBuf` (a single 0x40-byte buffer).
Mirror of `TaskObjF__FreeBuffers`.

## Round 95 (track 7, charlie)

`0x40` (twice) is `(TASKOBJF_MAX_FILES + 1) * sizeof(char *)` -- sixteen
pointers, the last left NULL by FreeUnusedBuffers -- `15` is
`TASKOBJF_MAX_FILES` and `0x41` `TASKOBJF_TITLE_SIZE` (65). Zero bytes.
