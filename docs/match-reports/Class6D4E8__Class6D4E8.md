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

## Naming

Round 79 (delta).

- **`Class6D4E8__Class6D4E8`** (was `func_80027228`) -- **tier A**. It is
  table slot +0x008 of D_8006D4E8 (`tools/classtable.py 0x8006D4E8 --vs
  0x8006D430`), the slot that holds `Class6D430__Class6D430` in the parent
  table and `BasicClass__BasicClass` in the root table. The body is the
  project's constructor shape exactly: parent ctor first
  (`GetClass6D430Methods()->ctor(self)`), then install its own table
  (`self->methods = GetClass6D4E8Methods()`), then its own fields
  (`unk28 = 0`), then `InitCdDrive()` (code_179d8_q: one-shot
  `CdSetDebug(0)` + set double-speed mode). `New_Class6D4E8` dispatches it.
- The two externs' notes that `GetClass6D4E8Methods` and `InitCdDrive` are
  "still INCLUDE_ASM" were stale (both are matched C in code_179d8_q.c) and
  are corrected in the unit.
- `unk28` (+0x28, s16) is kept: cleared here and by Class6D430's own ctor,
  read nowhere in this unit, so nothing establishes a meaning.
- The parent-table view `BaseCtorTable6D4E8` is renamed `Class6D430CtorView`
  (it IS D_8006D430 seen down to +0x008).
