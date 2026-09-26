# ObjM__OnDreamSysNotify -- MATCHED (82/82 words, first attempt)

> Renamed from `ObjM__HandleStateCode` on 2026-09-26 (tools/rename.py). Address 0x80053984.

> Renamed from `func_80053984` on 2026-09-24 (tools/rename.py). Address 0x80053984.

Unit `src/class_3bb8c_l.c`. Round 26, runner delta.

## What it is

An event/code dispatcher for `Obj87034_3bb8c_l` (method table `gObjMMethods`,
53 slots). Owns `jtbl_8001174C`, a dense 8-entry jump table for codes
`0xA..0x11`, dispatched through eight of the class's OWN method-table slots
(`self->methods->slotXX`):

```c
void ObjM__OnDreamSysNotify(Obj87034_3bb8c_l *self, void *arg1, s32 code) {
    if (self->unk20 == 0) {
        switch (code - 0xA) {
        case 0:
            self->methods->slot94(self);
            break;
        case 1:
            break;
        case 2:
            self->methods->slot98(self);
            break;
        case 3:
            self->methods->slot9C(self);
            break;
        case 4:
            self->methods->slotA0(self);
            break;
        case 5:
            self->methods->slotA4(self);
            break;
        case 6:
            self->methods->slotA8(self);
            break;
        case 7:
            self->methods->slotAC(self);
            break;
        }
    } else if (code >= 9) {
        self->unk3C->unk44 = 0;
    }
}
```

`self->unk20` (existing field, a phase/state tag already written by three
other functions in this unit) gates the whole switch: while it's zero, an
in-range `code` (0xA..0x11) dispatches through the jump table; codes 1
(0xB) and out-of-range fall through doing nothing. Once `unk20` is
nonzero, the ONLY thing this function does is clear `self->unk3C->unk44`
(a `DreamSysObj_3bb8c_l` field, new — see below), and only for `code >= 9`.

## Why it needed no reordering, unlike the head's broadcast

HEAD BROADCAST 1 (GCC 2.6.3 builds the compare tree from sorted case
values but lays out case bodies in source declaration order) was flagged
as possibly finally applicable here since every other runner had returned
a negative on it. It doesn't apply to THIS function: the jump table's own
entry order (`.L800539C8, .L80053ABC(empty), .L800539E8, .L80053A08,
.L80053A28, .L80053A48, .L80053A68, .L80053A88`) already matches ASCENDING
case-value order (0,1,2,3,4,5,6,7) with no reordering — retail's case
BODIES happen to be laid out in the same order as the SORTED compare tree
for this particular switch, so a plain ascending `switch` matched on the
first attempt with no source-order tricks needed. The broadcast's
mechanism is real (confirmed and used successfully elsewhere this round
per the coordinator), it just isn't triggered by every dense switch --
only ones where retail's actual case-value order and desired body-layout
order diverge.

## Struct discoveries (header edits, all additive)

`include/class_3bb8c.h` is shared by 11 units; every edit below is a pad
split or trailing append that preserves total struct size and every
already-used field's offset (verified by rebuilding after each edit
before writing any `.c` code, and the whole-image SHA1 stayed clean at
every step).

- `Obj87034Methods_3bb8c_l` (self's own vtable): added `slot94`, `slot98`,
  and `slotA0`/`slotA4`/`slotA8`/`slotAC` (all `void (*)(Obj87034_3bb8c_l
  *self)`), splitting `pad90[0x09C-0x090]` and `padA0[0x0C0-0x0A0]`.
  `slot9C` already existed (added by a PRIOR unit's function,
  `ObjM__EnterState5`) -- extended its comment to note this function ALSO
  calls it, rather than declaring a duplicate member.
- `DreamSysObj_3bb8c_l` (self->unk3C's pointee): added `s32 unk44` at
  +0x044, splitting `pad04[0x164-0x004]` into `pad04[0x040]` + `unk44` +
  `pad48[0x11C]` (64+4+284=352, matching the original 0x160). Verified no
  OTHER unit references `DreamSysObj_3bb8c_l` by name
  (`grep -rln DreamSysObj_3bb8c_l src/*.c` -> only this unit) before
  touching it.

## Notes for the sibling function in this unit

`ObjM__InitStyleAndWorld` (matched the same round, see its own report) needed THREE
more slots on `Obj87034Methods_3bb8c_l` (`slot5C`), `DreamSysMethods_3bb8c_l`
(`slot70`, `slotEC`, `slot1A0`), and `Obj14Methods_3bb8c_l` (`slot134`), plus
four more fields on `Obj87034_3bb8c_l` (`unk48`, `unk4C`, `unk6C`, `unk78`).
Those edits are described in that report rather than duplicated here, since
they were made together in the same session against the same shared header.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053984` | `ObjM__OnDreamSysNotify` | B | see below |

**Evidence.** vtable slot +0x090. The class's own state-transition dispatcher: gated on `self->phase == 0`, switches on `code - 0xA` (`jtbl_8001174C`, codes 0xA..0x11) and routes each case onto the SAME class's own `slot94`..`slotAC` -- i.e. `ObjM__EnterState4`, `ObjM__EnterState5`, `ObjM__EnterState6` (this unit) and `ObjM__EnterState7`, `ObjM__EnterState8`, `ObjM__EnterStateA`, `ObjM__NotifyParentsCodeB` (sibling unit class_3bb8c_m) one-to-one, confirmed directly off `tools/classtable.py 0x80087034`'s slot list. When `phase != 0` and `code >= 9` it instead clears `self->target->unk44`.
