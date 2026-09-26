# NodeGuardedViewport__NodeGuardedViewport

> Renamed from `Class869D8__Class869D8` on 2026-09-26 (tools/rename.py). Address 0x8004d2a4.

> Renamed from `func_8004D2A4` on 2026-09-22 (tools/rename.py). Address 0x8004d2a4.

**Unit:** class_3bb8c_c · **Size:** 21 words · **Status:** MATCHED (21/21)

## What it does

The constructor (`ctor`, slot +0x008) for `NodeGuardedViewport`. Standard
class-framework shape, same as e.g. class_39e08.c's `TimedTask__TimedTask`: chain
to a base class's ctor (fetched via `GetViewportMethods()`), install this
class's own vtable, then call the just-installed vtable's own
post-construct hook (slot +0x040, currently `NodeGuardedViewport__InitDefaults` -- already
matched, an empty body).

## The C

```c
void NodeGuardedViewport__NodeGuardedViewport(NodeGuardedViewport *self)
{
    GetViewportMethods()->ctor(self);
    self->methods = GetNodeGuardedViewportMethods();
    self->methods->slot40(self);
}
```

## Notes

`GetViewportMethods` is a base-ctor-table getter this unit has no other
evidence about beyond this one call site: the retail instruction sequence
sets up only `self` for the ctor call (no second argument register is
touched), so it's declared minimally as
`extern BaseCtorTable_3bb8c_c *GetViewportMethods(void);` with
`BaseCtorTable_3bb8c_c` holding only a `ctor(void *self)` field at +0x008
(see `include/class_3bb8c.h`). This is a fresh, file-local extern
declaration -- it does not need to agree with any other unit's own
(possibly differently-typed) declaration of the same external symbol,
since each translation unit's local prototype only has to reproduce ITS
OWN call site's register usage.

Built and verified byte-for-byte as part of a 9-function batch across this
whole unit; `./build-and-verify.sh` reports a clean whole-image SHA1 match.

## Naming

**NodeGuardedViewport__NodeGuardedViewport** -- tier A. Canonical ctor (`Class__Class`
convention, e.g. `Class866E8__Class866E8`): chains the base ctor, installs
this class's own vtable via `GetNodeGuardedViewportMethods()`, then dispatches the
freshly-installed table's own post-construct hook. The identity of the
class and the fact that this occupies its own `ctor` slot (+0x008) are both
evident from the body and from the vtable dump (`tools/classtable.py
0x800869D8`).

## Track 4 (2026-09-25, round 85, bravo)

NodeGuardedViewport's parent is Viewport (id 0x7, include/Viewport.h, round 85): the base ctor call is `GetViewportMethods()->ctor((Viewport *)self)`, replacing class_3bb8c.h's BaseCtorTable_3bb8c_c view. This is the ctor chain that makes 0x17 Viewport's subclass. NodeGuardedViewport's own views are unchanged. Byte-identical.

## Track 6 (2026-09-26, round 92, echo)

The class was renamed `Class869D8` -> `NodeGuardedViewport` (`tools/renametype.py`, tier B): its one behavioural override, update, runs Viewport__Update only while a view node is attached, which is what lets ObjM leave it detached after ExitSceneStyle. The name says the mechanism, not the viewport's role in the game. (renametype also rewrote the historical token `Class869D8__ForwardIfUnk10AndUnk70` above.)

The banner history that moved out of include/NodeGuardedViewport.h: IntermediateBaseInitArgs +0x010, where Class865C8__Class865C8 stores this object, was once viewed by class_39e08.h as `Obj0C::unk10`; the id tree 0x7 -> 0x17 was confirmed as the ctor chain in round 87 (track 4).
