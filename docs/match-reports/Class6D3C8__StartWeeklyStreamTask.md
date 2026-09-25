# Class6D3C8__StartWeeklyStreamTask

> Renamed from `func_80026348` on 2026-09-24 (tools/rename.py). Address 0x80026348.

**Unit:** code_1677c · **Size:** 50 instructions (0xC8 bytes) · **Status:** MATCHED (50/50 words, whole-image SHA1 green), first attempt

## What it does

`Class6D3C8Methods` slot `+0x054`. Gated by `self->arg->unk08 != 0` (a
second boolean/pointer gate on the ctor argument, sibling to
`Class6D3C8__LoadIntroLogoSequence`'s `unk0C` gate): builds a `StreamTask`, derives a type code
via `func_8004913C` (a day/week-style calculation, unrelated unit,
`psyq_memset.s`), looks it up via `func_800493C8`, and initializes+starts
the task the same way `Class6D3C8__LoadIntroLogoSequence` does -- minus that function's two
`Class6D3C8__StartLoaderTask` loader-task registrations.

## Derivation

Structurally identical to the second half of `Class6D3C8__LoadIntroLogoSequence` (already
matched), with `func_8004913C(&typeCode, 0)` in place of
`func_800490F4(&typeCode)`:

```c
void Class6D3C8__StartWeeklyStreamTask(Class6D3C8 *self) {
    s32 derivedValue;
    s32 typeCode;
    s32 typeLookup;
    StreamTask *task;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTaskObj(0, 0, 0, 0);
        derivedValue = func_8004913C(&typeCode, 0);
        typeLookup = func_800493C8(typeCode);
        task->methods->slot44(task, self->unk1C, derivedValue, typeLookup, 1);
        task->methods->slot4(task);
    }
}
```

Matched first attempt, entirely on the strength of `Class6D3C8__LoadIntroLogoSequence`'s
already-solved register-allocation-by-declaration-order lesson (declared
`derivedValue`/`typeCode` before `task`, mirroring that function's fix) and
its "the delay slot after `jalr` is the real 4th argument, not a scratch
reload" lesson (used `typeLookup`, not `task->methods`, as the 4th `slot44`
argument from the start).

## New struct/header knowledge

`include/Class6D3C8.h`: split `Class6D3C8CtorArgs`'s `+0x04..+0x0B` padding
to expose `+0x08` (`unk08`, this function's gate) as its own field,
matching the existing `+0x0C` (`unk0C`, `Class6D3C8__LoadIntroLogoSequence`'s gate). Declared
`func_8004913C` (day/week-style helper, `psyq_memset.s`, same "write an
index to *out, return a related but different value" shape as
`func_800490F4`).

## Proposed learning

Two sibling `Class6D3C8CtorArgs` gate fields (`+0x08`, `+0x0C`) each guard a
near-identical "build a StreamTask, derive+lookup a type code, init and
start it" block, differing only in which helper derives the type code and
whether extra `LoaderTask` registrations bookend it. Once one sibling is
solved, matching the next is close to mechanical -- worth checking
`Class6D3C8Methods`'s remaining untyped slots (`+0x058`, `+0x060`, `+0x064`)
for the same shape before re-deriving from scratch.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable -- single guard wraps the
  whole body, shared epilogue either way, no differing-return-value early
  exit.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.

## Naming

**`Class6D3C8__StartWeeklyStreamTask` -- tier B.** Mechanics: gated by
`arg->unk08`, builds a `StreamTask`, derives its type code via
`func_8004913C` -- documented in this unit's header as "day/week-style
calculation (divides SeedAndRandom's result by 7)" -- looks it up, then
configures and starts the task. "Weekly" is grounded in that documented
`/7` derivation inside `func_8004913C` (real evidence, not a guess from the
function's own body, which is otherwise the same generic StreamTask-launch
shape as its three siblings in this unit).
