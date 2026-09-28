# TaskObjF__EnableEvents

> Renamed from `func_8004F394` on 2026-09-20 (tools/rename.py). Address 0x8004f394.

**Unit:** class_3bb8c_c · **Size:** 10 words (0x28) · **Status:** MATCH

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
`class_3bb8c_c.c`'s `Node3bb8cE` (a distinct local view, its own header
comment) and this unit's `TaskObjF` were derived independently and never
declared the same type -- but `Node3bb8cE::threads[4]` at +0x014 is
filled by `TaskObjF__OpenEvents` via `OpenEvent(0xF4000001, gCardEventSpecs[i], 0x2000, 0)`,
and that same function's LAST statement is `TaskObjF__EnableEvents(self)`
with `self` still typed `Node3bb8cE *`, i.e. the same object pointer is
handed straight into this unit's own event-enable wrapper. That is
concrete cross-unit evidence the two "unrelated" classes are, at minimum,
layout-compatible at this offset (both hold 4 kernel event descriptors
there), and quite possibly the SAME class seen through two independent
partial local views -- not proof, but strong enough to flag for whoever
does track 4's class-table unification pass rather than let two separate
`typedef`s quietly diverge further. Posted to the round-60 broadcast.

## Round 95 (track 7, charlie)

### Moved from src/class_3bb8c_c.c

The kernel event declarations' comment, shortened in the unit:

```c
/* Psy-Q kernel event queue, linked from libapi/a12, libapi/a13 and
 * libapi/a11. Local view: these are Sony's, declared in the one unit that
 * calls them rather than in include/class_3bb8c.h, which 21 units include.
 * Each takes an event descriptor and returns a status word, which is what
 * makes them usable as TaskObjF__ForEachEvent's `s32 (*)(s32)` callback. */
```
