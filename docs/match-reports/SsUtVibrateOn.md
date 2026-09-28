# SsUtVibrateOn -- Sony's libsnd/vmanager, identified round 99 (2/2 words, splat-generated)

> Renamed from `func_8002E2F8` on 2026-09-27 (tools/rename.py). Address 0x8002e2f8.

Unit: `src/psyq/libsnd_vmanager.c`. Address `0x8002E2F8`.

Trivially matched: the empty C body compiles to retail's own `jr $ra; nop`
-- splat emitted this body itself when the unit was carved (round 24,
2026-09-08). Not decompilation work. The definition now takes LIBSND.H's
prototype, `void SsUtVibrateOn(short vc, short vibW, short vibT)`, because
the unit includes `<libsnd.h>`; an empty body with unused arguments is
still `jr $ra; nop`, byte-identical.

## Naming

**`SsUtVibrateOn`** -- Sony's name (FINISHING-PLAN: Sony keeps Sony's
names), identified by position, the `KeyOnCheck` precedent (round 79):

- **Fingerprint:** TINY. Two words (`jr ra; nop`) match every empty Sony
  stub, so `sdkname.py` evidence alone is worthless here.
- **Position:** `sdk/work/3.3/elf/libsnd/vmanager.o` lays out `SePitchBend`
  0x11B8, `SsUtVibrateOn` 0x1370, `SsUtVibrateOff` 0x1378, `SeAutoVol`
  0x1380; disc 3.5 the same order at 0x1180/0x1324/0x132C/0x1334. Retail:
  `SePitchBend` 0x8002E138 (112 words, so it ends at 0x8002E2F8), then this
  function, 8 bytes, `SsUtVibrateOff` 0x8002E300, 8 bytes, `SeAutoVol`
  0x8002E308. Both neighbours were already Sony's (vmanager). The two 8-byte
  gaps are exactly vmanager's.
- On disc 3.6 the pair moved to its own object, `libsnd/vm_vib.o`, which
  is further evidence they are one Sony unit, not game filler.
- No caller in `src/` and no table entry reaches this address, which fits
  a stubbed-out library entry point the game never calls.

Recorded in the symbols file with an `identified` comment, so
`progress.py` now counts it as library (241 -> 243 with its sibling).

## History: the round 75 naming pass (superseded)

Round 75 (runner alpha, FINISHING-PLAN track 3) kept the placeholder,
tier C: no caller anywhere in `src/`, no `asm/data/*.s` table reference at
this address (checked numerically and symbolically), so a game-style name
would have been a guess. That reasoning was right for a game function; the
function turned out not to be one.
