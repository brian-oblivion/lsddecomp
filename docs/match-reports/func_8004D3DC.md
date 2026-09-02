# func_8004D3DC

**Unit:** class_3bb8c_c · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

The constructor (`ctor`, slot +0x008) for `Class86AA0`. Chains to a base
ctor (fetched via `func_8001E57C(self)`), installs this class's own vtable,
then zeroes three of its own fields (`unk34` u16, `unk36` u16, `unk38`
s32) directly -- unlike func_8004D2A4's sibling ctor, there is no
post-construct hook call here (the retail instruction stream ends right
after the zero-stores).

## The C

```c
void func_8004D3DC(Class86AA0 *self)
{
    func_8001E57C(self)->ctor(self);
    self->methods = func_8004D508();
    self->unk34 = 0;
    self->unk36 = 0;
    self->unk38 = 0;
}
```

## Notes on func_8001E57C's declared arity

`func_8001E57C` is already declared elsewhere in the codebase
(`include/class_3ac78.h`) with a two-argument signature,
`void *func_8001E57C(Class866E8 *self, s32 arg1)`. This unit's own call
site never sets up a second argument register (`$a1`) before the `jal` --
the instruction immediately after is a plain `lw` on the return value, not
an `addu $a1, ...` -- so it is declared here, file-locally, as single-
argument: `extern BaseCtorTable_3bb8c_c *func_8001E57C(void *self);`. This
is safe: each translation unit gets its own extern prototype for a given
external symbol in this project (no shared declaration is enforced across
units), and the only thing that has to be right for THIS unit's codegen to
match is what THIS call site's own register usage requires.

## Proposed learning

When a project-wide helper (a base-ctor getter, an allocator, etc.) is
called with a smaller argument list at one site than another unit already
declares for it, trust the instruction stream at YOUR call site over the
other unit's declaration -- match arity to observed register setup, not to
consistency with a sibling file's extern prototype for the same symbol.
