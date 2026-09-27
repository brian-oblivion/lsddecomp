# BasicClass__GetNextParentRef

> Renamed from `BasicClass__func_1816c` on 2026-09-24 (tools/rename.py). Address 0x8001816c.

**Unit:** BMemPMgr · **Size:** 16 instructions · **Status:** MATCHED (16/16 words)

BasicClass vtable slot `+0x02C` (`getNextParentRef`) — see
`BasicClass__BasicClass.md` for the class's overall design, and
`BasicClass__GetNextChild.md` (`getNextChild`) for the `children`-list
sibling this one mirrors exactly, over `parentRefs` instead.

## What it does

The `parentRefs`-list iterator, identical shape to `getNextChild`: seeds
`*cursor` from `self->parentRefs` on the caller's first call (detected via
`*outParent == NULL`), then pop-and-advances via `GetNextBasicClass`.

## The C

```c
void BasicClass__GetNextParentRef(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor)
{
    if (*outParent == NULL) {
        *cursor = self->parentRefs;
    }
    GetNextBasicClass(outParent, cursor);
}
```

Matched first attempt — a plain field-name swap of `BasicClass__GetNextChild`.

## Provenance

round 11 (2026-09-03), runner delta, unit BMemPMgr, second pass.

## Naming (round 74)

`BasicClass__GetNextParentRef`, **tier A**: matches vtable slot `+0x02C`
(`getNextParentRef`), already documented in `BMemPMgr.h`. Identical
shape to `BasicClass__GetNextChild`, over `parentRefs` instead of
`children`.
