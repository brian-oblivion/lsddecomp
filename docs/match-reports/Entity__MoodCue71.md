# Entity__MoodCue71 -- MATCHED (60/60 words)

> Renamed from `func_80062570` on 2026-09-24 (tools/rename.py). Address 0x80062570.

Unit: `Entity_e` (round 13). Dispatches `slot148`, and on `out->unk4 == 0`
picks one of three `slotC8` variants via `rand() % 3`; separately fires
`SceneNode__FaceTarget` once `unkFC` crosses a threshold, then unconditionally
calls `slotC4`. `void Entity__MoodCue71(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue71(Entity *this, EntityMoodHandlerArg *out) {
    s32 r;

    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = 0;
        r = rand() % 3;
        this->methods->slotC8(this, r * 51200, 0);
    }
    if (this->unkFC >= 0x961) {
        SceneNode__FaceTarget(this, this->unk94, 1, 0, 0);
    }
    this->methods->slotC4(this, -0x1E, 0);
}
```

## Derivation notes

- `out->unk10 = this->methods->slot148(this)` is written UNCONDITIONALLY:
  the store lives in the `bnez`'s delay slot, which MIPS executes before
  the branch on `out->unk4 == 0` is even decided.
- `r * 51200` (where `r` is `rand() % 3`, so `r` is one of 0/1/2) reproduces
  retail's `slli 1 / addu / slli 3 / addu / slli 11` strength-reduction of
  the constant multiply -- confirmed against the pinned `cc1`
  (`int f(int x){return x*51200;}` emits the identical five-instruction
  sequence byte-for-byte).
- **Register-identity residue, closed by hoisting `rand() % 3` into a named
  temp.** The first attempt inlined it as `(rand() % 3) * 51200` directly
  in the `slotC8` call's argument list; that scored 49/60 with EVERY
  differing word confined to the mod/mul block, always the SAME
  instructions in the SAME order, just swapped between `a1`/`v1`/`a3` (e.g.
  retail's `subu a3,v1,a1` came out `subu a1,a1,v1`). Per
  `docs/MATCHING-GUIDE.md`'s residue guide, this is exactly the
  declaration-order-driven register-identity class, not a toolchain issue --
  giving the intermediate remainder its own `s32 r` local (matching how the
  neighboring `Entity__MoodCue64`/`Entity__MoodCue66` in this same file already
  hoist their own modulo results into a named `r`) resolved it to a clean
  60/60 with no other source change.
- `this->unkFC >= 0x961` reads off `slti $v0,$v0,0x961` + `bnez` skipping the
  call when the `slti` result is 1 (i.e. `unkFC < 0x961`); the call fires on
  the complementary condition.
- `slotC8` (`void (*)(Entity*, s32, s32)`) and `slotC4` (`void (*)(Entity*,
  s32, s32)`) are both already-typed vtable slots from earlier work in this
  unit; no signature changes.

No new struct or vtable-slot knowledge.

### Proposed learning

**When a call argument is `(rand() % N) * K` (or any modulo-then-scale
expression), hoist the modulo into a named local before writing the
multiply/call.** Inlining it directly reproduces the identical instruction
sequence but with `a1`/`v1`/`a3` register roles swapped relative to retail
-- a pure register-identity residue, resolved by naming the intermediate
the same way this unit's other mod-result locals (`r` in `Entity__MoodCue64`,
`Entity__MoodCue66`) are already named. Consistent with, and reinforcing, the
existing "declaration-order or lifetime difference" entry in
`docs/MATCHING-GUIDE.md`'s residue guide.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 71 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`.

Already cross-unit called before this rename: `Entity__MoodCue108` (Entity_g) forwards its own `(this, out)` straight through to this function; `include/Entity.h` carried its extern declaration under the old name and is updated by this rename (tree-wide, via `tools/rename.py`).

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). Local `r` (rand() % 3, times 51200 along x) is `lane`. Byte-identical (whole image green).
