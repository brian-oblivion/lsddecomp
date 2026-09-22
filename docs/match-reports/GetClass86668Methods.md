# GetClass86668Methods

> Renamed from `func_8004A4B8` on 2026-09-22 (tools/rename.py). Address 0x8004a4b8.

**Unit:** class_3ac78 · **Size:** 4 words · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `gClass86668Methods`, the 28-slot method table for
`Class86668`. A "Get_vtable" accessor, same shape as `Get_vtable_DreamSys`.

## Derivation

```
lui   $v0, %hi(gClass86668Methods)
addiu $v0, $v0, %lo(gClass86668Methods)
jr    $ra
```

A leaf that materializes a data address and returns it — no `$gp`-relative
form is used here (this address apparently falls outside whatever range
`-G0`'s relocation picked for it, or the compiler chose `%hi`/`%lo` for
another reason; either way it isn't the gp-relative blocker since there's no
`lui`+`lw` pair reading through `$gp`, just address materialization).
`gClass86668Methods` resolved as a 28-slot method table via `tools/classtable.py
gClass86668Methods`; declared `extern Class86668Methods gClass86668Methods;` in
`include/class_3ac78.h` (the table's own data bytes remain unmatched/raw —
this function only takes its address).

## Proposed learning

None beyond what's already documented for `Class86668`/`Class866E8` in
`Class86668__SetChildFlag8.md` and `New_Class866E8.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A4B8` | `GetClass86668Methods` | A | The whole body materializes `&gClass86668Methods` and returns it. `tools/classtable.py D_80086668` resolves that address as a 28-slot method table (header 0x230). Same shape and same name form as the already-established `GetClass6B5CCMethods` / `Get_vtable_BasicClass`. |
| `D_80086668` | `gClass86668Methods` | A | Classtable-verified method table; `gName` per the project's global convention, `Methods` per `gVabDriverMethods` / `gTaskCoreMethods`. The class itself keeps its address-derived placeholder name `Class86668` (tier C) -- nothing establishes what that class is. |
