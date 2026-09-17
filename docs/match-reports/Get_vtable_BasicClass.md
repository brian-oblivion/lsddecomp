> Renamed from `func_80018390` on 2026-09-17 (tools/rename.py). Address 0x80018390.

# Get_vtable_BasicClass

**Unit:** code_8220_b · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `D_8006B58C`, BasicClass's own 14-slot method table
(`BASICCLASS_METHODS` per `docs/research/class-framework.md`). Called from
`BasicClass__BasicClass` (`code_8220.c`) to install the base vtable on a
freshly-constructed `BasicClass`.

## The C

```c
BasicClassMethods *Get_vtable_BasicClass(void)
{
    return &D_8006B58C;
}
```

`D_8006B58C` had no extern declaration anywhere in the tree yet (only prose
references to it in `class_16334.h`, `code_171e0.h`, `code_55dd4.h`,
`code_d294.h`, `Class6D3C8.h`). Added one to `include/code_8220.h`:

```c
extern BasicClassMethods D_8006B58C;
```

The underlying data (`asm/data/57070.data.s`, `dlabel D_8006B58C`) is 0x40
bytes / 16 words — one header word, 14 method-pointer words matching every
field of `BasicClassMethods`, and a trailing `.word 0x00000000` past the
struct's own `0x03C` end. That extra word is not part of the C-visible
`BasicClassMethods` layout (nothing reads it) and did not need to be modeled
here; `Get_vtable_BasicClass` only takes the table's address, never indexes past its
declared fields.

## Provenance

round 12 (2026-09-03), runner charlie, unit code_8220_b (fresh carve).
Matched first attempt — simple `lui`/`addiu` address-of, no ambiguity.
