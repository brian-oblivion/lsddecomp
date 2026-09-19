# Get_vtable_Entity

**Unit:** Entity_b · **Size:** 4 words · **Status:** MATCHED (4/4 words,
whole-image build verified byte-exact)

## What it does

Trivial table-pointer accessor: returns `&ENTITY_METHODS`, the same
`EntityMethods` table `New_Entity`/`Entity__Entity` assign to
`this->methods`. Called directly by name (`jal Get_vtable_Entity`), not
through a vtable slot itself.

## Final C

```c
EntityMethods *Get_vtable_Entity(void) {
    return &ENTITY_METHODS;
}
```

Required one new declaration in `include/Entity.h`:

```c
extern EntityMethods ENTITY_METHODS;
```

`ENTITY_METHODS` itself stays a raw asm data blob (`asm/data/79528.data.s`,
offsets `0x000`..`0x180`) — only a correctly-typed `extern` was needed here,
per this unit's own convention for still-uncarved data (same as
`gEntityDefaultPos`/`gEntityDefaultOffset` already in this header).

## Attempt log

Matched on the first attempt — exactly as predicted by the sizing note (an
11-line accessor). The address-of-a-global low-half immediate briefly
disagreed with retail (`-0x652c` vs `-0x6528`) while this unit's other
functions were still wrong sizes and hence causing whole-image address drift;
it resolved to byte-exact as soon as every other function this round matched.

## Proposed learning

None beyond what's already documented (`lui`/`addiu` with no surrounding
`lw`/`sw` is `&symbol`, already in `docs/DECOMPILATION_LEARNINGS.md`).
