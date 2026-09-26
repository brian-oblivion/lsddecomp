# GetFileResourceMethods

> Renamed from `GetClass6D430Methods` on 2026-09-26 (tools/rename.py). Address 0x80026c9c.

> Renamed from `func_80026C9C` on 2026-09-18 (tools/rename.py). Address 0x80026c9c.

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

void *GetFileResourceMethods(void) {
    return D_8006D430;
}
```

Confirmed via `tools/classtable.py 0x8006D430`: slots `+0x004`/`+0x008`/`+0x00C`
are this unit's own `FileResource__Release`/`FileResource__FileResource`/`FileResource__Finalize` (dtor /
ctor-by-the-`+0x008`-convention / a third override), and `+0x058`.`+0x064`
are `FileResource__LoadFile`..`FileResource__SetFlag`, also this unit. So this function is the
"get my own class's methods" accessor for the class that owns roughly a third
of `code_171e0`'s remaining queue — worth knowing for whoever picks up
`FileResource__Release`, `FileResource__FileResource`, `FileResource__Finalize`, `FileResource__LoadFile`, or
`FileResource__FreeBuffer` next: they all dispatch through this same table (see
`include/code_171e0.h` for the full slot map, and the constructor-called-via
BasicClass base (`D_8006B58C`, resolved with `--vs`) for the inherited slots).

## Proposed learning

None beyond what `GetClass6D3C8Methods`'s report already says about this shape.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026C9C` | `GetFileResourceMethods` | A |

**Evidence.** `return D_8006D430;` -- the same "get my own vtable" shape as
`GetClass6D3C8Methods` (this round) and `GetCdDriverMethods` (round 51).
Pure leaf, mechanics are its purpose.

## Proposed type names

Not renamed (types aren't `rename.py` symbols, and `FileResource`/
`FileResourceMethods` are used by `code_179d8_h.c` too -- same
cross-unit exposure as the fields below), but proposed for the head to
apply as a whole-tree replace, since a `ClassNNNN__Method` function prefix
(this round gave the class's own functions `FileResource__` names) reading
against a struct still called `FileResource` is an inconsistency this
unit's own header already predicted ("Not yet named or typed field-by-field"):

| type | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `FileResource` | `FileResource` | C | Matches this round's `FileResource__*` function-name token (the vtable's own address, per the project's existing `Class6B5CC`/`Class6D3C8`/`Class6D4E8` convention for a class whose real name is not yet established). |
| `FileResourceMethods` | `FileResourceMethods` | C | Same convention, applied to the method-table type. |

Posted to the broadcast.

## Extern arity (round 59)

**Verdict: extern FIXED.** The second of the round's two genuinely wrong
declarations; same shape and same discriminator as `GetClass6B5CCMethods`.

**Callee evidence** (`0x80026C9C`, and the definition in `src/code_171e0.c`):

```
80026c9c:  lui   v0,0x8007
80026ca0:  addiu v0,v0,-11216     ; &D_8006D430
80026ca4:  jr    ra
80026ca8:  nop
```

No argument register is read. The definition is
`void *GetFileResourceMethods(void)`.

**The extra argument is not byte-load-bearing.** `CdDriver__CdDriver`
(src/code_179d8_o.c) calls it as `GetFileResourceMethods(self)->ctor(self)`:

```
80027234:  jal   80026c9c <GetFileResourceMethods>
80027238:  move  s0,a0                 <- callee-save spill of its OWN incoming
                                          self, not argument setup
```

`$a0` still holds `CdDriver__CdDriver`'s incoming `self` at the `jal` either way, so
the declaration's parameter list is free and must agree with the definition.

**Declaration site changed:** `src/code_179d8_o.c:99` —
`extern BaseCtorTable6D4E8 *GetFileResourceMethods(void *self);` ->
`extern BaseCtorTable6D4E8 *GetFileResourceMethods();`. Return type untouched
(this unit's own local view of the table, used for `->ctor` at +0x008); the
call site is untouched. The other two declarations
(`src/code_171e0.c`'s definition and `include/code_171e0.h`'s `(void)`) were
already correct.

**Stale comment corrected on the same line:** it read "still INCLUDE_ASM in the
code_179d8 remainder". It is not — it has a matched definition in
`src/code_171e0.c`, which is what made this finding decidable at all.

Oracle green (`build exit=0`, `OK: build matches retail`) after the edit.
