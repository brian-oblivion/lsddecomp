# GetClass869D8Methods

> Renamed from `func_8004D37C` on 2026-09-22 (tools/rename.py). Address 0x8004d37c.

**Unit:** class_3bb8c_c · **Size:** 4 words · **Status:** MATCHED (4/4)

## What it does

Get-vtable helper for a small sibling class: returns `&gClass869D8Methods`, the
vtable this unit calls `Class869D8Methods`. Same shape as the game's other
`func_80xxxxxx()->ctor(...)` vtable getters (e.g. class_3ac78.c's
`GetClass86668Methods`/`GetClass866E8Methods`).

## The C

```c
Class869D8Methods *GetClass869D8Methods(void)
{
    return &gClass869D8Methods;
}
```

## New type: Class869D8 / Class869D8Methods

`gClass869D8Methods` (asm/data/76DC8.data.s) is a plain-C vtable in the same shape
as every other one in this codebase: header word (0x17), then a
`BasicClass__Release` slot at +0x004, then a ctor at +0x008. Resolved by
reading the table directly (it isn't registered with `tools/classtable.py`,
since it isn't `D_800866E8`'s own table -- this is a distinct, smaller
class). Only the slots this unit's own functions reach are typed:
+0x008 (`ctor`, Class869D8__Class869D8) and +0x040 (a post-construct hook,
func_8004D2F8, already matched as an empty body). See
`include/class_3bb8c.h`.

## Proposed learning

Not every vtable in this executable is one of the ones `tools/classtable.py
--scan` already knows about -- a table can be found by tracing a
`func_8004Dxxx()->slotN(...)` call backward to the `lui`/`addiu` pair that
computes the getter's return value, then reading the table directly out of
`asm/data/*.s`. The "count header word, +0x004 BasicClass__Release"
opening two words are a strong fingerprint that a `.word` block found this
way really is a vtable and not incidental data.

## Naming

**GetClass869D8Methods** -- tier A. Pure vtable getter (`return
&gClass869D8Methods;`), the same shape and role as the project's other
`GetClassXMethods` getters (e.g. `GetClass86668Methods`, class_3ac78.c).
The mechanics -- "returns a pointer to this specific class's own methods
table" -- ARE the purpose, so tier A applies by the leaf-getter rule. The
underlying vtable global was renamed alongside it, `D_800869D8` ->
`gClass869D8Methods` (same `g` + getter-name-minus-"Get" pairing already
established by `gClass86668Methods`/`GetClass86668Methods`).
