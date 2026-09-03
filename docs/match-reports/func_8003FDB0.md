# func_8003FDB0 -- MATCH (31/31 words, first attempt)

Unit `code_2cc8c_e`, carved round 14.

`New_Class6E99C`-shaped allocator (the checked variant returns unconditionally
regardless of the ctor's own success -- see below):

```c
Class6E99CObj *func_8003FDB0(void *a1, s32 a2, s32 a3) {
    Class6E99CObj *self;

    self = func_80017B34(0xA0);
    if (self != NULL) {
        func_800404C0()->ctor(self, a1, a2, a3);
    }
    return self;
}
```

## Class discovery

This function anchors a THIRD independent class pair in this segment.
`tools/classtable.py D_8006E99C --vs D_8006B58C` shows `D_8006E99C` shares
its dtor (+0x00C) and three more slots (+0x010/+0x014/+0x018), plus all
seven BasicClass-inherited slots (+0x01C..+0x038), with `D_8006B5CC`
(code_d294.h's own `Class6B5CCMethods`) -- so `D_8006E99C` is a
`Class6B5CCObj` descendant, same fingerprint code_d294.h already
established for its own class. Full writeup and the "ClassEAC0"
intermediate class it turned out to sit on top of: see the header comment
in `include/code_2cc8c.h` right above `struct ClassEAC0Obj`.

`func_80017B34` is already declared project-wide (`include/code_2cc8c.h`,
confirmed across many allocators).

## Existing-declaration retype -- affects TWO already-matched call sites

`func_8003FDB0` was ALREADY forward-declared in `include/code_2cc8c.h`
(`extern SubHandleObj *func_8003FDB0(void *name, s32 arg1, s32 arg2);`),
used by two ALREADY-MATCHED functions in OTHER units:

- `src/code_2cc8c_c.c:183`: `obj = func_8003FDB0(D_8008A90C, 0, 0);`
- `src/Entity.c:48`: `sub = func_8003FDB0(name, 0, arg4);`

`include/Entity.h` also independently declares the SAME symbol as
`Unk100Obj *func_8003FDB0(void *name, s32 arg1, s32 arg2);` for its own
unit's use (Entity's `unk100`/`unk104` fields). Its slot layout (`+0x04`,
`+0x4C`, `+0x50`, `+0xD0`, `+0xD4`, `+0xD8`) lines up with the SAME offsets
I derived independently for `Class6E99CMethods` (`+0xD0`/`+0xD4`/`+0xD8` =
`func_8004001C`/`func_80040024`/`func_800400B0`, all this unit) -- strong
cross-unit confirmation this is genuinely the same class, seen from three
independent angles.

Retyped the header's parameter list to match the real definition
(`void *a1, s32 a2, s32 a3` -- unchanged from the existing declaration's
own arity/types except `name`->`a1`, purely cosmetic) and the RETURN type
to `Class6E99CObj *` (this unit's own view). Both existing callers keep
their OWN separately-typed local variable (`SubHandleObj *obj` /
`Unk100Obj *sub`), so this only changes an implicit pointer-conversion
warning at the assignment, not the compiled bytes -- to be confirmed with a
full `./build-and-verify.sh` once this unit's own remaining queue is done
(their object files do not get rebuilt again by anything in this unit's own
remaining work, so their own byte-exactness cannot regress from a
type-only header change either way).
