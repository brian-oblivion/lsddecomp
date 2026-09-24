# Class866E8__Reset -- MATCHED, round 45 (2026-09-15)

> Renamed from `func_8004AA10` on 2026-09-22 (tools/rename.py). Address 0x8004aa10.

**Unit:** `class_3ac78` · **Size:** 23 words · **Status:** MATCHED, 23/23 exact.

Previously filed as blocked by `gp_rel` (round 42 reopen note); that blocker
was RESOLVED in round 42 by `maspsx --gp-symbols`, pinned in the Makefile.
Rebuilt clean on the first attempt this round -- the earlier report's only
finding (the `lw $a1, %gp_rel(gDefaultGridSpan)($gp)` instruction) was correct, it
just isn't a blocker any more.

## What it does

`Class866E8`'s vtable slot `+0x040` (constructor-adjacent init, called by
`Class866E8__SetConfig`): zeroes `unk68`/`unkE8`/`unk88`, dispatches slot `+0x0DC`
with `self` and the loaded value of a lone global word `gDefaultGridSpan`, then
sets four new fields (`unk1CC`/`unk1D0`/`unk1D4`/`unk1D8`) to `-1`.

## Body

```c
extern s32 gDefaultGridSpan;

void Class866E8__Reset(Class866E8 *self)
{
    self->unk68 = NULL;
    self->unkE8 = 0;
    self->unk88 = 0;
    self->methods->slotDC(self, gDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}
```

## Derivation notes

- `gDefaultGridSpan` (`asm/data/7B12C.sdata.s`) is a single `.word 0x0000A000`
  with no other reference anywhere in the image (checked with
  `grep -rl gDefaultGridSpan asm/`). It sits in an unnamed top-level `sdata`
  segment, not owned by any carved unit, so it is declared `extern s32`
  directly in `src/class_3ac78.c` -- same pattern already used for
  `gDefaultOrigin` in this same file. `%gp_rel(gDefaultGridSpan)($gp)` loads its
  *value*, not its address, so the call argument is a plain `s32`, not a
  pointer.
- `self->unk0` (the vtable pointer) is read early in retail's own
  instruction order, and slot `+0x0DC` had no prior occupant or
  declaration -- added to `Class866E8Methods` as `slotDC(Class866E8*, s32)`,
  splitting the padding that used to run `0xD4..0xF4` at `0xDC`.
  `include/class_3ac78.h` updated (was already shared only within this
  runner's assignment this round; no cross-runner contention).
- Offsets `0x1CC`/`0x1D0`/`0x1D4`/`0x1D8` were inside the `Class866E8`
  struct's `pad1C4[0x1E0-0x1C4]` catch-all padding; split into four new
  `s32` fields (`unk1CC..unk1D8`), leaving a smaller `pad1DC[0x1E0-0x1DC]`
  behind them. No other function in this unit currently reads that byte
  range (checked via the header's own existing comments), so this is a
  pure padding narrowing, not a retype of anything already read elsewhere.
- Instruction ORDER in the C source (fields zeroed, then the dispatch,
  then the four `-1` stores) matches retail's own statement order; the
  `$a1`/`self->methods` loads appearing early in the raw asm is ordinary
  GCC 2.6.3 scheduling of independent loads ahead of the stores, not a
  sign the source order is wrong -- confirmed by the byte-exact result on
  the first attempt with straight-line, in-order C.

### Proposed learning

None new -- this is exactly the round-42 `gp_rel` resolution playing out:
the prior report's technical observation (the `%gp_rel` load) was correct
and complete, only its blocked-verdict framing needed rebuilding.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AA10` | `Class866E8__Reset` | B | Occupant of vtable slot `+0x040`. Body does nothing but put the object back to a known state: clears `config`, `acceptedTags` and `rectCount`, re-applies the default grid span, and writes the four `-1` sentinels at `+0x1CC..+0x1D8`. The sibling table `D_800865C8` names its own `+0x040` occupant `resetUnk3C` (`include/class_39e08.h`), so `+0x040` is a reset slot in this family. Tier B and not A because nothing establishes WHEN a reset is wanted -- its one known caller is `Class866E8__SetConfig`. |
| `D_8008A980` | `gDefaultGridSpan` | B | Value `0x0000A000`, and this is its only reader in the whole image. It is handed straight to `setGridSpan`, which derives `span >> 11 == 20` -- the byte-verified grid row stride -- and `span >> 12 == 10`. See `Class866E8__SetGridSpan.md` for the full arithmetic. |

Field names established here:

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `Class866E8+0x068` | `config` | B | Cleared here, set by `Class866E8__SetConfig`, read by `Class866E8__ApplyToSenderFootprint` as `config->unk4`; `class_3bb8c` reads the same pointee as `{s16 divisor, s16 count, s32 unk4}` from four functions -- a small parameter block, not an object. |
| `Class866E8+0x0E8` | `acceptedTags` | A | See `Class866E8__ForwardAcceptedCommand.md`: its only reader walks it as a NUL-terminated list of vtable header words and uses it to accept or reject a sender. |
| `Class866E8+0x088` | `rectCount` | A | Its only writers set it to 1 (`Class866E8__SetFootprintRect`) or save/restore it around a walk (`Class866E8__ApplyToSenderFootprint`), and its only readers bound a loop over `rects[]` (here, and `class_3bb8c_b`'s matched `Class866E8__SetFootprintCellFlag`). |

`unk1CC`/`unk1D0`/`unk1D4`/`unk1D8` deliberately keep placeholder names: all
that is known is that they are four consecutive words set to `-1` here and
never read by any decompiled function. Writing a name for them would be a
guess.
