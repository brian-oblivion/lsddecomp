# Unk18Obj__OnNotifyTag5 — MATCHED

> Renamed from `func_8003EE40` on 2026-09-23 (tools/rename.py). Address 0x8003ee40.

Unit: `code_2cc8c_d`. Round 14, runner delta. 18/18 words, full match.

## Signature

```c
void Unk18Obj__OnNotifyTag5(Unk18Obj *self, GenericObj *arg1, s32 arg2);
```

`Unk18ObjMethods`'s own `+0x094` slot occupant (`slot94`, dispatched by
`Unk18Obj__OnNotify`, this round).

## What it does

Increments `self->unk90` unconditionally, and additionally dispatches
`self->methods->slot9C` when `arg2` is 2 or 3.

```c
void Unk18Obj__OnNotifyTag5(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    self->unk90 = self->unk90 + 1;
    if (arg2 == 2 || arg2 == 3) {
        self->methods->slot9C(self);
    }
}
```

Note: retail's own `(unsigned)(arg2 - 2) < 2` range-check fold (a single
`sltiu`) is exactly what `arg2 == 2 || arg2 == 3` compiles to here — the
OPPOSITE lesson from this round's `Class6B5CC__TryAttachNearby`/`Class6B5CC__NotifyIfUnk20Active` (a
different unit), where the same fold was UNWANTED and had to be avoided
with guard clauses. Whether the fold is wanted is purely a property of
what retail's own disassembly shows, not a general rule either way.

## Header changes

`include/code_2cc8c.h`:
- `Unk18Obj` gains `unk90` (`+0x090`, `s32`).
- `Unk18ObjMethods` gains `slot9C` (`+0x09C`, `void (*)(Unk18Obj*)`),
  splitting the `pad09C` span added alongside `slot94`/`slot98` earlier
  this round.

## Naming

`Unk18Obj__OnNotifyTag5` -- tier B. The `slot94` occupant `Unk18Obj__OnNotify` dispatches to when the sender's dynamic-class tag is 5. Body always increments `unk90` and additionally dispatches `slot9C` (`Unk18Obj__Update`) when `event` is 2 or 3. Named after the dispatch mechanism (which tag reaches it), not after what tag 5 or event codes 2/3 mean in the game -- that is not established.
