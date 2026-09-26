# Entity__MoodCue85

> Renamed from `func_80063874` on 2026-09-25 (tools/rename.py). Address 0x80063874.

**Unit:** Entity_f · **Size:** 211 words · **Status:** MATCHED (211/211 words)

## What it does

The biggest function in this unit: a `this->unk44`-keyed state machine
(values `0`, `0xA`, `0xB`, `0xD`, `0xC`, in retail's ACTUAL test order —
see below) driving this Entity's mood/AI transitions. Calls
`SetCueTones7_7_7`/`SetCueTones18_3_3` (this unit's own trivial `out`-only
setters, matched separately) at several transition points, plus
`SceneNode__FaceTarget`, `Entity__GetOrCreateUnk100`, and two new `Unk100Methods` slots
(`slotD4`, `slotD8`).

## Derivation

**The same delay-slot-carried-comparison-value trap documented in
`Entity__MoodCue84`'s report, but this time it swapped which of TWO WHOLE
CODE BLOCKS belonged to `this->unk44 == 0xC` vs `== 0xD`** — a much more
consequential instance of the same mechanism, worth its own writeup.

The dispatch chain (`v1 = this->unk44`, tested link by link) actually runs
`0 → 0xA → 0xB → 0xD → 0xC`, not `0 → 0xA → 0xB → 0xC → 0xD` as a
first read suggested. The tell: at the link after `0xB`'s `bne`, the
delay slot loads `ori $v0, $zero, 0xD` — that value is what the NEXT
link's `bne` compares `this->unk44` against, i.e. the block immediately
following belongs to `unk44 == 0xD`, not `0xC`. The block's own CONTENT
(the `unkFC < 0x5A` / `Entity__GetOrCreateUnk100(...,0xA,...)` / `slotD8` /
`slot44(unk94,0,ROTATION_ZPLUS1)` logic, vs. the sibling block's `unkFC < 0xA`
/ `slot44(this,0,ROTATION_ZMINUS9)` / `SetCueTones18_3_3`+`slot16C`+`unk44=1`
logic) was derived correctly on the first pass; only the LABEL — which
`unk44` value routes to which block — was backwards, discovered by
re-tracing every delay-slot value link-by-link rather than reading each
`bne`'s neighboring `ori` as its own test constant.

**Getting the chain order right matters for more than labeling accuracy**:
an `if`/`else if` chain in C must test values in the SAME order the source
did, because the compiler's register/test-count reflects DECLARATION
order, not just which value matches which body. Writing the chain as
`0xC` before `0xD` (swapped) would have produced the wrong test SEQUENCE
even with each block's own content correct — confirmed by initially
building a version with the correct bodies but wrong order/labels and
getting a large word-count mismatch, only resolved by swapping to the
retail-verified `0xD`-before-`0xC` order.

Also hit, independently, the branch-polarity residue from
`Entity__MoodCue82`'s report: `this->methods->slot30(this, (rand() % 5 == 0)
? 0xC : 0xA)` compiled with the ternary's internal branch inverted
relative to retail. Fixed the same way — write the negated condition
(`!= 0`) with the arms swapped, no other change:
`(rand() % 5 != 0) ? 0xA : 0xC`.

## Header additions (`include/Entity.h`, additive only)

- `Unk100Methods::slotD8` — new slot at `+0xD8`, immediately after the
  existing `slotD4`.
- Comment addition to the EXISTING `Unk100Methods::slotD4` noting this
  function as a second caller (`slotD4(this->unk100, this->unk50, 7, 0)`,
  a different `arg2` from `Entity__MoodCue57`'s `4`). No type or name changed.

## Proposed learning

See `Entity__MoodCue84`'s report for the general delay-slot-chain lesson; this
function is the sharper example because the mislabeling swapped two
entire bodies rather than one stored constant, and because a chain's
LABELING error compounds with an ORDERING error if the C is written with
the (wrong) labels in their (also wrong) apparent order — both had to be
fixed together, not independently. **When any chain has 3+ links, trace
`v0`'s value into EVERY subsequent link explicitly (write it down) before
assigning C bodies to conditions**, and write the `if`/`else if` chain in
the SAME order as the traced links, not in numeric or "natural" order of
the constants themselves.

## Naming

Round 79, runner alpha.

| name | tier | evidence |
| --- | --- | --- |
| `Entity__MoodCue85` | B | `gEntityMoodHandlerTable` row 85 |

Why `MoodCue85`: the function's address is the `handler` word of
`gEntityMoodHandlerTable` row 85 (base 0x80089EB0, stride 0x10; the row's
first word), read from `disk/SLPS_015.56` directly rather than inferred from
address order (rounds 76-77 measured that row order does not track code
address). Nothing else references it. `Entity__StartSoundCue` hands the row's
handler to `InitSoundCueSet`, and `ServiceSoundCueSet` calls it once per tick
as `callback(owner, set)`, so `out` is the `SoundCueSet` (`EntityMoodHandlerArg`
is Entity.h's local view; field readings in `Entity__MoodCue07.md`
`## Proposed field names`: `unk4` tick, `unk10` attenuation, `unk1C`/`unk30`/`unk44`
voice 0/1/2 tone request (-2 = stop), `unk20`/`unk34`/`unk48` pitch offset).
Tier B, same as every sibling `Entity__MoodCueNN` (Entity_b..Entity_g): the
row mapping is a fact of the binary, which dream object or state a row is
for is not established. Row kept decimal so names sort in table order.

What it does, in the unit's current field names: Five-state machine (0, 0xA..0xE) that calls `SetCueTones7_7_7`/`SetCueTones18_3_3` at its transitions, yaws with `updateRotation(ROTATION_YAW_PLUS9 / ROTATION_ZMINUS9)`, rotates the target with `target->slot44(ROTATION_ZPLUS1 / ROTATION_YAW_PLUS180)`, drives `unk100` (via `Entity__GetOrCreateUnk100`) with `slotD4(unk50, 7, 0)`/`slotD8(unk50, 0, 0)`, and ends with `notifyParents(0xA or 0xC)` or stopping the cue.

### Data constants named (round 79)

Decoded from `disk/SLPS_015.56` in the format of the existing rotation
tables (three s16 `{num, den}` pairs, X / Y(yaw) / Z, e.g. `ROTATION_ZPLUS9`
= `(0,1, 0,1, 9,1)`):

| old | new | tier | bytes |
| --- | --- | --- | --- |
| `D_80089CD0` | `ROTATION_ZPLUS1` | A | `(0,1, 0,1, 1,1)` -- same spelling as `ROTATION_ZPLUS9`/`ROTATION_ZPLUS4`/`ROTATION_YAW_PLUS1`. Its consumer here is `target->methods->slot44(target, 0, ...)`, the same Unk94Methods slot that takes `ROTATION_YAW_PLUS180` two lines later and `ROTATION_YAW_MINUS90` in `Entity__MoodCue49` |
| `D_80089CDC` | `ROTATION_ZMINUS9` | A | `(0,1, 0,1, -9,1)`, passed to `updateRotation(this, 0, ...)` like every other `ROTATION_*` |

The name describes the table's contents, which is all the bytes establish;
the units of the middle numbers (degrees, per the existing names) are the
existing names' reading, not re-derived here.

## Proposed field names

| field | proposed | tier | evidence | accessor outside Entity_f |
| --- | --- | --- | --- | --- |
| `Entity::unk50` (+0x50) | `companion2` | B | Entity is a Class65650 subclass (`Entity__Entity` calls `Get_vtable_Class65650()->ctor`; gEntityMethods keeps Class65650's slots), and `code_55dd4.h` names Class65650's +0x50 `companion2` (the tag-5 BaseObjO companion). Here and in `Entity__MoodCue57` it is only passed opaquely as `unk100->slotD4/slotD8`'s arg1, so nothing in this unit contradicts or confirms it beyond the offset. | Entity_g (first compiler failure, `Entity_g.c:78`; make stops there, so a witness, not the full list) |

**Applied by the head at merge, round 79**, by type scope, each field separately with both oracles green. `moodDuration` (round 78) is now `todFrameCount`.

## Track 4 (2026-09-26, round 87, echo)

`this->unk100` is a `Class6E99C *` (include/Class6E99C.h); the slot
call through its +0x0D4 and +0x0D8 is now `startFadeDown`/`startFadeUp` (Class6E99C__StartFadeDown/StartFadeUp), with `companion2`, an
`s32` in Entity.h, cast `(BasicClass *)` as the fade's source (Class6E99C's
configure adds it as a child; no code). Image byte-identical.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
