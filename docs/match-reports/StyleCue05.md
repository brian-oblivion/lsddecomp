# StyleCue05 -- MATCHED (56/56 words)

> Renamed from `func_80055DB4` on 2026-09-23 (tools/rename.py). Address 0x80055db4.

Unit: `class_3bb8c_r` (round 17 continuation). Slot occupant #6 of
`gStyleCueCallbacks`, and the most structurally involved of the fourteen: a
four-way `if`/`else-if` mixing a discrete check, a range+modulo
condition, an unsigned-range trick, and a plain threshold, with a
field that both READS and WRITES `self->unk4` inside one arm.

## Final source

```c
void StyleCue05(ParamObj *ctx, ParamObj *self) {
    self->unk10 = ComputeStyleCueFalloff(ctx);
    if (self->unk4 == 0) {
        self->unk1C = 0x1E;
        self->unk20 = -1;
    } else if (self->unk4 < 0x32 && self->unk4 % 5 == 4) {
        self->unk30 = 0x1E;
        self->unk34 = 0;
        self->unk38 = self->unk38 - self->unk4 * 2;
        self->unk3C = self->unk38;
    } else if ((u32)(self->unk4 - 0x65) < 9) {
        self->unk44 = 0xD;
        self->unk48 = 1;
    } else if (self->unk4 >= 0xC9) {
        self->unk4 = -1;
    }
}
```

## Derivation

- **No cached `kind` local -- every branch re-reads `self->unk4`
  directly.** Unlike the discrete-`kind` siblings (`StyleCue00` etc.),
  this function's disassembly reloads `self->unk4` from memory at the
  START of the range+modulo arm, again inside that arm (for the
  `unk38 -= unk4*2` arithmetic), and again in the unsigned-range arm --
  even though nothing between those reads writes `self->unk4` itself.
  Caching into a local `kind` variable (as the simpler siblings do)
  compiled to ONE load reused everywhere, one word too few. Writing
  `self->unk4` directly at each site reproduces the reloads. The likely
  cause: an intervening STORE to a DIFFERENT field of the same struct
  (`self->unk34`, `self->unk38`) makes this compiler's weak alias
  analysis reload other fields conservatively on the next read, even
  though the actual field read (`unk4`) was never written -- the same
  family of caution documented for `DreamSys__SaveLinkSnapshot` in
  `DECOMPILATION_LEARNINGS.md`, seen here from the "do NOT cache" side
  rather than the "DO cache" side.
- **`(u32)(x - 0x65) < 9`** is the unsigned-range-check idiom for
  `x >= 0x65 && x < 0x6E`, transcribed directly from the disassembly's
  own `addiu`+`sltiu` pair rather than reconstructed as two separate
  comparisons (which would cost an extra instruction).
- **`self->unk4 % 5 == 4`**, not a hand-expanded remainder-4 test,
  matches the `mult`/`mfhi`/recombination chain for divisor 5 (same
  magic+shift already confirmed in `StyleCue04`'s report).

### Proposed learning

- **A field that gets reloaded even though nothing wrote it directly can
  still need "do not cache" treatment, if a DIFFERENT field of the same
  struct was written in between.** This is the mirror image of
  `DreamSys__SaveLinkSnapshot`'s already-documented "an intervening whole-struct
  assignment defeats CSE of a pointer field" entry -- here the
  intervening write is an ordinary SCALAR field store (`self->unk34 = 0`,
  `self->unk38 = ...`), not an aggregate assignment, and it still
  triggers the same conservative reload of a DIFFERENT field
  (`self->unk4`) on the next read. Worth checking for on any function
  with more than one struct-field write before a later read of an
  UNRELATED field of that same struct.
