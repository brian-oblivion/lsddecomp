# ItemList__HandleInputCode -- MATCHED (69/69 words, first attempt)

> Renamed from `Class86F88__HandleInputCode` on 2026-09-26 (tools/rename.py). Address 0x800522dc.

> Renamed from `func_800522DC` on 2026-09-24 (tools/rename.py). Address 0x800522dc.

Unit `src/world/dream_scene.c`. Round 26, runner delta.

## What it is

An event/code dispatcher for this unit's own local view of method table
`gObjMMethods` (`Obj87034_3bb8c_k` / `Class87034Methods_3bb8c_k`, both
already declared earlier in this same file for `New_ObjM`/
`ObjM__ObjM`/`ItemList__SetState`/`ItemList__TickClosing`). Owns `jtbl_800116F4`, a
sparse 22-entry jump table for codes `4..25`, six of which have real
handlers and the rest fall through doing nothing:

```c
void ItemList__HandleInputCode(Obj87034_3bb8c_k *self, void *arg1, s32 code) {
    switch (code) {
    case 25:
        self->methods->slot60(self, 0x10);
        self->methods->slot54(self, 2);
        break;
    case 23:
        self->methods->slot60(self, 0x10);
        self->methods->slot54(self, 3);
        break;
    case 5:
        self->methods->slot7C(self);
        break;
    case 4:
        self->methods->slot80(self);
        break;
    case 18:
        self->methods->slot84(self);
        break;
    case 19:
        self->methods->slot88(self);
        break;
    }
}
```

`arg1` (the incoming second register argument) is never referenced in the
body -- matches this project's established event-dispatcher signature
shape (`self`, an unused/opaque second argument, an `s32` code), the same
one `dream_scene`'s `ObjM__OnDreamSysNotify`/`ObjM__DispatchPadEvent` use.

## HEAD BROADCAST 1 (source-declaration-order case layout) DOES apply here

Unlike the sibling `ObjM__OnDreamSysNotify` (`dream_scene`, same round), whose
jump-table entry order already coincided with ascending case-value order,
THIS function's case bodies are laid out in the file in the order
**25, 23, 5, 4, 18, 19** -- neither ascending nor descending by value, and
clearly the SOURCE's own declaration order rather than anything the
compiler would choose on its own. The `switch` above lists the cases in
that exact order and matched on the first attempt with no further
reshaping. Between this function and `ObjM__OnDreamSysNotify`, the two ends of the
broadcast's claim are both now directly confirmed in this project: some
dense switches need the reorder, some don't, and the only way to tell is
to read the jump table's own body layout off the `.s` before writing the
`switch`.

## Struct edit (unit-local, NOT the shared header)

`Class87034Methods_3bb8c_k` is declared directly in
`src/world/dream_scene.c` (not `include/class_3bb8c.h`) -- this unit's own
independent, disjoint-slot view of table `gObjMMethods`, per the header's
own standing note that `dream_scene` reaches a different, non-overlapping
slot set on the identical table. Added six new slots, all inside the
existing `pad044[0x090-0x044]` (0x4C = 76 bytes):

```
pad044[0x10] + slot54(4) + pad058[8] + slot60(4) + pad064[0x18]
  + slot7C(4) + slot80(4) + slot84(4) + slot88(4) + pad08C[4]
= 16+4+8+4+24+4+4+4+4+4 = 76  (matches the original span exactly)
```

`slot40` (already used by `ObjM__ObjM`) and `slot90`/`slotB0`/`slotB4`
(already used by `ObjM__OnNotify`) keep their original offsets --
confirmed by rebuilding (whole-image SHA1 green) after the struct edit,
before writing this function's body. No cross-unit prototype and no new
type went into either shared header (`class_3bb8c.h` or `class_39e08.h`);
this unit already includes both, per the coordinator's specific caution
for this unit, and neither was touched.

## Proposed learning

Two back-to-back same-round data points on HEAD BROADCAST 1
(`ObjM__OnDreamSysNotify`: table order == ascending value order, lever not needed;
`ItemList__HandleInputCode`: table order == source declaration order, lever
essential) make the discriminator concrete: **check the jump table's own
label order against sorted case-value order before writing the switch,
every time** -- neither "always reorder" nor "never reorder" is safe, and
the check costs nothing (the labels are right there in the `.s`).

## Naming

Round 75 (bravo, track 3). `func_800522DC` -> `ItemList__HandleInputCode`, **tier B**.

Slot +0x05C (`tools/classtable.py gItemListMethods`), which ItemList__OnNotify dispatches for notifications from its tag-2 child (the one ItemList__AddChild caches as `inputSource`). Code 25: forwardToTarget(0x10) then setState(2); 23: forwardToTarget(0x10) then setState(3); 5: scrollRight; 4: scrollLeft; 18: cursorUp; 19: cursorDown (each resolved to its method through the same table). TaskObjF (title_menu) also branches on 0x19/0x17. Tier B: that these codes are controller buttons is not established. Retyped round 75 from the unit's ObjM view to `ItemList *`: the function's own table is gItemListMethods and every slot it calls holds a ItemList method.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/world/dream_scene.c`).

## Round 99 (delta, track 7)

The codes are Pad events (include/pad.h: an edge base plus the button's index): 25 = `PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT` (circle), 23 = `+ PAD_BUTTON_RDOWN` (cross), 5 = `PAD_EVENT_HELD + PAD_BUTTON_LRIGHT`, 4 = `PAD_EVENT_HELD + PAD_BUTTON_LLEFT`, 18 = `PAD_EVENT_PRESSED + PAD_BUTTON_LUP`, 19 = `+ PAD_BUTTON_LDOWN`, spelled as TextEntry__HandleCommand spells them. Circle closes with `ITEMLIST_RESULT_CHOSEN`, cross with `ITEMLIST_RESULT_CANCELLED`; the tone 0x10 is `1 << 4` (VAB program 1, tone 0, include/vab_stream_obj.h), as TextEntry writes it. The case order keeps a `MATCHING:` line.
