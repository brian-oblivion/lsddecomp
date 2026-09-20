> Renamed from `func_8004F394` on 2026-09-20 (tools/rename.py). Address 0x8004f394.

# TaskObjF__EnableEvents

**Unit:** class_3bb8c_f · **Size:** 10 words (0x28) · **Status:** MATCH

`s32 TaskObjF__EnableEvents(TaskObjF *self) { return TaskObjF__ForEachEvent(self,
func_80038F6C, 1); }` — a one-line forward to `TaskObjF__ForEachEvent` (see its
report for the `TaskObjF` class and the callback/array shape). No
independent lever; matched on first transcription once `TaskObjF__ForEachEvent`'s
own signature was established.

## Naming (round 60, track 3)

`func_8004F394` -> `TaskObjF__EnableEvents`. **Tier A.** A one-line
forward to `TaskObjF__ForEachEvent(self, EnableEvent, 1)` -- Sony's own
`EnableEvent` (linked, libapi/a12) applied across all 4 of this object's
events with the critical-section bracket engaged. Name mirrors the
callee directly.
