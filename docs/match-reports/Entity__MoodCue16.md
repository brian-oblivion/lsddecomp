# Entity__MoodCue16

> Renamed from `func_8005ED30` on 2026-09-23 (tools/rename.py). Address 0x8005ed30.

**Unit:** Entity_b · **Size:** 124 words · **Status:** MATCHED (124/124
words, whole-image build verified byte-exact)

## What it does

`(Entity *this) -> void`. Another `this->unk44` state-machine step (see
`Entity__MoodCue13.md`/`Entity__MoodCue12.md` for siblings):

1. If `this->unkFC == 0` and `(rand() & 1) != 0` (odd — NOTE the opposite
   polarity from `Entity__MoodCue12`'s `== 0`/even check; do not assume symmetry
   between sibling coin-flip gates), sets `this->unk44 = 0xB`.
2. If `this->unk44 == 0`, three-way dispatch on `this->unkFC`:
   - `< 0x40` (64): `this->methods->slotC4(this, -0x5A, 0);`
   - `== 0x40`: rolls `rand() & 1` to pick between two data rows
     (`ROTATION_YAW_MINUS90` default, `ROTATION_YAW_PLUS90` on a hit), calls
     `this->methods->slot44(this, 0, arg2);` then
     `this->methods->slotBC(this, TRANSLATE_Y_PLUS256);`
   - `> 0x40`: `this->methods->slotD0(this, -0x176, rand() % 2);`
3. Else if `this->unk44 == 0xB`: if `this->unkFC % 5 == 0`, calls
   `this->methods->slot44(this, 0, ROTATION_YAW_PLUS90);`; unconditionally calls
   `this->methods->slotC4(this, -0x800, 0);` then
   `this->methods->slot60(this, (rand() % 7) == 0);`.

## New field: `EntityMethods::slotD0`

`void (*)(Entity *self, s32 arg1, s32 arg2)`, called as `slotD0(this,
-0x176, rand() % 2)`. New slot; the offset's numeric proximity to
`Unk100Methods::slotD0` (a completely different struct/table) is coincidence
— see `Entity__MoodCue01.md` for why shared offsets across different tables
don't imply a shared occupant.

`EntityMethods::slot60` (already declared, 2-arg) is reused here as-is —
`this->methods->slot60(this, (rand() % 7) == 0)` type-checks and matches
directly against the existing `void (*slot60)(Entity *self, s32 arg1)`.

Three more `D_8008xxxx` opaque data rows declared at the top of this file:
`ROTATION_YAW_MINUS90`, `ROTATION_YAW_PLUS90`, `TRANSLATE_Y_PLUS256`.

## Final C

```c
void Entity__MoodCue16(Entity *this) {
    u8 *arg2;
    s32 roll;

    if (this->unkFC == 0) {
        if ((rand() & 1) != 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0) {
        if (this->unkFC < 0x40) {
            this->methods->slotC4(this, -0x5A, 0);
        } else if (this->unkFC == 0x40) {
            roll = rand() & 1;
            arg2 = ROTATION_YAW_MINUS90;
            if (roll != 0) {
                arg2 = ROTATION_YAW_PLUS90;
            }
            this->methods->slot44(this, 0, arg2);
            this->methods->slotBC(this, TRANSLATE_Y_PLUS256);
        } else {
            this->methods->slotD0(this, -0x176, rand() % 2);
        }
    } else if (this->unk44 == 0xB) {
        if (this->unkFC % 5 == 0) {
            this->methods->slot44(this, 0, ROTATION_YAW_PLUS90);
        }
        this->methods->slotC4(this, -0x800, 0);
        this->methods->slot60(this, (rand() % 7) == 0);
    }
}
```

## Attempt log

Two attempts. First attempt wrote `arg2 = ROTATION_YAW_MINUS90; if ((rand() & 1) !=
0) { arg2 = ROTATION_YAW_PLUS90; }` directly (default assignment textually before the
`rand()` call). This compiled the default assignment BEFORE the `jal rand`
in the RTL, forcing `arg2`'s live range across the call and promoting it
into a callee-saved register (`$s1`) — 2 extra words (a spurious
`sw`/`lw $s1` prologue/epilogue pair) that shifted every later function's
address (funcdiff read `2/124` with a 6-figure "outside range" count; `nm`
on the built ELF confirmed `Entity__MoodCue17` landed 8 bytes past retail).
Retail computes the default assignment AFTER `rand()` returns, keeping
`arg2` entirely in caller-saved `$a2` with no register pressure across the
call. Hoisting the `rand()` call into its own statement BEFORE the default
assignment (`roll = rand() & 1; arg2 = ROTATION_YAW_MINUS90; if (roll != 0) ...`)
matched immediately — with the call now textually first, GCC's RTL for the
default assignment falls after it, and no callee-saved register is needed.

## Proposed learning

**Textual/RTL statement order — not just data dependency — determines
whether a value spanning a `jal`/`jalr` needs a callee-saved register in
this compiler.** `arg2 = DEFAULT; if (cond_using_a_call) arg2 = OTHER;` and
`tmp = cond_using_a_call; arg2 = DEFAULT; if (tmp) arg2 = OTHER;` are
semantically identical and data-independent either way, but GCC 2.6.3
apparently does NOT reorder the independent default-assignment across the
call on its own — whichever order the SOURCE STATEMENTS appear in is the
order the RTL (and therefore the live-range-vs-call analysis) sees. When a
residue is "same values, but an extra callee-saved register save/restore
pair inflating the LENGTH," check whether a call inside the guarding
condition can be hoisted to before an unrelated assignment that doesn't
need to survive it. This is a new, sharper case of the existing
`docs/DECOMPILATION_LEARNINGS.md` "let GCC hoist its own loop invariants"
family of lessons, but for a straight-line default-value assignment rather
than a loop.

## Naming

`Entity__MoodCue16` -- tier B (round 71, runner echo, FINISHING-PLAN track 3). Renamed from `func_8005ED30`.

`gEntityMoodHandlerTable` row 16. Body (takes only `this`): on moodTimer 0 a coin flip may set phase `unk44 = 0xB`. In phase 0 it moves -0x5A below moodTimer 64; at 64 it turns +/-90 degrees at random and steps `TRANSLATE_Y_PLUS256`; after that it runs slot +0xD0(-0x176, rand() % 2). In phase 0xB it turns +90 every 5 ticks, moves -0x800, and calls slot +0x60 with `rand() % 7 == 0`.

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row NN (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), and nothing else references it. `Entity__StartSoundCue` passes `gEntityMoodHandlerTable[this->moodIndex].handler` to `InitSoundCueSet`, which stores it as `SoundCueSet::callback`; `ServiceSoundCueSet` (the per-tick cue driver, via `Entity__TickSoundCue`) resets the set's three voice slots and calls `callback(set->owner, set)` every tick. So the handler is the mood row's per-tick cue callback, and `out` is the `SoundCueSet` (`EntityMoodHandlerArg` is Entity.h's local view of it; see `Entity__MoodCue07.md` `## Proposed field names`). Tier B: the mechanics are established, which dream object a row belongs to is not. The row number is kept decimal and zero-padded so the names sort in table order.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4b (round 93, charlie) — 2026-09-26

The motion templates are declared once, in `include/Entity.h` (`ROTATION_*`/`SCALE_*` as `Ratio16[]`, `TRANSLATE_*` as `LongVec3[]`); the unit-local `u8[]` externs are gone. The local `arg2`, which holds `ROTATION_YAW_MINUS90` or `ROTATION_YAW_PLUS90` and is passed to `updateRotation`, is now `Ratio16 *` (was `u8 *`). A pointer local's pointee type changes no instruction and the slot takes `void *`, so the bytes held: whole image green, 0 new `-Wall` warnings, nonmatching green.

## Track 7 (round 94, delta)

Local `arg2` (an m2c name) renamed `turn` (tier A: the rotation template picked by the coin flip and passed to updateRotation). Every literal in the live body is in its base: decimal for moodTimer ticks, distances, TOD frames, VAB programs and `state` phases (hex remains only for masks). Byte-identical (whole image green).
