> Renamed from `func_8004F3BC` on 2026-09-20 (tools/rename.py). Address 0x8004f3bc.

# TaskObjF__DisableEvents

**Unit:** class_3bb8c_f · **Size:** 10 words (0x28) · **Status:** MATCH

`s32 TaskObjF__DisableEvents(TaskObjF *self) { return TaskObjF__ForEachEvent(self,
func_8003903C, 1); }` — same shape as `TaskObjF__EnableEvents`, different callback.
See `TaskObjF__ForEachEvent`'s report.

## Naming (round 60, track 3)

`func_8004F3BC` -> `TaskObjF__DisableEvents`. **Tier A.** Same shape as
`TaskObjF__EnableEvents`, forwarding to Sony's `DisableEvent` (libapi/a13).
