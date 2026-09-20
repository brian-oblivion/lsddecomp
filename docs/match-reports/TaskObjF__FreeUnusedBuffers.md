> Renamed from `func_8004F784` on 2026-09-20 (tools/rename.py). Address 0x8004f784.

# TaskObjF__FreeUnusedBuffers

**Unit:** class_3bb8c_f · **Size:** 35 words (0x8C) · **Status:** MATCH

`void TaskObjF__FreeUnusedBuffers(TaskObjF *self)`. Frees `self->unk38[i]` for `i` from
`self->unk2C` up to (not including) 15, storing each `func_80017CFC`
return back into the slot it just freed, then null-terminates the array at
whatever index the loop ended on (`self->unk38[i] = 0` — reached whether
the loop ran zero or more times, since `i` is read after the loop with
its final value, matching retail's own reload-`self->unk38`-then-index
shape at the tail).

Matched on first transcription — a plain `for (i = self->unk2C; i < 15;
i++) { self->unk38[i] = func_80017CFC(self->unk38[i]); } self->unk38[i] =
0;` reproduces retail's per-iteration reload of `self->unk38` (not cached
in a local) exactly.

## Naming (round 60, track 3)

`func_8004F784` -> `TaskObjF__FreeUnusedBuffers`. **Tier A.** Frees
`self->bufArray[i]` for `i` from `self->bufCount` up to (not including)
15 and null-terminates the array there -- i.e. releases only the entries
BEYOND how many buffers a just-finished operation actually used, keeping
the used ones (`[0, bufCount)`) allocated. Distinguished from
`TaskObjF__FreeBuffers` (full teardown) by name.
