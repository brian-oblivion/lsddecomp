# Entity__MoodCue43 -- MATCHED (61/61 words)

> Renamed from `func_800604DC` on 2026-09-24 (tools/rename.py). Address 0x800604dc.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch handler that
calls `Entity__func_80060710` (same unit, later ROM address) as a helper:
`void Entity__MoodCue43(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue43(Entity *this, EntityMoodHandlerArg *out) {
    s32 a1val;
    s32 r2;
    u8 *table;

    Entity__func_80060710(this);
    out->unk10 = this->methods->slot148(this);
    if (this->unk84 == 0 || this->unk84 == 0xF) {
        out->unk1C = 0x12;
        out->unk30 = 0x12;
    }
    if (this->unkFC >= 0x141) {
        a1val = (rand() & 1) ? -0x3C : 0x3C;
        this->methods->slotC8(this, a1val, 0);
        r2 = rand();
        table = ROTATION_YAW_PLUS9;
        if ((r2 & 3) != 0) {
            table = ROTATION_YAW_MINUS9;
        }
        this->methods->slot44(this, 0, table);
    }
}
```

## Derivation notes

- Calls `Entity__func_80060710`, which is defined LATER in this same file (higher
  ROM address). Needs a forward prototype above both definitions --
  `void Entity__func_80060710(Entity *this);` -- since C requires a declaration
  before use and `INCLUDE_ASM`/`void`-returning functions get no implicit
  declaration. The prototype is not itself a "function" for the ROM-order
  rule, only the definitions are.
- `Entity__func_80060710` takes a SINGLE argument (`Entity *this`), not the usual
  `(this, out)` pair -- confirmed by reading its own body (never touches a
  second incoming register) even though the CALL SITE here happens to have
  `a1` still holding this function's own `out` parameter at the call
  (unused by the callee, harmless). Typing the callee from its own body,
  not from what happens to be sitting in the caller's registers, avoided a
  wrong two-argument signature here.
- **The `ROTATION_YAW_PLUS9`/`ROTATION_YAW_MINUS9` table selection is the "default value,
  then conditionally overwritten" idiom** from
  `docs/DECOMPILATION_LEARNINGS.md`, not a ternary: retail loads
  `ROTATION_YAW_PLUS9` unconditionally right after the second `rand()` call, then
  overwrites it with `ROTATION_YAW_MINUS9` only if `rand() & 3 != 0`. Write it as
  `table = ROTATION_YAW_PLUS9; if (cond) table = ROTATION_YAW_MINUS9;`, not
  `table = cond ? ROTATION_YAW_MINUS9 : ROTATION_YAW_PLUS9;` -- the ternary form did not
  reproduce the load-then-conditionally-overwrite instruction shape when
  tried first.
- **The default assignment (`table = ROTATION_YAW_PLUS9;`) must come AFTER the
  `rand()` call that feeds the guarding condition, not before it**, even
  though it reads naturally to write it first. Assigning it before the call
  forces the compiler to keep `table` alive across the `jal`, promoting it
  to a callee-saved register (`s1`) where retail uses caller-saved `a2`
  because nothing calls between the default assignment and its last use.
  Introducing an explicit `r2 = rand();` local, then assigning the default
  from `r2`, fixed this. This is the SAME lever as `Entity__MoodCue41`'s
  `sScaleTemplateZDenom` pointer (see that report) applied to a plain data value
  instead of a pointer -- worth generalizing.

### Proposed learning

- **"Default value, then conditionally overwritten" needs its DEFAULT
  ASSIGNMENT positioned after any intervening call the guarding condition
  depends on**, not just written in default-then-override order at the
  source level. If the default is assigned before a call the condition
  needs, the value is forced into a callee-saved register across that call
  even though the retail source (and a naive C reading) would put the
  assignment first. Compute the call's result into its own local first,
  *then* assign the default from that.

## Naming

`Entity__MoodCue43` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 43, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity_d.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity_b.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
