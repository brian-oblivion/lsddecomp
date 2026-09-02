# func_8003CCDC — STALL

**Unit:** code_2cc8c · **Size:** 27 instructions · **Best reached:** 26/27 words

## Signature update (after this report was first filed)

Originally attempted as `s32 func_8003CCDC(Obj86B60 *self, s32 a1)`, an
unused-but-forwarded parameter matching `func_8003CBC0`'s ORIGINAL
signature. `func_8003C51C`'s own residue (matched separately, see its
report) proved this class of assumption wrong for its sibling slot
`slotAC`/`func_8003CBC0`: the `s32 a1` at that call site was never a real
argument, just a leftover caller-saved register value from an earlier,
unrelated call. `Obj86B60Methods::slotC0` (this function's own vtable
slot, `+0x0C0`) has been retyped to `s32 (*)(Obj86B60*)` (one argument) to
match. **This function itself was never rebuilt after the retype** (it is
still `INCLUDE_ASM`), so the residue analysis below (based on the
two-argument attempt) may already be partly stale -- whoever resumes this
function should re-attempt with the corrected one-argument signature
FIRST, since it is not yet known whether dropping the unused parameter
changes anything about the `li`-vs-`move` residue documented below.

## What it does

The `self->unk8C` counterpart to `func_8003CBC0`'s `self->unk88` (both set
by `func_8003CB30`/`func_8003CAF8` respectively). UNLIKE `func_8003CBC0`,
this function's control flow has a genuine extra early-exit: if the
callback returns 0, retail branches DIRECTLY to the shared epilogue,
skipping the `slot60` call entirely, rather than reaching the same skip via
a single unified `if (result != 0)` test the way `func_8003CBC0` does.

```c
s32 func_8003CCDC(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk8C != NULL) {
        result = self->unk8C(self);
        if (result == 0) {
            return result;
        }
    }
    self->methods->slot60(self, 8);
    return result;
}
```

## Progression (26/27, one word residue)

**Attempt 1: `func_8003CBC0`'s exact shape** (`result = 1; if (unk8C) result
= unk8C(self); if (result != 0) slot60(...); return result;`) scored
**23/27**, four words off, with a DIFFERENT branch immediate at the first
`beqz` and three more knock-on differences -- confirms this function's CFG
genuinely differs from `func_8003CBC0`'s (per DECOMPILATION_LEARNINGS'
"two structurally-similar residues want different C shapes" entry), not
just that the same C compiles slightly differently.

**Attempt 2: explicit early return inside the `if` block** (shown above)
scored **26/27** -- every branch target, every register, every instruction
OPCODE now matches retail, except ONE: at the early-return site, retail
computes the return value with `addu $v0,$s1,$zero` (`move v0,s1` -- copy
the already-known-zero register) where this compiles to `li $v0,0x0`
(load the immediate 0 directly). Both write the identical VALUE (0) to the
identical REGISTER (`v0`); the difference is purely in how the constant
gets there.

## Why this is a stall, not a solved residue

**Confirmed with an isolated reproducer that this is deterministic, not
project-context-dependent.** Compiling the exact function above (with the
real `Obj86B60`/`code_2cc8c.h` header, matching the actual build's
CPP/CC1/maspsx/as invocation) through the pinned toolchain in isolation
reproduces `li v0,0` byte-for-byte identically to the full project build.
Three further C-level rewrites were tried, all in isolation first to avoid
burning full-build attempts:
1. `if (!result) return result;` instead of `if (result == 0)` -- identical
   codegen, still `li v0,0`.
2. Moving the `if (result == 0) return result;` check OUTSIDE the
   `if (self->unk8C != NULL)` block (applying to both paths uniformly) --
   compiles to a LONGER, restructured body with an extra `j` and a
   relocated shared return, not closer to retail.
3. Splitting the callback's raw return into its own `ret` local, checked
   before ever touching `result` -- also longer (an extra `bnez`+`j`), not
   closer.

None reproduces retail's `move v0,s1`. GCC 2.6.3's constant propagation at
`-O2` appears to ALWAYS fold "value that was just compared `== 0` in the
immediately preceding branch, then returned on that same path" down to an
immediate load, regardless of how the comparison or the return is spelled
in C, for every shape tried. Whatever retail's actual source does to avoid
this, it was not found in ~8 attempts (5 in isolation, 3 against the real
build). Per CLAUDE.md's guidance on register-identity mismatches (which
this is adjacent to -- same register, different VALUE-MATERIALISATION
strategy) and the explicit ban on `register T v asm("$N")`/operand
constraints to force it, this is filed as a stall rather than continuing to
search blind.

## Best-reached body (26/27, does NOT compile to retail bytes)

```c
#if 0
s32 func_8003CCDC(Obj86B60 *self)
{
    s32 result;

    result = 1;
    if (self->unk8C != NULL) {
        result = self->unk8C(self);
        if (result == 0) {
            return result;
        }
    }
    self->methods->slot60(self, 8);
    return result;
}
#endif
```

### Proposed learning

`func_8003CBC0`/`func_8003CCDC` are the SAME idiom (a callback stored by a
matching setter, defaulting to 1, invoked and conditionally followed by a
`slot60` reason-code call) at two different offsets in the same class, and
they needed genuinely DIFFERENT C shapes despite that -- `func_8003CBC0`
wanted a flat `if (result != 0) call();`, `func_8003CCDC` wanted an
explicit early `return` nested inside the callback-present branch. Do not
assume the second instance of a matched idiom is a free win; check its own
branch targets first (this one differs at word 0 already). Separately: a
`return` of a locally-scoped value that the immediately preceding branch
already proved to be a specific constant is a real, reproducible spot
where GCC 2.6.3 at `-O2` prefers `li` over reusing the register that
already holds it -- confirmed with an isolated single-function reproducer,
independent of this unit's own header/struct. Worth a permuter run if this
function is revisited (a single-instruction "materialise a known constant
via a register copy instead of an immediate load" residue is exactly the
shape `new_class_6d3c8`/`strcat` fell to in round 8 -- not tried here for
time, but recommended first move for whoever picks this back up).

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 8 attempts (3 against the
real build, 5 in an isolated reproducer). Restored to `INCLUDE_ASM`.
