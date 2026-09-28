# Entity__MoodCue68 -- MATCHED (144/144 words)

> Renamed from `func_800621A8` on 2026-09-24 (tools/rename.py). Address 0x800621a8.

Unit: `Entity` (was `Entity_e`) (round 13). Sets `Entity::unk48` (the s16 field, a
different field from `EntityMoodHandlerArg::unk48`) via a coin flip, then
branches on `this->unk44`: a `== 0` path doing two independent modulo
checks (`% 10`, `% 20`) plus an "odd tick" gate that can promote `unk44` to
`0xA`, and an `== 0xA` path doing an `unkFC < 8` threshold split.
`void Entity__MoodCue68(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue68(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        this->unk48 = (rand() & 1) ? -0x176 : -0xC0;
    }
    if (this->unk44 == 0) {
        if (out->unk4 % 10 == 0) {
            out->unk1C = 0x1C;
        }
        if (out->unk4 % 20 == 0) {
            out->unk30 = 0x17;
            out->unk34 = -1;
            out->unk44 = 0x17;
            out->unk48 = -1;
        } else if (out->unk4 % 20 == 0xE) {
            out->unk30 = -2;
            out->unk44 = -2;
        }
        if ((this->unkFC & 1) == 0) {
            if (this->unk94->methods->slot100(this->unk94) != 0) {
                this->unkFC = -1;
                this->unk44 = 0xA;
                out->unk1C = 0x12;
            }
        }
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
        this->methods->slotC4(this, this->unk48, 1);
    } else if (this->unk44 == 0xA) {
        if (this->unkFC < 8) {
            this->methods->slot44(this, 0, ROTATION_ZPLUS9);
            this->methods->slotBC(this, TRANSLATE_Y_PLUS8);
        } else {
            u32 r;

            out->unk1C = 0x12;
            out->unk30 = 3;
            this->methods->slot16C(this);
            r = rand() & 1;
            this->unk44 = r < 1;
        }
    }
}
```

## Derivation notes

- `out->unk4 % 10 == 0` and `out->unk4 % 20` (checked twice, against 0 and
  against 0xE) both use the SAME `0x66666667` magic constant reciprocal --
  divisor 20's shift (3, one more than divisor 10's shift of 2) is
  consistent with 20 = 10*2, and was independently confirmed against the
  pinned `cc1` (`int f(int x){return x%20;}` reproduces the exact `sll
  2`/`addu`/`sll 2` reconstruction and final `subu`-to-remainder form,
  distinct from the `%10`/`%5`/`%30`/`%200`/`%300` idioms already
  documented in this unit's other reports).
- **The two `%20`-family checks and the `%10` check are three INDEPENDENT
  `if`s, not one combined dispatch**, even though they operate on the same
  dividend and (for the `%20` pair) partially share their multiply's
  hi/lo registers. The `%10==0` store to `out->unk1C` and the `%20`
  if/else-if are mutually non-exclusive at the C level (e.g. `unk4==20`
  satisfies both) -- disassembly confirms this: the `out->unk1C=0x1C` store
  from the `%10` check does not skip or gate the `%20` computation that
  follows it.
- **Residue: `this->unk44 = (rand() & 1) == 0;` compiled to `xori`+`andi`
  (an extra instruction) instead of retail's `andi`+`sltiu`.** Verified
  against the pinned `cc1` directly (not guessed): `g = (rand()&1)==0;`
  and `g = !(rand()&1);` both emit `xori`+`andi` for GCC 2.6.3, regardless
  of parenthesization; only hoisting the masked value into a named
  **unsigned** local and comparing THAT against `1` (`unsigned r = rand()
  & 1; g = r < 1;`) produces `andi`+`sltiu` (a signed local gives `slt`,
  not `sltiu` -- confirmed the opcode difference matters, not just
  instruction count). This is a genuinely different lowering path in
  GCC 2.6.3's equality-to-zero code, not a register-identity residue: the
  fix is a type+shape change (named unsigned temp + explicit `<`), not a
  declaration-order reshuffle.
- Everything else (`this->unk48 = (rand()&1) ? -0x176 : -0xC0`, the
  odd-tick `slot100` gate, the `unkFC < 8` split) matched on the first
  pass with no residue.

No new struct or vtable-slot knowledge; `slot148`, `slotC4`, `slot44`,
`slotBC`, `slot16C`, and `Unk94Methods::slot100` were all already typed
from earlier work in this unit. `ROTATION_ZPLUS9`/`TRANSLATE_Y_PLUS8` are new per-unit
`extern u8 [];` data-table externs, same convention as the rest of this
file.

### Proposed learning

**`x == 0` on a masked/bounded expression is not one canonical lowering in
GCC 2.6.3 -- signed vs. unsigned changes the opcode, not just whether an
extra instruction appears.** `(masked_expr) == 0` and `!(masked_expr)`
both lower to `xori`+`andi` (order matters: XOR happens first against the
UNMASKED value, then the mask is applied -- costing one instruction over
retail when retail used the cheaper form). To get `andi`+`sltiu` (one
instruction, and the CORRECT unsigned comparison opcode), the masked value
must be hoisted into a named **unsigned** local and compared with a bare
`<` against the bound; the identical comparison on a signed local instead
yields `andi`+`slt` (right instruction count, wrong opcode -- `slt` vs
`sltiu`, a class of near-miss easy to overlook since the word COUNT
matches). Verify with the pinned `cc1` directly rather than guessing at
which spelling GCC prefers; this is the third residue in this unit's
reports (after `Entity__MoodCue71`'s register-identity fix and
`Entity__MoodCue74`'s unsigned-cast fix) where the visible C had to diverge
from the "obvious" spelling to match retail's exact lowering.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 68 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`, `voice1Tone`, `voice1Pitch`, `voice2Tone`, `voice2Pitch`.

**Data constants renamed this round:** `D_80089CC4` -> `ROTATION_ZPLUS9` and `D_80089D6C` -> `TRANSLATE_Y_PLUS8`, tier B. Byte-decoded from `disk/SLPS_015.56` against the existing `sRotationYawPlus9`/`ROTATION_ZMINUS90`/`sTranslateYPlus256` tables: rotation tables are four s16 {num,den} pairs for X/Y(yaw)/Z/W, only one pair with den=1; translate tables are three consecutive s32 (X,Y,Z), only one nonzero. `D_80089CC4`'s nonzero pair is the THIRD (Z) slot at (9,1) -- same slot `sRotationX50YMinus120Z30` already confirmed is Z, so `ROTATION_ZPLUS9` follows `ROTATION_ZMINUS90`'s no-underscore single-letter-axis convention. `D_80089D6C`'s nonzero 32-bit slot is the second (Y) at +8, matching `TRANSLATE_Y_*`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). Local `r` is `coin` (rand() & 1); `state = coin < 1` is ENTITY_STATE_DONE half the time, else phase 0 (commented). program -2 is SOUND_CUE_STOP. Byte-identical (whole image green).
