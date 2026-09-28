# Actor__DispatchLinkCommand -- MATCHED (25/25 words)

> Renamed from `DispatchObjO__func_57320` on 2026-09-25 (tools/rename.py). Address 0x80057320.

> Renamed from `func_80057320` on 2026-09-18 (tools/rename.py). Address 0x80057320.

Unit: `ObjMStyleActor` (round 17). A tag-gated double dispatch: reads
`arg1`'s own vtable header LOW BYTE and calls one of two of `self`'s own
vtable slots depending on its value.

## Final source

```c
typedef struct DispatchObjO DispatchObjO;
typedef struct DispatchObjOMethods {
    u8 pad0[0xDC];                        /* +0x000 .. +0x0DB, unknown */
    void (*slotDC)(DispatchObjO *self);      /* +0x0DC */
    void (*slotE0)(DispatchObjO *self);        /* +0x0E0 */
} DispatchObjOMethods;
struct DispatchObjO {
    DispatchObjOMethods *methods;
};

void Actor__DispatchLinkCommand(DispatchObjO *self, TagByteObjO *arg1) {
    if (arg1->methods->tag == 0x34) {
        self->methods->slotDC(self);
    } else if (arg1->methods->tag == 0x24) {
        self->methods->slotE0(self);
    }
}
```

## Derivation

`lbu $v1,0($v0)` where `$v0 = *$a1` (i.e. `arg1->methods`) reads a BYTE,
not a masked word -- a DIFFERENT tag check than `Actor__AddChild`'s/
`Actor__RemoveChild`'s (which read the full header word and mask it). Kept as a
separate type, `TagByteObjO`, matching this. Both tag values (`0x34`,
`0x24`) are compile-time constants set up before the first comparison
(`ori $v0,$zero,0x24` sits in the FIRST branch's delay slot, ahead of where
it's actually used in the SECOND comparison -- ordinary delay-slot filler,
required no special handling once the `if`/`else if` was written in
retail's own textual order).

`self`'s own vtable dispatch (`self->methods->slotDC`/`slotE0`) is called
with NO explicit argument setup beyond `self` itself -- the call forwards
whatever was already in `$a0` (this function's own first parameter),
unchanged.

This function's OWN `self` type (`DispatchObjO`) is kept independent of
`BaseObjO` -- nothing else in this unit calls `Actor__DispatchLinkCommand`, and while
its "self" MIGHT be the same shared base class (the offsets `0xDC`/`0xE0`
are plausible padding gaps in `BaseObjOMethods` too), nothing here confirms
that relationship, so it is not asserted. `TagByteObjO` (the tag argument)
IS shared with `Actor__NotifyMove`'s `self->unk28` field, since both are
independently confirmed to compare the same byte against the same `0x34`
constant.

### Proposed learning

None -- see `Actor__NotifyMove`'s report for the shared `TagByteObjO` tag
convention.

## Naming

**`Actor__DispatchLinkCommand` -- tier C.** Mechanics are fully known (reads
`arg1`'s vtable tag byte, dispatches to one of `self`'s own two vtable
slots depending on whether it is `0x34` or `0x24`), but the report's own
class-identification section explicitly declines to assert a relationship
between `DispatchObjO` (this function's `self`) and `BaseObjO`, since
nothing in this unit calls `func_80057320` to test that relationship.
Kept `DispatchObjO` (this unit's own type for this unresolved class) as
the prefix rather than guessing `BaseObjO`.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `DispatchObjO__func_57320`. Override of +0x09C (SceneNode's dispatchLinkCommand), named for its slot: routes by the sender's class byte, 0x34 (an Actor) to onActorLinkCommand (+0x0DC), 0x24 (GridCell) to onGridCellLinkCommand (+0x0E0). The C now passes (self, sender, event) to both explicitly; the old view passed self alone and relied on $a1/$a2 being untouched, which they are (same bytes). The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of TodActor/Entity, DreamSys and StyleEffect. Any source block above is the pre-unification spelling; the live body in `src/ObjMStyleActor.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, alpha)

Name unchanged. The class-byte tests read `(u8)sender->methods->header` against ACTOR_CLASS_ID and GRIDCELL_CLASS_ID (added to include/GridCell.h: 0x24, two nibbles), the same lbu as the old `*(u8 *)sender->methods`. The comment that said both targets were called with self alone was stale since track 4 (the C passes sender and event); it now says what the routing is.
