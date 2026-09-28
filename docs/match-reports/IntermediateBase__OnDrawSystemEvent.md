# IntermediateBase__OnDrawSystemEvent — MATCH (35/35 words)

> Renamed from `IntermediateBase__OnTag1Notify` on 2026-09-28 (tools/rename.py). Address 0x8003e418.

> Renamed from `Obj86B60__OnTag1Notify` on 2026-09-25 (tools/rename.py). Address 0x8003e418.

> Renamed from `func_8003E418` on 2026-09-19 (tools/rename.py). Address 0x8003e418.

**Unit:** task · **Size:** 35 instructions

## What it does

`gIntermediateBaseMethods+0x054` (and `gTitleMenuMethods`'s own verbatim-inherited `+0x054`):
`self->methods->slot54`'s occupant, and one of `IntermediateBase__OnNotify`'s own
3-way `arg1->target->header` dispatch targets. `arg1` is entirely unused in
the body -- only `arg2` is read. Gated on `arg2 == 2`: dispatches `slot44`
on `self->unk10` (reinterpreted as a pointer, same alternate reading
`IntermediateBase__Deinit` established), then fetches `self->unkC->unk4` (the
`Obj86B60InitArgs` field `IntermediateBase__Init`/`IntermediateBase__Deinit` only ever forward
opaquely) and dispatches its OWN `slot44` and `slot48` on it -- the first
function in this unit to dereference that field as a real class instance
rather than an opaque child pointer.

## The C

```c
void IntermediateBase__OnDrawSystemEvent(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    Unk4ArgObj *obj4;

    if (arg2 == 2) {
        ((Unk10Obj *)self->unk10)->methods->slot44((Unk10Obj *)self->unk10);
        obj4 = ((Obj86B60InitArgs *)self->unkC)->unk4;
        obj4->methods->slot44(obj4);
        obj4->methods->slot48(obj4);
    }
}
```

Matched on the first build.

## Struct/table knowledge established

- `Unk10ObjMethods`: added `slot44`.
- New type `Unk4ArgObj`/`Unk4ArgObjMethods` (slots `slot44`/`slot48`) --
  `Obj86B60InitArgs->unk4`'s real pointee type. Retyped that field from
  generic `void *` to `Unk4ArgObj *` (an implicit-conversion-to-`void*`
  call site in the already-matched `IntermediateBase__Init` is unaffected -- same
  register, same bytes, per this project's established "retyping a field
  to something more specific doesn't reopen an already-matched caller"
  precedent, confirmed unaffected by the unchanged whole-image SHA1).

### Proposed learning

`arg1` (the `EventArg *` `IntermediateBase__OnNotify` forwards to `slot54`/`58`/`5C`)
went completely unused here -- a live-but-unconsumed register at this call
site, same shape as `TaskCore__Update`'s own `$a1` note in
`include/task.h`. Declaring the parameter with its full established
type (`EventArg *`, matching the vtable field) rather than degrading it to
`s32`/`void *` costs nothing and keeps the signature consistent with its
two siblings (`slot58`, `slot5C`) for whoever reads this table's occupants
side by side.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic at all in
  this function, only dispatch.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit task. Matched on the
first build.

## Naming

**IntermediateBase__OnDrawSystemEvent** (renamed from `func_8003E418`, round 55, runner
alpha). Tier B: confirmed to be `Obj86B60Methods::onTag1Notify` (`+0x054`,
exclusive to this unit, renamed from `slot54`), the handler
`IntermediateBase__OnNotify` dispatches to when the incoming `EventArg`'s target
class tag is 1 -- the "Tag1" in the name records that dispatch condition,
which is directly observed in the caller, rather than a guessed purpose.
On event code 2 it finalizes `self->unk10` and the `initArgs->unk4` helper
object; what event code 2 represents in the game is not established
(tier B).

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/intermediate_base.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__OnTag1Notify (class prefix). Occupies +0x054, slot `onTag1Notify`: OnNotify's case for a sender of root class 1 (gDrawSystemMethods). Parameters are (sender, event). The calls on unk10 and initArgs->unk4 use one local view in task.c (IntermediateBaseLinked); their classes are not established.

## Track 4 (2026-09-26, round 88, delta: FrameClock)

The `self->unk10` call is FrameClock's +0x044 `tick` (include/frame_clock.h): the call now casts to `FrameClock *` and names the slot, instead of the `IntermediateBaseLinked` view's `slot44` (that view still covers initArgs->unk4). Byte-identical.

## Track 7 (round 98, echo)

The object at initArgs->pad is a Pad (Application__InitSystems passes the
Pad; include/intermediate_base.h names the field for it), so the two calls
are Pad's +0x044 `updateMasks` and +0x048 `dispatchEvents`
(include/pad.h), not the unit-local `slot44`/`slot48`. The event test is
draw_system.h's `DRAWSYSTEM_EVENT_VSYNC` (2): this is onNotify's DrawSystem
case, and DrawSystem__RunLoop sends 2 every VSync pass. Local `obj4` ->
`pad`. Byte-identical.

The unit-local view these calls went through was removed; it read, verbatim
(the only history in it is the "not established" claim, which pad.h and
draw_system.h have since settled):

```c
/* One local reading of the objects IntermediateBase calls outside
 * BasicClass's slots: initArgs->unk0 (+0x048 in onState2, +0x04C in
 * onState3), initArgs->unk4 (+0x044, +0x048 in onTag1Notify) and unk10
 * (+0x044 in onTag1Notify). Their classes are not established; every call
 * passes the object alone. */
typedef struct IntermediateBaseLinked IntermediateBaseLinked;

typedef struct IntermediateBaseLinkedMethods {
    u8 pad000[0x044];
    void (*slot44)(IntermediateBaseLinked *self); /* +0x044 */
    void (*slot48)(IntermediateBaseLinked *self); /* +0x048 */
    void (*slot4C)(IntermediateBaseLinked *self); /* +0x04C */
} IntermediateBaseLinkedMethods;

struct IntermediateBaseLinked {
    IntermediateBaseLinkedMethods *methods; /* +0x000 */
};
```
