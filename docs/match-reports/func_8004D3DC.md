# func_8004D3DC

**Unit:** class_3bb8c_c · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

The constructor (`ctor`, slot +0x008) for `Class86AA0`. Chains to a base
ctor (fetched via `GetClass6B5CCMethods(self)`), installs this class's own vtable,
then zeroes three of its own fields (`unk34` u16, `unk36` u16, `unk38`
s32) directly -- unlike func_8004D2A4's sibling ctor, there is no
post-construct hook call here (the retail instruction stream ends right
after the zero-stores).

## The C

```c
void func_8004D3DC(Class86AA0 *self)
{
    GetClass6B5CCMethods(self)->ctor(self);
    self->methods = func_8004D508();
    self->unk34 = 0;
    self->unk36 = 0;
    self->unk38 = 0;
}
```

## Notes on GetClass6B5CCMethods's declared arity

`GetClass6B5CCMethods` is already declared elsewhere in the codebase
(`include/class_3ac78.h`) with a two-argument signature,
`void *GetClass6B5CCMethods(Class866E8 *self, s32 arg1)`. This unit's own call
site never sets up a second argument register (`$a1`) before the `jal` --
the instruction immediately after is a plain `lw` on the return value, not
an `addu $a1, ...` -- so it is declared here, file-locally, as single-
argument: `extern BaseCtorTable_3bb8c_c *GetClass6B5CCMethods(void *self);`. This
is safe: each translation unit gets its own extern prototype for a given
external symbol in this project (no shared declaration is enforced across
units), and the only thing that has to be right for THIS unit's codegen to
match is what THIS call site's own register usage requires.

## Proposed learning

> **Head correction, round 9.** The advice below is right about codegen and
> wrong about the function, and the difference matters. Corrected version
> first; the runner's original is kept under it because its practical half is
> what produced the match.

**Match your own call site — but a disagreement between two units'
declarations of one symbol means nobody has established the real signature
yet, and settling it is one `cat` away: read the CALLEE.**

Here that read settles it flatly. `GetClass6B5CCMethods`'s entire body
(`asm/code_d294.s`) is:

```
lui   $v0, %hi(D_8006B5CC)
addiu $v0, $v0, %lo(D_8006B5CC)
jr    $ra
 nop
```

It reads **neither `$a0` nor `$a1`**. It takes **no arguments** and returns
`&D_8006B5CC` — the plain no-parameter vtable getter already documented in
`docs/research/class-framework.md`, the same shape as `GetClass6D3C8Methods`. So the
2-argument declaration in `src/class_3ac78.c` and the 1-argument declaration
in `include/class_3bb8c.h` are **both wrong about the function**, and both are
**right about their own call site**, and both units are byte-exact.

**That is the actual finding, and it is more interesting than an arity
mismatch:** retail's own source called one zero-argument getter with two
arguments from one file and one argument from another. That is what C89 does
when no prototype is in scope — the call passes whatever is written and
nothing checks it — so this is direct evidence about how the original was
organised, not an inconsistency to tidy up.

**Consequences, now annotated at both declaration sites so nobody undoes
them:**

- Do NOT reconcile the two declarations, and do NOT reduce either to `(void)`.
  The declared arg list is what makes the caller emit its argument setup;
  changing it changes the bytes and breaks the match.
- When you meet a helper like this, read the callee before writing a
  signature. If it ignores its arguments, expect the call sites to disagree
  and expect each to need its own local declaration.

### The runner's original wording (superseded)

> When a project-wide helper (a base-ctor getter, an allocator, etc.) is
> called with a smaller argument list at one site than another unit already
> declares for it, trust the instruction stream at YOUR call site over the
> other unit's declaration -- match arity to observed register setup, not to
> consistency with a sibling file's extern prototype for the same symbol.

The practical instruction is sound. What it left out is that a divergence is a
signal the real signature is unknown, and that the callee settles it cheaply —
without which two wrong declarations sit in the tree looking like a resolved
question.
