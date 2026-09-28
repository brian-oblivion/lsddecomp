# TimedTask__CancelTimeout

> Renamed from `Class86668__CancelTimeout` on 2026-09-26 (tools/rename.py). Address 0x8004a294.

> Renamed from `func_8004A294` on 2026-09-23 (tools/rename.py). Address 0x8004a294.

**Unit:** dream_day · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED (12/12 words)

## What it does

Method-table slot +0x040 of `gTimedTaskMethods` (the sibling class, see
`TimedTask__Finalize.md` for how the sibling relationship was established). A
one-line wrapper: dispatches through `self->methods` at +0x06C with a
sentinel argument of -1.

## Derivation

```
lw    $v0, 0x0($a0)
lw    $v0, 0x6C($v0)
jalr  $v0
 addiu $a1, $zero, -1
```

Written as:

```c
void TimedTask__CancelTimeout(Obj865C8 *self) {
    self->methods->setUnk2C(self, -1);
}
```

Return type is `void`, not "unknown wrapper, assume the callee's type" --
this is NOT the ambiguous one-line-wrapper case CLAUDE.md warns about,
because the callee (`+0x06C`, `TimedTask__SetTimeout`, this unit's own function,
matched the same round) is confirmed void from its own disassembly: it ends
`jr $ra` / `nop` with no `$v0` ever set. Positive evidence, not silence.

`self->methods` is typed `DayTaskMethods *` even though the runtime
object is (per its ctor, `gTimedTaskMethods`) a sibling-class instance: both
tables agree on the field type at +0x06C (confirmed identical function
address in `tools/classtable.py 0x800865C8 --vs 0x8006E878` /
`0x80086668 --vs 0x8006E878`), so one struct type serves both call sites
without needing a full second method-table type.

## Proposed learning

None beyond what's already documented.

## Naming

`TimedTask__CancelTimeout` -- tier A. Occupies `gTimedTaskMethods` +0x040; one-line wrapper calling `self->methods->setTimeout(self, -1)` (the sentinel `SetTimeout` itself documents as 'disabled'). Mechanics are its purpose.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/timed_task.h`. Not renamed. It fills IntermediateBase's `resetCounters` slot (+0x040) but does not chain to IntermediateBase__ResetCounters: its whole body is `setTimeout(-1)`, so the name says what it does rather than what the slot is. `self` is `TimedTask *`. Image byte-identical.
