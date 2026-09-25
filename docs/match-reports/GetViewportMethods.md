# GetViewportMethods — MATCHED

> Renamed from `GetUnk18ObjMethods` on 2026-09-25 (tools/rename.py). Address 0x8003f24c.

> Renamed from `func_8003F24C` on 2026-09-23 (tools/rename.py). Address 0x8003f24c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 4/4 words, full match.

## Signature

```c
Unk18ObjMethods *GetViewportMethods(void);
```

Plain no-argument getter for `Unk18Obj`'s own vtable.

## What it does

```c
Unk18ObjMethods *GetViewportMethods(void) {
    return &gViewportMethods;
}
```

## Context

This symbol was already declared `extern` in `include/code_2cc8c.h` from
round 13 (alpha), with a comment saying it "lives in a still-uncarved
remainder... not this unit's function to write." Round 14's carve of
`code_2cc8c_d` brought it in, so it's matched here now — updated the
comment accordingly rather than leaving the stale "external" note.

## Header changes

`include/code_2cc8c.h`: added `extern Unk18ObjMethods gViewportMethods;` (the
table itself, `tools/classtable.py gViewportMethods`, needed so this definition
can return `&gViewportMethods`) and updated the `GetViewportMethods` extern's own
comment to reflect it is now matched, not external.

## Naming

`GetUnk18ObjMethods` -- tier A. Plain no-argument getter, `return &gViewportMethods;` -- `Unk18Obj`'s own class table. Named to match the established sibling convention for this exact shape (`Get_vtable_BasicClass`, `GetClass6E99CMethods`, `GetClass86AA0Methods`), which is a free function (no `self`), not a `Class__Method`.
