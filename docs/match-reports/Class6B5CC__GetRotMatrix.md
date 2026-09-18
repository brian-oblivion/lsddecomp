> Renamed from `func_8001D4DC` on 2026-09-18 (tools/rename.py). Address 0x8001d4dc.

# Class6B5CC__GetRotMatrix — MATCHED

Unit: `code_d294_b`. Round 13, runner delta. 35/35 words, full match on the
first attempt.

## Signature

```c
void Class6B5CC__GetRotMatrix(Class6B5CCObj *self, s32 a1, s32 a2);
```

## What it does

`self->unk14->unk44` is a 0x28-byte heap block (allocated by the ctor,
`func_8001CAF4`). Its own offset +0x10 holds a 4-`s16` quad (x, y, z, and an
unused 4th short) — call it `S16Quad_d294`. `a2` selects one of two ways to
build a local copy of that quad:

- `a2 != 0`: negate x/y/z individually into the local copy. The 4th short is
  **never written** on this path — retail's own disassembly has no store to
  it here, so the local copy leaves it uninitialised (garbage) exactly like
  retail does.
- `a2 == 0`: copy the whole quad verbatim.

The result (plus `a1`, forwarded unexamined) is passed to
`func_800160B0` (`the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s)`, PsyQ library, not decompiled in this
project).

## Source

```c
void Class6B5CC__GetRotMatrix(Class6B5CCObj *self, s32 a1, s32 a2) {
    S16Quad_d294 buf;
    S16Quad_d294 *src = &self->unk14->unk44->vec;

    if (a2) {
        buf.x = -src->x;
        buf.y = -src->y;
        buf.z = -src->z;
    } else {
        buf = *src;
    }
    func_800160B0(&buf, a1);
}
```

## Header changes

`include/code_d294.h`:

- New `S16Quad_d294` type: `{ s16 x, y, z, w; }`. All-`s16` members give it
  alignment 2, which is exactly what makes retail's whole-struct copy
  (`buf = *src;` on the `a2 == 0` path) compile to unaligned `lwl`/`lwr` +
  `swl`/`swr` instead of a plain `lw`/`sw` — the same idiom already recorded
  in DECOMPILATION_LEARNINGS for `func_8004B38C`/`FlashbackRotation`. This is
  what made the byte match land on the first attempt: I wrote the copy as a
  single struct assignment specifically to trigger that codegen, rather than
  as three/four scalar copies.
- New `Class6B5CCBlock44` type for `self->unk14->unk44`'s own block:
  `{ u8 pad0[0x10]; S16Quad_d294 vec; }`. Only +0x10 is known; the rest of
  the 0x28-byte block is still opaque.
- `Class6B5CCSub14::unk44` retyped from `void *` to `Class6B5CCBlock44 *`.
  Checked both other referencing sites in `src/code_d294.c` before doing
  this (`self->unk14->unk44 = blockB;` where `blockB` is `void *`, and
  `func_80017CFC(sub->unk44)`, which takes `void *`) — both are safe under
  implicit pointer conversion, no cast needed.
- New extern `func_800160B0(S16Quad_d294 *vec, s32 a1)`, PsyQ library,
  per-call-site typed (same precedent as `GetClass6B5CCMethods`'s note on
  `GetClass6B5CCMethods`/other cross-unit PsyQ calls) — `$v0` is never read after
  this call site's own `jal`, so it's declared `void`.

## Proposed learning

None beyond the already-recorded "all-`s16` struct gets unaligned
`lwl`/`lwr` copy codegen" idiom — this is a second confirming instance, not
a new discovery. Worth noting as a *technique*: when you see that shape in
the disassembly (word-pair `lwl`+`lwr` load followed by matching `swl`+`swr`
store, offsets not 0/4/8/... aligned to the base but 3/7/B/F-style `lwl`
offsets), look for a local struct of *only* `s8`/`s16` fields being copied
by value, and write it as one struct assignment rather than scalar-copying
each field — the single assignment is what reproduces the codegen. Writing
it as `buf.x = src->x; buf.y = src->y; ...` would very likely NOT reproduce
this (untested here since the struct-assignment form matched first try, but
consistent with how GCC 2.6.3 lowers struct copies elsewhere in this
project).

## Naming (round 54, bravo, track 3)

**Not renamed -- PROPOSED only.** Proposed name: `Class6B5CC__GetRotMatrix`
(tier B). Slot `+0x084` occupant (`tools/classtable.py D_8006B5CC`),
dispatched as `slot84(self, out, flag)` from both this unit's own
`Class6B5CC__ComposeAndApplyRotation` (`self` as receiver) and
`code_d294_c.c`'s `Class6B5CC__RotateLocalVector` (a different
Class6B5CCObj instance) -- so the SIGNATURE this file declares
(`Class6B5CC__GetRotMatrix(Class6B5CCObj *self, s32 a1, s32 a2)`) is really
`(self, MATRIX *out, s32 negate)`: builds this object's own rotation
quad (`self->unk14->unk44->vec`, the PSY-Q-identified `GsCOORD2PARAM.rotate`
per `include/code_d294.h`'s own "PSY-Q IDENTIFICATION" note -- negated
per-axis when `negate` is set, copied verbatim otherwise) and hands it
to Sony's `RotMatrix(vec, out)` to fill the caller's matrix. "GetRotMatrix"
describes the measured mechanics (compute-and-write-out this object's
rotation matrix, optionally mirrored); tier B because the negate flag's
in-game meaning (which callers want the mirrored form, and why) is not
established from this function's own body. Held back from an actual
rename because this symbol is referenced (in a comment) from
`src/code_d294_c.c:27` -- a different unit -- discussing exactly the
slot-84 relationship above. Posted to the broadcast.
