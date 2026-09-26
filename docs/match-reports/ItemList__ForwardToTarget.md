# ItemList__ForwardToTarget -- MATCH

> Renamed from `Class86F88__ForwardToTarget` on 2026-09-26 (tools/rename.py). Address 0x800523f0.

> Renamed from `func_800523F0` on 2026-09-24 (tools/rename.py). Address 0x800523f0.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py ItemList__ForwardToTarget`: 16/16 words match.

## Source

```c
void ItemList__ForwardToTarget(ItemList *self, s32 arg1)
{
    ItemList *other = self->unk3C;

    if (other != NULL) {
        other->methods->slot80(other, arg1, 0x60, 0x60);
    }
}
```

## Notes

`self->unk3C` is another instance of this same class (`ItemList`) --
this function forwards to that OTHER instance's own `slot80` (which this
unit's own occupant, `ItemList__ScrollLeft`, does not itself read past `self`;
the 3-argument shape here comes from this call site, per the project's
established per-call-site-arity convention, not from the occupant's body).
`arg1` is `ItemList__ForwardToTarget`'s own second parameter, forwarded verbatim and
otherwise unused -- an ordinary "unused-locally, live-at-the-call"
parameter, not something read from `self`.

Matched on the first attempt.

## Naming

Round 75 (bravo, track 3). `func_800523F0` -> `ItemList__ForwardToTarget`, **tier B**.

Slot +0x060 (`tools/classtable.py gItemListMethods`). If `target` (+0x03C) is set, calls its +0x080 with (code, 0x60, 0x60). `target` is whatever ItemList__AttachTarget stores from its arg3, which TaskObjF__AttachTextEntry/AttachItemList pass as their `childC` (TaskObjF's `sound`, a VabStreamObj, round 89); TaskObjF__PlaySound calls the same slot with (code, 0x7F, 0x7F). Called with 0x10 before closing and with 0 on every cursor move/refresh. A sound cue with volumes would fit, but nothing proves it, so the name says only what the code does.

ItemList, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).

## Round 94 (track 6, charlie): the target is a VabStreamObj

`TargetObj86ED0`/`TargetMethods86ED0` (include/class_3bb8c.h) are deleted.
Both attachTarget callers (TaskObjF, src/class_3bb8c_g.c) pass TaskObjF's
`sound`, already typed `struct VabStreamObj *`, and the one slot the view
named, +0x080, is VabStreamObj's `playTone(self, index, vol, endVol)`
(include/VabStreamObj.h): the `(code, 0x60, 0x60)` call plays tone `code`
at volume 0x60. TextEntry::target and ItemList::target (+0x03C) and both
attachTarget prototypes are `struct VabStreamObj *`, the casts at the call
sites are gone, and `slot80` is `playTone`. Zero bytes changed.
