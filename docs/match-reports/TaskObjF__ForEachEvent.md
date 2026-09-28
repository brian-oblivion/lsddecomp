# TaskObjF__ForEachEvent

> Renamed from `func_8004F40C` on 2026-09-20 (tools/rename.py). Address 0x8004f40c.

**Unit:** title_menu · **Size:** 38 words (0x98) · **Status:** MATCH

## The class this unit's back half operates on

Every function from here through `TaskObjF__OnNotify` in this unit shares one
object type — call it `TaskObjF` — that is **not** `Obj866E8`
(DayTaskStageMap/c/d/e's class) and not any previously-documented class in
this codebase. Established from first principles across this whole batch:

- `TaskObjF`'s vtable pointer sits at offset 0, and its first several
  slots (+0x010 `addChild`, +0x014 `removeChild`) are **inherited,
  unmodified `BasicClass` slots** (`include/code_8220.h`'s
  `BasicClassMethods`) — confirmed because `TaskObjF__OnNotify` fetches
  `BasicClass`'s own table directly (`GetBasicClassMethods()`, which
  `include/code_8220.h` already establishes returns `&gBasicClassMethods`) and
  dispatches its slot +0x038 with a `(self, arg1, arg2)` signature that
  matches `BasicClassMethods::slot38` exactly. `TaskObjF` is therefore a
  `BasicClass` subclass whose own new slots start at +0x044 (the same
  "inherited low slots, subclass slots from the first free offset after
  BasicClass's own 15" shape already documented for `GameApplicationMethods`).
- `BuildMemcardPath` (this unit's other matched function) is a **different,
  unrelated object** — see its own report.

`include/class_3bb8c.h` gets a new, purely additive section
(`TaskObjF`/`TaskObjFMethods`) for this — a second independent local view
in the same file as `Obj866E8`, same policy as `DayTaskStageMap.h`'s
`StageMapMethods` vs. this file's own `Obj866E8Methods`.

## What TaskObjF__ForEachEvent does

`s32 TaskObjF__ForEachEvent(TaskObjF *self, s32 (*callback)(s32), s32 flag)`.
Optionally brackets a scan with a lock/unlock pair
(`func_80024CE0`/`func_80024CF0`, called only when `flag` is set), then
calls `callback` on each of `self->field14[4]` in turn, stopping at the
first zero return. Returns the last callback result.

`self->field14` is a plain `s32[4]` at offset +0x14 (the gap right after
`BasicClass`'s own header fields) — GCC 2.6.3 compiles `for` over
`self->field14[i]` by advancing `self` itself as the loop's induction
register and folding the `+0x14` into the load's own displacement, which
is why the raw disassembly reads as an array of STRUCTS 4 bytes apart at a
FIXED +0x14 offset rather than `self[i].field14` — a real reading trap
worth flagging for whoever revisits this unit.

Three thin wrappers around this (this unit's other queued functions,
matched separately): `TaskObjF__EnableEvents`/`TaskObjF__DisableEvents`/`TaskObjF__TestEvents`, each
forwarding `self` unchanged with a different callback
(`func_80038F6C`/`func_8003903C`/`func_800390F4`) and flag.

## Header additions (`include/class_3bb8c.h`, additive only)

- New `TaskObjF`/`TaskObjFMethods` types (see above); only the slots and
  fields this unit's queued functions actually touch are concretely
  typed, everything else stays opaque padding, per this file's existing
  policy for `Obj866E8Methods`.
- `extern s32 func_80038F6C(s32 arg); extern s32 func_8003903C(s32 arg);
  extern s32 func_800390F4(s32 arg);` — the three callbacks (the first two
  are Psy-Q SPU routines, `the 0x272C8..0x2C054 Psy-Q block (now linked from lib/ where a disc owns it, formerly asm/psyq_SpuSetMute.s)`, uncarved; the third is
  also used directly by `WaitForReadyEvent`, see that report).
- `extern void func_80024CE0(void); extern void func_80024CF0(void);` —
  **externs for functions outside this unit** (typed purely from this call
  site's own register usage: no arguments set up, no return value read).
- New local view `BasicMethods866E8F` (a minimal `BasicClass` vtable slice,
  just slot +0x038) and `extern BasicMethods866E8F *GetBasicClassMethods(void);`
  — **an extern for a function declared elsewhere with a different return
  type** (`include/code_8220.h`'s `BasicClassMethods *GetBasicClassMethods(void)`,
  and several other units' own local views) — deliberately NOT unified
  with that header, same "independent local view" policy as everywhere
  else in this project; flagged per the round's header-collision rule
  even though it is additive, not a retype, since three runners share this
  file this round.

## Proposed learning

**A `self`-as-induction-variable array walk is a real reading trap.**
`lw $a0, 0x14($s0)` with `$s0` incrementing by 4 each iteration reads, at
first glance, like `self` is a raw `s32*` and the function walks a
STRIDE-4 array of things each holding a value at their OWN +0x14 — it is
actually `self->field14[i]` with `field14` an inline `s32[4]` at offset
0x14, and GCC folds the offset into the load's own displacement while
reusing `self`'s register as the loop pointer once `self` itself is no
longer needed for anything else. Worth a general callout for future
`self`-shaped loops in this codebase: check whether the "base" register
is ever used for a SECOND purpose after the loop before concluding it's a
raw array of small structs.

## Naming (round 60, track 3)

`func_8004F40C` -> `TaskObjF__ForEachEvent`. **Tier A.** The shared
generic helper `TaskObjF__EnableEvents`/`DisableEvents`/`TestEvents` all
forward into: optionally brackets a critical section, then calls
`callback` on each of `self->events[4]` in turn, stopping at the first
zero return. Mechanics and role (a for-each over this object's own event
array) both directly evident from the body and its three callers.

## Round 94 (track 6, charlie): history moved from include/class_3bb8c.h

The three callbacks TaskObjF__EnableEvents/DisableEvents/TestEvents forward
into ForEachEvent are EnableEvent/DisableEvent/TestEvent, linked from the
Psy-Q objects libapi/a12, a13, a11 (round 34), declared in
src/class_3bb8c_f.c only. An older comment called them "SPU routines"; they
are kernel event-queue calls, only their neighbours in the block are libspu.
WaitForReadyEvent's callback was `func_800390F4` until round 34 linked it as
Sony's TestEvent, and TaskObjF's `field14` became `events` in round 60.

## Round 95 (track 7, charlie)

### Naming

`flag` -> `critical` (it brackets the loop in Enter/ExitCriticalSection);
`i < 4` -> `i < ARRAY_COUNT(self->events)`. Zero bytes.

### Moved from src/ui/title_menu.c

```c
/* Psy-Q's kernel critical-section pair (libapi/a36, libapi/a37, linked from
 * Sony's own SDK objects), called with no arguments around this unit's scan
 * loop when its `flag` argument is set. These belong to another translation
 * unit, so they are declared LOCAL here rather than in class_3bb8c.h, which
 * twenty units include (CLAUDE.md's header-contention rule). Sony's
 * EnterCriticalSection returns int; no call site here reads it, so the local
 * view stays `void` -- per-call-site typing, the convention this block of
 * units already uses. */
```
