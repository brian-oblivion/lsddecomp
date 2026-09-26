# GetApplicationMethods

> Renamed from `GetClass6E4F0Methods` on 2026-09-26 (tools/rename.py). Address 0x8003b20c.

> Renamed from `func_8003B20C` on 2026-09-25 (tools/rename.py). Address 0x8003b20c.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 4 words · **Status:** MATCHED (4/4 words, whole-image SHA1 green)

## What it does

The class-table getter: returns `&gApplicationMethods`.

```c
ApplicationMethods *GetApplicationMethods(void) {
    return &gApplicationMethods;
}
```

## Declarations

`include/GameApplication.h` already declares `extern MiddleClassMethods
*func_8003B20C(void);` with its own view of the table. This unit does NOT
include that header and declares its own local `ApplicationMethods` view,
so the two never meet in one translation unit (the multiple-local-views
convention). A track-4 unification of gApplicationMethods would merge the two views:
MiddleClassMethods types +0x044 as `s32 (*)(void *, void *, void *, s32)`
(4 args, from code_1677c's call `slot44(self, a1, a2, 0)`), while the
occupant Application__InitSystems reads only three (self, source, arg). A void
body matches; the bytes cannot say whether callers use a return value.

## Naming

**Round 81 (delta), track 3. NOT RENAMED -- tool-blocked, see below.**
Proposed `GetApplicationMethods` (matches the sibling class's
`GetGameApplicationMethods`). **Tier A**: a pure getter, mechanics is the
purpose.

**Why it wasn't applied.** `func_8003B20C` matches `rename.py`'s
`PLACEHOLDER` regex (`func_800XXXXX`), so `symbol_address()` resolves its
address from the NAME itself and returns `symline=None` -- it never
consults `config/symbols.slps01556.lsdde.txt` even though this address
already has an explicit line there (a track-2 identification note, added
before this function was matched: `func_8003B20C = 0x8003B20C; // type:func`
followed by `// unidentified: masked 0.29 DrawPrim (libgpu/sys, tied across
2 discs) / SpuRead / SpuWrite, too weak and ambiguous.`). `rename.py` then
APPENDS a second, conflicting line for the same address instead of
replacing the existing one, and `make extract` fails with `Duplicate symbol
detected`. This is a general `rename.py` limitation (any `func_`/`D_`
placeholder-shaped name that already has an explicit symbols-file line hits
it), not specific to naming judgement, so it was reverted rather than
worked around by hand-editing the symbols file (CLAUDE.md hard rule 4/the
collision rules: never edit `config/symbols.slps01556.lsdde.txt` except
through `tools/rename.py`). Posted to the broadcast for the head.

Note in passing: this round's byte-exact match of `func_8003B20C` as
ordinary game C (`return &gApplicationMethods;`, called directly by
`GameApplication__GameApplication`, confirmed game code) also resolves that stale
track-2 ambiguity note -- it is not Sony's `DrawPrim`/`SpuRead`/`SpuWrite`,
it is this class's table getter. The note should be dropped when the head
applies the rename.

## Track 4

**2026-09-25, round 84 (echo).** Renamed `func_8003B20C` ->
`GetApplicationMethods` with `tools/rename.py`, which now replaces the
address's existing symbols-file line (the blocker above no longer
reproduces; the symbols line carried no stale `unidentified` note by this
round). Evidence unchanged: the body is `return &gApplicationMethods;`, Application's
own table, and both callers use it as the table getter
(`Application__Application` installs it; `GameApplication__GameApplication` and
`GameApplication__InitSystems` call the base class's ctor and
+0x044 slot through it). The name follows `GetGameApplicationMethods`/`GetFileResourceMethods`.
