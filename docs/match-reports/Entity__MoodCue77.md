# Entity__MoodCue77 -- MATCHED (134/134 words)

> Renamed from `func_80062A40` on 2026-09-24 (tools/rename.py). Address 0x80062a40.

Unit: `Entity_e` (round 13). A four-way dispatch: `slot148`+`%5` mood
setup, then an early-return branch on a new gate field (`unk7C`), then a
second early-return branch on `unk44 == 0` doing an unkFC-literal-set
check, and finally the fallthrough path doing the same check against two
different literals plus a threshold and an exact match.
`void Entity__MoodCue77(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue77(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % 5 == 0) {
        out->unk1C = 0x11;
        out->unk20 = -2;
    }
    if (this->unk7C == 0) {
        if (this->unkFC == this->unk80) {
            this->methods->slot128(this, 1);
            if (rand() & 1) {
                this->unk44 = 0xB;
            }
        }
        return;
    }
    if (this->unk44 == 0) {
        if (this->unkFC == 0x3C || this->unkFC == 0xD4 || this->unkFC == 0x122 || this->unkFC == 0x140) {
            this->methods->slot44(this, 0, ROTATION_YAW_PLUS90);
        }
        if (this->unkFC == 0x18E) {
            this->methods->slot44(this, 0, ROTATION_YAW_MINUS90);
        }
        this->methods->slotD0(this, -0x32, 0);
        return;
    }
    if (this->unkFC == 0x3C || this->unkFC == 0x8C) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS90);
    }
    if (this->unkFC < 0xAE) {
        this->methods->slotD0(this, -0x32, 0);
    }
    if (this->unkFC == 0xAE) {
        this->methods->slot16C(this);
        this->unk44 = 1;
    }
}
```

## New struct knowledge (`include/Entity.h`, additive)

- **`Entity` gains `unk7C`** (`s32`), split out of the previously-opaque
  `pad5C[0x80-0x5C]` range (now `pad5C[0x7C-0x5C]` + `unk7C` + `unk80`
  starting immediately after, no trailing pad needed).
- **`EntityMethods` gains `slot128`** (`void (*)(Entity*, s32)`), split out
  of `pad118[0x12C-0x118]` (now `pad118[0x128-0x118]` + `slot128`, with
  `slot12C` starting immediately after). Return discarded at its one known
  call site, `void` per this table's existing convention for such wrappers.
- `slot16C`'s existing comment gained this function as a third caller
  (append-only, no signature or caller-list entry removed).

**No existing field was retyped, renamed, resized, or moved** -- purely new
fields carved from previously-unlabeled padding, plus one comment append.

## Derivation notes

- The `out->unk4 % 5 == 0` check is the `0x66666667`/shift-1 magic-constant
  idiom for divisor 5, verified against the pinned `cc1`
  (`int f(int x){return x/5;}`), continuing the shift-fingerprints-the-
  divisor technique used throughout this unit's reports (5, 10, 30, 200,
  300 now all confirmed distinct shifts).
- **Both `unk7C == 0` and `unk44 == 0` blocks end the function with an
  explicit `return;`, mid-body, ahead of code that keeps running for the
  opposite case.** Each compiles to an unconditional `j` straight to the
  epilogue, confirmed by matching bytes for both branches -- there is no
  way to express "skip everything else" other than an early return once the
  function has more code after the block (an `if`/`else` wrapping the
  REST of the function would also work and is behaviorally identical, but
  the two-early-return reading kept each block's own logic visually
  self-contained and was the first form tried; it matched immediately).
- **A literal set membership test (`unkFC == 0x3C || ... || unkFC ==
  0x140`) compiles to a `beq`/`beq`/`beq`/`bne`-to-skip chain that shares
  its "then" block with a merge point**, not four independent `if`s each
  calling `slot44` separately -- confirmed by the single call site at
  `.L80062B40` that all three `beq`s target directly and the `bne`
  (`unkFC != 0x140`) skips past. Modeled directly as one `||`-chained
  condition guarding one call, which reproduced this exactly.
- `ROTATION_YAW_PLUS90`/`ROTATION_YAW_MINUS90` reuse this file's existing per-unit externs
  (already declared earlier in `Entity_e.c` for `Entity__MoodCue65`/
  `Entity__MoodCue73`); no new externs needed here.

No other new struct or vtable-slot knowledge; `slot148`, `slot44`, `slotD0`,
and `slot16C` were all already typed from earlier work in this unit.

### Head-broadcast levers -- applicability to this function/unit

Addressing the round-13 head broadcast (levers on `~x+1`-vs-`-x`,
multi-walker array induction, and switch/jump-table drift sizing):

- **Lever 1 (`~x + 1` vs `-x`)**: not encountered. Every negation in this
  function and the rest of this round's queue (`Entity__MoodCue74`'s
  `-((...) * 0x20)`, etc.) is a plain `negu`/`li -N`-style literal or
  arithmetic negation, no `nor`+`addiu` pair seen in any of this unit's
  `.s` files screened so far.
- **Lever 2 (multi-walker array induction)**: not applicable. Nothing in
  this unit's remaining queue loops over an array at all -- every function
  handled this round (`Entity__MoodCue69`, `Entity__MoodCue67`, `Entity__MoodCue79`,
  `Entity__MoodCue71`, `Entity__MoodCue65`, `Entity__MoodCue59`, `Entity__MoodCue74`,
  `Entity__MoodCue77`) is straight-line mood-handler dispatch logic with `if`
  chains, no loop construct anywhere in their disassembly.
- **Lever 3 (switch/jump-table drift)**: not applicable. None of this
  unit's functions compile to a `switch`; every dispatch seen is `beq`/
  `bne` chains on discrete `unkFC` literals, not a dense jump table, so
  funcdiff's whole-image byte count has been a trustworthy oracle for
  every function matched in this unit this round (each also cross-checked
  with `build exit=0` and a clean word-count match, never drift).
- **Blocker screen on this function**: `grep -nE 'gp_rel|addiu *\$at,
  *\$at, *%lo'` against `Entity__MoodCue77.s` -- no hit, consistent with round
  head's own pre-screen of this queue.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 77 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

Reading this function's `out->` writes with the proposed `SoundCueSet` field names (`Entity__MoodCue07.md` `## Proposed field names`, tier A/B, proposal only -- `EntityMoodHandlerArg` is shared with Entity/Entity_d/Entity_g): `tick`, `attenuation`, `voice0Tone`, `voice0Pitch`.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 93, echo)

Every literal in the live body is in its base: decimal for moodTimer and cue-set ticks, TOD frames, distances, VAB programs and `state` phases (the unit has no hex literal left). state 1 after stopSoundCue is ENTITY_STATE_DONE (include/Entity.h: Entity__UpdateActivationState will not re-activate it). Byte-identical (whole image green).
