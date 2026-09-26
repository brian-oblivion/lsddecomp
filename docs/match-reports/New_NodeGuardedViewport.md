# New_NodeGuardedViewport

> Renamed from `New_Class869D8` on 2026-09-26 (tools/rename.py). Address 0x8004d254.

> Renamed from `func_8004D254` on 2026-09-22 (tools/rename.py). Address 0x8004d254.

**Unit:** class_3bb8c_c · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

`New_NodeGuardedViewport`: the allocator for `NodeGuardedViewport`. Standard
allocate-null-check-ctor shape, identical to class_3ac78.c's
`New_Class866E8`/class_39e08.c's several `New_X` functions -- allocate a
fixed-size block (`0xDC` bytes here), and on success run the class's ctor
(`NodeGuardedViewport__NodeGuardedViewport`, fetched through `GetNodeGuardedViewportMethods()->ctor`) and return the
new instance; return `NULL` on allocation failure.

## The C

```c
NodeGuardedViewport *New_NodeGuardedViewport(void)
{
    NodeGuardedViewport *self;

    self = BMemPMgrAlloc(0xDC);
    if (self != NULL) {
        GetNodeGuardedViewportMethods()->ctor(self);
        return self;
    }
    return NULL;
}
```

## Notes

Matched on the first attempt -- this is a direct copy of the established
New_X idiom already confirmed several times elsewhere in this codebase
(see docs/MATCHING-GUIDE.md, "Writing a class method" / "Allocation sites
look like..."). No residue.

## Naming

**New_NodeGuardedViewport** -- tier A. Pure `New_X` allocator idiom (allocate fixed
size, null-check, ctor, return); the allocator's mechanics ARE its purpose
by the tier-A leaf rule. Matches the project's established `New_Class866E8`/
`New_X` naming convention (class_3ac78.c, class_39e08.c) exactly, and this
name was already in use in this function's own report prose before the
round-68 rename made it real.

## Track 6 (2026-09-26, round 92, echo)

The class was renamed `Class869D8` -> `NodeGuardedViewport` (`tools/renametype.py`, tier B): its one behavioural override, update, runs Viewport__Update only while a view node is attached, which is what lets ObjM leave it detached after ExitSceneStyle. The name says the mechanism, not the viewport's role in the game. (renametype also rewrote the historical token `Class869D8__ForwardIfUnk10AndUnk70` above.)
