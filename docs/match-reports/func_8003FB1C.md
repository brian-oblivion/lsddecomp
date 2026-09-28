# func_8003FB1C -- CONVERTED to a linked SDK object (round 34). NOT game code.

> **ROUND 34 (2026-09-12), runner bravo. THIS FUNCTION IS NOW LINKED FROM
> SONY'S OWN OBJECT `libgs/gs_123.o` (Psy-Q 3.3) AND IS NAMED
> `Gssub_make_matrix`.** The object's 0xC8 of text covers exactly it. It sat
> one function INTO `screen_widgets`, so that unit is split
> `[c libgs_gs_101][o libgs/gs_123][c screen_widgets]` and GsSetNearClip, the
> single function in front, moved to `src/psyq/libgs_gs_101.c`.
>
> **This RECLASSIFIES a matched function out of the game-code count, which is
> the correction CLAUDE.md asks for, not a regression.** `libgs/gs_131`, linked
> since round 33, already REFERENCED this symbol by name; `gs_123` defines it.
>
> **The 0x1908 rodata attach left with it, and that is the interesting part.**
> jtbl_80011108 is this function's own jump table, and it is `gs_123.o`'s own
> `.rdata` section -- so the table now arrives from the object alongside the
> code that indexes it, and the `.L`-labels-cannot-cross-a-segment problem
> that forced the attach in round 14 (and the split in round 20) simply stops
> existing. `place` reports `bytes:not-found` for that section, which is the
> expected relocated-words case: readelf shows 0x118 bytes of `.rel.rdata` for
> 0x8C of data.
>
> **Everything below is kept as the derivation it was, not as live guidance.**
> The `Matrix2cc8c` struct view it establishes is still correct and still
> used -- it is the PSX `MATRIX`, and round 33's own note about two units
> holding independent views of `D_8008E98C` is unaffected.

Unit: `screen_widgets` (until round 34) · Size: 50 words · Round 23 (2026-09-07), head.
**Matched on the FIRST attempt**, fresh ground (no prior report).

## What it is

A rotation-matrix builder in the Psy-Q `RotMatrix` mould: copy an identity
matrix, then overwrite the four entries that a single-axis rotation touches,
selected by an axis LETTER.

```c
void func_8003FB1C(Matrix2cc8c *dst, s16 sin, s16 cos, u8 axis) {
    *dst = D_8008E98C;
    switch (axis) {
    case 'X':
    case 'x':
        dst->m[1][1] = cos;
        dst->m[2][2] = cos;
        dst->m[1][2] = -sin;
        dst->m[2][1] = sin;
        break;
    case 'Y':
    case 'y':
        dst->m[0][0] = cos;
        dst->m[2][2] = cos;
        dst->m[0][2] = sin;
        dst->m[2][0] = -sin;
        break;
    case 'Z':
    case 'z':
        dst->m[0][0] = cos;
        dst->m[1][1] = cos;
        dst->m[0][1] = -sin;
        dst->m[1][0] = sin;
        break;
    }
}
```

with, unit-locally in `src/ui/screen_widgets.c`:

```c
typedef struct Matrix2cc8c {
    s16 m[3][3];
    s32 t[3];
} Matrix2cc8c;

extern Matrix2cc8c D_8008E98C;
```

## How each piece was established, since none of it was guessed

- **The type is the PSX `MATRIX`.** The prologue copies 0x20 bytes out of
  `D_8008E98C` (three `lw` / three `sw`, twice, then two of each — GCC 2.6.3's
  `movstrsi` batching for a plain whole-struct assignment), and the arms then
  write **`s16`s at offsets 0, 2, 4, 6, 8, 0xA, 0xC, 0xE, 0x10** — a 3x3 array
  of halfwords. `short m[3][3]; long t[3];` is 18 bytes plus `t` on its own
  4-alignment at +0x14, total 0x20. Exactly fits.
- **The axis is a CHARACTER, and the range says so.** `andi $a3, 0xFF` then
  `addiu $a3, -0x58` then `sltiu $v0, 0x23` is a 35-entry table over
  `[0x58, 0x7A]` = `['X', 'z']`. The jump table sends indices 0/1/2 (`'X'`,
  `'Y'`, `'Z'`) and 32/33/34 (`'x'`, `'y'`, `'z'`) to three arms and the other
  29 to the epilogue. A numeric mode argument would not span exactly the
  letters.
- **`sin` vs `cos` is fixed by which one is negated.** `$a2` goes on the
  diagonal in every arm; `$a1` appears both raw and through `negu`. So `$a2` is
  `cos` and `$a1` is `sin`. Cross-checked against the standard forms: arm 1 is
  `m[1][1]=c, m[1][2]=-s, m[2][1]=s, m[2][2]=c` (rotation about X), arm 2 is
  the Y form with the sign on `m[2][0]`, arm 3 the Z form with it on `m[0][1]`.
  All three have the sign on the conventional entry, which is the check that
  says the assignment is right rather than merely self-consistent.
- **Store order within each arm was read off the `.s`, not assumed** — both
  `cos` writes first, then `-sin`, then `sin`.

## `D_8008E98C` already had a DIFFERENT local view, and that is fine

`include/class_3bb8c.h` declares the same symbol as `QueryTemplate866E8` for
`StageMap__ComputeFootprintFromRotation` (round 13) — "8 words, offsets 0x00-0x1C, the first five
opaque". **The two views agree on the bytes and are in fact consistent**: that
reading's opaque `unk0[5]` is this one's `m[3][3]` plus its tail pad, and its
`unk14`/`unk18`/`unk1C` are `t[0]`/`t[1]`/`t[2]` — which also explains why
`StageMap__ComputeFootprintFromRotation` zeroes two of them and sets the third, i.e. it is setting a
translation vector on a copied identity matrix.

Neither declaration was moved. `screen_widgets` does not include
`class_3bb8c.h`, so there is no collision, and the matrix view lives in
`src/ui/screen_widgets.c` rather than in the six-unit `Task.h`. This is the
multiple-independent-local-views convention working as intended: the
cross-reference belongs in a report, not in a shared header.

### Proposed learning

**The switch case order was PREDICTED before writing a line, and that is
stronger evidence for the round-23 case-order lever than the four instances
that fixed it after the fact.** The recipe (see DECOMPILATION_LEARNINGS, "A
`switch`'s CASE ORDER is recoverable from the binary") says: sort the arm labels
by address, map them through the jump table, and the arm with no `j` of its own
— the one that falls through into the epilogue — is the LAST case in source.
Here that gave `X, Y, Z` with `Z` falling through, it was written that way
first, and the function matched on attempt one at 50 words. **Used predictively
the lever costs nothing; used diagnostically it costs an attempt.** Read the
arm addresses before writing any dense switch.

**Second, narrower: three `lw` / three `sw` in repeating batches, over a
contiguous run of offsets, is GCC 2.6.3's whole-struct assignment** (`movstrsi`
with three-register batching), tailing off to whatever remains — here 3, 3, then
2. It is one `*dst = src;` and needs no field-by-field spelling. The tell that
it is a struct copy rather than hand-written field copies is that the loads are
BATCHED ahead of the stores rather than interleaved one-for-one.

## Round 94 (track 6)

The second view is gone: DayTaskStageMap.c declares `extern MATRIX D_8008E98C;`
from `<libgte.h>`, so both readers now use Sony's type.
