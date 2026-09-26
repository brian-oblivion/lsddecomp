# Class865C8__Init — MATCHED (41/41 words)

> Renamed from `Obj865C8__Init` on 2026-09-26 (tools/rename.py). Address 0x80049a1c.

> Renamed from `func_80049A1C` on 2026-09-23 (tools/rename.py). Address 0x80049a1c.

`Obj865C8`'s vtable slot +0x044.

## Disassembly shape

```
addiu $sp, $sp, -0x20
sw    $s1, 0x14($sp)
addu  $s1, $a0, $zero        ; s1 = self
sw    $ra, 0x18($sp)
sw    $s0, 0x10($sp)
lw    $s0, 0x38($s1)         ; s0 = self->unk38
lw    $v0, 0xC($s1)          ; self->unk0C
lw    $v1, 0x0($s0)          ; s0->methods
lw    $a1, 0x4($v0)          ; (self->unk0C)->unk4
lw    $v0, 0x10($v1)         ; methods->slot10
jalr  $v0
 addu $a0, $s0, $zero        ; s0->methods->slot10(s0, self->unk0C->unk4)
lw    $v0, 0xC($s1)          ; self->unk0C (reload)
lw    $v1, 0x0($s0)          ; s0->methods (reload)
lw    $a1, 0x8($v0)          ; (self->unk0C)->unk8
lw    $v0, 0x10($v1)         ; methods->slot10 (same slot, 2nd call)
jalr  $v0
 addu $a0, $s0, $zero        ; s0->methods->slot10(s0, self->unk0C->unk8)
lw    $v0, 0xC($s1)          ; self->unk0C (reload)
lw    $v1, 0x0($s0)          ; s0->methods (reload)
lw    $a1, 0x10($v0)         ; (self->unk0C)->unk10
lw    $v0, 0x110($v1)        ; methods->slot110
jalr  $v0
 addu $a0, $s0, $zero        ; s0->methods->slot110(s0, self->unk0C->unk10)
jal   GetClass86668Methods
 nop
addu  $a0, $s1, $zero        ; self
lw    $a1, 0xC($a0)          ; self->unk0C (passed as the RAW pointer, not deref'd)
lw    $v0, 0x44($v0)         ; gClass86668Methods's own +0x044
jalr  $v0
 addu $a2, $zero, $zero      ; GetClass86668Methods()->slot44(self, self->unk0C, 0)
...
jr $ra
```

## Final C

```c
void Class865C8__Init(Obj865C8 *self) {
    SubObjD *sub = self->unk38;

    sub->methods->slot10(sub, self->unk0C->unk4);
    sub->methods->slot10(sub, (s32)self->unk0C->unk8);
    sub->methods->slot110(sub, (s32)self->unk0C->unk10);
    GetClass86668Methods()->slot44(self, (s32)self->unk0C, 0);
}
```

(The `(s32)` casts on `unk8`/`unk10` were added in round 2026-09-02's
`Class865C8__Finalize` pass, after that function proved both fields are really
`SubObjG *` — see "Correction" note below. This snippet reflects the
CURRENT committed source, not what compiled at the time this report was
first written.)

`GetClass86668Methods()->slot44` resolves to this unit's own `Class86668__Init`
(`gClass86668Methods`'s +0x044, already matched: zeroes `self->unk28`, forwards to
the base's own slot44, returns `self->unk28`) — its `s32` return is discarded
here.

## New struct knowledge (`include/class_39e08.h`)

Continues carving `Obj0C` (fields `unk8`, `unk10` added alongside the
already-known `unk4`) and `SubObjDMethods` (`slot10` added alongside the
already-known `slot14`/`slot110`), both established by `Class865C8__Deinit` in
this same round. `Class86668Methods::slot44` added, typed `s32 (*)(Obj865C8
*self, s32 arg1, s32 arg2)` from `Class86668__Init`'s own established
signature.

## Attempts

1 (matched on first attempt — straightforward once `unk0C`/`unk38`/
`SubObjD` were already carved by `Class865C8__Deinit` earlier this round).

### Proposed learning

None new beyond `Class865C8__Deinit`'s (same session, same fields) — this
function is corroborating evidence for that one's opaque-struct carve, not a
new lever.

## Correction (added by Class865C8__Finalize, same round, not a rewrite of the above)

At the time this report was written, `Obj0C::unk8`/`unk10` were typed `s32`
because THIS function's own call sites only ever forward them as opaque
register values through a vtable call that never dereferences them —
consistent with either a scalar or a pointer. `Class865C8__Finalize`
(`docs/match-reports/Class865C8__Finalize.md`) later dereferences both fields
directly (`->methods->slot4`) and settles it: they are `SubObjG *`. This
function's own derivation above is left as originally written (it was, and
remains, an accurate account of what THIS function's disassembly shows);
the field type in `include/class_39e08.h` and the two call-site casts above
have been updated to match the corrected type, and the build/funcdiff for
this function was reconfirmed at 41/41 after the change.

## Naming

`Class865C8__Init` -- tier B. Occupies +0x044 -- compared against `gIntermediateBaseMethods`'s own +0x044 slot, which forwards to `IntermediateBase__Init` (code_2cc8c.h), the established base-class Init slot at this exact offset. This override configures the `subD` sub-object and forwards to the sibling class's own +0x044 (`Class86668__Init`); what 'init' accomplishes for THIS class beyond that is not established.

## Track 4 (2026-09-26, round 88, Class865C8)

The class (table D_800865C8, id 0x1F230, Class86668's subclass) is unified as Class865C8 in include/Class865C8.h; the Obj865C8/Class865C8Methods views in class_39e08.h are gone. Prefix only. Now returns s32 (the parent init's result, `return GetClass86668Methods()->init(...)`), byte-identical: the slot is s32 and PollStatusObj switches on it. It takes self alone, unlike the slot's (self, args, mode); the slot keeps IntermediateBase's type and PollStatusObj calls through Class865C8InitFn.
