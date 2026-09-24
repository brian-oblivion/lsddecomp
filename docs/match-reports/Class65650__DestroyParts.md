# Class65650__DestroyParts

> Renamed from `func_80065F2C` on 2026-09-24 (tools/rename.py). Address 0x80065f2c.

**Unit:** code_55dd4 · **Size:** 43 words (0xAC bytes) · **Status:** MATCHED
(43/43 words, whole-image `./build-and-verify.sh` green)

## What it does

The `+0x70`/`+0x74` array pair's teardown body (dispatched through
`slot_teardown70`, already known from `Class65650__TeardownParts`'s report). If both
arrays are non-NULL, releases each element via its own vtable slot `+0x004`
(a new slot on `Unk70ElemMethods`, distinct from the already-known
`+0x060`/`+0x070`), resets `self->unk68`, then unconditionally frees and
clears both array pointers:

```c
void Class65650__DestroyParts(Class65650 *self)
{
    Unk70ElemObj **p;

    if (self->unk70 != NULL && self->unk74 != NULL) {
        p = self->unk70;
        while (self->unk6C-- > 0) {
            (*p)->methods->slot4(*p);
            p++;
        }
        self->unk68 = 0;
    }
    self->unk74 = BMemPMgrFree(self->unk74);
    self->unk70 = BMemPMgrFree(self->unk70);
}
```

## The loop shape: post-decrement in the condition, not a separate decrement statement

The key thing that made this a one-attempt match: `self->unk6C` is
reloaded fresh from memory on EVERY condition check, including the very
FIRST one (before the loop body has run even once), and each check both
*reads* the current value for the comparison AND *stores* the
already-decremented value back, before the branch decides whether to
enter the body. That is exactly `while (self->unk6C-- > 0)`, not
`while (self->unk6C > 0) { ...; self->unk6C--; }` — the latter would
decrement only after entering the body, but retail's `bgtz` delay slot
stores the decrement UNCONDITIONALLY, even on iterations that don't run
(this is visible directly in the disassembly: the guard-check block at
`.L80065F90` and the loop-body's own re-check are the SAME basic block,
reached once before the first iteration via an unconditional `j`, and
again after each call — the decrement-and-store is baked into that one
shared block, which only makes sense if the decrement is part of the loop
CONDITION expression itself). This is the "post-decrement in a loop
guard" sibling of the project's already-documented "`while (*p++)` vs.
`while (*p) { p++; }`" idiom (`strcat`, `docs/DECOMPILATION_LEARNINGS.md`),
just on a struct field instead of a pointer.

`self->unk6C` has to be reloaded fresh each time (rather than cached in a
local across the `(*p)->methods->slot4(*p)` call) because that call is an
indirect dispatch through an unknown vtable slot — the compiler can't
prove it doesn't alias `self->unk6C`, so it conservatively reloads. No
special handling was needed to get this: writing the natural
`while (self->unk6C-- > 0)` reproduced it directly.

The trailing two frees follow the by-now-familiar
`self->fieldA = BMemPMgrFree(self->fieldA); self->fieldB = BMemPMgrFree(self->fieldB);`
shape, and GCC's own scheduler deferred the first call's result-store into
the second call's delay slot without any source-level hinting needed.

### Proposed learning

**A loop guard that re-reads and re-stores a struct field on every check,
including the very first, is `while (field-- > 0)`, not a `while` with a
separate trailing decrement.** The tell in the disassembly: the
loop-entry test and the end-of-iteration test are the SAME basic block
(one unconditional jump into a shared test, not two separate blocks) — if
they were architecturally distinct, a plain "check then separately
decrement in the body" would be more likely. This is a mechanical
generalization of the project's known `p++`-in-condition idiom to a
"field-in-condition" case; add it if this recurs.
