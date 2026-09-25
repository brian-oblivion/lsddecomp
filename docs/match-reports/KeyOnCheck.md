# KeyOnCheck (0x800303FC) -- Sony's libsnd/vmanager, identified round 79

Matched as C by splat itself (`void KeyOnCheck(void) {}`, a `jr $ra; nop`
leaf); it was never matching work. Formerly `func_800303FC`, the one game-
counted definition left in `code_179d8_j`.

## Identification (FINISHING-PLAN track 2)

- **Fingerprint:** `sdkname.py` EXACT masked 1.00 but TINY: two words match
  many Sony stubs (`KeyOnCheck`, `SsUtVibrateOn/Off`, `__main`, ...).
- **Position:** `sdk/work/3.3/elf/libsnd/vmanager.o` lays out
  `SpuVmSeKeyOn` 0x314C, `SpuVmSeKeyOff` 0x3238, `KeyOnCheck` 0x326C,
  `SpuVmSetSeqVol` 0x3274. Retail's gaps around this address (0x800302DC,
  0x800303C8, 0x800303FC, 0x80030404) are the same 0xEC, 0x34 and 0x8, and
  both neighbours were already identified as that object (round 71).
- No header prototype: `KeyOnCheck` is vmanager-internal, and no game code
  calls this address.

Found by bravo (round 79, wave 2) during a track 3 naming pass's Sony check;
renamed by the head with `tools/rename.py`, byte-identical. The whole unit is
now Sony's.
