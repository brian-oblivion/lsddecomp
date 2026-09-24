# TaskObjF__RemoveChild — MATCH (40/40 words)

> Renamed from `func_8004E4E8` on 2026-09-24 (tools/rename.py). Address 0x8004e4e8.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__RemoveChild(Node3bb8cE *self, Res3bb8cE *res)`. Mirror of
`TaskObjF__AddChild`: if `res` is non-NULL, dispatch on
`res->methods->header`'s tag (2/5/0x10/0x20) and zero the matching one of
self's four typed resource slots, then unconditionally chain to the base
class's `removeChild` (`Get_vtable_BasicClass()`'s +0x014 slot) — even when no
tag matched.

## Result

Matched immediately, applying `TaskObjF__AddChild`'s already-derived lesson
(narrow `tag & 0xF` mask shared by the 2/5 comparisons, widened to
`tag & 0xFF` only for 0x10/0x20) up front:

```c
void TaskObjF__RemoveChild(Node3bb8cE *self, Res3bb8cE *res)
{
    s32 tag;

    if (res == NULL) {
        return;
    }
    tag = res->methods->header;
    if ((tag & 0xF) == 2) {
        self->unk60 = NULL;
    } else if ((tag & 0xF) == 5) {
        self->unk64 = NULL;
    } else if ((tag & 0xFF) == 0x10) {
        self->unk78 = NULL;
    } else if ((tag & 0xFF) == 0x20) {
        self->unk7C = NULL;
    }
    Get_vtable_BasicClass()->removeChild(self, res);
}
```

Note the shape difference from `TaskObjF__AddChild`: this one uses an
`else if` chain (falling through to the shared `removeChild` call after
whichever branch fires, or after none), not early `return`s, because
retail's own control flow re-converges on one shared tail block
regardless of which tag matched — confirmed by each case-arm's own `j`
target being the same shared instruction that also serves as the
"no tag matched" fallthrough.

### Proposed learning

Confirms `TaskObjF__AddChild`'s narrow-then-wide mask lesson generalizes to
its structural mirror. See that report for the full derivation.

## Naming (round 78, track 3)

`func_8004E4E8` -> `TaskObjF__RemoveChild`. **Tier A.** Sits at `gTaskObjFMethods` +0x014, the offset this unit's own `BaseMethods3bb8cE` view had already named `removeChild`. Clears whichever typed resource slot the tag word selects, then chains to the base class's `removeChild` -- the exact mirror of `TaskObjF__AddChild`.
