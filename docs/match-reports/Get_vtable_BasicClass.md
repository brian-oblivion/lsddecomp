# Get_vtable_BasicClass

> Renamed from `func_80018390` on 2026-09-17 (tools/rename.py). Address 0x80018390.

**Unit:** TmdRenderer · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `gBasicClassMethods`, BasicClass's own 14-slot method table
(`BASICCLASS_METHODS` per `docs/research/class-framework.md`). Called from
`BasicClass__BasicClass` (`BMemPMgr.c`) to install the base vtable on a
freshly-constructed `BasicClass`.

## The C

```c
BasicClassMethods *Get_vtable_BasicClass(void)
{
    return &gBasicClassMethods;
}
```

`gBasicClassMethods` had no extern declaration anywhere in the tree yet (only prose
references to it in `class_16334.h`, `GameApplicationFileResource.h`, `code_55dd4.h`,
`code_d294.h`, `GameApplication.h`). Added one to `include/code_8220.h`:

```c
extern BasicClassMethods gBasicClassMethods;
```

The underlying data (`asm/data/57070.data.s`, `dlabel gBasicClassMethods`) is 0x40
bytes / 16 words — one header word, 14 method-pointer words matching every
field of `BasicClassMethods`, and a trailing `.word 0x00000000` past the
struct's own `0x03C` end. That extra word is not part of the C-visible
`BasicClassMethods` layout (nothing reads it) and did not need to be modeled
here; `Get_vtable_BasicClass` only takes the table's address, never indexes past its
declared fields.

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt — simple `lui`/`addiu` address-of, no ambiguity.

## Naming (round 51, bravo)

`func_80018390` -> `Get_vtable_BasicClass`. **Tier A** -- pure leaf
returning one known address.

Evidence: the whole body is `return &gBasicClassMethods;`, and `gBasicClassMethods` is
BasicClass's own 14-slot method table (`tools/classtable.py gBasicClassMethods`).
The spelling is the house convention rather than an invention:
`Get_vtable_DreamSys` (`src/DreamSys.c`) and `Get_vtable_Entity`
(`src/Entity.c`) are the two existing vtable accessors in the tree and
both are `Get_vtable_<Class>`. 62 files reference this function, which is
what makes matching the existing convention worth more than a tidier one.

### Proposed field/global name for the head

`gBasicClassMethods` -> `BASICCLASS_METHODS`. **Tier A.** It is the name
`docs/research/class-framework.md` already uses for this exact table, and
it matches `gDreamSysMethods`, the one method table in the tree that is
already named. **`tools/rename.py` refuses it** and I did not work around
it: its "NEW already appears" guard fires because six files
(`docs/match-reports/BasicClass__BasicClass.md`, this report,
`Pad__LoadButtonTable.md`, `Get_vtable_Pad.md`, `include/class_16334.h`,
`include/class_3bb8c.h`) already contain the string `BASICCLASS_METHODS` as
PROSE describing this very symbol. The guard is correct in general and
wrong in this instance; see `### Proposed learning` below.

### Proposed learning

**`tools/rename.py`'s "NEW already appears" guard has a systematic false
positive on exactly the names track 3 most wants to use.** The guard exists
to stop a rename creating two symbols with one name, so it greps for the
new name as a whole word anywhere in `src/`, `include/` and the reports.
But the best candidate name for a placeholder symbol is very often the name
the project's own PROSE has been using for that symbol all along -- that is
what makes it the right name -- and prose hits fire the guard just as hard
as a real symbol would. Measured twice this round on one symbol
(`gBasicClassMethods`, six prose hits, zero symbol hits). A prose-only hit is
distinguishable from a real one: a real collision appears in a
DECLARATION or the symbols file, and the tool already parses the symbols
file. This is a tooling escalation, not something to route around by
editing the docs first.
