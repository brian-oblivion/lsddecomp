# DayTask__OnInit — MATCHED (63/63 words)

> Renamed from `Class865C8__OnInit` on 2026-09-26 (tools/rename.py). Address 0x80049b54.

> Renamed from `Obj865C8__StartSubA` on 2026-09-26 (tools/rename.py). Address 0x80049b54.

> Renamed from `func_80049B54` on 2026-09-23 (tools/rename.py). Address 0x80049b54.

`Obj865C8`'s vtable slot +0x04C.

## Disassembly shape

```
addiu $sp, $sp, -0x28
sw    $s1, 0x1C($sp)
addu  $s1, $a0, $zero        ; s1 = self
sw    $ra, 0x20($sp)
sw    $s0, 0x18($sp)
lw    $v0, 0xC($s1)          ; self->unk0C
nop
lw    $a0, 0x0($v0)          ; self->unk0C->obj
nop
lw    $v0, 0x0($a0)          ; obj->methods
nop
lw    $v0, 0x7C($v0)         ; methods->slot7C
lw    $s0, 0x18($s1)         ; s0 = self->subA
jalr  $v0
 addu $a1, $zero, $zero      ; obj->methods->slot7C(obj, 0)
lw    $v1, 0x0($s0)          ; subA->methods
addu  $a0, $s0, $zero
lw    $v1, 0x44($v1)         ; methods->slot44
jalr  $v1
 addu $a1, $v0, $zero        ; a1 = result of slot7C call -- subA->methods->slot44(subA, result)
lw    $v0, 0x0($s0)          ; subA->methods (reload)
nop
lw    $v0, 0xAC($v0)         ; methods->slot0xAC
jalr  $v0
 addu $a0, $s0, $zero        ; subA->methods->slot0xAC(subA)
lw    $v1, 0x0($v0)          ; (returned obj)->methods
addu  $a0, $v0, $zero
lw    $v0, 0x60($v1)         ; methods->slot60
jalr  $v0
 ori  $a1, $zero, 0x1        ; ret->methods->slot60(ret, 1)
lw    $v0, 0x0($s0)          ; subA->methods (reload)
addu  $a0, $s0, $zero
lw    $v0, 0x4C($v0)         ; methods->slot4C
jalr  $v0
 ori  $a1, $zero, 0x4B0      ; subA->methods->slot4C(subA, 0x4B0)
lw    $v0, 0x0($s0)          ; subA->methods (reload)
lui   $a2, %hi(sDayViewPoint)
addiu $a2, $a2, %lo(sDayViewPoint)
sw    $zero, 0x10($sp)       ; 5th (stack) arg = 0
lw    $a1, 0x38($s1)         ; self->unk38
lw    $v0, 0x70($v0)         ; methods->slot70
lui   $a3, %hi(sDayViewRef)
addiu $a3, $a3, %lo(sDayViewRef)
jalr  $v0
 addu $a0, $s0, $zero        ; subA->methods->slot70(subA, self->unk38, &sDayViewPoint, &sDayViewRef, 0)
lw    $v0, 0x0($s0)          ; subA->methods (reload)
nop
lw    $v0, 0x8C($v0)         ; methods->slot8C
jalr  $v0
 addu $a0, $s0, $zero        ; subA->methods->slot8C(subA)
ori   $v0, $zero, 0x1
sw    $v0, 0x3C($s1)         ; self->unk3C = 1
...
jr $ra
```

## Final C

```c
void DayTask__OnInit(Obj865C8 *self) {
    SubObjE *obj;
    SubObjA *subA;
    SubObjF *ret;
    s32 result;

    obj = self->unk0C->obj;
    subA = self->subA;
    result = obj->methods->slot7C(obj, 0);
    subA->methods->slot44(subA, result);
    ret = subA->methods->slot0xAC(subA);
    ret->methods->slot60(ret, 1);
    subA->methods->slot4C(subA, 0x4B0);
    subA->methods->slot70(subA, self->unk38, sDayViewPoint, sDayViewRef, 0);
    subA->methods->slot8C(subA);
    self->unk3C = 1;
}
```

## New struct knowledge (`include/class_39e08.h`)

- `Obj0C::obj` (+0x000) named: a `SubObjE *`, previously unnamed padding.
  `Obj0C` itself has no vtable of its own; this is the first evidence its
  offset-0 field POINTS to something that does.
- New opaque type `SubObjE`/`SubObjEMethods` (`slot7C`, `s32 (*)(SubObjE
  *self, s32 arg1)` — return value is used, so not void, matching CLAUDE.md's
  "discarded return is never evidence of void" from the OTHER direction: here
  the return genuinely IS consumed).
- `SubObjAMethods` extended with `slot44`, `slot4C`, `slot70`, `slot8C`,
  `slot0xAC` (all `Obj865C8::subA`'s vtable, alongside the already-known
  `slot74`/`slot90`).
- New opaque type `SubObjF`/`SubObjFMethods` (`slot60`, `void (*)(SubObjF
  *self, s32 arg1)`) — what `slot0xAC` returns.
- Two new rodata symbol externs, `sDayViewPoint`/`sDayViewRef` (`extern u8 [];`,
  address-only, real element type unknown) — both sit just before this
  unit's own `gTimedTaskMethods` vtable in memory (0x18 and 0xC bytes before it
  respectively), passed straight through to `slot70` without dereferencing.

## Header mechanics note (not a match residue, just a C89 trap)

Two forward-declared-then-defined types (`SubObjA`, needed by
`SubObjAMethods::slot0xAC`'s own self-parameter before `SubObjA` was
otherwise in scope) initially kept a `typedef struct SubObjA { ... }
SubObjA;` at the definition site IN ADDITION to the earlier `typedef struct
SubObjA SubObjA;` forward declaration -- cc1 rejected the duplicate typedef
with `parse error before '*'` at every subsequent `SubObjA *` use in the
same struct (the error location is misleading; it points at unrelated later
lines, not the duplicate itself). Fixed by dropping the second `typedef`
keyword at the definition site (`struct SubObjA { ... };`, no repeated
name) -- matching the convention this header's `SubObjD`/`Obj4C` already
use. Worth remembering: forward-declare with `typedef struct X X;` once,
then define the body as a bare `struct X { ... };`, never `typedef struct X
{ ... } X;` a second time in the same header.

## Attempts

2. First attempt (before the forward-declare fix) failed to build with
   `parse error before '*'` at 4 sites in the header -- a header-mechanics
   bug, not a match residue. Fixed and rebuilt clean, 63/63 on the first
   successful build.

### Proposed learning — direct answer to the head's question

Still **no second instance** of "field name describes layout, not which
function runs once `self->methods` is reassigned" in this function either —
no vtable-pointer reassignment happens here, just straight-line dispatch
through `self->subA`'s and other objects' own stable vtables. Continuing to
report negative explicitly per the head's request.

Separately, a real (if narrow) **new** mechanical trap worth carrying
forward: redeclaring a forward-declared opaque type's `typedef` a second
time at its definition site produces a C89 parse error whose reported
location is unrelated later lines, not the duplicate itself — always define
forward-declared struct bodies as a bare `struct X { ... };`, never repeat
`typedef struct X { ... } X;`.

## Naming

`DayTask__OnInit` -- tier B. Sets up `subA` with a fixed constant (0x4B0) and two rodata addresses, then unconditionally sets `state = 1`: a clear 'begin' step for the state machine, but what `subA` itself represents in the game is not established (opaque vtable-only view).

## Track 4 (2026-09-26, round 88, DayTask)

The class (table D_800865C8, id 0x1F230, TimedTask's subclass) is unified as DayTask in include/DayTask.h; the Obj865C8/DayTaskMethods views in class_39e08.h are gone. Renamed from Obj865C8__StartSubA: it is the +0x04C onInit override (IntermediateBase__Init calls it). subA was IntermediateBase's viewport (+0x018), which holds the NodeGuardedViewport the ctor made, and SubObjA's slots are Viewport's: setScreenSize, getSubHandle (then SceneNode setDisplay(1)), setUnk44(0x4B0), attachViewChild(dreamSys, &sDayViewPoint, &sDayViewRef, NULL), initOt. It takes self alone; the slot keeps (self, s32, s32, s32).

## History moved from comments (track 7, round 99, charlie)

The comment on the two vectors (then `D_80086650`/`D_8008665C`, now
`sDayViewPoint`/`sDayViewRef`) ended "the data right before
gTimedTaskMethods"; they are `asm/data/76DC8.data.s`'s words at 0x80086650
and 0x8008665C.

## Naming (track 7, round 99, charlie)

- `sDayViewPoint` = (0, -1200, 0) and `sDayViewRef` = (0, -1200, 10000)
  (tier A: they are attachViewChild's `vp` and `vr` arguments, Viewport.h
  +0x070). Unit-static (`s`): no other code reads them.
- Locals `obj`/`ret` -> `drawSystem`/`fadeBox`: the init args' `drawSystem`
  and Viewport's `getFadeBox` result.
- SubObjE's +0x07C `slot7C` -> `getDims`: the object is the init args'
  `drawSystem`, and DrawSystem.h's +0x07C is `getDims` (DrawSystem__GetDims).
- `setUnk44(vp, 1200)`: decimal; Viewport.h says unk44 x unk48 is each
  buffer's packet area (default 2000) without settling which is the count, so
  the value keeps no name.
