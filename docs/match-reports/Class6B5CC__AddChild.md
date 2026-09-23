# Class6B5CC__AddChild

> Renamed from `func_8001CC48` on 2026-09-23 (tools/rename.py). Address 0x8001cc48.

**Unit:** code_d294 · **Size:** 27 words · **Status:** MATCHED (27/27 words)

## What it does

`Class6B5CC` vtable slot `+0x010`, one of the five BasicClass overrides
(`tools/classtable.py gClass6B5CCMethods --vs D_8006B58C`). Forwards unconditionally
to the base class's own `+0x010` slot (`Get_vtable_BasicClass()->slot10`), then, if
`other`'s own vtable header tag (`other->methods->header & 0xF`) is `9`,
additionally calls `Class6B5CC__LinkModel(self, other)` (still uncarved, next
slice). Its sibling `Class6B5CC__RemoveChild` (`+0x014`) is the mirror-image
"detach" of this "attach".

## The C

```c
void Class6B5CC__AddChild(Class6B5CCObj *self, GenericObj_d294 *other) {
    Get_vtable_BasicClass()->slot10(self, other);
    if ((other->methods->header & 0xF) == 9) {
        Class6B5CC__LinkModel(self, other);
    }
}
```

## Provenance

round 11 (2026-09-03), runner charlie, second pass, unit
code_d294. Matched on the first build. Established
`BasicClassMethodsD294`'s `+0x010` slot (2-arg, `(self, other)`) and the
`GenericObj_d294` generic-dispatch type (header tag comparison), reused by
`Class6B5CC__RemoveChild`/`Class6B5CC__DetachAttachedChildren` below.

## Naming

Round 71 (alpha). `func_8001CC48` -> `Class6B5CC__AddChild`, **tier A**. Overrides BasicClass slot +0x010 `addChild` (include/code_8220.h). Forwards to the base first, then if the child's class tag is 9 (D_8006BEA0) calls Class6B5CC__LinkModel on it. Class6B5CC__AttachToParent reaches this slot on the parent with `self` as the child.

## Proposed field names

For the head to apply by type scope. Each one fails to compile in another unit when renamed in the definition, so this unit did not apply it.

- `Class6B5CCObj.unk18` -> `tmd` (tier B): GsDOBJ2.tmd by offset. Class6B5CC__LinkModel stores the tag-9 child's +0x10 word there and UnlinkModel clears it. Accessors: code_d294, code_d294_c.
- `Class6B5CCObj.unk20` -> `linkedModel` (tier B): Class6B5CC__LinkModel stores the tag-9 child object itself; code_d294_b passes it to the psyq_fa50 helpers. Accessors: code_d294, code_d294_b, code_d294_c.
