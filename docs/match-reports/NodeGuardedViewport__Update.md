# NodeGuardedViewport__Update

> Renamed from `Class869D8__Update` on 2026-09-26 (tools/rename.py). Address 0x8004d300.

> Renamed from `NodeGuardedViewport__ForwardIfUnk10AndUnk70` on 2026-09-26 (tools/rename.py). Address 0x8004d300.

> Renamed from `func_8004D300` on 2026-09-22 (tools/rename.py). Address 0x8004d300.

**Unit:** class_3bb8c_c · **Size:** 23 words · **Status:** MATCHED (23/23)

## What it does

One of `NodeGuardedViewport`'s own methods. Gates a base-class hook call on two of
the object's own fields both being nonzero, then forwards to the base
ctor table's own `+0x09C` slot with `self` as the only argument.

## The C

```c
void NodeGuardedViewport__Update(NodeGuardedViewport *self)
{
    if (self->unk10 != 0 && self->unk70 != 0) {
        GetViewportMethods()->slot9C(self);
    }
}
```

## New/changed header content (`include/class_3bb8c.h`)

- `NodeGuardedViewport` struct: added `unk10` (s32, +0x010) and `unk70` (s32,
  +0x070), both gate fields read here. Padding added to keep the struct at
  its allocation size, 0xDC (from `New_NodeGuardedViewport`'s `BMemPMgrAlloc(0xDC)`
  call) -- purely additive, the existing `methods` field at +0x000 is
  untouched.
- `BaseCtorTable_3bb8c_c` (GetViewportMethods's return type): added `slot9C`
  (`void (*)(void *self)`, +0x09C). Additive; the existing `ctor` slot
  (+0x008, used by `NodeGuardedViewport__NodeGuardedViewport`) is untouched.

## Notes

Matched on the first attempt, no residue. The two-field nonzero gate
(`self->unk10 != 0 && self->unk70 != 0`) compiles straightforwardly to the
retail two-branch sequence (`beqz` on each field in turn, both skipping to
the same tail) with no reordering needed.

## Proposed learning

None beyond what's already documented for this unit's base-ctor-table
pattern.

## Naming

**NodeGuardedViewport__Update** -- tier B. Mechanics are fully
evident from the body (forward to the base ctor table's `slot9C` iff both
`unk10` and `unk70` are nonzero) but the fields' real meaning, and so the
forward's in-game purpose, is not established -- only that both gate the
call (see the struct comments). Named on the "NotifyIfUnkNActive"-style
precedent already used elsewhere in this codebase for a gated-forward shape
(e.g. `SceneNode__NotifyWithHull`, include/code_d294.h) rather than
inventing a semantic verb ("Notify"/"Release"/etc.) the body does not
support. `unk10`/`unk70` themselves are left unnamed -- no evidence beyond
"nonzero gate" exists for either.

## Track 4 (2026-09-25, round 85, bravo)

The forward is Viewport's +0x09C `update` (`GetViewportMethods()->update((Viewport *)self)`, include/Viewport.h, round 85). In Viewport's layout, `unk10` is `viewNode` and `unk70` is `otReady`; NodeGuardedViewport's own view keeps its names. Byte-identical.

## Track 4 (2026-09-26, round 87)

Renamed from `NodeGuardedViewport__ForwardIfUnk10AndUnk70` for its slot. It occupies
gNodeGuardedViewportMethods +0x09C, Viewport's `update` (gViewportMethods +0x09C holds
`Viewport__Update`), and forwards to exactly that occupant through
`GetViewportMethods()->update`. The two gate fields are Viewport's own:
+0x010 is `viewNode` (the class-4 child AddChild caches) and +0x070 is
`otReady` (InitOt sets it). Viewport__Update already returns early on
`otReady == 0` but then dereferences `viewNode->parent` unguarded, so what
the override adds is a NULL `viewNode` guard: the class updates only once a
view node is attached. That is the slot's job with a precondition, not more
than the slot name says.

## Track 6 (2026-09-26, round 92, echo)

The class was renamed `Class869D8` -> `NodeGuardedViewport` (`tools/renametype.py`, tier B): its one behavioural override, update, runs Viewport__Update only while a view node is attached, which is what lets ObjM leave it detached after ExitSceneStyle. The name says the mechanism, not the viewport's role in the game. (renametype also rewrote the historical token `Class869D8__ForwardIfUnk10AndUnk70` above.)

The update guard is the evidence for the class name. Viewport__Update also dereferences `refView.super`, which RemoveChild clears together with `viewNode`, so both unguarded dereferences are covered by the one `viewNode != NULL` test.
