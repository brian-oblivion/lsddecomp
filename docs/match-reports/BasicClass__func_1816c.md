# BasicClass__func_1816c

**Unit:** code_8220 · **Size:** 16 instructions · **Status:** MATCHED (16/16 words)

BasicClass vtable slot `+0x02C` (`getNextParentRef`) — see
`BasicClass__BasicClass.md` for the class's overall design, and
`BasicClass__func_180bc.md` (`getNextChild`) for the `children`-list
sibling this one mirrors exactly, over `parentRefs` instead.

## What it does

The `parentRefs`-list iterator, identical shape to `getNextChild`: seeds
`*cursor` from `self->parentRefs` on the caller's first call (detected via
`*outParent == NULL`), then pop-and-advances via `GetNextBasicClass`.

## The C

```c
void BasicClass__func_1816c(BasicClass *self, BasicClass **outParent, BasicClassListNode **cursor)
{
    if (*outParent == NULL) {
        *cursor = self->parentRefs;
    }
    GetNextBasicClass(outParent, cursor);
}
```

Matched first attempt — a plain field-name swap of `BasicClass__func_180bc`.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220, second pass.
