# Entity__MoodCue74 -- MATCHED (105/105 words)

> Renamed from `func_80062730` on 2026-09-24 (tools/rename.py). Address 0x80062730.

Unit: `Entity` (round 13). Ignores `out` entirely. A first-tick block
dispatches through a brand-new nested object (`unk94->unk5C`'s own
vtable), rolls `unk44 = rand() % 3` and clamps it to 0 based on a
`z`-position gate; the rest of the function is an `if (unk44 != 0)` /
`else` pair, each side making two more `Unk94Methods`-table calls (two new
slots, `slotC4` and `slot134`, discovered here).
`void Entity__MoodCue74(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue74(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        this->unk94->unk5C->methods->slot64(this->unk94->unk5C, sMoodCue74ClearColor);
        this->unk44 = rand() % 3;
        if (this->unk94->unk14->z < 0x262) {
            this->unk44 = 0;
        }
    }
    if (this->unk44 != 0) {
        if (this->unk80 / 2 < this->unkFC) {
            this->unk94->methods->slotC4(this->unk94, 0x80, 0);
        }
        if (this->unkFC == this->unk80 - 0x1E) {
            this->methods->slot30(this, 0xA);
        }
    } else {
        if ((u32)(this->unkFC - 0x14) < 0x64) {
            this->unk94->methods->slotC4(this->unk94, -((this->unkFC - 0x13) * 0x20), 1);
            if (this->unkFC == 0x55) {
                this->unk94->methods->slot134(this->unk94, 1, 1);
            }
        }
    }
}
```

## New struct knowledge (`include/Entity.h`, additive)

- **`Unk94Obj` gained a field, `unk5C`** (was previously only known through
  `+0x14`, unpadded past it): `u8 pad18[0x5C-0x18]; Unk5CObj *unk5C;`. It is
  itself a class-framework object -- dereferenced through its OWN vtable at
  `+0x00`/`+0x64`, same pointer-to-object-with-its-own-vtable convention as
  `Entity::unk4C`/`Entity::unk100`.
- **New type `Unk5CObj`/`Unk5CMethods`**, named for the offset it sits at in
  its immediate parent (`Unk94Obj`), the same convention `Unk94Obj` itself,
  `Unk4CObj` and `Unk100Obj` already use for the offset THEY sit at in
  `Entity`. Only `slot64(self, arg1)` is known (called here as
  `slot64(unk94->unk5C, sMoodCue74ClearColor)`, no third argument set up before the
  `jalr` -- `a2` is never written on this call path).
- **`Unk94Methods` gained two slots**, both split out of previously-opaque
  padding ranges (no existing field moved or retyped):
  - `slotC4` (`void (*)(Unk94Obj*, s32, s32)`), was inside `pad0BC[0xCC-0xBC]`
    -- called here as `slotC4(unk94, 0x80, 0)` and
    `slotC4(unk94, -N, 1)`, return discarded at both, so `void` per the
    project's standing "discarded return is not positive void evidence, but
    is a safe read" convention already used for this table's other slots.
  - `slot134` (`void (*)(Unk94Obj*, s32, s32)`), was inside `pad134[0x1A0-0x134]`
    (immediately after `slot130`) -- called as `slot134(unk94, 1, 1)`,
    return discarded, same caveat.
- `sMoodCue74ClearColor` (a single-word data table at `asm/data/7B3F8.sdata.s`,
  immediately after the already-known `gEntityFadeBoxDefaultSize`/`gEntityFadeBoxDefaultOffset`) gets its
  own per-unit `extern u8 sMoodCue74ClearColor[];` in `Entity.c`, same convention
  as this file's other opaque data-table externs.

**No existing declaration was retyped, renamed, or resized** -- every change
above either fills previously-unlabeled padding or extends a struct past its
previous known extent. Safe to union with any other unit's concurrent view
of `Unk94Methods`/`Unk94Obj` as long as it did not independently claim the
same byte ranges.

## Derivation notes

- **A register-identity-free "divisible by 3" and "value halved" idiom**,
  both already-confirmed patterns from earlier reports in this unit
  (`Entity__MoodCue67`'s `rand() % 3`, and `this->unk80` field's own
  documented `(x + (unsigned)x>>31) >> 1` signed-halving comment in
  `include/Entity.h`).
- **`(u32)(this->unkFC - 0x14) < 0x64` needs an explicit unsigned cast** to
  reproduce retail's `sltiu`. Writing the bare signed subtraction/compare
  (`this->unkFC - 0x14 < 0x64`) compiles to `slti` (opcode 0x28) instead of
  `sltiu` (opcode 0x2C) -- one word, byte-identical count, wrong opcode.
  This is the range-check idiom `a >= X && a < X+N` folded to
  `(unsigned)(a-X) < N`; GCC only takes this path when the comparison is
  ALREADY unsigned, so the source has to say so explicitly rather than
  relying on the fold to happen from two signed comparisons.
- **The `unkFC == 0x55` check is NESTED inside the range-check `if`, not a
  sibling `if` after it -- a control-flow residue, not a register one.**
  Writing it as a second top-level `if` in the `else` block scored 104/105:
  every instruction byte-identical except the range check's `beqz` target,
  which pointed past BOTH remaining blocks in a from-scratch sibling
  version but only past the FIRST in retail (retail's failure path skips
  straight to the function epilogue). Since `0x55` already lies inside
  `[0x14, 0x78)`, nesting is also the semantically tighter reading: the
  `unkFC == 0x55` branch is dead code whenever the range check fails, and
  retail's control-flow graph reflects exactly that redundancy instead of
  re-testing it. Confirms CLAUDE.md's "branch targets disagree -> CFG
  difference, not a scheduling artifact" discriminator before touching
  anything else.

### Proposed learning

**An unsigned range-check idiom (`(unsigned)(x - LO) < N` for `x in
[LO, LO+N)`) needs an explicit cast in the C to get `sltiu` rather than
`slti`** -- GCC 2.6.3 does not infer unsignedness from two signed
comparisons being combinable; the source has to already be unsigned typed
at the subtraction. Byte-identical instruction count either way, only the
opcode's low bits differ, so this is easy to misdiagnose as "close enough."

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 74 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity.

**`sMoodCue74ClearColor` left unnamed this round.** Already documented (this report's own body, pre-rename) as a single-word data table adjacent to `gEntityFadeBoxDefaultSize`/`gEntityFadeBoxDefaultOffset`, not a rotation/scale/translate-style {num,den} or s32-triple table, and passed to `Unk5CObj::slot64` whose own purpose is unestablished -- no evident value to name it from.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). `D_8008AC1C` -> `sMoodCue74ClearColor` (`python3 tools/rename.py`, tier A): a ViewportRgb {0, 100, 190} in .sdata (the word 0x00BE6400) whose address this function alone passes to the viewport's setClearColor; declared as the ViewportRgb it is, so the cast went. notifyParents' 10 is ENTITY_EFFECT_LINK_STAGE; setTickCallbacks(1, 1) is MOVE_CALLBACK_TICK_MOVE, LOOK_CALLBACK_STEP_LOOK. The range test `(u32)(moodTimer - 0x14) < 0x64` reads as `moodTimer >= 20 && moodTimer < 120`: the same bytes, cc1 folds the pair into the unsigned test (as in Entity). Byte-identical (whole image green).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
