> Renamed from `func_8004F4A4` on 2026-09-20 (tools/rename.py). Address 0x8004f4a4.

# TaskObjF__FindReadyEvent

**Unit:** class_3bb8c_f · **Size:** 9 words (0x24) · **Status:** MATCH

`s32 TaskObjF__FindReadyEvent(TaskObjF *self) { return FindFirstReadyEvent(self->field14,
4); }` — a one-line forward to `FindFirstReadyEvent` (see its report), passing
`TaskObjF::field14` (the same 4-word array `TaskObjF__ForEachEvent` walks) as a
plain `s32*`. Matched on first transcription.

## Naming (round 60, track 3)

`func_8004F4A4` -> `TaskObjF__FindReadyEvent`. **Tier A.** A one-line
forward to `FindFirstReadyEvent(self->events, 4)` -- finds which of this
object's 4 events is ready (per `TestEvent`). Name mirrors the callee.
