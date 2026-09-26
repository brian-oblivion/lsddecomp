# StageMap__ForwardAcceptedCommand — MATCH

> Renamed from `Class866E8__ForwardAcceptedCommand` on 2026-09-26 (tools/rename.py). Address 0x8004add8.

> Renamed from `func_8004ADD8` on 2026-09-22 (tools/rename.py). Address 0x8004add8.

**Unit:** class_3ac78 · **Size:** 51 instructions · **Result:** 51/51 words

## What it does

`StageMapMethods` slot `+0x0D0` (`slotD0`, already documented as such by
`StageMap__DispatchLinkCommand`'s comment before this round). Gates on `count`: only
proceeds for `count` in `{2,3,5,6,7,8}` (`count < 2`, `count == 4`, and
`count >= 9` all bail early). If the gate passes, walks `self->unkE8` as a
NUL-terminated `s32` array of tag values; for every entry equal to
`list`'s own vtable header word
(`((GenericObject*)list)->methods->header`), calls
`self->methods->slot12C(self, list, count)`.

## Final source

```c
void StageMap__ForwardAcceptedCommand(StageMap *self, void *list, s32 count)
{
    s32 *p;
    u8 unused[24];

    switch (count) {
    case 2:
    case 3:
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    default:
        return;
    }

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

**Update (head-requested follow-up, one attempt):** the original match
used a four-branch `if`/`goto` chain (`if (count<2) return; if (count<4)
goto scan; if (count>=9) return; if (count<5) return;`) that reproduced
retail's exact `slti`/branch sequence but read as an opaque puzzle. Tried
rewriting the gate as the `switch` shown above — **it reaches the
identical 51/51 bytes, whole-image SHA1 still green, zero-cost.** Kept the
`switch` as final since it says what the gate actually IS (`count` is one
of six specific values) instead of encoding that as branch arithmetic.
This doesn't resolve what `count` enumerates or why `4` specifically is
excluded (see the note below), but it's a positive data point: the real
source could plausibly have been a `switch` over a small case set, which
is a narrower, more plausible hypothesis than an arbitrary compound
boolean.

`self->unkE8` was previously typed plain `s32` (set by `StageMap__SetAcceptedTags`,
already matched); this function reads it back as a pointer to a
NUL-terminated tag array. Left the FIELD's declared type as `s32` and cast
locally (`(s32 *)self->unkE8`) rather than changing the field type, since
`StageMap__SetAcceptedTags`'s parameter is genuinely just a raw word from its own
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
documented in `Class65650__FindPartIndex.md` and `Class65650__SetLightMode.md`
(DECOMPILATION_LEARNINGS territory, GCC 2.6.3-specific): **an entirely
dead, never-read-or-written local still gets a stack slot from this
compiler.** Adding `u8 unused[24];` (sized to exactly the missing 24
bytes, declared but never referenced) grew the frame from `-0x28` to
`-0x40` and closed every remaining word, all in one attempt.

**This local is evidence, not an explanation.** `u8 unused[24]` reproduces
retail's BYTES, but nothing says the real source declared a 24-byte
`char` buffer specifically — GCC 2.6.3 reserving a stack slot for a
provably-dead local most likely means the ORIGINAL source had some local
aggregate (a struct, a small array of a different element type, or
several separate locals whose combined size is 24 bytes) that a slightly
different reconstruction of THIS function's logic would actually use —
most plausibly by passing its address somewhere, which is the normal way
a local ends up needing a real stack slot even after the compiler could
otherwise prove it dead. The padding local closes the byte diff without
identifying what that real local was. Anyone revisiting this function (or
`StageMap__SetAcceptedTags`/`self->unkE8`'s neighbours) should treat the 24 bytes as
a size constraint on the missing piece, not a solved question.

### Proposed learning

**When every visible instruction already matches (branches, calls,
register choices) but the prologue/epilogue stack-adjust and save/restore
offsets are all off by a constant, don't touch the logic — pad the frame.**
Compute the gap (retail's `addiu sp,sp,-N` minus yours), add a `u8
unused[gap];` local that's never referenced, and rebuild. This is now the
THIRD confirmed instance of this exact idiom in the project (after
`Class65650__FindPartIndex`, `Class65650__SetLightMode`), reinforcing it as a general GCC 2.6.3
quirk rather than something specific to those two functions' unit.

## Provenance

round 2026-09-02, runner ALPHA, unit class_3ac78. First attempt 39/51
(pure frame-size gap, logic already exact); second attempt (padding local)
closed it, 51/51. Follow-up (same day, head-requested): `switch` rewrite
of the gate, one attempt, also 51/51 — adopted as final.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004ADD8` | `StageMap__ForwardAcceptedCommand` | A | Occupant of vtable slot `+0x0D0`. Body is a filter-and-forward and nothing else: bail unless the command is in `{2,3,5,6,7,8}`, then walk `acceptedTags` and, on a match against the sender's own vtable header word, dispatch `applyToSenderFootprint(self, sender, command)`. Mechanics are the purpose. |

**The third parameter is a COMMAND CODE, not a count** -- this report and the
declaration both called it `count`, and that was wrong. Three independent
witnesses: this function gates it on a small non-contiguous set (a count would
not skip 4); the base occupant of the sibling slot `+0x09C`,
`SceneNode__DispatchLinkCommand(self, a1, a2)`, switches on the same-position
parameter over `{2,3,4}`; and `SceneNode__OnNotify` in `code_d294` dispatches these
slots as `(self, sender, event)`. Renamed `list` -> `sender`,
`count` -> `command`, in the definition, the slot declarations and the two
callers. Byte-neutral, oracle green.

Posted to the round broadcast, because `class_3bb8c`'s own view of these
slots inherits the same wrong word.
