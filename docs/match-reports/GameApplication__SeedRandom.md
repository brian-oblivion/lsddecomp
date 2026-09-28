# GameApplication__SeedRandom

> Renamed from `GameApplication__SetDayFromTickCount` on 2026-09-26 (tools/rename.py). Address 0x800260a4.

> Renamed from `Class6D3C8__SetDayFromTickCount` on 2026-09-26 (tools/rename.py). Address 0x800260a4.

> Renamed from `func_800260A4` on 2026-09-24 (tools/rename.py). Address 0x800260a4.

**Unit:** GameApplicationFileResource · **Size:** 25 instructions · **Status:** MATCHED (25/25 words)

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
`s32`, read once per call. m2c (`tools/m2ctx.py GameApplicationFileResource --sig 'void
GameApplication__SeedRandom(void)' --run`) independently produced the same `% 365`
reading, confirming the divisor and that GCC reproduces its own magic
constant without any hand-tuning needed:

```c
void GameApplication__SeedRandom(void) {
    SeedAndRandom(*(s32 *)0x1F800000 % 365, 0);
}
```

`SeedAndRandom` (in `psyq_memset.s`, still `asm`) is called with `(day, 0)`
— second argument's purpose unknown, always a literal 0 at every call in
this unit's family (see `GameApplication__ShowIntroLogos`/`GameApplication__PlayOpeningMovie` etc., all of which
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

**`GameApplication__SeedRandom` -- tier B.** Mechanics: reads the
running tick count kept at the PS-X scratchpad address `0x1F800000`, reduces
it mod 365 (a `%` on the day-count range), and forwards the result to
`SeedAndRandom(day, 0)`. "Set day" describes the mechanical destination
(the second call takes what looks like a day index) rather than asserting
why the game does this at this vtable slot (`+0x040`, dispatched once from
the ctor); `SeedAndRandom` itself is uncarved, so its own purpose (and
therefore this function's ultimate game role) is not confirmed here.

## Track 4 (2026-09-26, round 88)

Now declared `void GameApplication__SeedRandom(GameApplication *self)`: it
occupies Application's `setScreenDims` slot (+0x040) and its only caller, the
ctor, passes `self` (in `$a0`), which the body never reads, so the added
parameter emits nothing (image byte-identical). The slot keeps the
inherited `setScreenDims` type; the ctor calls it through
`GameApplicationSeedRandomFn` (`include/GameApplication.h`). The name is kept rather than
renamed for the slot: the body seeds the RNG from the day count and does
nothing a screen-dimensions setter would.

## Track 6 (round 93, echo)

Renamed `GameApplication__SetDayFromTickCount` -> `GameApplication__SeedRandom`
(`tools/rename.py`); its cast type `GameApplicationSetDayFn` ->
`GameApplicationSeedRandomFn`. **Tier A**, from the body alone now that the
callee is matched C (src/GameFiles.c): `SeedAndRandom(seed, unused)` is
`if (seed != 0) srand(seed); return rand();`, and this function discards the
result, so all it does is seed the C library's RNG with the scratchpad word at
0x1F800000 reduced mod 365. Nothing stores a day, so "SetDay" claimed more than
the body shows. What that scratchpad word holds at boot is not established
(the old name's "TickCount" was a reading, dropped rather than carried).

It fills Application's +0x040 (setScreenDims) with a self-only signature; the
ctor calls it once through the cast type, after installing this class's table.
Application's own ctor called the slot earlier under Application's table, so
the default screen size is still set. The function comment above the body in
GameApplicationFileResource.c still says "advances the day cursor": that is track 7's to fix.

## Track 7 polish (round 100, echo)

Body changes, all byte-identical: *(s32 *)0x1F800000 % 365 -> *(s32 *)getScratchAddr(0) % DAYS_PER_YEAR (libetc.h's macro; DreamSys.h); the SeedAndRandom extern returns s32, as defined (GameFiles.c).

### History: code_1677c.c comments before the round-100 polish

Moved here from the source, verbatim (names as they stood then, where the tools had not already rewritten them).

```c
/* Seeds the C library's random generator from the scratchpad word at
 * 0x1F800000 (the PS-X data-cache-as-RAM region) reduced mod 365; the
 * random value SeedAndRandom returns is discarded. */
```
