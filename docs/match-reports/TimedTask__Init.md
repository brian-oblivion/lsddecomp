# TimedTask__Init — MATCHED (24/24 words)

> Renamed from `Class86668__Init` on 2026-09-26 (tools/rename.py). Address 0x8004a2c4.

> Renamed from `func_8004A2C4` on 2026-09-23 (tools/rename.py). Address 0x8004a2c4.

`Obj865C8`'s vtable slot +0x044 (`TimedTaskMethods`, i.e. the sibling class
`gTimedTaskMethods` overriding `gClass865C8Methods`'s +0x044 — see `class_39e08.h`'s existing
note that this function is one of gTimedTaskMethods's known overrides at
+0x008/+0x00C/+0x040/+0x044/+0x048).

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s0, 0x10($sp)
addu  $s0, $a0, $zero        ; s0 = self
sw    $s1, 0x14($sp)
addu  $s1, $a1, $zero        ; s1 = arg1
sw    $s2, 0x18($sp)
addu  $s2, $a2, $zero        ; s2 = arg2
sw    $ra, 0x1C($sp)
jal   Get_vtable_IntermediateBase
 sw   $zero, 0x28($s0)       ; self->unk28 = 0 (delay slot)
addu  $a0, $s0, $zero
addu  $a1, $s1, $zero
lw    $v0, 0x44($v0)         ; base vtable slot +0x044
nop
jalr  $v0
 addu $a2, $s2, $zero
lw    $v0, 0x28($s0)         ; return self->unk28
lw    $ra, 0x1C($sp)
...
jr    $ra
 nop
```

## Final C

```c
s32 TimedTask__Init(Obj865C8 *self, s32 arg1, s32 arg2) {
    self->unk28 = 0;
    Get_vtable_IntermediateBase()->slot44(self, arg1, arg2);
    return self->unk28;
}
```

## Header change

`include/class_39e08.h`'s `IntermediateBaseMethods` (the view of
`Get_vtable_IntermediateBase()`'s table, `gIntermediateBaseMethods`) had no `+0x044` slot typed yet —
only `dtor` (+0x00C), `slot48` (+0x048), `slot60` (+0x060). Added:

```c
void (*slot44)(void *self, s32 arg1, s32 arg2); /* +0x044 */
```

typed `(void *self, s32, s32)`, matching the sibling unit's existing
`code_2c054.h` `TaskUtilMethods::slot44`, which resolves through the SAME
accessor (`Get_vtable_IntermediateBase()`, same `gIntermediateBaseMethods` table). That sibling's
occupant, `TaskCore__Init`, is a near-identical shape one level down a
different delegation chain: `Get_vtable_IntermediateBase()->slot44(self, a1, a2); return
self->unk38;` — no pre-zero, just call-then-return. This function pre-zeroes
`self->unk28` in the `jal`'s own delay slot before the call and reads it back
after, i.e. "default value, base call may overwrite" rather than "base call
always sets it" — consistent with `TimedTask__Init` being an *override* of this
slot for `gTimedTaskMethods` while `TaskCore__Init` is a different class occupying
the analogous position in its own chain.

## Attempts

1 (matched on first attempt — the sibling-unit precedent for the same
`Get_vtable_IntermediateBase()->slot44` accessor/slot pair made the shape unambiguous).

### Proposed learning

`Get_vtable_IntermediateBase()` (table `gIntermediateBaseMethods`) slot +0x044 is confirmed
`void (*)(void *self, s32 arg1, s32 arg2)` from two independent call sites in
two different units (`code_2c054.c`'s `TaskCore__Init`, this unit's
`TimedTask__Init`) — both immediately store the same two register-passed
arguments into the call and immediately read a `self`-relative `s32` field
back out. Worth typing consistently anywhere else this same accessor/slot
pair turns up.

## Naming

`TimedTask__Init` -- tier B. Occupies +0x044 (the same Init-slot convention as `Class865C8__Init`, see above): zeroes `eventCode`, forwards to the base's own +0x044, returns `eventCode`. Named by slot-offset convention, not by an established in-game meaning.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/TimedTask.h`. Not renamed. Signature `s32 (TimedTask *self, IntermediateBaseInitArgs *args, s32 mode)`, IntermediateBase's `init` slot; `eventCode` is `result`, which GameApplication__PollStatusObj switches on through Obj865C8 (1 is a timeout, TimedTask__SetState's state 4). Class865C8__Init and ObjM__AttachTarget call it as `GetTimedTaskMethods()->init((TimedTask *)self, (IntermediateBaseInitArgs *)..., mode)`. Image byte-identical.
