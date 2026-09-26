# PlacementGrid__SetFlag

> Renamed from `Class6D940__SetFlag` on 2026-09-26 (tools/rename.py). Address 0x8002c238.

> Renamed from `func_8002C238` on 2026-09-24 (tools/rename.py). Address 0x8002c238.

**Unit:** code_179d8_d · **Size:** 16 instructions (0x40 bytes) ·
**Status: MATCHED 16/16**, whole-image SHA1 green.

## Role

Sets a field then forwards to the same base accessor `PlacementGrid__Finalize`
uses, a different slot:

```c
s32 PlacementGrid__SetFlag(s32 *self)
{
    self[0xC] = 1;
    return GetActiveDataSourceMethods()->slot64(self);
}
```

`self[0xC]` writes byte offset `0x30` (`s32` index * 4) -- matches
retail's `sw $v0, 0x30($s0)` exactly, `$v0` having just been loaded with
the literal `1`.

Same tail-call return-type ambiguity as `PlacementGrid__Finalize` (this unit, same
round): the call is the function's last action with nothing touching
`$v0` afterward, so `void` and `s32` compile identically. Typed `s32` and
returned per the project's default, absent positive void evidence.
`slot64` (`BaseTable6D940::slot64`, `+0x064`) added to the same unit-local
table `PlacementGrid__Finalize` uses.

## Naming (round 77, charlie -- track 3)

Renamed `func_8002C238 -> PlacementGrid__SetFlag`, tier B (mechanics, not
purpose). Occupies `+0x064` of `D_8006D940` -- the exact slot
`FileResource__SetFlag` fills in the base class (`tools/classtable.py
0x8006D430`) and its own verbatim-shared copy in `gCdDriverMethods`
(`tools/classtable.py 0x8006D4E8`). Named by SLOT POSITION, not by
asserted behavior: this override does NOT just OR in a flag bit like the
base -- it sets `self[0xC]` (offset 0x30, a field beyond `FileResource`'s
own layout) then forwards through `GetActiveDataSourceMethods()->slot64(self)`.
That mechanics difference is why this is tier B and not A.

## Track 4 (2026-09-26, round 87, echo)

Now `void PlacementGrid__SetFlag(PlacementGrid *self)`, the type of slot +0x064 (`setFlag`, include/FileResource.h), byte-identical. `self[0xC]` is `loaded` (+0x030), zeroed by the ctor. `BaseTable6D940` was the active driver's table, `FileResourceMethods`; the call is `GetActiveDataSourceMethods()->setFlag`.
