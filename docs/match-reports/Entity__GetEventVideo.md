# Entity__GetEventVideo -- MATCHED 9/9 words, round 24 (2026-09-08)

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
> Matched 9/9 words on the first attempt with the C the report itself had
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
s32 Entity__GetEventVideo(Entity *this) {
    return sEntityEventVideoTable[this->moodIndex * 0x10] - 1;
}
```

Exactly the expression the report below derived, with the `(s8)` cast
dropped -- `sEntityEventVideoTable` is already declared `extern s8 []` in
`include/Entity.h`, so the cast was redundant rather than wrong.

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


**Unit:** Entity · **Size:** 9 instructions · **Status:** STALLED, class TOOLCHAIN (maspsx `addiu_at` macro-expansion mismatch — NEW, distinct from the gp-relative blocker)

## What it does

`return (s8)sEntityEventVideoTable[this->moodIndex * 0x10] - 1;` — a signed-byte table
lookup indexed by `this->moodIndex` (a 16-byte stride, the same index used
by `Entity__GetMoodEffect`/`Entity__GetUnlockEffect`/`Entity__GetLinkStage`,
all four keyed off the same field, each with its own table).

## The residue

```
retail:  lui   $at,%hi(sEntityEventVideoTable)
         addiu $at,$at,%lo(sEntityEventVideoTable)
         addu  $at,$at,$v0
         lb    $v0,0x0($at)
built:   lui   $at,%hi(sEntityEventVideoTable)
         addu  $at,$at,$v0
         lb    $v0,%lo(sEntityEventVideoTable)($at)
```

Both compute the identical final address; retail fully resolves the symbol
into `$at` (4 instructions total for the load), the build folds the `%lo`
relocation into the `lb`'s own displacement field (3 instructions). This is
1 word short, every time, regardless of how the C is written.

**This is not a compiler codegen choice — it happens below cc1.** Isolated
with the pinned pipeline per CLAUDE.md's reproducer recipe:

```c
typedef signed char s8;
typedef signed int s32;
extern s8 sEntityEventVideoTable[];
s32 test(s32 mood) {
    return sEntityEventVideoTable[mood * 0x10] - 1;
}
```

`cc1`'s own output (before maspsx ever runs) is a single symbolic
pseudo-instruction:

```
sll  $4,$4,4
lb   $2,sEntityEventVideoTable($4)
j    $31
addu $2,$2,-1
```

`cc1` never chooses between the two forms above at all — it emits one
generic `lb $reg,sym($reg)` pseudo-op and leaves the hi/lo split entirely to
the assembler. So **no C-level reshaping can affect this**, which matches
what was actually observed: four different rewrites (a local `s8 *p =
&arr[idx]; return *p;` intermediate, a separate `s32 idx = ...;` statement, a
separate `s32 mood = this->moodIndex;` copy, and indexing through a 16-byte
`struct { s8 value; u8 pad[15]; }` array instead of raw byte arithmetic) all
produced byte-identical output for this instruction sequence.

**The real decision is `maspsx`'s `addiu_at` flag**
(`tools/maspsx/maspsx/__init__.py`, the `elif is_addend and r_source:` branch
around line 970). Piping the same `cc1` output through `maspsx.py` with two
different `--aspsx-version` values, changing nothing else:

| `--aspsx-version` | expansion | matches retail? |
| --- | --- | --- |
| `2.34` (**the project's pin**) | `lui / addu / lb %lo(sym)($at)` | **no** |
| `2.29` | `lui / addiu / addu / lb 0x0($at)` | **yes** |

`tools/maspsx/maspsx.py`'s own `config_for_aspsx_version()` sets
`addiu_at = True` only when `aspsx_version < (2, 30)`, and the project pins
`2.34`. So the maspsx version-behavior model, at exactly the pinned version,
disagrees with retail for this specific pseudo-instruction shape
(register-indexed byte/word load from a non-`$gp` symbol — `lb $reg,
sym($reg)`), even though `2.34` is presumably correct for whatever evidence
originally justified the pin (probably the `div`/`li` macro shapes documented
elsewhere, or the gp-relative behavior, which is a *different* code path in
maspsx entirely — see `docs/research/gp-relative-blocker.md`; that finding is
about `$gp`-relative addressing being unreachable at `-G0`, this one is about
a *non-`$gp`* indexed load choosing the wrong hi/lo split at the pinned
version).

## Scope

At minimum, this blocks all three of `Entity__GetUnlockEffect`,
`Entity__GetLinkStage`, and `Entity__GetEventVideo` in this unit — every
function here that *loads through* a table symbol with a runtime register
index. `Entity__GetMoodEffect` (matched, see its own report) is unaffected
because it only ever forms the address (`&arr[idx]`), never loads through
it, which routes through a different maspsx code path (`is_addend and
r_source is None` above, or the plain address-formation macro, not the
load-expansion branch). Any function anywhere in the game that reads a
global/rodata table using a non-constant index is a candidate for this same
residue — this is likely comparable in scope to the gp-relative blocker,
possibly larger, since indexed table lookups are a very common shape.

## What is NOT yet established

- Whether `--aspsx-version=2.29` (or `addiu_at=True` some other way) would
  regress any of the 46 already-matched functions elsewhere in the project.
  **Untested against the real build** — the comparison above used only the
  isolated `/tmp` reproducer pipeline, never `build-and-verify.sh`, per
  CLAUDE.md's "escalate, do not experiment" rule. Given the gp-relative
  blocker's own experience (a global `-G` flip looked clean in isolation but
  broke 19148 bytes elsewhere once actually rebuilt), a global version flip
  here should be assumed equally risky until an operator authorizes and runs
  the real experiment with a full `rm -rf build` rebuild.
- Whether the "right" fix is a different global `--aspsx-version`, or
  whether `maspsx` needs a per-construct override (its `addiu_at` config is
  a single project-wide bool, not conditional on instruction shape) — i.e.
  whether the *true* Sony ASPSX build of this game mixed both hi/lo
  expansion styles depending on context in a way no single maspsx version
  flag currently models.
- What other constructs, if any, gate on `addiu_at` inside maspsx besides
  this indexed-load path (grep hit at least one more call site,
  `tools/maspsx/maspsx/__init__.py:1059`, `if self.addiu_at and op != "la"`,
  not yet investigated).

## Preserved bodies

```c
#if 0
s32 Entity__GetUnlockEffect(Entity *this) {
    return gEntityUnlockKindTable[this->moodIndex * 0x10] * 1000;
}
#endif
```

```c
#if 0
s32 Entity__GetLinkStage(Entity *this) {
    s32 v;

    v = sEntityLinkStageTable[this->moodIndex * 0x10];
    if (v < 0) {
        return ~v;
    }
    return v - 1;
}
#endif
```

```c
#if 0
s32 Entity__GetEventVideo(Entity *this) {
    return sEntityEventVideoTable[this->moodIndex * 0x10] - 1;
}
#endif
```

All three preserve the correct control flow and field/table derivation
(`Entity__GetLinkStage`'s branch, in particular, is verified structurally
correct — same branch targets as retail, confirmed with `asm-differ`, which
is exactly the check the head's round-2026-08-30-a broadcast on `strcat`
said to make before calling anything compiler-internal; here the CFG matches
and the *only* residue is the one-instruction addressing-mode gap above).
Any of the four table declarations (`s8 gEntityUnlockKindTable/AB/AC[]`) and
`Entity.h`'s `moodIndex` field are correct regardless of this stall.

## Proposed learning

**New toolchain finding, sibling to the gp-relative blocker but a different
mechanism and a different maspsx flag (`addiu_at`, not `-G`).** Any function
whose retail disassembly shows a *fully resolved* symbol+register address
(`lui`+`addiu`+`addu`+`load 0x0(...)`, four instructions) where the pinned
toolchain produces a *folded* form (`lui`+`addu`+`load %lo(sym)(...)`, three
instructions) is this class, not a reshaping target — check with
`tools/gcc263/cc1` + `tools/maspsx/maspsx.py` directly (varying only
`--aspsx-version` in isolation) before spending attempts. Escalating per
CLAUDE.md's "report with a reproducer, never experiment mid-round" — the
reproducer above is self-contained and takes under a second to re-run.

## Naming

**Tier A, pre-existing (round 2026-08-30-a), confirmed this round.** A pure
getter over `sEntityEventVideoTable` (named this round). Not renamed.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a TodActor subclass whose table and object expand `TODACTOR_SLOTS`/`TODACTOR_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Polish (round 96, bravo, track 7)

- Step 2: The flat mood-row "tables" this body read are columns of sEntityMoodTable's
16-byte row (their symbols are the row base 0x80089EA4 plus the column
offset: gEntityUnlockKindTable +0x02, sEntityLinkStageTable +0x07,
sEntityEventVideoTable +0x08, gEntityProximityThresholdTable +0x0A,
gEntityMoodHandlerTable +0x0C), now EntityMoodRow fields; byte-identical.
Entity.h's old claim that they were "SEPARATE global arrays (own base
symbols, own lui/addiu) ... not sub-fields of the sEntityMoodTable row" was
wrong: GCC spells a constant-offset field of a global array as
%hi/%lo(sym + off), which splat labels as its own symbol.
