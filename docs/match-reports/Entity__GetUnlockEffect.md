# Entity__GetUnlockEffect -- MATCHED 14/14 words, round 24 (2026-09-08)

> **VERDICT CORRECTED, round 24 (2026-09-08). THIS FUNCTION IS MATCHED.**
> Everything below this box was RIGHT about the mechanism and WRONG about the
> conclusion, for a reason that had nothing to do with its analysis: the
> residue it characterises is the `addiu_at` folded-vs-unfolded form, and
> **`addiu_at` was RESOLVED in round 21** (maspsx gained a `--addiu-at` flag,
> `tools/patches/maspsx-addiu-at.patch`; see
> `docs/research/addiu-at-blocker.md`). The report below even states the
> blocker cannot be fixed because "maspsx exposes no `--addiu-at` flag" --
> true when written, false since round 21.
>
> Matched 14/14 words on the first attempt with the C the report itself had
> already derived, unchanged. Whole-image SHA1 green.
>
> **How it stayed hidden for three rounds is the part worth keeping.** Round
> 22 built the `REOPENED -- ASSIGNABLE` marker and swept for reports citing
> `addiu_at`; round 23 found it had missed `CheckDreamAuxTriggerCondition` (99/100) because
> that report "does not look like a stub"; this one and its siblings were
> missed for the same reason, one step further. They open with a **HEAD
> ADJUDICATION** box certifying the diagnosis as independently reproduced and
> written up project-wide with a 502-of-502 corpus census. Nothing reads as
> less like stale ground than that -- and every word of it was true. The
> adjudication verified the MECHANISM, which never changed; what expired was
> the premise that the mechanism was unfixable.
>
> **A blocker's death invalidates the strongest reports as thoroughly as the
> weakest ones, and it invalidates them WITHOUT touching anything they say.**
> Rank by `tools/nearmiss.py`, which screens from the ASM, and treat any
> contradiction between it and a report's verdict as the report being the
> wrong half -- however well argued, and especially when the argument is
> good enough that nobody re-reads it.


## The matched C

```c
s32 Entity__GetUnlockEffect(Entity *this) {
    return gEntityUnlockKindTable[this->moodIndex * 0x10] * 1000;
}
```

The `* 1000` is retail's shift-add expansion, not a source constant of its
own: `v1<<5 - v1` is 31x, `<<2` is 124x, `+v1` is 125x, `<<3` is 1000x. Five
instructions for one multiply, which is ordinary GCC 2.6.3 strength reduction
for a constant multiplicand with no `mult` needed.

---

## Original report, kept verbatim as the historical record

> **HEAD ADJUDICATION, round 2026-08-30-a.** The diagnosis in this report is
> CORRECT and the head reproduced it independently from scratch. It is now
> written up project-wide in **`docs/research/addiu-at-blocker.md`**, with an
> isolated reproducer and a corpus census: retail uses the unfolded (`addiu_at`)
> form for **502 of 502** runtime-indexed global accesses across 39 files, and
> the folded form **zero** times. There is no counterexample anywhere in the
> executable.
>
> **One correction to the proposed remedy.** Repinning `--aspsx-version` to 2.29
> is not surgical and should not be presented as the fix. `config_for_aspsx_version`
> flips **four** flags below 2.30, not one — `addiu_at` plus three nop-insertion
> rules (`nop_at_expansion`, `nop_mflo_mfhi`, `nop_lw_lw`) that affect constructs
> throughout the image, including inside the 57 functions that currently match.
> maspsx exposes no `--addiu-at` flag, so the behaviour cannot be enabled alone
> without patching maspsx. This is the same shape as the already-rejected `-G`
> experiment. See the research document; the ruling is the operator's.


**Unit:** Entity · **Size:** 14 instructions · **Status:** STALLED, class TOOLCHAIN (maspsx `addiu_at` macro-expansion mismatch)

## What it does

`return (s8)gEntityUnlockKindTable[this->moodIndex * 0x10] * 1000;` — same mood-indexed
table-lookup family as `Entity__GetMoodEffect`/`Entity__GetLinkStage`/
`Entity__GetEventVideo`. The `* 1000` is a genuine multiply-by-constant,
confirmed structurally correct (matches retail's shift/subtract/shift/add/
shift strength-reduction sequence exactly: `v1*32 - v1 = v1*31`, `*4 = v1*124`,
`+v1 = v1*125`, `*8 = v1*1000`).

## The residue

Identical class and mechanism to `Entity__GetEventVideo.md` (read that report
for the full isolated reproducer and the maspsx source citation) — the table
load itself is 1 word short:

```
retail:  lui $at,%hi(gEntityUnlockKindTable) / addiu $at,$at,%lo(gEntityUnlockKindTable) / addu $at,$at,$v0 / lb $v1,0x0($at)
built:   lui $at,%hi(gEntityUnlockKindTable) / addu $at,$at,$v0 / lb $v1,%lo(gEntityUnlockKindTable)($at)
```

Every instruction *after* the load (the strength-reduced `* 1000`) matches
retail exactly once the drift from this one missing word is accounted for —
confirmed by rebuilding with this function and `Entity__GetLinkStage`/
`Entity__GetEventVideo` all reverted to `INCLUDE_ASM` (restoring the whole
image to green) and then re-diffing the *other* functions in this unit
cleanly, with zero residue anywhere else in the batch.

## Preserved body

```c
#if 0
s32 Entity__GetUnlockEffect(Entity *this) {
    return gEntityUnlockKindTable[this->moodIndex * 0x10] * 1000;
}
#endif
```

## Proposed learning

See `Entity__GetEventVideo.md` for the full writeup — same toolchain class,
documented once there to avoid repeating the reproducer three times.

## Naming

**Tier A, pre-existing (round 2026-08-30-a), confirmed this round.** A pure
getter over `gEntityUnlockKindTable` (named this round), `* 1000` (a GCC
2.6.3 strength-reduction expansion, not a source constant). Not renamed.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 2: The flat mood-row "tables" this body read are columns of gEntityMoodTable's
16-byte row (their symbols are the row base 0x80089EA4 plus the column
offset: gEntityUnlockKindTable +0x02, gEntityLinkStageTable +0x07,
sEntityEventVideoTable +0x08, gEntityProximityThresholdTable +0x0A,
gEntityMoodHandlerTable +0x0C), now EntityMoodRow fields; byte-identical.
Entity.h's old claim that they were "SEPARATE global arrays (own base
symbols, own lui/addiu) ... not sub-fields of the gEntityMoodTable row" was
wrong: GCC spells a constant-offset field of a global array as
%hi/%lo(sym + off), which splat labels as its own symbol.
