> Renamed from `func_80040F28` on 2026-09-18 (tools/rename.py). Address 0x80040f28.

# Obj6EAC0__SetText — MATCHED (30/30 words)

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
void Obj6EAC0__SetText(Obj6EAC0 *self, u8 *a1) {
    Obj6EAC0 **elemp = self->unkB4;
    u8 *p = a1;
    if (p != NULL && *p != 0) {
        do {
            Obj6EAC0 *elem = *elemp;
            elem->methods->slotC4(elem, *p);
            p++;
            elemp++;
        } while (*p != 0);
    }
}
```

Derived-table occupant of `Obj6EAC0Methods::slotCC`. Walks a
NUL-terminated byte string in lockstep with `self->unkB4`, dispatching
each child's `slotC4` with the current byte. `p != NULL && *p != 0` is
the guarded-`do`/`while` idiom already established elsewhere in this
project; matched cleanly with no register-identity surprises, unlike
this unit's `unkAC..unkAC+unkAB`-indexed loop family (see
`Obj6EAC0__LayoutChildrenWithGap`/`func_80040C00`/`Obj6EAC0__QueryChildren`/`Obj6EAC0__PropagateColor`/
`Obj6EAC0__LayoutChildren`'s stall reports) -- this loop's bound is a simple
NUL check, not a re-read-every-iteration byte-field sum, which may be
why it was reachable on the first try.

### Proposed learning

None beyond what's already recorded.
