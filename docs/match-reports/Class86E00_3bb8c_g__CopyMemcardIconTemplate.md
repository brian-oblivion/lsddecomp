# Class86E00_3bb8c_g__CopyMemcardIconTemplate — MATCHED (round 45, 60/60 words)

> Renamed from `func_800507F8` on 2026-09-23 (tools/rename.py). Address 0x800507f8.

**Unit:** class_3bb8c_g · **Size:** 60 words (0xF0 bytes)

Filed as a `gp_rel`-blocked stub in round 14. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile).

## Derivation

```c
extern s32 atoi(char *s);
extern u8 *D_8008AAC4;

typedef struct {
    s8 raw[6];
} Buf6_3bb8c_g;
typedef struct {
    s8 raw[12];
} Buf12_3bb8c_g;
typedef struct {
    s8 a, b;
} Pair2_3bb8c_g;

s32 Class86E00_3bb8c_g__CopyMemcardIconTemplate(s32 arg0, s32 arg1)
{
    u8 *self = (u8 *)arg0;
    u8 *src = (u8 *)arg1;
    s32 t0;
    s32 idx;
    u8 *p;

    if (src != NULL) {
        t0 = ((u32)(src[0xE] - 0x38) < 2) ? 0xE : 0xD;

        *(Pair2_3bb8c_g *)(self + 0x18) = *(Pair2_3bb8c_g *)(D_8008AAC4 + 0x1E);
        *(Buf12_3bb8c_g *)(self + 0x6) = *(Buf12_3bb8c_g *)(D_8008AAC4 + 0x1E);

        idx = atoi((char *)(src + t0)) - 1;
        p = D_8008AAC4 + idx * 2;
        *(Pair2_3bb8c_g *)(self + 0x8) = *(Pair2_3bb8c_g *)p;
        return (s32)p;
    } else {
        u8 *q = D_8008AAC4;

        *(Buf6_3bb8c_g *)(self + 0x6) = *(Buf6_3bb8c_g *)(q + 0x1E);
        return (s32)q;
    }
}
```

**Signature was NOT free to choose.** `include/class_3bb8c.h` already
carries `extern s32 Class86E00_3bb8c_g__CopyMemcardIconTemplate(s32 arg0, s32 arg1);` (`class_3bb8c_m`'s
own caller, `TaskObjF__WriteMemcardSaveFile`), visible in this same translation unit via the
shared header, so the definition here has to match it exactly
(`conflicting types` otherwise) even though every real use inside the body
is pointer arithmetic. Cast `arg0`/`arg1` to `u8 *` locally instead.

**Three struct-copy shapes, all instances of this round's alignment lever**
(`FormatNumberIntoBuffer`'s report): declare the copied range ALL-`s8` so its
alignment is 1, which is what makes GCC use the unaligned `lwl`/`lwr` word
chunk(s) retail has, with any leftover non-multiple-of-4 bytes as
individual loads/stores rather than a merged halfword.
- `Pair2_3bb8c_g` (2 bytes) — used twice: copying the raw 2-byte prefix
  `D_8008AAC4[0x1E..0x20)` into `self[0x18..0x1A)`, and copying a 2-byte
  entry out of a `D_8008AAC4`-relative lookup table (indexed by
  `atoi(...)  - 1`, doubled) into `self[0x8..0xA)`.
- `Buf12_3bb8c_g` (12 bytes, exactly 3 word chunks, no tail) — the `src !=
  NULL` path's bulk copy `self[0x6..0x12) = D_8008AAC4[0x1E..0x2A)`.
- `Buf6_3bb8c_g` (6 bytes, one word chunk + 2 tail bytes) — the `src ==
  NULL` path's shorter copy `self[0x6..0xC) = D_8008AAC4[0x1E..0x24)`,
  identical shape to `FormatNumberIntoBuffer`'s own struct this round.

**The one register-identity trap, closed on the third attempt:** the
function's return value is a POINTER into the `D_8008AAC4` template
(confirmed from retail's own register content at `jr $ra` — whichever
branch runs, `$v0` still holds a `D_8008AAC4`-derived pointer, never
reloaded fresh at the very end). Writing `return (s32)D_8008AAC4;` as a
fresh expression in the `else` branch cost one extra word: the compiler
reloads the global via a second `%gp_rel` `lw` rather than reusing the
value already sitting in a register from the struct-copy statement just
above it. The fix was a local pointer variable holding the SAME value,
reused for both the copy and the return — but that variable had to be
scoped to the `else` block alone (`u8 *q = D_8008AAC4;` declared at the top
of that block, not the function's own top-level locals): sharing ONE
function-wide local across both branches for two semantically different
pointers (the template base in one branch, an indexed lookup pointer in
the other) forced the compiler to keep a consistent register for it across
the WHOLE function, which shuffled every other register assignment and cost
far more than it saved (36/60 instead of 59/60 on that attempt).

### Proposed learning

When retail's exit-time register already holds the value you need to
return, don't write a fresh global-read expression for the `return`
statement — reuse whatever local already holds it (computed earlier in the
SAME block) so the compiler doesn't emit a second reload. But scope that
reused local NARROWLY: a function-wide local shared across an if/else for
two DIFFERENT pointer values (even if both ultimately return through the
same statement shape) pins one register for the variable's entire lifetime
across the whole function and can cost far more in register-allocation
ripple than the reload it was meant to save. Prefer a block-scoped local
declared at the top of just the branch that needs it.
