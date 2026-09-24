# Obj86B60__ForwardToChild — MATCH (16/16 words)

> Renamed from `func_8003C7B4` on 2026-09-24 (tools/rename.py). Address 0x8003c7b4.

**Unit:** code_2cc8c · **Size:** 16 instructions

## What it does

```c
void Obj86B60__ForwardToChild(Obj86B60 *self, s32 a1)
{
    Unk48Obj *child;

    child = self->unk48;
    if (child != NULL) {
        child->methods->slot80(child, a1, 0x60, 0x60);
    }
}
```

`self->unk48` is a pointer to a DIFFERENT class from `Obj86B60` -- its own
vtable slot `+0x080` takes four arguments (self, a1, a2, a3), whereas
`Obj86B60`'s OWN slot `+0x080` (`Obj86B60__func_8003C944`) takes none beyond self. Two
literal `0x60` constants are passed as the trailing two arguments; `a1` is
forwarded from the caller unchanged.

## Struct knowledge established

- `Obj86B60::unk48` (`Unk48Obj *`, +0x048) -- OBSERVED here, only user in
  this unit.
- `Unk48Obj` -- a minimal opaque type: `methods` at +0x000, one modelled
  slot (`slot80`, `void (*)(Unk48Obj*, s32, s32, s32)` at +0x080).

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c.

## Naming (round 78, delta)

**Tier B.** `func_8003C7B4` -> `Obj86B60__ForwardToChild`. Occupies slot70.
Body: when `self->unk48` (a distinct, still-`Unk48Obj`-typed child) is
non-NULL, forwards `(child, a1, 0x60, 0x60)` to `child->methods->slot80`.
Mechanics are clear (a conditional forward to a child object, two args
fixed); what the fixed `0x60, 0x60` pair or the child's real identity
represent in the game is not established, so tier B rather than A.
