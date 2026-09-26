# Viewport__Viewport — MATCH (41/41 words)

> Renamed from `Unk18Obj__Unk18Obj` on 2026-09-25 (tools/rename.py). Address 0x8003e628.

> Renamed from `func_8003E628` on 2026-09-19 (tools/rename.py). Address 0x8003e628.

**Unit:** code_2cc8c_c · **Size:** 41 instructions

## What it does

`gViewportMethods+0x008` -- the ctor of the class `New_Viewport`'s New_X
allocator constructs (`Unk18Obj`, 0xBC bytes). Runs the BasicClass ctor,
installs its own vtable, zeroes two fields (`unkC`/`unk10`), stashes the
return of `New_SceneNode` (a `New_SceneNode` allocator, already matched
elsewhere as `code_d294.c`) into `unkAC`, constructs a `SubHandleObj` via
`New_Class6E99C` (already known elsewhere as `include/Entity.h`'s own
`Unk100Obj`/`New_Class6E99C`) into `unkB0`, dispatches that object's own
`slot4C` with `(obj, self->unkAC, &D_8008A904)`, then runs its own freshly
installed `slot40`.

## The C

```c
void Viewport__Viewport(Unk18Obj *self)
{
    SubHandleObj *obj;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetViewportMethods();
    self->unkC = 0;
    self->unk10 = 0;
    self->unkAC = New_SceneNode();
    obj = New_Class6E99C(D_8008A90C, 0, 0);
    self->unkB0 = obj;
    obj->methods->slot4C(obj, self->unkAC, D_8008A904);
    self->methods->slot40(self);
}
```

Matched on the first build.

## A delay-slot trap worth flagging for the next reader

First read-through of this function's disassembly misattributed
`sw $v0, 0xAC($s0)` (the delay slot of `jal New_Class6E99C`) as storing
`New_Class6E99C`'s OWN return value -- it does not. A `jal`'s delay slot
executes BEFORE the callee runs, using whatever `$v0` held from the
PRECEDING call (`New_SceneNode`'s return), not the value about to come
back. Re-reading confirmed: `self->unkAC = New_SceneNode()` (stored in the
delay slot of the NEXT call), and `self->unkB0 = New_Class6E99C(...)`'s
actual return (read from `$v0` only AFTER that call, in `addu $a0,$v0,$zero`
one instruction later). Getting this backwards would have produced a
plausible-looking but wrong struct/field derivation with no diagnostic --
worth the explicit callout since nothing in the build would have caught it
before the byte-level score did.

## Struct/table knowledge established

- `Unk18Obj`: added `unkC`, `unk10` (both `s32`, zeroed by the ctor),
  `unkAC` (`void *`, from `New_SceneNode`, never dereferenced by this unit),
  `unkB0` (`SubHandleObj *`, from `New_Class6E99C`).
- `Unk18ObjMethods`: added `slot40` -- a DIFFERENT function from
  `Obj86B60Methods::slot40` despite the identical offset; `gViewportMethods`'s own
  `+0x040` occupant is `Viewport__InitDefaults` (per `tools/classtable.py
  gViewportMethods`), not `IntermediateBase__ResetCounters`. Two unrelated tables, same offset,
  different occupants -- ordinary vtable-layout coincidence, not evidence
  of a shared ancestor at this slot (contrast with the GENUINELY shared
  slots this unit has documented elsewhere, e.g. `IntermediateBase__OnState2`).
- New type `SubHandleObj`/`SubHandleObjMethods` (`slot4C`) -- this unit's own
  local view of `include/Entity.h`'s `Unk100Obj`/`New_Class6E99C`, per this
  project's independent-local-views convention.
- `New_SceneNode`: local view added, returning `void *` (this unit never
  dereferences it) -- `include/code_d294.h`'s own view types it
  `SceneNodeObj *`, unaffected since it's a separate header.
- `D_8008A90C`/`D_8008A904`: two new address-taken-only globals.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build (after correctly re-reading the delay-slot ordering above).

## Naming

**Unk18Obj__Unk18Obj** (renamed from `func_8003E628`, round 55, runner
alpha). Tier A: the `Class__Class` constructor convention (matching
`BasicClass__BasicClass`) -- confirmed as `Unk18ObjMethods::ctor`'s
occupant (called by `New_Viewport` via `GetViewportMethods()->ctor(self)`,
`GetViewportMethods` being this class's own vtable getter, code_2cc8c_d.c).
Chains `Get_vtable_BasicClass()->ctor` first, then installs its own vtable
and sets up `self->unkAC`/`self->unkB0` -- the standard base-then-derived
construction shape.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__Unk18Obj`. The +0x008 ctor. Chains to BasicClass's ctor first (Get_vtable_BasicClass()->ctor); Class869D8__Class869D8 chains to this one, so the id tree 0x0 -> 0x7 -> 0x17 is the ctor chain. Fields: +0x00C drawSystem, +0x010 viewNode, +0x0AC sceneRoot (New_SceneNode), +0x0B0 subHandle (New_Class6E99C, attached under sceneRoot through SceneNode's attachToParent slot with D_8008A904 cast to LongVec3 *, because the occupant, BoxFill__AttachToParent, takes a screen position), then the +0x040 initDefaults slot. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
