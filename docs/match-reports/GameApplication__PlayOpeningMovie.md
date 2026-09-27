# GameApplication__PlayOpeningMovie

> Renamed from `GameApplication__StartWeeklyStreamTask` on 2026-09-27 (tools/rename.py). Address 0x80026348.

> Renamed from `Class6D3C8__StartWeeklyStreamTask` on 2026-09-26 (tools/rename.py). Address 0x80026348.

> Renamed from `func_80026348` on 2026-09-24 (tools/rename.py). Address 0x80026348.

**Unit:** code_1677c · **Size:** 50 instructions (0xC8 bytes) · **Status:** MATCHED (50/50 words, whole-image SHA1 green), first attempt

## What it does

`GameApplicationMethods` slot `+0x054`. Gated by `self->arg->unk08 != 0` (a
second boolean/pointer gate on the ctor argument, sibling to
`GameApplication__ShowIntroLogos`'s `unk0C` gate): builds a `StreamTask`, derives a type code
via `PickOpeningMovie` (a day/week-style calculation, unrelated unit,
`psyq_memset.s`), looks it up via `GetMovieFrameCount`, and initializes+starts
the task the same way `GameApplication__ShowIntroLogos` does -- minus that function's two
`GameApplication__ShowImage` loader-task registrations.

## Derivation

Structurally identical to the second half of `GameApplication__ShowIntroLogos` (already
matched), with `PickOpeningMovie(&typeCode, 0)` in place of
`GetAsmkMovie(&typeCode)`:

```c
void GameApplication__PlayOpeningMovie(GameApplication *self) {
    s32 derivedValue;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        derivedValue = PickOpeningMovie(&typeCode, 0);
        typeLookup = GetMovieFrameCount(typeCode);
        task->methods->slot44(task, self->unk1C, derivedValue, typeLookup, 1);
        task->methods->slot4(task);
    }
}
```

Matched first attempt, entirely on the strength of `GameApplication__ShowIntroLogos`'s
already-solved register-allocation-by-declaration-order lesson (declared
`derivedValue`/`typeCode` before `task`, mirroring that function's fix) and
its "the delay slot after `jalr` is the real 4th argument, not a scratch
reload" lesson (used `typeLookup`, not `task->methods`, as the 4th `slot44`
argument from the start).

## New struct/header knowledge

`include/GameApplication.h`: split `GameApplicationConfig`'s `+0x04..+0x0B` padding
to expose `+0x08` (`unk08`, this function's gate) as its own field,
matching the existing `+0x0C` (`unk0C`, `GameApplication__ShowIntroLogos`'s gate). Declared
`PickOpeningMovie` (day/week-style helper, `psyq_memset.s`, same "write an
index to *out, return a related but different value" shape as
`GetAsmkMovie`).

## Proposed learning

Two sibling `GameApplicationConfig` gate fields (`+0x08`, `+0x0C`) each guard a
near-identical "build a StreamTask, derive+lookup a type code, init and
start it" block, differing only in which helper derives the type code and
whether extra `LoaderTask` registrations bookend it. Once one sibling is
solved, matching the next is close to mechanical -- worth checking
`GameApplicationMethods`'s remaining untyped slots (`+0x058`, `+0x060`, `+0x064`)
for the same shape before re-deriving from scratch.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable -- single guard wraps the
  whole body, shared epilogue either way, no differing-return-value early
  exit.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.

## Naming

**`GameApplication__PlayOpeningMovie` -- tier B.** Mechanics: gated by
`arg->unk08`, builds a `StreamTask`, derives its type code via
`PickOpeningMovie` -- documented in this unit's header as "day/week-style
calculation (divides SeedAndRandom's result by 7)" -- looks it up, then
configures and starts the task. "Weekly" is grounded in that documented
`/7` derivation inside `PickOpeningMovie` (real evidence, not a guess from the
function's own body, which is otherwise the same generic StreamTask-launch
shape as its three siblings in this unit).

## Track 4 (2026-09-25, round 84, alpha)

GameApplication.h's StreamTask view names +0x004 `release` (BasicClass's, `void *`), was `start` (track 4 round 84; see GameApplication__PlayCinematic for the bytes that settled the return type). Byte-identical.
