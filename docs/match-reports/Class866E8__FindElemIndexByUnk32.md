# Class866E8__FindElemIndexByUnk32 — MATCHED (18/18 words)

> Renamed from `func_8004C588` on 2026-09-24 (tools/rename.py). Address 0x8004c588.

Not a vtable slot by call-site inspection needed — it IS one, per
`tools/classtable.py D_800866E8` (`+0x120`), though nothing in this unit
dispatches through that slot itself. A plain linear-search helper: walks
`self->arr` looking for the element whose `unk4->unk32` matches a key,
returning its index (or 0 if never found — the loop's initial `result`
default, never overwritten).

## Residue and how it closed (1 residue, 2 attempts)

First attempt wrote the natural indexed loop directly:

```c
if (self->arr[i].unk4->unk32 == key) { ... }
```

This compiled to a MOVING-POINTER loop (`self` itself incremented by
`0x1C` per iteration, reading a fixed `+0xF0` offset from the current
pointer) — GCC 2.6.3 reusing the `self` register as its own induction
variable, since (in this shape) nothing needs `self` again after the
loop. Retail instead keeps a genuinely SEPARATE running byte-offset
register alongside the loop index, never touching `self` itself. Adding
an explicit intermediate pointer variable, exactly mirroring the sibling
function `Class866E8__FindElemByUnk32`'s already-matched shape, closed it:

```c
Elem *e = &self->arr[i];
if (e->unk4->unk32 == key) { ... }
```

Second attempt (with `e`) matched immediately.

## Final C

```c
s32 Class866E8__FindElemIndexByUnk32(Obj866E8 *self, s32 key) {
    s32 result;
    s32 i;
    Elem *e;

    result = 0;
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            result = i;
            break;
        }
    }
    return result;
}
```

## New struct knowledge

None new (reuses `Elem`/`ElemTarget::unk32`, both already established by
`class_3bb8c.c`'s `Class866E8__FindElemByUnk32`/`Class866E8__CountFlaggedElements`).

## Attempts

2 (see residue above).

### Proposed learning

**An explicit intermediate element pointer (`Elem *e = &self->arr[i];`)
is not just style — it changes whether GCC 2.6.3 reuses the array's BASE
pointer register as its own induction variable.** Without it, when the
base pointer (here, `self`) is dead after the loop, GCC freely repurposes
it into a moving pointer, dropping any separately-tracked byte offset.
Retail's own code keeps a genuinely separate running-offset register in
this function — the same shape `Class866E8__FindElemByUnk32` (already matched) uses.
This generalizes the existing "let GCC hoist its own loop invariants"
guidance in `docs/DECOMPILATION_LEARNINGS.md`: that entry showed
*keeping* an explicit local matters for a hand-hoisted `base`; here the
opposite-looking fix (adding an explicit per-iteration element pointer)
serves the same underlying purpose — controlling which value GCC treats
as the loop's own induction variable. Confirmed again on the very next
function, `Class866E8__FindElemIndexByUnk30` (own report), so this is now a two-instance
pattern for this project, not a one-off.

## Naming

**Tier A.** Vtable slot +0x120 of `D_800866E8` (`tools/classtable.py`),
class prefix `Class866E8` confirmed against that same table's other
already-named slots (e.g. `Class866E8__ApplyToSenderFootprint` at +0x12C).
Pure linear-search leaf: the body IS the evidence -- walk `self->arr`,
compare `e->unk4->unk32` to `key`, return the matching index (default 0).
Named by the field it searches on (`unk32`, still unrenamed -- it is a
cross-unit `ElemTarget` field, also read by `Class866E8__FindElemByUnk32` in
class_3bb8c.c, so this unit does not own it) rather than by a guessed
purpose, per this project's "name what the code does" rule.

## Proposed field names

**Head, round 76: NOT APPLIED.** The set is internally inconsistent: `unk30` and `unk32` cannot both be `key`, and `include/class_3bb8c.h`'s view records `unk30` as a raw rate that Class866E8__ComputeFootprintDescriptor sign-extends, so equality-compared-here is not enough for `key`. `unk2C -> enabled` rests on "nonzero enables" alone. Re-propose from the base class's naming pass (class_3bb8c.c), where all readers are in one unit.

`ElemTarget::unk32` (also read by class_3bb8c.c's `Class866E8__FindElemByUnk32`, so
cross-unit -- not renamed here). Proposed: `key` -- it is exactly what
both `FindElemIndexByUnk32` and its caller-side context treat it as, a
value compared for equality to select an element. Evidence: this
function's entire body is `e->unk4->unk32 == key`.
