# New_Class86AA0

> Renamed from `func_8004D38C` on 2026-09-22 (tools/rename.py). Address 0x8004d38c.

**Unit:** class_3bb8c_c · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

`New_Class86AA0`: the allocator for `Class86AA0`, mirroring New_Class869D8
exactly but for the sibling class (`0x3C`-byte allocation, ctor
`Class86AA0__Class86AA0` fetched through `GetClass86AA0Methods()->ctor`).

## The C

```c
Class86AA0 *New_Class86AA0(void)
{
    Class86AA0 *self;

    self = BMemPMgrAlloc(0x3C);
    if (self != NULL) {
        GetClass86AA0Methods()->ctor(self);
        return self;
    }
    return NULL;
}
```

## Notes

Matched on the first attempt; same idiom as New_Class869D8. No residue.

## Naming

**New_Class86AA0** -- tier A. Same `New_X` allocator idiom as
`New_Class869D8` (see that report), mirrored exactly for this sibling
class.
