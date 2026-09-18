> Renamed from `func_80026C9C` on 2026-09-18 (tools/rename.py). Address 0x80026c9c.

# GetClass6D430Methods

**Unit:** code_171e0 · **Size:** 4 instructions · **Status:** MATCHED (4/4 words)

## What it does

Returns the address of `D_8006D430`, a 44-slot method table (header word
`0x00000003`) — the vtable of the class most of this unit's own functions
belong to. Same "get methods" shape as `GetClass6D3C8Methods` immediately above,
just for a different class.

## Derivation

```
lui   $v0, %hi(D_8006D430)
jr    $ra
 addiu $v0, $v0, %lo(D_8006D430)
```

```c
extern s32 D_8006D430[];

void *GetClass6D430Methods(void) {
    return D_8006D430;
}
```

Confirmed via `tools/classtable.py 0x8006D430`: slots `+0x004`/`+0x008`/`+0x00C`
are this unit's own `DestroyChained`/`Class6D430__Class6D430`/`Class6D430__Destroy` (dtor /
ctor-by-the-`+0x008`-convention / a third override), and `+0x058`.`+0x064`
are `Class6D430__AllocBuffer`..`Class6D430__SetFlag`, also this unit. So this function is the
"get my own class's methods" accessor for the class that owns roughly a third
of `code_171e0`'s remaining queue — worth knowing for whoever picks up
`DestroyChained`, `Class6D430__Class6D430`, `Class6D430__Destroy`, `Class6D430__AllocBuffer`, or
`Class6D430__FreeBuffer` next: they all dispatch through this same table (see
`include/code_171e0.h` for the full slot map, and the constructor-called-via
BasicClass base (`D_8006B58C`, resolved with `--vs`) for the inherited slots).

## Proposed learning

None beyond what `GetClass6D3C8Methods`'s report already says about this shape.
