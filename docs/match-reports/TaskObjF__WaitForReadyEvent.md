# TaskObjF__WaitForReadyEvent

> Renamed from `TaskObjF__FindReadyEvent` on 2026-09-20 (tools/rename.py). Address 0x8004f4a4.

> Renamed from `func_8004F4A4` on 2026-09-20 (tools/rename.py). Address 0x8004f4a4.

**Unit:** class_3bb8c_f · **Size:** 9 words (0x24) · **Status:** MATCH

`s32 TaskObjF__WaitForReadyEvent(TaskObjF *self) { return WaitForReadyEvent(self->field14,
4); }` — a one-line forward to `WaitForReadyEvent` (see its report), passing
`TaskObjF::field14` (the same 4-word array `TaskObjF__ForEachEvent` walks) as a
plain `s32*`. Matched on first transcription.

## Naming (round 60, track 3)

`func_8004F4A4` -> `TaskObjF__WaitForReadyEvent`. **Tier A.** A one-line
forward to `WaitForReadyEvent(self->events, 4)` -- finds which of this
object's 4 events is ready (per `TestEvent`). Name mirrors the callee.

**Head correction at merge (round 60).** Renamed from
`TaskObjF__FindReadyEvent` together with the free function it forwards to;
the reason is in `docs/match-reports/WaitForReadyEvent.md` -- the callee's
outer loop has no exit but its `return`, so this blocks rather than
searches. Byte-identical.
