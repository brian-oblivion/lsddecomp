# NodeGuardedViewport__InitDefaults

> Renamed from `Class869D8__InitDefaults` on 2026-09-26 (tools/rename.py). Address 0x8004d2f8.

> Renamed from `func_8004D2F8` on 2026-09-26 (tools/rename.py). Address 0x8004d2f8.

**Unit:** class_3bb8c_c · **Size:** 2 words · **Status:** MATCHED (2/2)

## What it does

Empty vtable stub: `gNodeGuardedViewportMethods`'s own slot +0x040. Splat-generated
body (`jr $ra; nop`) -- no logic, no arguments read, nothing to derive.

## The C

```c
void NodeGuardedViewport__InitDefaults(void) {}
```

## Notes

Matched trivially; not itself round-68 work (already matched before this
naming pass touched the unit). Filed here only because track 3 asks for a
report per function, matched ones included.

## Naming

**NodeGuardedViewport__InitDefaults** -- tier C, kept deliberately. This project's established
precedent for a genuinely empty, no-established-purpose vtable stub is to
keep the bare `func_` name even when the occupying class IS known
(`StageMap__NoOpSlotD8` in `src/class_3ac78.c`, `SceneNode__NoOpSlot5C` in
`src/SceneNode.c` -- both documented "keeps its placeholder name
deliberately"). This function fits the same shape exactly: `void (void)`,
zero registers read, no caller in any carved unit dispatches it with
information this unit could use to infer purpose. A `NodeGuardedViewport__func_...`
form was considered and rejected -- it would assert a tie to this specific
class's own semantics that the body does not support (the function reads
nothing, including no `self`, so nothing here is actually class-specific
behavior).

## Track 4 (2026-09-26, round 87)

Renamed from `func_8004D2F8` for its slot. It occupies gNodeGuardedViewportMethods
+0x040, which is Viewport's `initDefaults` (gViewportMethods +0x040 holds
`Viewport__InitDefaults`; `classtable.py gNodeGuardedViewportMethods --vs
gViewportMethods` lists it as an override). The body is empty, so the
Viewport ctor's closing `self->methods->initDefaults(self)` does nothing for
a NodeGuardedViewport: none of Viewport's defaults are written by the class itself.
It is typed `void (*)(NodeGuardedViewport *)` in the table (the slot's type); the
definition keeps its `(void)` parameter list because an empty body reads no
argument and the bytes do not show one.

## Track 6 (2026-09-26, round 92, echo)

The class was renamed `Class869D8` -> `NodeGuardedViewport` (`tools/renametype.py`, tier B): its one behavioural override, update, runs Viewport__Update only while a view node is attached, which is what lets ObjM leave it detached after ExitSceneStyle. The name says the mechanism, not the viewport's role in the game. (renametype also rewrote the historical token `Class869D8__ForwardIfUnk10AndUnk70` above.)

**Correction to the round-87 note above.** The Viewport ctor
(`Viewport__Viewport`) installs `GetViewportMethods()` before its closing
`self->methods->initDefaults(self)`, so that call dispatches
`Viewport__InitDefaults` and every Viewport default IS written. This class's
ctor then installs its own table and calls initDefaults a second time; the
empty override makes that second call a no-op. It does not suppress the
defaults. Name kept (tier A for the slot: it is the class's initDefaults,
and the body is empty).
