# GameApplication__ShowImage

> Renamed from `GameApplication__StartLoaderTask` on 2026-09-27 (tools/rename.py). Address 0x80026254.

> Renamed from `Class6D3C8__StartLoaderTask` on 2026-09-26 (tools/rename.py). Address 0x80026254.

> Renamed from `func_80026254` on 2026-09-24 (tools/rename.py). Address 0x80026254.

**Unit:** GameApplicationFileResource · **Size:** 53 instructions (0xD4 bytes) · **Status:** MATCHED (53/53 words, whole-image SHA1 green), first attempt

## What it does

Registers a "loader" task for a named resource: allocates a `LoaderTask`
(`New_TaskCore`, a `New_X`-shaped allocator in uncarved `task`, 0xA4
bytes), gives it a completion callback (`GameApplication__RegisterFilesCallback`, already matched in
this unit) and a context pointer (`self`), then sets its remaining
parameters (`path`, `self->unk1C`) and starts it. Called twice by
`GameApplication__ShowIntroLogos` (already matched, same unit), once for
`"ETC\ASMKLOGO.TIM"` and once for `"ETC\OSDLOGO.TIM"`.

## Derivation

```
jal  New_TaskCore(0, 0, 0)             ; -> s0 = task
lui  $a1, %hi(GameApplication__RegisterFilesCallback)
addiu $a1, $a1, %lo(GameApplication__RegisterFilesCallback)
lw   $v0, 0x0($s0)                       ; task->methods
lw   $v0, 0x98($v0)                       ; slot98
jalr $v0                                    ; task->methods->slot98(task, &GameApplication__RegisterFilesCallback, self)
 a2 = s1 (self)
lw   $v0, 0x0($s0)
lw   $v0, 0x6C($v0)                          ; slot6C
jalr $v0                                        ; task->methods->slot6C(task, 0)
 a1 = 0
lw   $v0, 0x0($s0)
a1 = s2 (path)
lw   $v0, 0xD4($v0)                              ; slotD4
jalr $v0                                            ; task->methods->slotD4(task, path, 0)
 a2 = 0
lw   $v0, 0x0($s0)
a1 = s1->unk1C (self->unk1C)
lw   $v0, 0x44($v0)                                  ; slot44
jalr $v0                                                ; task->methods->slot44(task, self->unk1C, 0)
 a2 = 0
lw   $v0, 0x0($s0)
lw   $v0, 0x4($v0)                                       ; slot4
jalr $v0                                                    ; task->methods->slot4(task)
 a0 = s0
```

```c
void GameApplication__ShowImage(GameApplication *self, const char *path) {
    LoaderTask *task = New_TaskCore(0, 0, 0);

    task->methods->slot98(task, GameApplication__RegisterFilesCallback, self);
    task->methods->slot6C(task, 0);
    task->methods->slotD4(task, path, 0);
    task->methods->slot44(task, self->unk1C, 0);
    task->methods->slot4(task);
}
```

Matched first attempt: the caller (`GameApplication__ShowIntroLogos`) had already forced a
careful read of this exact vtable-call idiom (task->methods reloaded fresh
before every dispatch), and `GameApplication__RegisterFilesCallback`'s signature (`s32 (void)`)
happened to be exactly the type `slot98`'s callback parameter needed, so no
cast was required.

## New struct/header knowledge

`include/GameApplication.h`: added `LoaderTaskMethods`/`LoaderTask` (the class
behind `New_TaskCore`, distinct from `StreamTaskMethods` used by
`GameApplication__ShowIntroLogos` -- different allocator, different signatures at the same
slot offsets) and the `New_TaskCore` extern. Forward-declared
`GameApplication__RegisterFilesCallback` (defined later in this same file/ROM order) so it can be
passed as `slot98`'s callback argument.

## Proposed learning

Two different "task" classes in this game (`New_TaskCore`'s and
`New_StreamTask`'s allocators) share several slot offsets (`+0x004`,
`+0x044`) with *different* signatures -- confirms these are separate
vtables, not the same class called two ways, and is a reminder not to
assume a shared offset number implies a shared method signature across
unrelated class hierarchies. Resolve each with its own allocator's
constructor call before typing the slot, rather than pattern-matching the
offset against an already-typed sibling class.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable -- no early exit at all,
  straight-line code.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.

## Naming history (before round 100)
**`GameApplication__ShowImage` -- tier A.** Pure leaf helper, mechanics ARE
the purpose: allocates a `LoaderTask` (`New_TaskCore`), registers a
completion callback and context (`slot98`, `GameApplication__RegisterFilesCallback`,
`self`), sets its remaining parameters (path, `self->unk1C`) via `slot6C`/
`slotD4`/`slot44`, and starts it (`start`, this unit's renamed
`LoaderTaskMethods.start`). Called twice from
`GameApplication__ShowIntroLogos` with two different fixed paths, so the
name is generic to the mechanic (registering and starting a LoaderTask for
a given resource path) rather than either specific asset.

## Track 4 (2026-09-25, round 84, alpha)

The task these functions build with New_TaskCore is a plain TaskCore (include/TaskCore.h, track 4 round 84); GameApplication.h's LoaderTask view is gone and the calls use TaskCore's slot names (setCallback, setFrameBound, setSubHandle, init, release). The old `start` slot at +0x004 is BasicClass's release, and StreamTask's own +0x004 is typed `void *(*release)` too: with one void and one value-returning, StartCinematicStream's two branches stopped cross-jumping into one call (+6 instructions). Byte-identical.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__ShowImage` -- tier A** (renamed from `GameApplication__StartLoaderTask` with tools/rename.py). Evidence: a TaskCore given setSubHandle(path, NULL) makes its sub-handle New_TimImage(path) (TaskCore.h, subHandle) and shows it; both callers pass a logo .TIM (ShowIntroLogos), and PlayCinematic builds the same task for a special day's TIM. There is no LoaderTask class (TaskCore.h, round 84).

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Registers a "loader" task for the given resource path: allocates the
 * task, gives it a completion callback (GameApplication__LoaderTaskDoneCallback) and context
 * (self), then sets its remaining parameters (path, the parent's aux as its init args) and
 * starts it. */
```
