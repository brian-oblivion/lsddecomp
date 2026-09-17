> Renamed from `func_80027E68` on 2026-09-17 (tools/rename.py). Address 0x80027e68.

# GetClass6D4E8Methods

**Unit:** code_179d8_q (fresh carve) · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## Class identity

This function is the "get my own method table" accessor for the class whose
vtable is `D_8006D4E8` (29 slots, header word `0x13`, resolved with
`tools/classtable.py D_8006D4E8`). Slot map:

- `+0x004` `func_800269F0` (own-class slot, shared with `D_8006D430` at the
  identical offset)
- `+0x008` `func_80027228` (ctor, by the project's `+0x008` convention)
- `+0x00C` `func_80027274` (dtor)
- `+0x010`..`+0x038` the 13 inherited `BasicClass__func_*` slots, verbatim
- `+0x040`..`+0x074` own slots, including `Class6D4E8__RequestLoadFile`/`Class6D4E8__StopCdService`/
  `Class6D4E8__CancelRequests` (this unit's next three queued functions, at `+0x06C`/
  `+0x070`/`+0x074`)

Compared against `D_8006D430` (`include/code_171e0.h`'s
`UnkFlagsObjMethods_171e0`) with `classtable.py D_8006D4E8 --vs D_8006D430`:
`func_800269F0` at `+0x004` and `func_80026C20`/`func_80026C80`/
`func_80026C88` at identical offsets (`+0x05C`/`+0x060`/`+0x064`) are shared
between the two tables, strongly suggesting `D_8006D4E8`'s class is a
subclass or close sibling of `D_8006D430`'s, inheriting the same BasicClass
slot block and several of the same concrete method implementations.

`GetClass6D4E8Methods` itself is the same "return my own vtable's address"
accessor the project already names elsewhere: `func_800269E0` for
`D_8006D3C8` and `func_80026C9C` for `D_8006D430` (both in
`include/code_171e0.h`'s doc comment).

## The C

```c
/* D_8006D4E8's own method table -- see class-identity note above. */
extern s32 D_8006D4E8[];

s32 *GetClass6D4E8Methods(void)
{
    return D_8006D4E8;
}
```

## Why `lui`/`addiu`, not `%gp_rel`

`D_8006D4E8` lives in `.data` (confirmed in `asm/data/5DB70.data.s`), not
`.sdata`, so retail takes its address with an absolute `lui $v0,
%hi(D_8006D4E8)` / `addiu $v0, $v0, %lo(D_8006D4E8)` pair rather than a
`$gp`-relative load. Declaring it `extern s32 D_8006D4E8[];` and returning
the array (which decays to its address) reproduces that exactly.

## Provenance

round 45 (2026-09-15), runner echo, unit code_179d8_q (fresh carve).
