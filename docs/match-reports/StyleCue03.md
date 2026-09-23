# StyleCue03 -- MATCHED (56/56 words)

> Renamed from `func_80055BC8` on 2026-09-23 (tools/rename.py). Address 0x80055bc8.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #4 of
`gStyleCueCallbacks`. Divisibility-gated instead of discrete-`kind`-gated: two
independent `% N == 0` checks on `self->kind`, plus unconditional trailing
writes.

## Final source

```c
void StyleCue03(StyleCueParam *ctx, StyleCueParam *self) {
    self->falloff = ComputeStyleCueFalloff(ctx);
    if (self->kind % 20 == 0) {
        self->unk1C = 0x1E;
        self->unk24 = 0x20;
        self->unk20 = 0;
        self->unk28 = 0xA;
    }
    if (self->kind % 400 == 0) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
    }
    self->unk44 = 6;
    self->unk4C = 0x20;
    self->unk48 = 0;
    self->unk50 = 0xA;
}
```

## Derivation

- **`% 20` and `% 400`, not `% 15`/`% 240`.** The magic-multiply
  divisors have to be solved arithmetically from the RECOMBINATION chain
  (`sll`/`addu`/`sll` sequences that reconstruct `q*N` for comparison
  against the dividend), not guessed from the magic constant's shift
  alone -- the first pass here mis-derived both as 15/240 by
  under-counting an `sll` in the recombination, and the resulting C
  compiled to the wrong DIVISOR entirely (a different but structurally
  similar `mult`/`mfhi`/shift sequence), which the build/funcdiff loop
  caught immediately as a real word-count mismatch rather than a subtle
  register-identity residue. Confirmed 20/400 by re-deriving each
  recombination chain instruction-by-instruction:
  `sll,addu,sll` = `((q<<2)+q)<<2` = `q*20`;
  `sll,addu,sll,addu,sll` = `q*3, *8, +q, *16` = `q*400`.
- **Two independent `if`s, not `else if`** -- both conditions are checked
  regardless of each other's outcome (confirmed: both blocks' guarding
  `bne`/branch targets are independent, with the SECOND check's dividend
  reloaded fresh from `self->kind` rather than gated on the first).
- Field write order within each block follows the disassembly exactly
  (`unk1C`, `unk24`, `unk20`, `unk28` for the `%20` block -- note `unk24`
  before `unk20`, an artifact of register/constant reuse in retail's own
  codegen, reproduced by writing the C statements in that same order).

### Proposed learning

- **Re-derive a magic-multiply divisor from its full recombination
  chain, not from the shift amount alone, and re-verify by counting
  every `sll`/`addu` in the chain rather than trusting a first pass.**
  This function's own first derivation attempt (never written to
  `src/`, caught before committing) used the shift-only heuristic and
  landed on 15/240 instead of the correct 20/400 -- both instruction
  SHAPES look identical (`mult`/`sra`/`mfhi`/`sra`/`subu` then
  `sll`/`addu`/`sll`), so only the actual recombination arithmetic
  distinguishes them.

## Naming

**Tier B.** `StyleCue03` is row +0x010 of `gStyleCueCallbacks` (`tools/classtable.py 0x800874B0`, round 73). `TryStartStyleCue` (class_3bb8c_n.c) installs `gStyleCueCallbacks[sub->countSign]` as `SoundCueSet::callback` via `InitSoundCueSet` (code_179d8_e.c) -- the same per-tag sound-cue-callback slot `gEntityMoodHandlerTable`'s `MoodCueNN` occupants hold for `Entity` (`Entity__MoodCueNN` match reports). The `StyleCueNN` numbering follows table row order, same convention as `MoodCueNN`. Mechanics are established (a per-tag callback that reads `self->kind` and writes a handful of numeric fields, calling `ComputeStyleCueFalloff` first); which dream/style object or which field means what in the running game is not, so the specific `kind` branches and the numeric literals they write stay unnamed.
