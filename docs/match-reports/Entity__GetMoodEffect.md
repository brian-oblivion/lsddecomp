# Entity__GetMoodEffect

**Unit:** Entity · **Size:** 6 instructions · **Status:** MATCHED (6/6 words, whole-image build verified byte-exact)

## What it does

`return &sEntityMoodTable[this->moodIndex];` — forms the address of a 16-byte-stride
table row selected by `this->moodIndex`, without loading through it. Same
index (`moodIndex`, `entity.h` offset `+0x98`) as the other three
`Entity__Get*Effect/Stage/Video` functions in this unit, each keyed to its
own table.

**Updated in round 2026-09-01 (runner bravo, Entity 11-function pass):**
`sEntityMoodTable` is no longer typed as a bare `u8[]` indexed with a manual
`* 0x10`. `Entity__UpdateActivationState`/`Entity__UpdateDeactivationState` (this same table's other readers,
matched in that pass) needed named sub-byte fields inside each 16-byte row
(`detachKind` at +0x3, `linkKind` at +0x4, plus `unk5`/`unk9`), so the table
is now `extern EntityMoodRow sEntityMoodTable[];` (see `include/entity.h`) and this
function's own indexing changed from `sEntityMoodTable[this->moodIndex * 0x10]` to
`sEntityMoodTable[this->moodIndex]` to match — `sizeof(EntityMoodRow)` is 16, so
the compiler's own array-stride multiply reproduces the identical
`sll $v0,$v0,4` either way. Re-verified byte-exact after the change; the
disassembly below is unchanged.

## Derivation

```
lw   $v0, 0x98($a0)          ; v0 = this->moodIndex
lui  $v1, %hi(sEntityMoodTable)
addiu $v1, $v1, %lo(sEntityMoodTable)
sll  $v0, $v0, 4             ; v0 = moodIndex * 16
jr   $ra
 addu $v0, $v0, $v1           ; return &sEntityMoodTable[moodIndex*16]
```

## Final C

```c
void *Entity__GetMoodEffect(Entity *this) {
    return &sEntityMoodTable[this->moodIndex];
}
```

`sEntityMoodTable` is declared `extern EntityMoodRow sEntityMoodTable[];` in `entity.h`
(a real 16-byte struct now, see the note above — was `extern u8 sEntityMoodTable[]`
with a manual `* 0x10` before this round).

## Attempt log

Matched on the first attempt. Notably, this function does **not** exhibit
the maspsx `addiu_at` residue that blocks its three siblings
(`Entity__GetUnlockEffect`/`GetLinkStage`/`GetEventVideo`, see
`Entity__GetEventVideo.md`) — it only ever *forms* the table address
(`lui`+`addiu`+`addu`), never loads through it, which routes through a
different maspsx expansion path than the `lb $reg,sym($reg)` load macro that
triggers the bug. This is worth remembering as the dividing line for this
whole table-lookup family: address-only accessors are safe, anything that
dereferences through a runtime-indexed symbol is not.

Note also that before the sibling functions were reverted to `INCLUDE_ASM`,
this function transiently showed a 1-word mismatch (a differing embedded
relocation constant, same instruction) purely from address drift caused by
their now-stalled residue — that was never a bug in this function itself.

## Proposed learning

See `Entity__GetEventVideo.md`. This function is the useful control case
proving the maspsx bug is specific to the *load* macro, not table-lookup
address arithmetic generally.

## Naming

**Tier A, pre-existing (round 2026-08-30-a), confirmed this round.** A pure
getter over `sEntityMoodTable` (named this round) indexed by `moodIndex`. A
getter's mechanics are its purpose by definition. Not renamed.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
