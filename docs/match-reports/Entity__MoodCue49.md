# Entity__MoodCue49 -- MATCHED (111/111 words)

> Renamed from `func_80060B34` on 2026-09-24 (tools/rename.py). Address 0x80060b34.

Unit: `Entity` (second pass, round 2026-09-03). The largest and most
structurally complex function matched in this unit so far -- a three-way
state machine on `this->unk44` (0, 0xA, 0xB), each state doing its own
dispatch into `this->unk94`'s vtable.
`void Entity__MoodCue49(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue49(Entity *this, EntityMoodHandlerArg *out) {
    Unk94Methods *methods94;
    void *a1;

    if (out->unk4 == 6) {
        out->unk10 = 0;
        out->unk1C = 4;
        out->unk30 = 4;
        out->unk44 = 4;
    }
    if (this->unkF4 != 0) {
        if (this->unk44 == 0) {
            this->unk44 = 0xA;
            this->unkFC = 0;
        } else if (this->unk44 == 0xA) {
            if (this->unkFC == 0xA) {
                this->methods->slot30(this, 0xA);
            } else if (this->unk94->methods->slot100(this->unk94) != 0) {
                methods94 = this->unk94->methods;
                a1 = this->unk0C ? (u8 *)this->unk14 + 0x38 : NULL;
                methods94->slotB8(this->unk94, a1);
                this->unk94->methods->slot44(this->unk94, 1, ROTATION_YAW_MINUS90);
                this->unk94->methods->slot130(this->unk94, 0);
                this->unkFC = 0;
                this->unk44 = 0xB;
            }
        } else if (this->unk44 == 0xB) {
            methods94 = this->unk94->methods;
            a1 = this->unk0C ? (u8 *)this->unk14 + 0x38 : NULL;
            methods94->slotB8(this->unk94, a1);
            if (this->unkFC == 0x64) {
                this->methods->slot30(this, 0xA);
            }
        }
    }
    this->methods->slotC4(this, -0x100, 0);
}
```

## Derivation notes

Matched first attempt, no iteration needed, despite being the largest and
most branch-heavy function tackled in this unit -- worth recording WHY it
went cleanly, since the shape recurs.

- **Two new `Unk94Methods` vtable slots discovered in one function:
  `slot44` (`void (*)(Unk94Obj*, s32, void*)`, called as `slot44(unk94, 1,
  ROTATION_YAW_MINUS90)`) and `slotB8` (`void (*)(Unk94Obj*, void*)`, called twice
  with the identical argument-computation shape). Both carved out of the
  struct's leading pad gap, ahead of the already-known `slotCC` at `+0xCC`.
  `Unk94Obj` now has five resolved slots (`+0x44`, `+0xB8`, `+0xCC`,
  `+0x100`, `+0x130`, `+0x1A0`, `+0x200` -- seven, correcting the count) out
  of an unknown total, entirely from this unit's own functions.
- **The `slotB8` argument is `this->unk0C` acting as a boolean gate on
  whether to pass `(u8 *)this->unk14 + 0x38` or `NULL`.** `+0x38` is past
  the end of the currently-modeled `EntityPos` struct (which only reaches
  `+0x24`), so this is raw byte-pointer arithmetic on `unk14`, not a named
  field access -- `unk14` evidently points to a larger structure than
  `EntityPos` models, consistent with `EntityPos`'s own header comment
  noting its size is unconfirmed beyond the fields actually read.
- **`this->unk94->methods` is fetched into an explicit `methods94` local
  ONLY for the two `slotB8` call sites, and NOT for the following
  `slot44`/`slot130` calls, which stay as fresh `this->unk94->methods->...`
  chains.** This reproduces retail's own asymmetric caching: retail loads
  `unk94->methods` once into a scratch register right before computing the
  `a1` argument and issuing the `slotB8` call (no intervening call between
  the load and its use, so no promotion needed), then RELOADS it fresh for
  every subsequent vtable call because `slotB8` itself is an intervening
  indirect call that could have written back through the object (the
  general "value reused after an intervening indirect call needs an
  explicit local" trap from `docs/DECOMPILATION_LEARNINGS.md`, but applied
  in the NEGATIVE direction here -- retail deliberately does NOT cache
  across that call, and writing the later calls as fresh chains, not
  through `methods94`, was what reproduced it).
- **The `unk44 == 0xA` and `unk44 == 0xB` branches share the exact
  `this->methods->slot30(this, 0xA)` call** (reached via `unkFC == 0xA` in
  the first, `unkFC == 0x64` in the second) -- writing it twice, once per
  arm, as an ordinary duplicate statement reproduced retail's shared tail
  block via the compiler's own cross-jump merge, no `goto` needed (same
  family as `Entity__MoodCue44`'s three-branch case: each call site is a
  complete, self-contained statement, so GCC merges the trailing bytes on
  its own without any source-level sharing).
- The ternary `this->unk0C ? (u8 *)this->unk14 + 0x38 : NULL` reproduced
  retail's `beqz`/delay-slot-`addiu` idiom for computing one of two pointer
  values directly, with no residue.

### Proposed learning

- **A vtable pointer cached for ONE call site but not the ones after it (in
  the same function) is not a contradiction -- it is retail correctly NOT
  caching across an intervening indirect call while correctly caching
  where there is no such call in between.** Read each call site's own
  surrounding instructions for whether a reload happens; do not assume a
  single caching policy applies uniformly across one function just because
  it applied at the first site.

## Naming

`Entity__MoodCue49` -- tier B (round 76, runner delta, FINISHING-PLAN track 3). Same
convention as `Entity__MoodCue00` (round 71): the function's address is the
handler word of `gEntityMoodHandlerTable` (`asm/data/79528.data.s`, base
0x80089EB0, 0x10-byte stride) at row 49, read directly from
`disk/SLPS_015.56` (not inferred from address proximity -- see
`src/Entity.c`'s unit header comment, which flags that row order does NOT
track code address once row 115 is reached). Mechanics established
(mood-tick sound-cue-set callback, per `Entity__StartSoundCue`/
`Entity.c`'s own header comment); which dream object owns the row is not.

## Track 4 (2026-09-26, round 88, echo)

`(u8 *)this->coord2 + 0x38` is now `this->coord2->unk38` (SceneNode's world position), same bytes.

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, bravo)

Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs, volumes and `state` phases (hex remains only for masks). Named: `ENTITY_EFFECT_LINK_STAGE` (evidence on each definition: EntityEffect and ENTITY_STATE_DONE in include/Entity.h, SOUND_CUE_STOP in include/SoundCueSet.h). clearTickCallbacks' bool clearLook is `false`. Byte-identical (whole image green).

## Proposed field names

- `SceneNodeSub14::unk38` (the `coord2->unk38` this handler passes to the
  peer's setTranslation): include/SceneNode.h already documents it as
  `workm.t`, the world position, and SceneNodeSub14 is the node's
  GsCOORDINATE2 ("GsDOBJ2.coord2: the ctor's 0x50-byte GsCOORDINATE2"). The
  fix is track 6's: Sony's GsCOORDINATE2 in place of SceneNodeSub14, which
  makes this `coord2->workm.t`. Not applied here (a shared header, many
  accessors).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
