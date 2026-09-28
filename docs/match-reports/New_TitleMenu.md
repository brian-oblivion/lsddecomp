# New_TitleMenu

> Renamed from `New_Class86B60` on 2026-09-26 (tools/rename.py). Address 0x8004d518.

> Renamed from `func_8004D518` on 2026-09-22 (tools/rename.py). Address 0x8004d518.

**Unit:** class_3bb8c_c · **Size:** 24 words · **Status:** MATCHED (24/24)

## What it does

`New_TitleMenu`: the allocator for a third small sibling class (alongside
`NodeGuardedViewport`/`New_NodeGuardedViewport` and `GridCell`/`New_GridCell`). Allocates
0xC4 bytes and, on success, calls the ctor (`TitleMenu__TitleMenu`, occupying this
class's own vtable slot +0x008) with the allocated object and the caller's
own `dreamSys` argument.

Also externally visible as a `PollTaskCtor` callback -- `include/GameApplication.h`
(a different unit) already declares this exact symbol,
`extern PollTask *New_TitleMenu(void *dreamSys);`, used by `GameApplication__RunTitleMenu`
as `GameApplication__RunTask`'s `ctor` argument. That declaration's return/param
naming is kept as-is there (independent local view); this unit's own
`dreamSys` parameter name/type was chosen to match it.

## The C

```c
TitleMenu *New_TitleMenu(void *dreamSys)
{
    TitleMenu *self;

    self = BMemPMgrAlloc(0xC4);
    if (self != NULL) {
        GetTitleMenuMethods()->ctor(self, dreamSys);
        return self;
    }
    return NULL;
}
```

## New header content (`include/class_3bb8c.h`)

All additive/new -- see `TitleMenu__TitleMenu`'s report (the ctor, matched
alongside this function in the same round) for the full new-type list;
both functions were derived and typed together.

## Notes

Matched on the first attempt, same shape as `New_NodeGuardedViewport`/`New_GridCell`
(this unit's other two `New_X` allocators) plus one extra forwarded
argument.

## Proposed learning

None beyond what's already documented for this unit's `New_X` idiom.

## Naming

**New_TitleMenu** -- tier A. Same `New_X` allocator idiom as
`New_NodeGuardedViewport`/`New_GridCell`, one extra forwarded argument
(`dreamSys`). Also externally used as a `PollTaskCtor` callback
(`include/GameApplication.h`); that unit's own independent local view keeps its
own return/param naming and is untouched by this rename (function names
are unique symbols, tree-wide by construction, so that call site now reads
`New_TitleMenu` too).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). Its parameter is now `struct DreamSys *dreamSys` (the ctor's; GameApplication__RunTitleMenu passes self->dreamSys). GameApplicationFileResource's local `PollTask *` extern is gone; it casts to PollTaskCtor as for New_GraphRoom. Byte-identical (whole image green, 0 new warnings, nonmatching green).

## Constants (round 100, track 7)

`BMemPMgrAlloc(0xC4)` is `BMemPMgrAlloc(sizeof(TitleMenu))`: TitleMenu's last
field is `saveBlockSize` at +0x0C0, and the image is byte-identical.
