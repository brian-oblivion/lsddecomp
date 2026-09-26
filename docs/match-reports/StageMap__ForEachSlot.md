# StageMap__ForEachSlot — MATCHED (36/36 words)

> Renamed from `StageMap__ForEachElem` on 2026-09-26 (tools/rename.py). Address 0x8004d140.

> Renamed from `Class866E8__ForEachElem` on 2026-09-26 (tools/rename.py). Address 0x8004d140.

> Renamed from `func_8004D140` on 2026-09-24 (tools/rename.py). Address 0x8004d140.

Iterates `self->arr[0..6]`, invoking an optional per-element callback
(`arg2`, called `(self, &arr[i])` when non-NULL) and then always forwarding
`(self, arg1, &arr[i])` to `StageMap__ForEachEntryChild`. Already had a prototype and a
two-hop derivation in `include/class_3bb8c.h` from a previous round
(established from `StageMap__StepScaleRamp`/`StageMap__EndScaleRamp`'s call sites); this round
supplied the body.

Matches the "explicit intermediate element pointer in an array loop" idiom
from `docs/DECOMPILATION_LEARNINGS.md`: `e = &self->arr[i];` rather than
repeated `self->arr[i]` field access.

## Final C

```c
void StageMap__ForEachSlot(Obj866E8 *self, void (*arg1)(Obj866E8 *self, EntryChildObj *item), void (*arg2)(Obj866E8 *self, Elem *item)) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (arg2 != 0) {
            arg2(self, e);
        }
        StageMap__ForEachEntryChild(self, arg1, e);
    }
}
```

## Attempts

1 (matched on first attempt — the header's existing derivation and the
established "intermediate element pointer" idiom were enough to reproduce
the loop exactly, including the `0xEC`-based offset accumulator retail
uses instead of re-deriving `&arr[i]` from scratch each iteration).

### Proposed learning

None new — confirms the existing "intermediate element pointer" idiom
generalizes to a loop whose body is entirely calls (no field reads other
than the pointer itself).

## Naming

**Tier A.** Not a vtable slot -- a generic iteration helper: loops
`self->arr[0..6]`, optionally invoking a per-`Elem` callback (`arg2`),
then always forwarding to `StageMap__ForEachEntryChild` for each
element. Mechanics-is-purpose: it is exactly what its name says, a
for-each over the object's `Elem` array, used by both the rate/countdown
callers this round and (per the header's existing prototype) elsewhere.
