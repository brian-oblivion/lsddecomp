# TaskObjF__TestEvents

> Renamed from `func_8004F3E4` on 2026-09-20 (tools/rename.py). Address 0x8004f3e4.

**Unit:** TitleMenuTaskObjF · **Size:** 10 words (0x28) · **Status:** MATCH

`s32 TaskObjF__TestEvents(TaskObjF *self) { return TaskObjF__ForEachEvent(self,
func_800390F4, 0); }` — same shape as `TaskObjF__EnableEvents`/`TaskObjF__DisableEvents`,
different callback and flag (0, so `TaskObjF__ForEachEvent`'s own lock/unlock
bracket is skipped here). See `TaskObjF__ForEachEvent`'s report.

## Naming (round 60, track 3)

`func_8004F3E4` -> `TaskObjF__TestEvents`. **Tier A.** Same shape as
`TaskObjF__EnableEvents`/`TaskObjF__DisableEvents`, forwarding to Sony's
`TestEvent` (libapi/a11) with the critical-section bracket OFF (flag 0)
-- a pure query needs no lock.
