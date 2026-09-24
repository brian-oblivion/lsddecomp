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
function `func_8004C434`'s already-matched shape, closed it:

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
`class_3bb8c.c`'s `func_8004C434`/`func_8004BCE0`).

## Attempts

2 (see residue above).

### Proposed learning

**An explicit intermediate element pointer (`Elem *e = &self->arr[i];`)
is not just style — it changes whether GCC 2.6.3 reuses the array's BASE
pointer register as its own induction variable.** Without it, when the
base pointer (here, `self`) is dead after the loop, GCC freely repurposes
it into a moving pointer, dropping any separately-tracked byte offset.
Retail's own code keeps a genuinely separate running-offset register in
this function — the same shape `func_8004C434` (already matched) uses.
This generalizes the existing "let GCC hoist its own loop invariants"
guidance in `docs/DECOMPILATION_LEARNINGS.md`: that entry showed
*keeping* an explicit local matters for a hand-hoisted `base`; here the
opposite-looking fix (adding an explicit per-iteration element pointer)
serves the same underlying purpose — controlling which value GCC treats
as the loop's own induction variable. Confirmed again on the very next
function, `Class866E8__FindElemIndexByUnk30` (own report), so this is now a two-instance
pattern for this project, not a one-off.
