# TaskObjF__EnableEvents

> Renamed from `func_8004F394` on 2026-09-20 (tools/rename.py). Address 0x8004f394.

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

### Proposed learning

**Two classes documented as "unrelated" in different rounds may share
concrete evidence worth cross-checking before track 4 unifies types.**
`class_3bb8c_e.c`'s `Node3bb8cE` (a distinct local view, its own header
comment) and this unit's `TaskObjF` were derived independently and never
declared the same type -- but `Node3bb8cE::threads[4]` at +0x014 is
filled by `TaskObjF__OpenEvents` via `OpenEvent(0xF4000001, D_80086E78[i], 0x2000, 0)`,
and that same function's LAST statement is `TaskObjF__EnableEvents(self)`
with `self` still typed `Node3bb8cE *`, i.e. the same object pointer is
handed straight into this unit's own event-enable wrapper. That is
concrete cross-unit evidence the two "unrelated" classes are, at minimum,
layout-compatible at this offset (both hold 4 kernel event descriptors
there), and quite possibly the SAME class seen through two independent
partial local views -- not proof, but strong enough to flag for whoever
does track 4's class-table unification pass rather than let two separate
`typedef`s quietly diverge further. Posted to the round-60 broadcast.
