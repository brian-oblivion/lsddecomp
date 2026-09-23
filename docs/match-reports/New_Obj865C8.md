# New_Obj865C8

> Renamed from `func_80049608` on 2026-09-23 (tools/rename.py). Address 0x80049608.

**Unit:** class_39e08 · **Status:** MATCHED (31/31 words)

A `New_X` class allocator: allocate 0x50 bytes, and if that succeeds dispatch
the class's constructor slot (`+0x008`) with the caller's arguments forwarded
unchanged. Returns the new object, or NULL.

## The match

```c
Obj865C8 *New_Obj865C8(Obj0C *arg1, SubObjD *arg2, s32 arg3)
{
    ...

    self = func_80017B34(0x50);
    if (self != NULL) {
        GetObj865C8Methods()->ctor(self, arg1, arg2, arg3);
        return self;
    }
    return NULL;
}
```

See `src/class_39e08.c` for the exact text.

## Why it matched: `return NULL;` goes LAST

This is one of five instances of the `New_X` epilogue-merge residue class
closed in a single pass. **The full account, the measurement table for the
four source shapes that do NOT work, and the sub-shape that this rule does
NOT close, are all in `docs/match-reports/New_Class866E8.md`** -- read that
one rather than re-deriving from here.

The one-line version: retail materializes the return value twice, from two
different operands, which means two `return` statements in the source. Put the
`return NULL;` after the success return and GCC 2.6.3 produces retail's
epilogue merge. Put it first and it costs an extra `j`.

## History

Previously filed as **NOT ATTEMPTED** -- a predicted instance of the class,
deliberately reported so a cold runner would not be staffed onto it. That was
the right call at the time; the prediction was correct and the disposition is
now obsolete.

Typing this one needed two small header changes, both recorded in
`include/class_39e08.h`: `Class865C8Methods::ctor` was `void *` and is now a
typed pointer carrying Obj865C8__Obj865C8's own signature, and `GetObj865C8Methods`
gained a prototype because it is defined later in the unit (ROM order) than
the function that dispatches through it.

## Provenance

Matched by the head during divergence resolution, 2026-09-02, as part of
closing the epilogue-merge class. `docs/research/epilogue-merge-residue.md` is
updated with what survived and what did not.

## Naming

`New_Obj865C8` -- tier A. Allocator, matches the established `New_X` idiom used everywhere else in this project (allocate fixed size, ctor via the class's own vtable accessor, return NULL on failure): mechanics are its purpose.
