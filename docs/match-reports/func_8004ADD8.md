# func_8004ADD8 — MATCH

**Unit:** class_3ac78 · **Size:** 51 instructions · **Result:** 51/51 words

## What it does

`Class866E8Methods` slot `+0x0D0` (`slotD0`, already documented as such by
`func_8004AB88`'s comment before this round). Gates on `count`: only
proceeds for `count` in `{2,3}` or `[5,8]` (`count == 4` and `count >= 9`
both bail early, matching the four independent `slti`/branch checks
retail performs — NOT a single boolean expression, see below). If the gate
passes, walks `self->unkE8` as a NUL-terminated `s32` array of tag values;
for every entry equal to `list`'s own vtable header word
(`((GenericObject*)list)->methods->header`), calls
`self->methods->slot12C(self, list, count)`.

## Final source

```c
void func_8004ADD8(Class866E8 *self, void *list, s32 count)
{
    s32 *p;
    u8 unused[24];

    if (count < 2)
        return;
    if (count < 4)
        goto scan;
    if (count >= 9)
        return;
    if (count < 5)
        return;

scan:
    p = (s32 *)self->unkE8;
    if (p == NULL)
        return;
    if (*p == 0)
        return;

    do {
        if (*p == ((GenericObject *)list)->methods->header) {
            self->methods->slot12C(self, list, count);
        }
        p++;
    } while (*p != 0);
}
```

`self->unkE8` was previously typed plain `s32` (set by `func_8004ADD0`,
already matched); this function reads it back as a pointer to a
NUL-terminated tag array. Left the FIELD's declared type as `s32` and cast
locally (`(s32 *)self->unkE8`) rather than changing the field type, since
`func_8004ADD0`'s parameter is genuinely just a raw word from its own
caller's perspective and changing it wasn't needed for either function to
match.

## Residue and the fix that closed it

First attempt reached 39/51 with an unusual signature: the loop body,
range-check branches, and call setup were ALL already byte-identical
(every mismatch fell in the prologue register-save offsets and the
matching epilogue restore offsets) — i.e. retail's frame is `-0x40` (64
bytes) where the straightforward translation produced `-0x28` (40 bytes),
a flat 24-byte (6-word) gap, with the same 4 callee-saved registers
(`$s0`-`$s3`) plus `$ra` in both.

This is the exact "unused local reserves stack space anyway" class
documented in `func_80065D64.md` and `func_80065AE0.md`
(DECOMPILATION_LEARNINGS territory, GCC 2.6.3-specific): **an entirely
dead, never-read-or-written local still gets a stack slot from this
compiler.** Adding `u8 unused[24];` (sized to exactly the missing 24
bytes, declared but never referenced) grew the frame from `-0x28` to
`-0x40` and closed every remaining word, all in one attempt.

### Proposed learning

**When every visible instruction already matches (branches, calls,
register choices) but the prologue/epilogue stack-adjust and save/restore
offsets are all off by a constant, don't touch the logic — pad the frame.**
Compute the gap (retail's `addiu sp,sp,-N` minus yours), add a `u8
unused[gap];` local that's never referenced, and rebuild. This is now the
THIRD confirmed instance of this exact idiom in the project (after
`func_80065D64`, `func_80065AE0`), reinforcing it as a general GCC 2.6.3
quirk rather than something specific to those two functions' unit.

## Provenance

round 2026-09-02, runner ALPHA, unit class_3ac78. First attempt 39/51
(pure frame-size gap, logic already exact); second attempt (padding local)
closed it, 51/51.
