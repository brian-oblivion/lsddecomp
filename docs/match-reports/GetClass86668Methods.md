# GetClass86668Methods

> Renamed from `func_8004A4B8` on 2026-09-22 (tools/rename.py). Address 0x8004a4b8.

**Unit:** class_3ac78 · **Size:** 4 words · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `D_80086668`, the 28-slot method table for
`Class86668`. A "Get_vtable" accessor, same shape as `Get_vtable_DreamSys`.

## Derivation

```
lui   $v0, %hi(D_80086668)
addiu $v0, $v0, %lo(D_80086668)
jr    $ra
```

A leaf that materializes a data address and returns it — no `$gp`-relative
form is used here (this address apparently falls outside whatever range
`-G0`'s relocation picked for it, or the compiler chose `%hi`/`%lo` for
another reason; either way it isn't the gp-relative blocker since there's no
`lui`+`lw` pair reading through `$gp`, just address materialization).
`D_80086668` resolved as a 28-slot method table via `tools/classtable.py
D_80086668`; declared `extern Class86668Methods D_80086668;` in
`include/class_3ac78.h` (the table's own data bytes remain unmatched/raw —
this function only takes its address).

## Proposed learning

None beyond what's already documented for `Class86668`/`Class866E8` in
`Class86668__SetChildFlag8.md` and `New_Class866E8.md`.
