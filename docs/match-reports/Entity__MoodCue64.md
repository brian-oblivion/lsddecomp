# Entity__MoodCue64 -- MATCHED (52/52 words)

> Renamed from `func_80061E60` on 2026-09-24 (tools/rename.py). Address 0x80061e60.

Unit: `Entity_e` (round 12). A mood handler that reduces `out->unk4` modulo
300 and dispatches on the remainder.
`void Entity__MoodCue64(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue64(Entity *this, EntityMoodHandlerArg *out) {
    s32 r = out->unk4 % 300;

    out->unk10 = this->methods->slot148(this);
    if (r < 0x14) {
        out->unk1C = 5;
        out->unk20 = -2;
    } else if (r == 0x16) {
        out->unk1C = -2;
    }
    this->methods->slotC4(this, -0xA, 0);
}
```

## Derivation notes

The disassembly's division-by-constant magic (`0x1B4E81B5`, `sra` shift 5,
`mfhi`/`subu` sign-correction, no `addu` correction step since the magic is
`< 0x80000000`) followed by a `q*5, *15 (via *16-1), *4` reconstruction of
`q*300` and a final `subu` against the original dividend is GCC 2.6.3's
standard signed-remainder-via-division idiom. **Verified directly against
the pinned toolchain** rather than derived by hand-decoding the magic
constant (see CLAUDE.md's "Escalate, do not experiment" reproducer
pipeline): compiling `int mod300(int x){return x%300;}` through
`tools/gcc263/cc1` at `-mips1 -mcpu=3000 -O2` reproduces this exact
instruction sequence (magic, shift, and the `sll 2/addu/sll 4/subu/sll 2`
reconstruction) byte-for-byte. This is the same idiom as
`Entity__MoodCue66` below (`% 30`) and `Entity__MoodCue40` in `Entity_d.c` (`% 7`)
-- worth having a reproducer command on hand rather than re-deriving the
magic-number-to-divisor mapping by arithmetic each time, which is
error-prone (a first pass on this function mis-guessed divisor 150 instead
of 300 from the magic constant alone).

`this->methods->slot148` and `slotC4` are both already-typed vtable slots
from `Entity_d`'s work; no new struct knowledge here.

### Proposed learning

**Division/modulo-by-constant magic numbers are worth verifying against the
actual pinned `cc1`, not hand-decoded.** `tools/gcc263/cpp ... | cc1
-mips1 -mcpu=3000 -quiet -G0 -O2` on a tiny one-line probe (`return x %
N;`) takes under a second and gives the exact magic/shift/reconstruction
GCC would emit for a candidate divisor -- far faster and more reliable than
reconstructing the divisor from the Hacker's-Delight magic-number formula
by hand.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 64 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
