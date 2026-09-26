# GetClass6D3C8Methods

> Renamed from `func_800269E0` on 2026-09-18 (tools/rename.py). Address 0x800269e0.

**Unit:** code_171e0 · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `D_8006D3C8`, a 25-slot hand-rolled-class method table
(per `tools/classtable.py`, header word `0x00001F60`). It is a "get the method
table" accessor, the same shape as `Get_vtable_DreamSys` in `include/DreamSys.h`.

## Derivation

```
lui   $v0, %hi(D_8006D3C8)
jr    $ra
 addiu $v0, $v0, %lo(D_8006D3C8)
```

Just an address computation, no load — this is `&D_8006D3C8`, not
`*D_8006D3C8`. Confirmed by its one caller, `New_Class6D3C8` in
`asm/nonmatchings/code_1677c/New_Class6D3C8.s`: it calls this function, then
does `lw $v0, 0x8($v0)` on the result and `jalr`s that — fetching the
constructor slot (`+0x008`, `Class6D3C8__Class6D3C8`) from the table this function
returned, exactly the "allocate, get methods, call ctor slot" idiom from
CLAUDE.md's "Writing a class method".

```c
extern s32 D_8006D3C8[];

void *GetClass6D3C8Methods(void) {
    return D_8006D3C8;
}
```

`D_8006D3C8` is declared `s32[]` (not typed as the owning class's vtable
struct) because that struct doesn't exist yet — the table itself still lives
in `asm/data/57070.data.s` as raw words, owned by neither this unit nor any
carved one yet. Whoever carves that data slot should replace this `extern`
with a proper vtable-typed one.

## Proposed learning

A function that only does `lui/addiu` to a symbol with no `lw`/`sw` around it
is returning `&symbol`, not a value read from it — worth checking who calls it
before guessing a dereferencing signature.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800269E0` | `GetClass6D3C8Methods` | A |

**Evidence.** A two-instruction address-of returning `&D_8006D3C8`. `Class6D3C8`
is already an established type name in `src/code_1677c.c` (that unit's own
functions are typed against it), and the "return my own vtable" shape is
already named twice in this project (`GetCdDriverMethods`,
`GetClass6D430Methods`, this same round). Pure leaf whose mechanics are its
purpose.

## Track 4 (2026-09-26, round 88)

Retyped to `Class6D3C8Methods *GetClass6D3C8Methods(void)`, returning
`&D_8006D3C8`; both are declared once, in `include/Class6D3C8.h`
(`include/code_171e0.h`'s `extern s32 D_8006D3C8[]` view is deleted, and
New_Class6D3C8 no longer casts the result). Image byte-identical.
