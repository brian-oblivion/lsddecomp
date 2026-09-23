# Class876FC__ReleaseByKind -- MATCHED (31/31 words)

> Renamed from `func_80056718` on 2026-09-23 (tools/rename.py). Address 0x80056718.

Unit `class_3bb8c_s`. `self` is this unit's local `LinkNode` (see the unit's
own file banner / `class_3bb8c_s.c` for the full type; kept local per the
multiple-independent-local-views convention, not shared with
`class_3bb8c_o.c`'s `LinkOwnerObj`).

## Classification

Clean on all four carve-time screens. Trivial once the dispatch shape was
clear: a `switch` on `self->unk54` (the same state field `Class876FC__InitByKind` and
`Class876FC__UpdateByKind`, its two siblings in this unit, both also switch on) with
four cases, two of which forward straight into `class_3bb8c_o.c`'s
`LinkOwnerObj__ReleaseLinks`/`LinkOwnerObj__ReleaseLinksB`.

## Body

```c
void Class876FC__ReleaseByKind(LinkNode *self) {
    switch (self->unk54) {
    case 0:
        Class876FC__ReleaseModelChildren(self);
        break;
    case 2:
        LinkOwnerObj__ReleaseLinks(self);
        break;
    case 3:
        LinkOwnerObj__ReleaseLinksB(self);
        break;
    default:
        break;
    }
}
```

## Notes

`Class876FC__InitByKind`/`Class876FC__UpdateByKind` (still `INCLUDE_ASM`, `gp_rel`-blocked) switch
on the SAME `self->unk54` field with DIFFERENT case->callee mappings -- read
as three separate per-phase handlers (e.g. update/draw/free) sharing one
state selector, not three views of the same table. Do not assume they share a
callee list.

### Proposed learning

None beyond what's already documented -- a clean, ordinary dispatch.
