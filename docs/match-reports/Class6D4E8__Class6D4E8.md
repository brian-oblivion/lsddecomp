# Class6D4E8__Class6D4E8 -- MATCHED (19/19 words)

> Renamed from `func_80027228` on 2026-09-25 (tools/rename.py). Address 0x80027228.

Unit `code_179d8_o`, round 26 (2026-09-09). This class's constructor --
confirmed via `tools/classtable.py 0x8006D4E8 --vs 0x8006B58C` as the
override of the base "BasicClass" hierarchy's constructor slot (`+0x008`);
see `New_Class6D4E8.md` for the full table-comparison finding shared by
this whole unit.

## What it is

```c
void Class6D4E8__Class6D4E8(Obj6D4E8 *self)
{
    GetClass6D430Methods(self)->ctor(self);
    self->methods = GetClass6D4E8Methods();
    self->unk28 = 0;
    InitCdDrive();
}
```

Standard "further-base constructor first" idiom (same shape as
`class_3ac78.c`'s own local `BaseCtorTable_3ac78`, independently
established for a different class hierarchy in that unit): `GetClass6D430Methods`
(still `INCLUDE_ASM` in the `code_179d8` remainder) returns a further-base
class's own ctor-dispatch table, called through its `+0x008` slot with
`self`. The rest is straight-line: set `self->methods` (offset 0, the
project's usual table-pointer-at-object-offset-0 convention) to this
class's own table, clear the halfword field at `+0x028`, and call
`InitCdDrive()` (also still `INCLUDE_ASM`, zero-argument -- confirmed by
the retail `jal`'s having no register load anywhere above it that a0 could
be attributed to).

Matched on the first attempt -- a direct, source-order translation of the
retail instruction sequence (no scheduling residue to fight).
