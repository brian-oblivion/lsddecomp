# Obj6EAC0__Layout -- MATCH (32/32 words, 1 attempt)

> Renamed from `func_80040664` on 2026-09-18 (tools/rename.py). Address 0x80040664.

Unit `code_2cc8c_f`. `Obj6EAC0Methods::slot4C`'s own occupant (this is the
FUNCTION the vtable slot points to, not a caller of it -- the header's
existing "IS Obj6EAC0__Layout" note on that slot already said as much before
this round). Body: when `self->unkC == 0`, forwards to the shared
"default handler" table's own `slot4C` (`GetClass6B5CCMethods()`, already typed
in `include/code_2cc8c.h` as `D6B5CCGetterMethodsCC8C`, with a `slot4C`
member at the exact offset/arity this call site needs -- no header changes
were necessary), then dispatches `self->methods->slotBC(self, a2)` --
`slotBC` was likewise already typed (`void *a1`, "a 2-word struct
pointer").

```c
void Obj6EAC0__Layout(Obj6EAC0 *self, s32 a1, void *a2) {
    if (self->unkC == 0) {
        GetClass6B5CCMethods()->slot4C(self, a1, 0);
        self->methods->slotBC(self, a2);
    }
}
```

3 distinct callee-saved registers (`$s0`/self, `$s1`/a1, `$s2`/a2), well
under the 7+ saturation band this project's `MATCHING-GUIDE.md` flags as
low-yield. Matched on the first attempt with a direct transliteration of
the disassembly -- both vtable slot types needed were already established
by earlier rounds' work on sibling functions in this same unit
(`Obj6EAC0__SetPosition`/`Obj6EAC0__LayoutChildren` for `slotBC`'s struct shape;
`code_2cc8c_e`'s ctor call for `D6B5CCGetterMethodsCC8C::slot4C`), so
there was no struct derivation left to do here -- just reading the
existing header carefully enough to recognize both slots were already on
file.

No header changes; no residue.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040664` | `Obj6EAC0__Layout` | B |

**Evidence.** Base occupant of `slot4C`: when `self->hasChildren == 0`
(the leaf case), forwards to the shared default handler's own `slot4C`
then dispatches `self->methods->slotBC(self, a2)`
(`Obj6EAC0__SetPosition`, this unit) -- i.e. for a leaf instance, "lay
out" reduces to "apply the default handler, then set my own position".
The derived occupant of the SAME slot
(`Obj6EAC0__LayoutChildrenWithGap`) does the container-side equivalent
(recursively position every child). Named for the slot's own common
mechanic across both occupants (position/layout), not for `a1`'s specific
meaning here, which is not established -- tier B.
