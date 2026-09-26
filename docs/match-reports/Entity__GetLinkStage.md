# Entity__GetLinkStage -- MATCHED 15/15 words, round 24 (2026-09-08)

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
> Matched 15/15 words on the first attempt with the C the report itself had
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
s32 Entity__GetLinkStage(Entity *this) {
    s32 linkStage = gEntityLinkStageTable[this->moodIndex * 0x10];

    if (linkStage < 0) {
        return ~linkStage;
    }
    return linkStage - 1;
}
```

The two arms are `abs(linkStage) - 1` in disguise -- for negative x, `~x` is
`-x - 1` -- but they must be written as the explicit two-arm form, because
the `nor` on the negative path is a literal `~`. An `abs()`-shaped source
does not produce it.

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


**Unit:** Entity · **Size:** 15 instructions · **Status:** STALLED, class TOOLCHAIN (maspsx `addiu_at` macro-expansion mismatch)

## What it does

```c
s32 v = gEntityLinkStageTable[this->moodIndex * 0x10];  /* signed byte */
if (v < 0) {
    return ~v;
}
return v - 1;
```

Same mood-indexed table family as the other three `Entity__Get*` functions.
The branch (`bltz`) genuinely tests the loaded value's sign, taking a
`~v` (bitwise NOT, via `nor $v0,$zero,$v0`) path on negative and `v - 1` on
non-negative — confirmed via `asm-differ` that **both branch targets agree
with retail exactly** (`.L8005D9C4` / `.L8005D9C8` land in the same relative
positions once the load-instruction-count residue below is accounted for).
Per the head's round-2026-08-30-a broadcast on `strcat` — check whether
branch *targets* differ, not just delay-slot filler, before calling
something compiler-internal — this one was checked and the control flow is
byte-correct; the CFG is not in question here.

## The residue

Identical class and mechanism to `Entity__GetEventVideo.md` (full isolated
reproducer there) — the table load is 1 word short, everything else
(including the branch and both return paths) matches:

```
retail:  lui $at,%hi(gEntityLinkStageTable) / addiu $at,$at,%lo(gEntityLinkStageTable) / addu $at,$at,$v0 / lb $v0,0x0($at)
built:   lui $at,%hi(gEntityLinkStageTable) / addu $at,$at,$v0 / lb $v0,%lo(gEntityLinkStageTable)($at)
```

## Preserved body

```c
#if 0
s32 Entity__GetLinkStage(Entity *this) {
    s32 v;

    v = gEntityLinkStageTable[this->moodIndex * 0x10];
    if (v < 0) {
        return ~v;
    }
    return v - 1;
}
#endif
```

## Proposed learning

See `Entity__GetEventVideo.md` for the full writeup — same toolchain class.
Additionally worth recording: this function is the one place in this batch
with a real branch, and it was the one case where the head's "check branch
targets" rule from the `strcat` adjudication was directly exercised — and it
cleared the check (targets agree), which is what let this be classified
TOOLCHAIN rather than re-attempted as a reshaping problem.

## Naming

**Tier A, pre-existing (round 2026-08-30-a), confirmed this round.** A pure
getter over `gEntityLinkStageTable` (named this round), `abs(x) - 1` written
as an explicit two-arm form. This round's `Entity__NotifyLinkStage` and the
`Entity__Activate`/`Entity__UpdateActivationState` naming reasoning both cite
this function's "Link" vocabulary as the reason NOT to reuse "Link" for the
unrelated `unkF0` active/inactive toggle. Not renamed.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
