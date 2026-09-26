# StageMap__Disable — MATCHED (16/16 words)

> Renamed from `Class866E8__Disable` on 2026-09-26 (tools/rename.py). Address 0x8004b57c.

> Renamed from `StageMap__func_8004B57C` on 2026-09-24 (tools/rename.py). Address 0x8004b57c.

> Renamed from `func_8004B57C` on 2026-09-24 (tools/rename.py). Address 0x8004b57c.

`Obj866E8`'s vtable slot +0x0F0.

## Disassembly

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
addu  $s0, $a0, $zero        ; s0 = self
sw    $ra, 0x14($sp)
lw    $v0, 0x0($s0)          ; self->methods
nop
lw    $v0, 0xC0($v0)         ; methods->slotC0
jalr  $v0
 nop                          ; self->methods->slotC0(self)
sw    $zero, 0x70($s0)       ; self->unk70 = 0
...
jr $ra
```

## Final C

```c
void StageMap__Disable(Obj866E8 *self) {
    self->methods->slotC0(self);
    self->unk70 = 0;
}
```

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8Methods::slotC0` typed `void (*)(Obj866E8 *self)`, return
  discarded.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B57C` | `StageMap__Disable` | A | Occupant of `gStageMapMethods` +0x0F0. Body: `self->methods->slotC0(self); self->enabled = 0;` -- dispatches a teardown slot, then clears the same field `StageMap__Enable` sets. See `StageMap__Enable.md` for the cross-unit confirmation of the `enabled` field name. |
