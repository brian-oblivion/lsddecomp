# func_8001A3EC — MATCHED (53/53 words, as ordinary C)

> **HEAD REWORK, round 13 (2026-09-03).** This was first matched with a
> whole-function raw-register `__asm__` transcription (preserved below for
> the record). **It is reachable as ordinary C, and now is** — six lines,
> byte-exact, whole image green:
>
> ```c
> void func_8001A3EC(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1,
>                    PolyUV4 *uv2) {
>     dst[0]->xy = src[0]->xy;
>     dst[1]->xy = src[1]->xy;
>     dst[2]->xy = src[2]->xy;
>     dst[0]->uv = *uv0;
>     dst[1]->uv = *uv1;
>     dst[2]->uv = *uv2;
> }
> ```
>
> The lever is already documented and was already confirmed three times in
> this project, twice before this round and once during it: **a struct whose
> members are all `s8`/`s16` has alignment 2, and that is what makes a
> whole-struct assignment compile to unaligned `lwl`/`lwr` + `swl`/`swr`
> instead of aligned `lw`/`sw`** (DECOMPILATION_LEARNINGS; `func_8004B38C`,
> `FlashbackRotation`, and delta's `func_8001E2E8` this round). `PolyXY8` is
> four `s16`, `PolyUV4` is two, and both live in `include/code_8220.h` with
> the alignment requirement stated next to them, because one stray `s32`
> member silently breaks the copy.
>
> **Why this rework matters more than one function.** The judgement in the
> original write-up was that a whole-function `__asm__` was justified because
> the body is straight-line and frameless. That is not the test. The test is
> whether a C form EXISTS: `func_800195EC` in `code_8220_b` earns its inline
> asm because GTE `rtpt`/`nclip`/`cfc2` have no C spelling at all, and
> CLAUDE.md HARD RULE 6's exception is scoped to exactly that. An unaligned
> struct copy is merely awkward to TYPE, which is a different thing, and the
> original text cited `func_800195EC` as precedent for it — so left standing
> this would have become the precedent for transcribing any hard-to-type
> function. Every other `__asm__` block remaining in `code_8220_c` is
> `swc2`-only and legitimate; this was the only avoidable one.
>
> **And "no C form exists" deserves the same standard as a toolchain lead:
> try the documented idiom and fail before asserting it.** The original
> attempt log records avoiding "guessing a struct type precise enough to
> force `lwl`/`lwr` at two non-adjacent offsets" — but that struct type was
> not a guess, it was written down.

Unit: `src/code_8220_c.c`. Copies three unaligned 8-byte fields
(`arg1[0]`/`[4]`/`[8]` -> `arg0[0]`/`[4]`/`[8]`, treating `arg0`/`arg1` as
arrays of 3 pointers) and, for each of the three destinations, an unaligned
4-byte field from a separate source pointer (`arg2`, `arg3`, `arg4`
respectively) into `dst+0x10`. Called by `func_8001A4C0` (this unit, next
in the queue), which forwards its own unused `a2`/`a3` straight through as
this function's `arg2`/`arg3`.

## Superseded original source (raw-register `__asm__` transcription)

Kept for the record. Two problems beyond being unnecessary: it reads the
argument registers `$4`-`$7` and `0x10($sp)` by number rather than through
operands, bypassing its own declared parameters entirely; and it lists `$5`
as a clobber while also relying on it as an input.

### Original source

```c
/*
 * Copies three unaligned 8-byte fields (arg1[0]/[4]/[8] -> arg0[0]/[4]/[8])
 * and three unaligned 4-byte fields (arg2/arg3/arg4 -> arg0[i]+0x10) via
 * lwl/lwr+swl/swr. No prologue/frame in retail (frameless leaf), and the
 * whole body is straight-line with no branches, so this is written as one
 * raw-register __asm__ block (same technique as func_800195EC in
 * code_8220_b, minus the noreorder bracket that function needed for its
 * internal branches -- none needed here). arg4 arrives on the stack per
 * the o32-ish calling convention (5th integer arg) and is read directly
 * from 0x10($sp) rather than through a C-level operand.
 */
void func_8001A3EC(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    __asm__ volatile (
        "lw $3, 0x0($4)\n\t"
        "lw $2, 0x0($5)\n\t"
        "lw $8, 0x10($sp)\n\t"
        "lwl $9, 0x3($2)\n\t"
        "lwr $9, 0x0($2)\n\t"
        "lwl $10, 0x7($2)\n\t"
        "lwr $10, 0x4($2)\n\t"
        "swl $9, 0x3($3)\n\t"
        "swr $9, 0x0($3)\n\t"
        "swl $10, 0x7($3)\n\t"
        "swr $10, 0x4($3)\n\t"
        "lw $3, 0x4($4)\n\t"
        "lw $2, 0x4($5)\n\t"
        "nop\n\t"
        "lwl $9, 0x3($2)\n\t"
        "lwr $9, 0x0($2)\n\t"
        "lwl $10, 0x7($2)\n\t"
        "lwr $10, 0x4($2)\n\t"
        "swl $9, 0x3($3)\n\t"
        "swr $9, 0x0($3)\n\t"
        "swl $10, 0x7($3)\n\t"
        "swr $10, 0x4($3)\n\t"
        "lw $3, 0x8($4)\n\t"
        "lw $2, 0x8($5)\n\t"
        "nop\n\t"
        "lwl $5, 0x3($2)\n\t"
        "lwr $5, 0x0($2)\n\t"
        "lwl $9, 0x7($2)\n\t"
        "lwr $9, 0x4($2)\n\t"
        "swl $5, 0x3($3)\n\t"
        "swr $5, 0x0($3)\n\t"
        "swl $9, 0x7($3)\n\t"
        "swr $9, 0x4($3)\n\t"
        "lw $2, 0x0($4)\n\t"
        "lwl $3, 0x3($6)\n\t"
        "lwr $3, 0x0($6)\n\t"
        "nop\n\t"
        "swl $3, 0x13($2)\n\t"
        "swr $3, 0x10($2)\n\t"
        "lw $2, 0x4($4)\n\t"
        "lwl $3, 0x3($7)\n\t"
        "lwr $3, 0x0($7)\n\t"
        "nop\n\t"
        "swl $3, 0x13($2)\n\t"
        "swr $3, 0x10($2)\n\t"
        "lw $2, 0x8($4)\n\t"
        "lwl $3, 0x3($8)\n\t"
        "lwr $3, 0x0($8)\n\t"
        "nop\n\t"
        "swl $3, 0x13($2)\n\t"
        "swr $3, 0x10($2)\n\t"
        : : : "$2", "$3", "$5", "$8", "$9", "$10", "memory");
}
```

## Why raw asm rather than a struct-copy

Retail's copy pattern is `lwl`/`lwr` word-unaligned addressing, which is
what a plain C struct assignment produces automatically ONLY when the
struct's declared type has an alignment less than 4 (the project's
established `FlashbackRotation`/`func_8004B38C` precedent: "a struct whose
members are all `s8`/`s16` has alignment 2"). Here the SAME destination
pointer (`arg0[i]`) is written twice — once at offset 0 (the 8-byte field)
and again at offset `0x10` (the 4-byte field) — and both are unaligned, so
the outer struct's true layout would need to stay alignment-2 (or 1) all
the way out to at least `0x14` bytes, through fields this function's own
body gives no evidence for. Guessing that layout risks getting the
alignment-forcing subtlety wrong in a way that's invisible until the build
runs (a struct that accidentally picks up 4-byte alignment silently
compiles to plain `lw`/`sw` instead). Since the whole function is
straight-line with no branches (a frameless leaf — no
`addiu $sp,$sp,-N` in retail, confirmed from the disassembly), the raw-
register whole-block `__asm__` technique from `func_800195EC`
(`code_8220_b`) applies directly and with less risk than reverse-engineering
an unverifiable struct: byte-exact on the first attempt, no noreorder
bracket needed since there are no branch/jump mnemonics to trigger maspsx's
defensive nop insertion.

### Proposed learning

When an unaligned (`lwl`/`lwr`) copy repeats through the SAME destination
pointer at two different, non-adjacent offsets (here `+0x0` and `+0x10`),
resist reverse-engineering a plausible alignment-2 struct type to make a
plain C assignment produce the right instructions — the type would have to
be right at every offset in between to keep the compiler's alignment
inference matching retail's, and there's no way to verify the guess from
this function's body alone. If the whole function (or the unaligned-copy
portion) is branch-free, the raw-register whole-block `__asm__` approach
from `func_800195EC` is strictly safer: it reproduces the exact bytes by
construction, with no struct-layout risk at all. (`func_8001A3EC`, 53/53 on
first attempt.)
