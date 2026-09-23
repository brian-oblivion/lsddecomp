# Unk18Obj__DetachViewChild — MATCHED

> Renamed from `func_8003EB84` on 2026-09-23 (tools/rename.py). Address 0x8003eb84.

Unit: `code_2cc8c_d`. Round 14, runner delta. 16/16 words, full match.

## Signature

```c
void Unk18Obj__DetachViewChild(Unk18Obj *self);
```

`Unk18ObjMethods`'s own `+0x074` slot occupant.

## What it does

Teardown counterpart to `Unk18Obj__AttachViewChild`'s init (this unit, this round):
removes `self->unk10` as a child via the inherited BasicClass
"removeChild" (`slot14`), only if it was ever set.

```c
void Unk18Obj__DetachViewChild(Unk18Obj *self) {
    if (self->unk10 != NULL) {
        self->methods->slot14(self, self->unk10);
    }
}
```

## Header changes

None beyond the prototype — `slot14` was already typed from round 13.

## Naming

`Unk18Obj__DetachViewChild` -- tier B. Teardown counterpart of `Unk18Obj__AttachViewChild`: removes `self->unk10` as a child through the inherited `removeChild` slot, only if it was ever set. Same tier-B caveat on "view" as the attach side.
