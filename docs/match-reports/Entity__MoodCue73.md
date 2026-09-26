# Entity__MoodCue73 -- MATCHED (52/52 words)

> Renamed from `func_80062660` on 2026-09-24 (tools/rename.py). Address 0x80062660.

Unit: `Entity_e` (round 12). First function in this unit to dispatch through
`Unk94Methods::slot44`/`slot130` directly (both slots were already typed
from `Entity_d`'s `Entity__MoodCue49`/`Entity__MoodCue01` comments, but not yet
exercised as a *call site* in this unit).
`void Entity__MoodCue73(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue73(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = 0;
    if (out->unk4 == 0) {
        this->unk94->methods->slot44(this->unk94, 1, ROTATION_YAW_PLUS90);
        this->unk94->methods->slot130(this->unk94, 1);
        out->unk1C = 0x19;
        out->unk30 = 0x19;
        out->unk44 = 0x19;
    } else if (out->unk4 == 0x14) {
        out->unk30 = 0xD;
    }
    if (this->unkFC == this->unk80 - 1) {
        this->methods->slot160(this);
    }
}
```

## Derivation notes

**Real bug, not a scheduling residue**: my first pass nested `out->unk10 =
0;` inside the `if (out->unk4 == 0)` block, reading the disassembly's `sw
zero, 0x10(s0)` as ordinary sequential code following the branch. It is
actually the **branch's own delay slot** (`bnez $v1, .L800626DC / sw
$zero, 0x10($s0)`), which executes unconditionally regardless of whether
the branch is taken. `out->unk10 = 0` therefore happens on every call, not
just the `out->unk4 == 0` path.

This mis-read also explained a knock-on register-scheduling difference:
with the store nested inside the `if`, GCC had no independent instruction
to fill the *load's* delay slot (`lw $v1, 0x4($s0)` immediately followed by
`bnez $v1`, a load-use hazard requiring a filler) and emitted a bare `nop`
there, while retail fills that slot with the early `this`-to-`s1` register
copy (hoisted up because, with `out->unk10 = 0` correctly unconditional,
nothing else was available to fill the *branch's* delay slot except that
store). Moving the statement to be unconditional fixed both problems at
once -- the extra `nop` disappeared and the whole function landed at
retail's exact size (0xD0) on the next build, which also resolved an 8-byte
address-drift ripple through every later function in the unit.

### Proposed learning

**A store or side-effecting statement that shows up as a branch's delay
slot in the disassembly is unconditional, not part of the branch's guarded
body**, even though it's textually adjacent to the branch and easy to
misread as "the first line of the true/false-branch's block". Check
specifically: is the instruction indented under the branch mnemonic (a
delay slot, runs always) or is it the first instruction *after* the branch
target/fallthrough point (genuinely guarded)? Getting this backwards
produces C that's still *plausible* -- it compiles, and the guarded
codepath even runs the store when expected -- but is wrong on the other
path, and the resulting size/scheduling mismatch can cascade into
seemingly unrelated diffs in every later function in the same translation
unit (per CLAUDE.md's "Address drift" note), which makes the root cause
much harder to spot from the diff alone than from just re-reading the delay
slot correctly up front.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 73 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity_b/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`, `voice1Tone`, `voice2Tone`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). clearTickCallbacks' clearLook 1 is `true`. Byte-identical (whole image green).
