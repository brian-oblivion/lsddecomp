# func_8002BFA8 -- CONVERTED to a linked SDK object (round 34). NOT game code, NOT a stall.

> **ROUND 34 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libcd/iso9660.o` (Psy-Q 3.3) AND IS `cd_read`.** It was
> matched C counted as game code; the object owns its bytes, so the C is gone from
> `src/` and the game-code count shrank by it -- the correction CLAUDE.md asks
> for, not a regression. The run `libcd/iso9660` + `libc2/strcmp` +
> `libc2/strncmp` tiles 0x1BE40..0x1C92C and crosses the libcd_bios /
> PlacementGridVabSound boundary; both units trimmed. Whole-image SHA1 green. Nothing
> here is assignable and there is no stall left to work. The text below is the
> pre-conversion record.

## Original report

# func_8002BFA8

**Unit:** PlacementGridVabSound · **Size:** 27 instructions (0x6C bytes) ·
**Status: MATCHED 27/27**, whole-image SHA1 green.

## Role

A plain sequence of four calls into charlie's sibling unit
(`libcd_bios`, all four still `INCLUDE_ASM` there as of this round),
returning whether the last call's result is zero:

```c
s32 func_8002BFA8(void *p0, void *p1, void *p2)
{
    s32 buf[2];

    func_800292F4(p1, buf);
    func_80028DF0(2, buf, 0);
    func_80029274(p0, p2, 0x80);
    return (u32)func_80029254(0, 0) < 1;
}
```

## Cross-unit prototypes

`func_800292F4`, `func_80028DF0`, `func_80029274`, `func_80029254` are
all defined in `libcd_bios`, not this unit, and all four are still
`INCLUDE_ASM` there (no established signature anywhere). Declared LOCAL
to `PlacementGridVabSound.c`, typed purely from this call site's own register
usage, per the project's cross-unit-prototype rule and this round's
"no shared `code_179d8*.h`" rule for the sibling slices.

## The one-word residue: signed vs unsigned zero-test

Retail's final comparison is `sltiu $v0, $v0, 1` (UNSIGNED
less-than-1, true only when the value is exactly 0) -- not `slti`
(signed). Writing the natural `return func_80029254(0, 0) < 1;` on a
plain `s32` compiled to signed `slti` (26/27, one word off) since the
comparison's operands are both signed by default. Casting the callee's
result to `u32` before the comparison (`return (u32)func_80029254(0, 0) <
1;`) reproduced the unsigned instruction exactly. This reads as
`func_80029254`'s true return type being unsigned (or the call site
treating it as a boolean/count where negative isn't meaningful) rather
than a stylistic choice -- the cast is doing real work, not decoration.

`buf` (the local 8-byte scratch passed to the first two calls) declared
as `s32 buf[2]` -- its real element type/count is unestablished beyond
"8 bytes, address taken, not otherwise read in this function".
