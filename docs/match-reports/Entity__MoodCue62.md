# Entity__MoodCue62 -- MATCHED (141/141 words)

> Renamed from `func_80061C2C` on 2026-09-24 (tools/rename.py). Address 0x80061c2c.

Unit: `Entity_e` (round 13). The unit's longest match so far this round: a
`% 30` mood-code check, an `unkFC`-threshold `Class6B5CC__FaceTarget` call, an
unconditional `slotC4`, a compound `unkFC==0x12C && slot144()<0x1000`
vs. `unkFC==0x1F4` dispatch, a `rand()`-driven state machine that seeds
`out`'s fields, and a final combined-condition `slot30` call.
`void Entity__MoodCue62(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue62(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unk84 % 30 == 0) {
        out->unk1C = 3;
        out->unk10 = 0;
        out->unk20 = -2;
    }
    if (this->unkFC >= 0x65) {
        Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
    }
    this->methods->slotC4(this, -5, 0);
    if (this->unkFC == 0x12C && this->methods->slot144(this, this->unk94) < 0x1000) {
        this->unk94->methods->slot130(this->unk94, 0);
    } else if (this->unkFC == 0x1F4) {
        this->unk94->methods->slot134(this->unk94, 1, 1);
    }
    if (this->unk44 == 0) {
        if (this->methods->slot144(this, this->unk94) < 0x400) {
            if (rand() & 1) {
                out->unk30 = 6;
                out->unk10 = 0;
                out->unk34 = -1;
                if (rand() & 1) {
                    this->unk4C->methods->slot138(this->unk4C, -1, 0);
                }
                this->unkFC = 0;
                this->unk44 = 0xA;
            } else {
                this->unk44 = 0xB;
            }
        }
    }
    if (this->unk44 == 0xA && this->unkFC == 0x46) {
        this->methods->slot30(this, (rand() & 1) ? 0xC : 0xB);
    }
}
```

## Derivation notes

- `this->unk84 % 30 == 0` reuses the same divisor-30 magic constant
  (`0x88888889`, with the negative-magic `addu`-before-`sra 4` rounding
  correction) already confirmed in `Entity__MoodCue64`'s match report (its
  `out->unk4 % 300` uses a *different* divisor and thus a different magic
  constant/shift, but the same idiom family; `Entity__MoodCue66`'s `% 30` is the
  direct precedent for THIS exact magic/shift pair).
- **Residue: the `unkFC==0x12C` / `unkFC==0x1F4` dispatch is NOT
  `if (A) { if (B) C; } else if (D) E;`, it is `if (A && B) C; else if (D)
  E;` -- a compound condition on the first branch, not a nested one.**
  First attempt used the nested form and scored 69/141 with drift (a wrong
  in-range read past ~0x524d8, WARNING: differs outside range too). The
  nested reading calls `slot130` and then, on the *failure* path of the
  inner check, simply does nothing (skips the `unk94==0x1F4` check
  entirely by falling out of the outer `if`) -- but retail's disassembly
  shows the `unkFC==0x1F4` compare is reachable from BOTH the "outer
  condition false" path (`unkFC != 0x12C`, jumping straight into the
  `0x1F4` compare reusing the already-loaded `unkFC` value) AND the "outer
  true, inner false" path (`unkFC==0x12C` but `slot144(...) >= 0x1000`,
  which reloads `unkFC` and redundantly re-materializes the `0x1F4`
  constant before falling into the SAME compare) -- only the "both true"
  path (`slot130` call) explicitly jumps PAST the `0x1F4` check. That
  three-way reachability -- shared entry into one compare from two
  different upstream conditions, skipped only by the branch that already
  acted -- is exactly what `A && B` short-circuit evaluation produces and
  a nested `if` does not.
- The `rand()`-driven state machine reads as two SEPARATE `rand() & 1`
  calls (not one result reused): the outer roll picks between the
  `unk44 = 0xA` branch (which does real work: seeds `out->unk30`/
  `out->unk10`/`out->unk34`, an inner roll possibly firing
  `unk4C->methods->slot138`, then resets `unkFC` to 0) and the
  `unk44 = 0xB` branch (does nothing else). The inner `if (rand() & 1)`
  guarding `slot138` is itself unconditionally followed by
  `this->unkFC = 0; this->unk44 = 0xA;` regardless of whether the inner
  roll fired -- confirmed by the inner `beqz`'s target landing exactly on
  the `unkFC = 0` store, not past it.
- The final `slot30` call's second argument is a ternary
  `(rand() & 1) ? 0xC : 0xB`, reproducing retail's "set `a1 = 0xB`
  unconditionally in the branch's delay slot, then overwrite it to `0xC`
  only on the fallthrough (non-taken) path" shape -- the classic
  delay-slot-default-then-overwrite pattern for a boolean-selected
  literal.

No new struct or vtable-slot knowledge; every field/slot here (`slotC4`,
`slot144`, `slot130`, `slot134`, `unk4C`'s `slot138`, `slot30`) was already
typed from earlier work in this unit.

### Proposed learning

**When two branch paths converge on the SAME comparison, but only one of
several upstream paths skips it, that is `A && B` short-circuit form, not
nested `if`s.** The tell in the disassembly: the shared comparison's
operand gets redundantly RE-MATERIALIZED on at least one of the converging
paths (here, `v0 = 0x1F4` loaded twice -- once in the very first branch's
delay slot, again after the inner `slot144` check fails) because the
register holding it was clobbered along that path and the compiler has to
restore it before the shared compare. A single re-materialized delay-slot
constant reachable from two different upstream branches is a strong sign
of `&&`, worth checking before writing nested `if`s whenever a residue
shows drift concentrated right after a compound-looking condition.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 62 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `ENTITY_METHODS`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
