# Obj6EAC0__SetText — MATCHED (30/30 words)

> Renamed from `func_80040F28` on 2026-09-18 (tools/rename.py). Address 0x80040f28.

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

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040F28` | `Obj6EAC0__SetText` | B |

**Evidence.** Derived occupant of `slotCC` (base occupant is
`BoxFill__SetMask`, an unrelated bitmask setter -- this slot means
different things on the two tables, per this project's per-call-site
convention): walks a NUL-terminated byte string in lockstep with
`self->children`, dispatching `elem->methods->slotC4(elem, *p)` for each
byte -- i.e. sets each child's character one string byte at a time. This
is the strongest single piece of evidence for the whole unit's
text/digit-display reading (see the unit header comment): it is a
literal "for each character in this string, tell the next child glyph
what it is" loop. Tier B: the loop mechanics are unambiguous, but nothing
outside this unit confirms the in-game role.
