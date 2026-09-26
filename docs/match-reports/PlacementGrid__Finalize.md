# PlacementGrid__Finalize

> Renamed from `Class6D940__Finalize` on 2026-09-26 (tools/rename.py). Address 0x8002c200.

> Renamed from `PlacementGrid__Destroy` on 2026-09-26 (tools/rename.py). Address 0x8002c200.

> Renamed from `func_8002C200` on 2026-09-24 (tools/rename.py). Address 0x8002c200.

**Unit:** code_179d8_d · **Size:** 14 instructions (0x38 bytes) ·
**Status: MATCHED 14/14**, whole-image SHA1 green.

## Role

Plain forwarding wrapper: dispatches `GetActiveDataSourceMethods()->slot0C(self)`. One
of the "plain forwarding wrapper" shapes flagged as recurring in this
region by charlie's sibling-slice note.

```c
s32 PlacementGrid__Finalize(void *self)
{
    return GetActiveDataSourceMethods()->slot0C(self);
}
```

This function's WHOLE body is one call with nothing after it -- exactly
the ambiguous case CLAUDE.md warns about ("a void wrapper around a
non-void tail call is byte-identical"). No caller of `PlacementGrid__Finalize`
exists in this window, so there is no positive evidence either way; per
the project's stated default, `slot0C` is typed `s32` and the value is
returned rather than discarded. Byte-verified identical either way -- this
is a documented choice, not a measured one.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C200 -> PlacementGrid__Finalize`, tier A. `+0x00C` (dtor)
slot of `gPlacementGridMethods` (confirmed by `tools/classtable.py 0x8006D940`),
matching the `FileResource__Finalize` naming precedent at the same slot
position in the base class.

## Track 4 (2026-09-26, round 87, echo)

Renamed `PlacementGrid__Destroy -> PlacementGrid__Finalize`: it occupies slot
+0x00C of gPlacementGridMethods, which is `finalize` in every class
(`include/BasicClass.h`; `tools/classtable.py gPlacementGridMethods --vs gFileResourceMethods`
shows it overriding `FileResource__Finalize`), and its body is only the
parent's finalize, reached through the active data source's table
(`GetActiveDataSourceMethods()->finalize`). The slot is `void`, so the
function is now `void` too: byte-identical, which settles the tail-call
ambiguity above in favour of the slot's type.
