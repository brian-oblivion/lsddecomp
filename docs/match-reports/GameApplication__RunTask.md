# GameApplication__RunTask

> Renamed from `GameApplication__RunPollTask` on 2026-09-27 (tools/rename.py). Address 0x80026518.

> Renamed from `Class6D3C8__RunPollTask` on 2026-09-26 (tools/rename.py). Address 0x80026518.

> Renamed from `func_80026518` on 2026-09-24 (tools/rename.py). Address 0x80026518.

**Unit:** game_shell · **Size:** 29 instructions (0x74 bytes) · **Status:** MATCHED (29/29 words, whole-image SHA1 green), first attempt

## What it does

A small helper used by `GameApplication__RunTitleMenu` (matched just before this): constructs
a `PollTask` by calling the caller-supplied constructor function pointer
directly (`ctor(dreamSys)` -- not through any vtable, the function pointer
itself IS the constructor), dispatches `slot44(task, extra, 0)` and
`slot4(task)` on the result, and returns `slot44`'s result (`slot4`'s own
return is discarded).

## Derivation

```
addu $v0, $a0, $zero       ; v0 = ctor (the incoming a0)
addu $a0, $a1, $zero        ; a0 = dreamSys (the incoming a1)
sw   $s1, 0x14($sp)
addu $s1, $a2, $zero          ; s1 = extra (the incoming a2)
sw   $ra, 0x18($sp)
jalr $v0                        ; task = ctor(dreamSys)
 sw  $s0, 0x10($sp)
addu $s0, $v0, $zero              ; s0 = task
addu $a0, $s0, $zero
lw   $v0, 0x0($s0)                  ; task->methods
addu $a1, $s1, $zero
lw   $v0, 0x44($v0)                   ; slot44
nop
jalr $v0                                ; task->methods->slot44(task, extra, 0)
 addu $a2, $zero, $zero
lw   $v1, 0x0($s0)
addu $a0, $s0, $zero
lw   $v1, 0x4($v1)                        ; slot4
nop
jalr $v1                                     ; task->methods->slot4(task)
 addu $s0, $v0, $zero                          ; delay: s0 = v0 = slot44's return (PRE-call value)
addu $v0, $s0, $zero                              ; return that saved value
... epilogue ...
```

```c
s32 GameApplication__RunTask(PollTaskCtor ctor, void *dreamSys, s32 extra) {
    PollTask *task = ctor(dreamSys);
    s32 result = task->methods->slot44(task, extra, 0);

    task->methods->slot4(task);
    return result;
}
```

Matched first attempt -- the "delay slot after a `jalr` captures the
*preceding* call's return value, not the one about to run" idiom (first
found the hard way in `GameApplication__ShowIntroLogos`) was already the expected read here.

## Proposed learning

None beyond what `GameApplication__ShowIntroLogos`'s report already recorded; this function is
a clean instance of the same idiom with no new residue.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable -- straight-line code, no
  branches at all.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.

## Naming history (before round 100)
**`GameApplication__RunTask` -- tier A.** Pure leaf helper, mechanics ARE the
purpose: constructs a `PollTask` via the caller-supplied `ctor`, dispatches
`slot44(task, extra, 0)` and `slot4(task)` on it (fire-and-forget teardown),
returns `slot44`'s result. Generic across both call sites
(`New_GraphRoom`/`New_TitleMenu`, both used only by
`GameApplication__RunTitleMenu`), so it is named for what it mechanically
does (build, query, dispose a PollTask) rather than for either specific
caller.

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__RunTask` -- tier A** (renamed from `GameApplication__RunPollTask` with tools/rename.py). Evidence: mechanics are the purpose: newTask(dreamSys), init(initArgs, 0) (IntermediateBase's mode 0 runs the task to its end), release, return init's result. Callers pass New_GraphRoom and New_TitleMenu.

Body changes, all byte-identical: the unit-local PollTask/PollTaskMethods view (slot4 = BasicClass's release, slot44 = IntermediateBase's init) is retired: the task is an IntermediateBase (include/IntermediateBase.h, unified) and the allocator type is NewTaskFn, IntermediateBase *(*)(struct DreamSys *). Parameters ctor/void *dreamSys -> newTask/struct DreamSys *dreamSys. Byte-identical.

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Constructs a PollTask via the caller-supplied `ctor`, dispatches
 * slot44(task, initArgs, 0) and slot4(task) on it (fire-and-forget), and
 * returns slot44's result. */

/* The PollTasks GameApplication__RunPollTask runs (New_GraphRoom, New_TitleMenu)
 * are TaskCore-family classes; this is this unit's own minimal view of
 * them: +0x004 is BasicClass's release, +0x044 IntermediateBase's init.
 * Constructed directly by a caller-supplied function pointer
 * (GameApplication__RunPollTask's own a0) rather than a New_X-style allocator. */
typedef struct PollTaskMethods { ... slot4 at +0x004, slot44 at +0x044 ... } PollTaskMethods;
typedef struct PollTask { PollTaskMethods *methods; } PollTask;
typedef PollTask *(*PollTaskCtor)(void *arg);

/* This unit's own function, defined later in ROM order (forward declared
 * for GameApplication__PollGraphRoomStatus, which comes first). Constructs a PollTask via the
 * caller-supplied `ctor`, dispatches slot44(task, initArgs, 0) and slot4(task)
 * on it, and returns slot44's result. */

/* PollTask constructors (not this unit's to write). Called directly (not
 * through any vtable) as GameApplication__RunPollTask's `ctor` argument.
 * New_GraphRoom is include/GraphRoom.h's and New_TitleMenu
 * include/TitleMenu.h's, each cast to PollTaskCtor. */
```
