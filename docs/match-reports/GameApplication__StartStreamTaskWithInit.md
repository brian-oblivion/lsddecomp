# GameApplication__StartStreamTaskWithInit

> Renamed from `Class6D3C8__StartStreamTaskWithInit` on 2026-09-26 (tools/rename.py). Address 0x80026900.

> Renamed from `func_80026900` on 2026-09-24 (tools/rename.py). Address 0x80026900.

**Unit:** code_1677c · **Size:** 56 instructions (0xE0 bytes) · **Status:** MATCHED (56/56 words, whole-image SHA1 green), first attempt

## What it does

`GameApplicationMethods` slot `+0x064`, the last function in this unit's queue.
Gated by `self->arg->unk08` (the same gate `GameApplication__StartWeeklyStreamTask` and
`GameApplication__StartGraphRoomStreamTask` use). Builds a `StreamTask`, runs its `slot12C`, derives a
type code via `GetStreamChannelInit` (a new library helper with the same
"write-to-`*out`, return-a-separate-value" shape as
`GetAsmkMovie`/`PickOpeningMovie`/`GetGraphRoomStreamChannel`), looks it up via
`GetStreamGroupForType`, initializes the task with it, then starts it -- the same
overall shape as `GameApplication__LoadIntroLogoSequence`/`GameApplication__StartWeeklyStreamTask`, with a `slot12C` call
added (matching `GameApplication__StartGraphRoomStreamTask`'s use of that slot).

## Final C

```c
void GameApplication__StartStreamTaskWithInit(GameApplication *self) {
    StreamTask *task;
    s32 typeCode;
    s32 outerValue;
    s32 typeLookup;

    if (self->arg->unk08 != 0) {
        SetActiveDataSourceDriverMode(0, 0, 0);
        task = New_StreamTask(0, 0, 0, 0);
        task->methods->slot12C(task, 0);
        outerValue = GetStreamChannelInit(&typeCode, 0);
        typeLookup = GetStreamGroupForType(typeCode);
        task->methods->slot44(task, self->unk1C, outerValue, typeLookup, 1);
        task->methods->slot4(task);
    }
}
```

Matched first attempt: this unit's own established idioms carried straight
across --

- the `task->methods->slot44(..., outerValue, typeLookup, 1)` shape, where
  `outerValue` is a PRECEDING call's return (`GetStreamChannelInit`, captured via
  the delay slot of the `GetStreamGroupForType` `jal` right after it) and
  `typeLookup` is the REAL 4th argument (the delay slot of `slot44`'s own
  `jalr`, per the idiom `GameApplication__LoadIntroLogoSequence`'s report first documented);
- declaring `typeCode`/`outerValue`/`typeLookup` before `task` (the
  register-allocation-by-declaration-order lesson from `GameApplication__LoadIntroLogoSequence`),
  though here it turned out not to matter -- the natural declaration order
  already put `task` last relative to its own first use, so no reordering
  was needed this time.

## Proposed learning

None beyond what this unit's earlier reports already established; this
function is a clean fourth instance of the "StreamTask init" shape
(`GameApplication__LoadIntroLogoSequence`, `GameApplication__StartWeeklyStreamTask`, `GameApplication__StartGraphRoomStreamTask`, now this one), each
gated by a different `GameApplicationConfig` field and differing only in
which library helper derives the type code and whether extra slots
(`slot12C`) are involved.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable -- single guard wraps
  the whole body, void function, no early exit.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.

## Naming

**`GameApplication__StartStreamTaskWithInit` -- tier B.** Mechanics: gated by
`arg->unk08` (same gate as `GameApplication__StartWeeklyStreamTask`), builds a
`StreamTask`, runs its `slot12C` (a step none of the other three
StreamTask-launcher siblings besides `GameApplication__StartCinematicStream`
perform), derives a type code via `GetStreamChannelInit`, looks it up, configures
and starts the task. "WithInit" names the one mechanical difference from
its closest sibling `GameApplication__StartWeeklyStreamTask` (the extra
`slot12C` call); `GetStreamChannelInit`'s own meaning is not established, so no
stronger, purpose-based name is supported yet.

## Track 4 (2026-09-25, round 84, alpha)

GameApplication.h's StreamTask view names +0x004 `release` (BasicClass's, `void *`), was `start` (track 4 round 84; see GameApplication__StartCinematicStream for the bytes that settled the return type). Byte-identical.
