# Class865C8__Deinit — MATCHED (37/37 words)

> Renamed from `Obj865C8__Deinit` on 2026-09-26 (tools/rename.py). Address 0x80049ac0.

> Renamed from `func_80049AC0` on 2026-09-23 (tools/rename.py). Address 0x80049ac0.

`Obj865C8`'s vtable slot +0x048.

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s1, 0x14($sp)
addu  $s1, $a0, $zero        ; s1 = self
sw    $ra, 0x18($sp)
sw    $s0, 0x10($sp)
lw    $s0, 0x38($s1)         ; s0 = self->unk38
jal   GetClass86668Methods
 nop
lw    $v0, 0x48($v0)         ; gClass86668Methods's own +0x048
jalr  $v0
 addu $a0, $s1, $zero        ; GetClass86668Methods()->slot48(self)
lw    $v0, 0x0($s0)          ; s0->methods
addu  $a0, $s0, $zero
lw    $v0, 0x110($v0)        ; methods->slot110
jalr  $v0
 addu $a1, $zero, $zero      ; s0->methods->slot110(s0, 0)
lw    $v0, 0xC($s1)          ; self->unk0C
lw    $v1, 0x0($s0)          ; s0->methods (reload)
lw    $a1, 0x4($v0)          ; (self->unk0C)->unk4
lw    $v0, 0x14($v1)         ; methods->slot14
jalr  $v0
 addu $a0, $s0, $zero        ; s0->methods->slot14(s0, self->unk0C->unk4)
lw    $v0, 0x0($s0)          ; s0->methods (reload)
lw    $a1, 0x10($s1)         ; self->unk10
lw    $v0, 0x14($v0)         ; methods->slot14 (same slot, 2nd call)
jalr  $v0
 addu $a0, $s0, $zero        ; s0->methods->slot14(s0, self->unk10)
...
jr $ra
```

`GetClass86668Methods()->slot48` is gClass86668Methods's own +0x048, which is
`Class86668__Deinit` (this unit, already matched:
`void Class86668__Deinit(Obj865C8 *self) { Get_vtable_IntermediateBase()->slot48(self); }`) —
i.e. this function forwards to the SIBLING class's slot48 override
explicitly, not to its own (`Class865C8__Deinit` itself occupies `gClass865C8Methods`'s
+0x048 slot — this is a self-referential-looking but actually cross-class
call, resolved by `tools/classtable.py`, not by inspection).

## Final C

```c
void Class865C8__Deinit(Obj865C8 *self) {
    SubObjD *sub = self->unk38;

    GetClass86668Methods()->slot48(self);
    sub->methods->slot110(sub, 0);
    sub->methods->slot14(sub, self->unk0C->unk4);
    sub->methods->slot14(sub, self->unk10);
}
```

## New struct knowledge (`include/class_39e08.h`)

- `Obj865C8::unk0C` retyped from `s32` (its only other use so far,
  `Class865C8__StartObjM`'s forwarded arg to `Get_vtable_IntermediateBase()->slot44`, a plain
  register-passthrough that never dereferences it) to `Obj0C *` — this
  function dereferences it (`->unk4`) directly. Updated `Class865C8__StartObjM`'s
  call site with an explicit `(s32)` cast; same register value either way,
  confirmed by rebuilding both functions together (33/33 and 37/37 both
  hold).
- `Obj865C8::unk10` (s32, new) — passed as a plain register value to a
  `slot14` call, never dereferenced.
- `Obj865C8::unk38` retyped from `s32` to `SubObjD *` for the same reason as
  `unk0C` — `Class865C8__StartObjM` never dereferences it either, cast added there
  too.
- New opaque type `Obj0C` (only field known: `unk4`, plain scalar,
  no vtable dispatch through it in this unit).
- New opaque type `SubObjD`/`SubObjDMethods` (vtable at offset 0, slots
  `+0x014` and `+0x110` reached here), same "only the dispatched slots
  named" policy as `SubObjA`/`SubObjB`/`Obj4C`.
- `Class86668Methods::slot48` added at +0x048, typed `void (*)(Obj865C8
  *self)` — occupied by this unit's own already-matched `Class86668__Deinit`.

## Attempts

1 (matched on first attempt).

### Proposed learning

The same struct offset can be dereferenced (pointer semantics) in one
function and only register-forwarded through an opaque `s32`-typed vtable
slot in another. When a later function proves the field is really a
pointer, retype the FIELD and add an explicit `(s32)` cast at the older,
opaque call site rather than leaving the field typed `s32` project-wide —
the cast reproduces the identical register move, and the field's real type
carries forward to every future reader. Confirmed here with two functions
(`Class865C8__Deinit`, `Class865C8__StartObjM`) sharing `Obj865C8::unk0C`/`unk38` with
opposite usage shapes; rebuilding both together after the retype held both
matches.

## Naming

`Class865C8__Deinit` -- tier B. Occupies +0x048, the mirror of Init's slot (`gIntermediateBaseMethods`'s +0x048 forwards to `IntermediateBase__Deinit`). Undoes what Init configured on the sub-object and forwards to `Class86668__Deinit`; the same caveat as Init applies to its specific purpose here.

## Track 4 (2026-09-26, round 88, Class865C8)

The class (table D_800865C8, id 0x1F230, Class86668's subclass) is unified as Class865C8 in include/Class865C8.h; the Obj865C8/Class865C8Methods views in class_39e08.h are gone. Prefix only. Accessors: unk38 -> dreamSys, unk0C->unk4 -> initArgs->unk4, unk10 -> IntermediateBase's unk10.
