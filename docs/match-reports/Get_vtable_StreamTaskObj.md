# Get_vtable_StreamTaskObj

> Renamed from `func_8003BE84` on 2026-09-23 (tools/rename.py). Address 0x8003be84.

**Unit:** code_2c054 · **Size:** 4 instructions (0x10 bytes) · **Status:** MATCHED (4/4 words, whole-image SHA1 green), first attempt

## What it does

The class's own "GetMethods" accessor -- returns `&gStreamTaskObjMethods` and nothing
else, the same shape as `Get_vtable_Entity` in `include/Entity.h` and
`Get_vtable_Class65650` in `include/code_55dd4.h`. Called (still `INCLUDE_ASM`, not
this batch) by `New_StreamTaskObj` (the allocator) and `StreamTaskObj__StreamTaskObj` (the
constructor) to fetch the class's ctor at slot `+0x008` and to install the
methods pointer at object offset 0, respectively.

`gStreamTaskObjMethods` (`tools/classtable.py gStreamTaskObjMethods`, 77 slots) is this unit's
biggest single piece of context this round: five of its slots
(`+0x124`..`+0x134`) are the plain setters `StreamTaskObj__SetUnkC4`.."`7C`", and
several more (`+0x00C`, `+0x040`, `+0x044`, `+0x080`, `+0x084`) are other
functions matched in this same batch (`StreamTaskObj__Destroy`, `StreamTaskObj__Reset`,
`StreamTaskObj__Configure`, `StreamTaskObj__func_8003BD74`, `StreamTaskObj__func_8003BDAC`) -- see their own reports.

## Derivation

```
lui   $v0, %hi(gStreamTaskObjMethods)
addiu $v0, $v0, %lo(gStreamTaskObjMethods)
jr    $ra
 nop
```

```c
StreamTaskObjMethods *Get_vtable_StreamTaskObj(void) {
    return &gStreamTaskObjMethods;
}
```

Matches the `lui`/`addiu` idiom already confirmed elsewhere in this project:
"returns `&symbol`, not a value read from it" (`GetClass6D3C8Methods`,
`docs/DECOMPILATION_LEARNINGS.md`).

## New struct/header knowledge

Added `include/code_2c054.h`'s `StreamTaskObjMethods` (currently just the
header word -- no other slot of this table is dereferenced by anything in
this unit's queue) and `extern StreamTaskObjMethods gStreamTaskObjMethods;`.

## Proposed learning

`gStreamTaskObjMethods` sits right next to a second class's table, `gTaskCoreMethods`
(`Class6D3C8.h`'s `LoaderTaskMethods`, established from a completely
different allocator/unit, `New_TaskCore`). `StreamTaskObj`'s own slots
`+0x00C`/`+0x080`/`+0x084` (`StreamTaskObj__Destroy`/`StreamTaskObj__func_8003BD74`/`StreamTaskObj__func_8003BDAC`)
forward straight through to `gTaskCoreMethods`'s implementations of the *same*
offsets, passing `self` on unchanged -- a delegation pattern between two
independent sibling classes, not inheritance (confirmed because `gTaskCoreMethods`
has its own distinct overrides elsewhere, e.g. `+0x040`/`+0x044` are
different functions from `gStreamTaskObjMethods`'s). Worth knowing before assuming an
accessor-returned table is always the object's *own* class: sometimes a
class's own vtable slot body reaches for a *different* class's table via a
second accessor and calls straight through it.

## Naming

**Get_vtable_StreamTaskObj** -- tier A. The class's own "GetMethods"
accessor (returns `&gStreamTaskObjMethods`, no other side effect), matching
the established `Get_vtable_<Class>` convention exactly
(`Get_vtable_Entity`, `Get_vtable_TaskCore`, `Get_vtable_IntermediateBase`).
