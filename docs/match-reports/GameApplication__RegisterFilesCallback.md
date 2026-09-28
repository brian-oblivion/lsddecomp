# GameApplication__RegisterFilesCallback

> Renamed from `GameApplication__LoaderTaskDoneCallback` on 2026-09-27 (tools/rename.py). Address 0x80026328.

> Renamed from `Class6D3C8__LoaderTaskDoneCallback` on 2026-09-26 (tools/rename.py). Address 0x80026328.

> Renamed from `func_80026328` on 2026-09-24 (tools/rename.py). Address 0x80026328.

**Unit:** GameApplicationFileResource · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

One-line forwarding wrapper: `return RegisterRecordTableFiles(0);`

## Derivation

```
addiu $sp, $sp, -0x18
sw    $ra, 0x10($sp)
jal   RegisterRecordTableFiles
 addu $a0, $zero, $zero
lw    $ra, 0x10($sp)
addiu $sp, $sp, 0x18
jr    $ra
 nop
```

No instruction touches `$v0` between the `jal` and the `jr $ra` — the
callee's return value passes straight through in the register. Per
CLAUDE.md ("the byte match tells you NOTHING about the return type" for a
one-line tail-call wrapper), the *bytes* alone don't distinguish `void
GameApplication__RegisterFilesCallback(void)` from a value-returning one. Positive evidence that
`RegisterRecordTableFiles` itself returns a value: its own body (`asm/DayTaskStageMap.s`,
around `RegisterRecordTableFiles`) ends with `beqz $v0, .L8004A104` gating the loop
exit on `$v0`, i.e. it's actively computed and meaningful, not incidentally
left in the register. Written to forward it:

```c
extern s32 RegisterRecordTableFiles(s32 a0);

s32 GameApplication__RegisterFilesCallback(void) {
    return RegisterRecordTableFiles(0);
}
```

## Proposed learning

None beyond the standing "wrapper return type" rule already in CLAUDE.md —
this is a clean instance of it: the callee's own use of `$v0` as a real
result (a loop-exit condition) is what tips the type call, not the wrapper's
bytes.

## Naming history (before round 100)
**`GameApplication__RegisterFilesCallback` -- tier A.** Pure leaf: `return
RegisterRecordTableFiles(0);`, mechanics ARE the purpose (a forwarding wrapper). Named
from its one use, `GameApplication__ShowImage`'s `task->methods->slot98(task,
GameApplication__RegisterFilesCallback, self)` call -- `slot98` registers a
completion callback and a context pointer on a `LoaderTask`, so this
function's role (not its ultimate game purpose, which depends on the
uncarved `RegisterRecordTableFiles`) is exactly "the callback a LoaderTask runs on
completion".

## Track 7 polish (round 100, echo)

### Naming

**`GameApplication__RegisterFilesCallback` -- tier A** (renamed from `GameApplication__LoaderTaskDoneCallback` with tools/rename.py). Evidence: the body is RegisterRecordTableFiles(0) (DayTaskStageMap.c, registers gRecordTable's files with the CD driver); its one use is ShowImage's setCallback, the TaskCore's view callback, which refreshViewValue calls as the image ends.

Body changes, all byte-identical: RegisterRecordTableFiles's extern parameter a0 -> all, as DayTaskStageMap.h declares it.

