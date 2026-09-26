# TextRow__SetCellAt — MATCHED (17/17 words)

> Renamed from `Obj6EAC0__SetChildChar` on 2026-09-26 (tools/rename.py). Address 0x80040edc.

> Renamed from `func_80040EDC` on 2026-09-18 (tools/rename.py). Address 0x80040edc.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
void TextRow__SetCellAt(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 *elem = self->unkB4[a2];
    elem->methods->slotC4(elem, a1 & 0xFF);
}
```

Derived-table occupant of `Obj6EAC0Methods::slotC4`, dispatching a
CHILD's own `slotC4` at 2 explicit arguments. This is exactly why
`slotC4` was made unprototyped (K&R style, no parameter list) in the
previous commit -- the base occupant `BoxFill__AttachAbsolute` forwards 4 raw
args through the same named slot, and calling through an unprototyped
function pointer needs no cast for either arity.

### Proposed learning

Confirms the unprototyped-slot lever from `TextRow__AttachToParent`'s header
change actually pays off at a real call site with no cast needed.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040EDC` | `TextRow__SetCellAt` | B |

**Evidence.** Derived occupant of `slotC4`: `elem = self->children[a2];
elem->methods->slotC4(elem, a1 & 0xFF);` -- dispatches a single 8-bit
value to ONE child selected by index, through the same slot
`TextRow__SetText` (this unit) drives across ALL children one string
byte at a time. See `BoxFill__AttachAbsolute`'s own report for the full
cross-reference; same tier B.
