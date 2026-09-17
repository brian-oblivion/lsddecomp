# BasicClass__func_180bc

**Unit:** code_8220 · **Size:** 16 instructions · **Status:** MATCHED (16/16 words)

BasicClass vtable slot `+0x01C` (`getNextChild`) — see
`BasicClass__BasicClass.md` for the class's overall design.

## What it does

The `children`-list iterator: `GetNextBasicClass` pop-and-advances a cursor
(see `BasicClass__func_18040.md` for that function's established
signature), and this wraps it with a "first call" seed — if `*outChild`
is still `NULL` (the caller's sentinel for "haven't started yet"),
`*cursor` is (re)seeded from `self->children` before popping. Lets a
caller do:

```c
BasicClass *item = NULL;
BasicClassListNode *cursor;
do {
    self->methods->getNextChild(self, &item, &cursor);
    if (item == NULL) break;
    /* use item */
} while (1);
```

## The C

```c
void BasicClass__func_180bc(BasicClass *self, BasicClass **outChild, BasicClassListNode **cursor)
{
    if (*outChild == NULL) {
        *cursor = self->children;
    }
    GetNextBasicClass(outChild, cursor);
}
```

Matched first attempt, straight transliteration of the disassembly — the
generic-object-iterator pattern was already fully derived while matching
`BasicClass__func_18040` last round, so this one and its `parentRefs`
sibling (`BasicClass__func_1816c`) needed no reshaping at all.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220, second pass.
