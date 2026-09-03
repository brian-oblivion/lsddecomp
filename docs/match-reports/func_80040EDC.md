# func_80040EDC — MATCHED (17/17 words)

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
void func_80040EDC(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 *elem = self->unkB4[a2];
    elem->methods->slotC4(elem, a1 & 0xFF);
}
```

Derived-table occupant of `Obj6EAC0Methods::slotC4`, dispatching a
CHILD's own `slotC4` at 2 explicit arguments. This is exactly why
`slotC4` was made unprototyped (K&R style, no parameter list) in the
previous commit -- the base occupant `func_80040854` forwards 4 raw
args through the same named slot, and calling through an unprototyped
function pointer needs no cast for either arity.

### Proposed learning

Confirms the unprototyped-slot lever from `func_80040AE8`'s header
change actually pays off at a real call site with no cast needed.
