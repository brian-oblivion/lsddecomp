# Get_vtable_StreamTask

> Renamed from `Get_vtable_StreamTaskObj` on 2026-09-26 (tools/rename.py). Address 0x8003be84.

> Renamed from `func_8003BE84` on 2026-09-23 (tools/rename.py). Address 0x8003be84.

**Unit:** code_2c054 · **Size:** 4 instructions (0x10 bytes) · **Status:** MATCHED (4/4 words, whole-image SHA1 green), first attempt

## What it does

The class's own "GetMethods" accessor -- returns `&gStreamTaskMethods` and nothing
else, the same shape as `Get_vtable_Entity` in `include/Entity.h` and
`GetTodActorMethods` in `include/code_55dd4.h`. Called (still `INCLUDE_ASM`, not
this batch) by `New_StreamTask` (the allocator) and `StreamTask__StreamTask` (the
constructor) to fetch the class's ctor at slot `+0x008` and to install the
methods pointer at object offset 0, respectively.

`gStreamTaskMethods` (`tools/classtable.py gStreamTaskMethods`, 77 slots) is this unit's
biggest single piece of context this round: five of its slots
(`+0x124`..`+0x134`) are the plain setters `StreamTask__SetUnkC4`.."`7C`", and
several more (`+0x00C`, `+0x040`, `+0x044`, `+0x080`, `+0x084`) are other
functions matched in this same batch (`StreamTask__Finalize`, `StreamTask__Reset`,
`StreamTask__Init`, `StreamTask__OnPadPrev`, `StreamTask__OnPadNext`) -- see their own reports.

## Derivation

```
lui   $v0, %hi(gStreamTaskMethods)
addiu $v0, $v0, %lo(gStreamTaskMethods)
jr    $ra
 nop
```

```c
StreamTaskObjMethods *Get_vtable_StreamTask(void) {
    return &gStreamTaskMethods;
}
```

Matches the `lui`/`addiu` idiom already confirmed elsewhere in this project:
"returns `&symbol`, not a value read from it" (`GetGameApplicationMethods`,
`docs/DECOMPILATION_LEARNINGS.md`).

## New struct/header knowledge

Added `include/code_2c054.h`'s `StreamTaskObjMethods` (currently just the
header word -- no other slot of this table is dereferenced by anything in
this unit's queue) and `extern StreamTaskObjMethods gStreamTaskMethods;`.

## Proposed learning

`gStreamTaskMethods` sits right next to a second class's table, `gTaskCoreMethods`
(`GameApplication.h`'s `LoaderTaskMethods`, established from a completely
different allocator/unit, `New_TaskCore`). `StreamTaskObj`'s own slots
`+0x00C`/`+0x080`/`+0x084` (`StreamTask__Finalize`/`StreamTask__OnPadPrev`/`StreamTask__OnPadNext`)
forward straight through to `gTaskCoreMethods`'s implementations of the *same*
offsets, passing `self` on unchanged -- a delegation pattern between two
independent sibling classes, not inheritance (confirmed because `gTaskCoreMethods`
has its own distinct overrides elsewhere, e.g. `+0x040`/`+0x044` are
different functions from `gStreamTaskMethods`'s). Worth knowing before assuming an
accessor-returned table is always the object's *own* class: sometimes a
class's own vtable slot body reaches for a *different* class's table via a
second accessor and calls straight through it.

## Naming

**Get_vtable_StreamTask** -- tier A. The class's own "GetMethods"
accessor (returns `&gStreamTaskMethods`, no other side effect), matching
the established `Get_vtable_<Class>` convention exactly
(`Get_vtable_Entity`, `Get_vtable_TaskCore`, `Get_vtable_IntermediateBase`).

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Returns &gStreamTaskMethods.
