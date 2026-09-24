# Class6D940__Destroy

> Renamed from `func_8002C200` on 2026-09-24 (tools/rename.py). Address 0x8002c200.

**Unit:** code_179d8_d · **Size:** 14 instructions (0x38 bytes) ·
**Status: MATCHED 14/14**, whole-image SHA1 green.

## Role

Plain forwarding wrapper: dispatches `GetActiveDataSourceMethods()->slot0C(self)`. One
of the "plain forwarding wrapper" shapes flagged as recurring in this
region by charlie's sibling-slice note.

```c
s32 Class6D940__Destroy(void *self)
{
    return GetActiveDataSourceMethods()->slot0C(self);
}
```

This function's WHOLE body is one call with nothing after it -- exactly
the ambiguous case CLAUDE.md warns about ("a void wrapper around a
non-void tail call is byte-identical"). No caller of `Class6D940__Destroy`
exists in this window, so there is no positive evidence either way; per
the project's stated default, `slot0C` is typed `s32` and the value is
returned rather than discarded. Byte-verified identical either way -- this
is a documented choice, not a measured one.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C200 -> Class6D940__Destroy`, tier A. `+0x00C` (dtor)
slot of `D_8006D940` (confirmed by `tools/classtable.py 0x8006D940`),
matching the `Class6D430__Destroy` naming precedent at the same slot
position in the base class.
