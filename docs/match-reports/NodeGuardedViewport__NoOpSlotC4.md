# NodeGuardedViewport__NoOpSlotC4

> Renamed from `func_8004D374` on 2026-09-26 (tools/rename.py). Address 0x8004d374.

**Unit:** class_3bb8c_c · **Size:** 2 words · **Status:** MATCHED (2/2)

## What it does

Empty vtable stub: `gNodeGuardedViewportMethods`'s own slot +0x0C4. Splat-generated
body (`jr $ra; nop`) -- no logic, no arguments read, nothing to derive.

## The C

```c
void NodeGuardedViewport__NoOpSlotC4(void) {}
```

## Notes

Matched trivially; not itself round-68 work (already matched before this
naming pass touched the unit). Filed here only because track 3 asks for a
report per function, matched ones included.

## Naming

**NodeGuardedViewport__NoOpSlotC4** -- tier C, kept deliberately. This project's established
precedent for a genuinely empty, no-established-purpose vtable stub is to
keep the bare `func_` name even when the occupying class IS known
(`func_8004B324` in `src/class_3ac78.c`, `SceneNode__NoOpSlot5C` in
`src/code_d294.c` -- both documented "keeps its placeholder name
deliberately"). This function fits the same shape exactly: `void (void)`,
zero registers read, no caller in any carved unit dispatches it with
information this unit could use to infer purpose. A `NodeGuardedViewport__func_...`
form was considered and rejected -- it would assert a tie to this specific
class's own semantics that the body does not support (the function reads
nothing, including no `self`, so nothing here is actually class-specific
behavior).

## Track 6 (2026-09-26, round 92, echo)

The class was renamed `Class869D8` -> `NodeGuardedViewport` (`tools/renametype.py`, tier B): its one behavioural override, update, runs Viewport__Update only while a view node is attached, which is what lets ObjM leave it detached after ExitSceneStyle. The name says the mechanism, not the viewport's role in the game. (renametype also rewrote the historical token `Class869D8__ForwardIfUnk10AndUnk70` above.)

Renamed from its `func_` placeholder to `NodeGuardedViewport__NoOpSlotC4` (`tools/rename.py`, tier A: an empty leaf, its mechanics are its purpose). The "keep the bare func_ name" precedent cited above is superseded: `SceneNode__NoOpSlot5C`, `ObjM__NoOpSlotBC`, `Actor__NoOpSlotD8` and two dozen others now use the `Class__NoOpSlotXX` form for an empty occupant of a known class's own slot. Nothing calls the slot (+0x0C4).
