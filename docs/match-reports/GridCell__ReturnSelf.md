# GridCell__ReturnSelf

> Renamed from `Class86AA0__ReturnSelf` on 2026-09-26 (tools/rename.py). Address 0x8004d500.

> Renamed from `ReturnSelf` on 2026-09-22 (tools/rename.py). Address 0x8004d500.

> Renamed from `func_8004D500` on 2026-09-22 (tools/rename.py). Address 0x8004d500.

**Unit:** TitleMenuTaskObjF · **Size:** 2 words · **Status:** MATCHED (2/2)

## What it does

Identity accessor: returns its own argument unchanged (`addu $v0, $a0,
$zero; jr $ra`). No call site was found anywhere in the currently-carved
units (it is one of gGridCellMethods's own vtable slots, +0x0FC), so its real
argument/return type could not be pinned down beyond "a pointer".

## The C

```c
void *GridCell__ReturnSelf(void *self)
{
    return self;
}
```

## Notes

Pure register move; no residue to discuss. `self` is typed `void *` since
no caller in this unit's own carved code dereferences the value in a way
that would narrow it further.

## Naming

**GridCell__ReturnSelf** -- tier A. The entire function is `return
self;` (`addu $v0, $a0, $zero; jr $ra`) -- mechanics and purpose are
identical by the tier-A leaf rule ("a pure leaf whose mechanics ARE its
purpose... is tier A by definition"). Kept the `GridCell__` prefix
(method convention) rather than a bare free-function name since it is
found only as one of `gGridCellMethods`'s own vtable slots (+0x0BC, the
table's last entry) with no evidence it is shared by any other class's
table.

## Track 4 (2026-09-26, round 88, alpha)

Parameter retyped `void *` -> `GridCell *` (a slot occupant takes its
class as `self`); the header names the slot +0x0BC `returnSelf`. Still no
known caller; bytes unchanged. The "+0x0FC" in "What it does" above is a
typo for +0x0BC (the table has 47 slots, `classtable.py gGridCellMethods`).
