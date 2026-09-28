# TaskObjF__ReadMemcardFile

> Renamed from `func_8004ED40` on 2026-09-20 (tools/rename.py). Address 0x8004ed40.

**Unit:** class_3bb8c_c · **Size:** 32 words (0x80) · **Status:** MATCH

## What it does

`s32 TaskObjF__ReadMemcardFile(TaskObjF *self, char *suffix, void *outBuf, s32
outSize)`. A bounded-retry wrapper: calls `TaskObjF__TryReadMemcardFile` (this unit, also
matched — the actual CD-stream open/read/seek/close) up to 11 times (a
counter starting at 10, checked and decremented in a way that allows one
extra iteration — see the lever below), stopping early the first time it
returns nonzero.

Flagged predicted-hard by this round's register census (5 distinct
callee-saved registers) — matched, but needed a genuine lever (below), not
a first-pass transcription.

## Lever

**`do { ...; if (result) break; } while (count-- != 0);`, not a `for(;;)`
with two separate exit checks.** The first attempt (`for (;;) { result =
call(); if (result) break; if (count == 0) break; count--; }`) compiled to
32 words with the SAME shape but needed two extra `move`s (`v1 = v0;` /
`v0 = v1;`) to shuttle the call result across the intervening "is count
zero" test — retail avoids both entirely. Testing `count` with a
post-decrement in the loop CONDITION (`count-- != 0`) reproduces retail's
own "check old value, unconditionally decrement in the branch's delay
slot" shape exactly, letting the call's return value stay in `$v0`
untouched through to the function's own `return`.

## Header additions

- Forward declaration `s32 TaskObjF__TryReadMemcardFile(TaskObjF *self, char *suffix,
  void *outBuf, s32 outSize);` in `src/class_3bb8c_c.c` (defined later in
  ROM order).

See `TaskObjF__TryReadMemcardFile`'s report for the callee's own signature and the
`TaskObjF` class context (`TaskObjF__ForEachEvent`'s report).

## Naming (round 60, track 3)

`func_8004ED40` -> `TaskObjF__ReadMemcardFile`. **Tier A.** A bounded
(11-attempt) retry wrapper around `TaskObjF__TryReadMemcardFile`, stopping
early on the first success -- both the retry mechanics and the purpose
(read a memory-card file, tolerating transient failures) are evident from
the body plus the callee's own established behaviour.

## Round 95 (track 7, charlie)

### Naming

`count` -> `retries`; the `10` is `MEMCARD_RETRIES` (include/TaskObjF.h:
attempts after the first; the loop runs 11 times). Zero bytes.

### Moved from src/class_3bb8c_c.c

The unit banner before track 7 (its history -- "track 4 round 89" -- is
here; the new banner describes the file only):

```c
/*
 * class_3bb8c_c: `TaskObjF` methods (include/TaskObjF.h, track 4 round 89),
 * slots +0x064..+0x078 and the +0x038 onNotify override, with the unit's
 * helpers:
 *
 * - The memory-card file API (BuildMemcardPath,
 *   TaskObjF__ReadMemcardFile/TryReadMemcardFile,
 *   TaskObjF__WriteMemcardSaveFile/TryWriteMemcardSaveFile): opens BIOS
 *   `bu00:`/`bu10:` paths directly to read save data and to write a save
 *   file whose header is structurally exact to the standard PS1 memory-
 *   card save format (magic, icon-frame count, block count, title, icon
 *   palette, up to three icon frames).
 * - The event helpers over `events[4]` (EnableEvents/DisableEvents/
 *   TestEvents/ForEachEvent/WaitForReadyEvent).
 * - The two operations the parent starts, TaskObjF__BeginLoad and
 *   TaskObjF__BeginSave, with their card check (TaskObjF__Validate) and the
 *   16-entry title buffer pool (Alloc/FreeUnused/FreeBuffers); TaskObjF__Init
 *   and Deinit; TaskObjF__OnNotify, which routes a child's notification by
 *   the child's class id.
 */
```
