# Class866E8__GetCurrentCellKey

> Renamed from `func_8004B31C` on 2026-09-22 (tools/rename.py). Address 0x8004b31c.

**Unit:** class_3ac78 · **Size:** 2 words · **Status:** MATCHED (2/2 words)

## What it does

`Class866E8`'s slot +0x0D4 accessor: returns the address of an embedded
field/sub-array at `self`+0x1C0. Address-of only — nothing reads through the
returned pointer here, so its pointee's real element type is unconfirmed.

## Derivation

```
jr    $ra
 addiu $v0, $a0, 0x1C0
```

A leaf computing `&self->unk1C0` and returning it. Sized the trailing
`unk1C0` field as `u8 unk1C0[0x1E8 - 0x1C0]` (0x28 bytes) in
`include/class_3ac78.h` so `Class866E8`'s total size comes out to exactly
0x1E8 — the same constant `New_Class866E8`'s allocator call uses — without
asserting anything about the field's internal structure.

## Proposed learning

None beyond what's already documented for `Class866E8` in `Class86668__SetChildFlag8.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B31C` | `Class866E8__GetCurrentCellKey` | B | Occupant of vtable slot `+0x0D4`; returns `&self->curCellTag`. That address is the start of the 4-byte record `Class866E8__DispatchToRectCells` rewrites immediately before every cell visit: a tag halfword copied from `cellTag`, then the current column and row as bytes. So the accessor hands out "which cell is being visited right now". Tier B -- the record's contents are established, what a consumer does with the pointer is not (no decompiled function reads through it). |

This supersedes the earlier reading in this report, which sized `+0x1C0` as
one opaque 0x28-byte block because nothing then reached inside it.
`Class866E8__DispatchToRectCells` does, and its three writes are what named
the fields.
