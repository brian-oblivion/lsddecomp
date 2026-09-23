# AdjustRgbByDelta -- MATCHED, round 46 (2026-09-15)

> Renamed from `func_80054B1C` on 2026-09-23 (tools/rename.py). Address 0x80054b1c.

Unit `class_3bb8c_n`. **13/13 words, byte-exact.** First build.

## What it was

Fresh ground, carved round 45, never attempted. No blockers, a plain leaf
function -- no globals, no calls.

## Derivation

```
/* 4531C 80054B1C 0000A290 */  lbu   $v0, 0x0($a1)
/* 45324 80054B24 23104600 */  subu  $v0, $v0, $a2
/* 45328 80054B28 000082A0 */  sb    $v0, 0x0($a0)
/* 4532C 80054B2C 0100A290 */  lbu   $v0, 0x1($a1)
/* 45334 80054B34 23104600 */  subu  $v0, $v0, $a2
/* 45338 80054B38 010082A0 */  sb    $v0, 0x1($a0)
/* 4533C 80054B3C 0200A290 */  lbu   $v0, 0x2($a1)
/* 45344 80054B44 21104600 */  addu  $v0, $v0, $a2
/* 4534C 80054B4C 020082A0 */   sb   $v0, 0x2($a0)
```

Three independent byte ops on two 3-byte buffers with a shared delta --
reads like an RGB colour nudge (two channels subtracted, one added), which
fits this class's neighbourhood: `class_3bb8c_m.c` (the sibling unit just
before this one) reads a 3-byte-stride colour table (`D_800872C4`) into the
same region of globals this unit's other functions touch.

```c
void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}
```

### Proposed learning

None -- routine leaf match, no lever needed.

## Naming

**`AdjustRgbByDelta`, tier A.**

Pure 3-line leaf: `dst[0]=src[0]-delta; dst[1]=src[1]-delta;
dst[2]=src[2]+delta;` -- no globals, no calls, no control flow. Per track 3's
tier rule, "a pure leaf whose mechanics ARE its purpose (a getter, a clamp,
a list push) is tier A by definition." The two-subtract-one-add pattern on
two 3-byte buffers with a shared delta is exactly what the name says and
nothing more is claimed (not named e.g. "ApplyColorFade", which would assert
a purpose the body alone does not establish). MATCHED, 13/13, first
build.
