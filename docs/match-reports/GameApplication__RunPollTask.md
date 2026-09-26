# GameApplication__RunPollTask

> Renamed from `Class6D3C8__RunPollTask` on 2026-09-26 (tools/rename.py). Address 0x80026518.

> Renamed from `func_80026518` on 2026-09-24 (tools/rename.py). Address 0x80026518.

**Unit:** code_1677c · **Size:** 29 instructions (0x74 bytes) · **Status:** MATCHED (29/29 words, whole-image SHA1 green), first attempt

## What it does

A small helper used by `GameApplication__PollGraphRoomStatus` (matched just before this): constructs
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
s32 GameApplication__RunPollTask(PollTaskCtor ctor, void *dreamSys, s32 extra) {
    PollTask *task = ctor(dreamSys);
    s32 result = task->methods->slot44(task, extra, 0);

    task->methods->slot4(task);
    return result;
}
```

Matched first attempt -- the "delay slot after a `jalr` captures the
*preceding* call's return value, not the one about to run" idiom (first
found the hard way in `GameApplication__LoadIntroLogoSequence`) was already the expected read here.

## Proposed learning

None beyond what `GameApplication__LoadIntroLogoSequence`'s report already recorded; this function is
a clean instance of the same idiom with no new residue.

## HEAD BROADCAST cross-check (this round's two levers)

- **goto/return early-exit lever:** not applicable -- straight-line code, no
  branches at all.
- **hand-hoisted loop invariant lever:** not applicable -- no loop.

## Naming

**`GameApplication__RunPollTask` -- tier A.** Pure leaf helper, mechanics ARE the
purpose: constructs a `PollTask` via the caller-supplied `ctor`, dispatches
`slot44(task, extra, 0)` and `slot4(task)` on it (fire-and-forget teardown),
returns `slot44`'s result. Generic across both call sites
(`New_GraphRoom`/`New_TitleMenu`, both used only by
`GameApplication__PollGraphRoomStatus`), so it is named for what it mechanically
does (build, query, dispose a PollTask) rather than for either specific
caller.
