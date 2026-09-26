# Entity__MoodCue66 -- MATCHED (30/30 words)

> Renamed from `func_8006204C` on 2026-09-24 (tools/rename.py). Address 0x8006204c.

Unit: `Entity_e` (round 12). A mood handler testing `out->unk4 % 30 == 0`.
`void Entity__MoodCue66(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue66(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 30 == 0) {
        out->unk1C = 0xD;
    }
}
```

## Derivation notes

Same division-by-constant idiom as `Entity__MoodCue64` above, this time for
`% 30` (magic `0x88888889`, `sra` shift 4, WITH the `addu` sign-correction
step since this magic is `>= 0x80000000` -- the "negative magic" case).
Verified against the pinned `cc1` the same way.

**One real bug caught by the byte diff, not by re-reading the disassembly
correctly the first time**: my first pass read the `bne $a0,$v1,skip` as
"skip the store when EQUAL" and wrote `if (out->unk4 % 30 != 0)`. It is the
opposite -- `bne` skips the store when the two operands are *unequal*, so
the store (a full-match `EntityMoodHandlerArg::unk1C` write) only happens
on the fallthrough, i.e. when they *are* equal. The first build attempt
compiled fine and funcdiff reported 29/30 with a single `bne`/`beq` opcode
diff at the branch -- a same-size, wrong-polarity residue, immediately
diagnosable from the one-word diff. Fixed by flipping `!=` to `==`.

### Proposed learning

**A `bne`-guards-a-store shape reads backwards on a quick pass.** `bne
$a, $b, target` skips to `target` when `$a != $b`; the guarded code
therefore runs when the two ARE equal. Easy to invert by habit when
skimming quickly, especially right after having read several `bnez`
("branch if truthy, skip a default") patterns in the same unit. Worth a
second look specifically at the *equality direction* before transcribing
any `bne`/`beq` that gates a single store, since a same-size polarity
inversion compiles clean and only shows up as a single-opcode byte diff
that's easy to mis-file as "close enough" instead of "wrong".

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 66 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity_b/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
