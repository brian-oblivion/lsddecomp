# GameApplication__SetDayFromTickCount

> Renamed from `Class6D3C8__SetDayFromTickCount` on 2026-09-26 (tools/rename.py). Address 0x800260a4.

> Renamed from `func_800260A4` on 2026-09-24 (tools/rename.py). Address 0x800260a4.

**Unit:** code_1677c · **Size:** 25 instructions · **Status:** MATCHED (25/25 words)

## What it does

Reduces a running tick/day counter mod 365 and forwards it to
`SeedAndRandom(day, 0)`.

## Derivation

The body is GCC 2.6.3's magic-multiply expansion of `x % 365`:

```
lui   $v0, (0xB38CF9B1 >> 16)
lui   $a1, (0x1F800000 >> 16)
lw    $a1, (0x1F800000 & 0xFFFF)($a1)   ; a1 = *(s32 *)0x1F800000
ori   $v0, $v0, (0xB38CF9B1 & 0xFFFF)
mult  $a1, $v0
sra   $v1, $a1, 31
mfhi  $v0
addu  $v0, $v0, $a1
sra   $v0, $v0, 8
subu  $v0, $v0, $v1        ; v0 = a1 / 365
sll   $a0, $v0, 3
addu  $a0, $a0, $v0         ; *9
sll   $a0, $a0, 3
addu  $a0, $a0, $v0          ; *73
sll   $v0, $a0, 2
addu  $a0, $a0, $v0           ; *365
subu  $a0, $a1, $a0            ; a1 - 365*(a1/365) = a1 % 365
jal   SeedAndRandom
 addu $a1, $zero, $zero
```

`0x1F800000` is the PS-X scratchpad region (data cache used as fast RAM,
not a hardware register) — some running counter lives there as a plain
`s32`, read once per call. m2c (`tools/m2ctx.py code_1677c --sig 'void
GameApplication__SetDayFromTickCount(void)' --run`) independently produced the same `% 365`
reading, confirming the divisor and that GCC reproduces its own magic
constant without any hand-tuning needed:

```c
void GameApplication__SetDayFromTickCount(void) {
    SeedAndRandom(*(s32 *)0x1F800000 % 365, 0);
}
```

`SeedAndRandom` (in `psyq_memset.s`, still `asm`) is called with `(day, 0)`
— second argument's purpose unknown, always a literal 0 at every call in
this unit's family (see `GameApplication__LoadIntroLogoSequence`/`GameApplication__StartWeeklyStreamTask` etc., all of which
pass 0/0/0 into the sibling helper `SetActiveDataSourceDriverMode`, a related pattern this
runner did not decompile).

## Proposed learning

A `mult` by a literal, `mfhi`, `sra`/`subu` sign-fix, immediately followed
by a `sll`+`addu` chain that reconstructs `divisor * quotient` and subtracts
it from the original value, is GCC 2.6.3's standard `x % N` expansion for a
compile-time-constant `N` — writing the plain `%` operator in C reproduces
it byte-for-byte with no manual reconstruction of the multiply-shift
sequence needed. Worth checking any other modulo-looking arithmetic in this
game (day/mood cycles are a strong candidate area) against this shape before
assuming it needs special handling.

## Naming

**`GameApplication__SetDayFromTickCount` -- tier B.** Mechanics: reads the
running tick count kept at the PS-X scratchpad address `0x1F800000`, reduces
it mod 365 (a `%` on the day-count range), and forwards the result to
`SeedAndRandom(day, 0)`. "Set day" describes the mechanical destination
(the second call takes what looks like a day index) rather than asserting
why the game does this at this vtable slot (`+0x040`, dispatched once from
the ctor); `SeedAndRandom` itself is uncarved, so its own purpose (and
therefore this function's ultimate game role) is not confirmed here.

## Track 4 (2026-09-26, round 88)

Now declared `void GameApplication__SetDayFromTickCount(GameApplication *self)`: it
occupies Application's `setScreenDims` slot (+0x040) and its only caller, the
ctor, passes `self` (in `$a0`), which the body never reads, so the added
parameter emits nothing (image byte-identical). The slot keeps the
inherited `setScreenDims` type; the ctor calls it through
`GameApplicationSetDayFn` (`include/GameApplication.h`). The name is kept rather than
renamed for the slot: the body seeds the RNG from the day count and does
nothing a screen-dimensions setter would.
