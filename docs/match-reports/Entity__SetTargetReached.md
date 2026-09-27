# Entity__SetTargetReached

> Renamed from `Entity__SetUnkF4` on 2026-09-24 (tools/rename.py). Address 0x8005daac.

> Renamed from `func_8005DAAC` on 2026-09-19 (tools/rename.py). Address 0x8005daac.

**Unit:** Entity · **Size:** 20 instructions · **Status:** MATCHED (20/20 words, whole-image build verified byte-exact)

## What it does

`Entity__SetTargetReached(Entity *this, s32 arg1)`: if `arg1` is nonzero, calls this
entity's own vtable slot `+0x030` with a literal `9`; unconditionally stores
`arg1` into `this->unkF4`.

## Derivation

```
move  $s1, $a1                ; s1 = arg1
beqz  $s1, .L8005DAE0          ; if (arg1 == 0) skip the call
 sw   $ra, 0x18($sp)            ; delay slot -- always runs regardless of branch
lw    $v0, 0x0($s0)
lw    $v0, 0x30($v0)
jalr  $v0                       ; this->methods->slot30(this, 9)
 ori  $a1, $zero, 0x9
.L8005DAE0:
sw    $s1, 0xF4($s0)            ; this->unkF4 = arg1
```

The `sw $ra,...` register save lands in the branch's delay slot (runs on
both paths), which is just ordinary callee-save scheduling, not something
the C needs to express explicitly.

## Final C

```c
void Entity__SetTargetReached(Entity *this, s32 arg1) {
    if (arg1 != 0) {
        this->methods->slot30(this, 9);
    }
    this->unkF4 = arg1;
}
```

## Attempt log

Matched on the first attempt. The single branch here never affects the
return value (the function is `void`, and the store after the branch runs
unconditionally either way) — not the `goto`-lever shape from
`New_Pad`/`New_TodActor`, just a plain `if`.

## Proposed learning

None new.

## Naming

**Tier A.** Renamed from `func_8005DAAC` this round (tools/rename.py). A
setter (`this->unkF4 = arg1`) plus a conditional notify when going nonzero
-- the store's mechanics fully describe the function even though `unkF4`'s
own broader significance (read by every Entity_x unit) is not established.

## Proposed field names

- `EntityMethods::slot30` -> `notifyParents` -- **tier B.** `tools/
  classtable.py` on `gEntityMethods` shows +0x030 occupied by the already-
  named `BasicClass__NotifyParents` (a slot inherited from the shared
  ancestor `GetTodActorMethods()` also returns -- same idiom, confirmed by
  offset match against that table). CROSS-UNIT: `slot30` is dispatched from
  every one of Entity_b/c/d/e/f/g (`grep -rn -- '->slot30(' src/Entity_*.c`)
  as well as this unit's own `Entity__SetTargetReached`/`Entity__NotifyIfTargetInRange` (the latter
  in Entity_b.c), so proposed rather than applied.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 3: parameter arg1 -> reached (the slot's own parameter name).

- Step 4: notifyParents(9) -> ENTITY_EFFECT_LOG_MOOD (the enum's own comment already named this sender).
