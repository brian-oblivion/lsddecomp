# New_Class869D8

> Renamed from `func_8004D254` on 2026-09-22 (tools/rename.py). Address 0x8004d254.

**Unit:** class_3bb8c_c · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

`New_Class869D8`: the allocator for `Class869D8`. Standard
allocate-null-check-ctor shape, identical to class_3ac78.c's
`New_Class866E8`/class_39e08.c's several `New_X` functions -- allocate a
fixed-size block (`0xDC` bytes here), and on success run the class's ctor
(`Class869D8__Class869D8`, fetched through `GetClass869D8Methods()->ctor`) and return the
new instance; return `NULL` on allocation failure.

## The C

```c
Class869D8 *New_Class869D8(void)
{
    Class869D8 *self;

    self = func_80017B34(0xDC);
    if (self != NULL) {
        GetClass869D8Methods()->ctor(self);
        return self;
    }
    return NULL;
}
```

## Notes

Matched on the first attempt -- this is a direct copy of the established
New_X idiom already confirmed several times elsewhere in this codebase
(see docs/MATCHING-GUIDE.md, "Writing a class method" / "Allocation sites
look like..."). No residue.
