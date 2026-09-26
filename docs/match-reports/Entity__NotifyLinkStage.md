# Entity__NotifyLinkStage -- MATCHED (62/62 words)

> Renamed from `func_8005D560` on 2026-09-19 (tools/rename.py). Address 0x8005d560.

**Unit:** Entity · Runner: charlie, round 23.

## What it does

Range-gates on `arg2` and the "GetLinkStage" mood table (`gEntityLinkStageTable`) before
calling a new `BasicClassMethods` slot (`slotDC`, forwarding `arg1`/`arg2`),
then, only when `arg2 == 4` and the link stage is positive, dispatches a
small state code (0xA/0xB/0xC) into `EntityMethods::slot30` -- 0xA unless the
link stage is exactly `0x7F`, in which case a second table
(`gEntityEventVideoTable`, "GetEventVideo") picks between 0xB and 0xC.

## Final C

```c
void Entity__NotifyLinkStage(Entity *this, s32 arg1, s32 arg2) {
    s32 linkStage;

    linkStage = gEntityLinkStageTable[this->moodIndex * 0x10];
    if ((u32)(arg2 - 2) < 7) {
        if (linkStage <= 0) {
            return;
        }
    }
    Get_vtable_Class65650()->slotDC(this, arg1, arg2);
    if (arg2 != 4) {
        return;
    }
    if (linkStage <= 0) {
        return;
    }
    if (linkStage != 0x7F) {
        arg2 = 0xA;
    } else if (gEntityEventVideoTable[this->moodIndex * 0x10] != 0) {
        arg2 = 0xB;
    } else {
        arg2 = 0xC;
    }
    this->methods->slot30(this, arg2);
}
```

Byte-exact, whole-image build verified.

## Attempt log (4 attempts)

1. First attempt used a separate `code` local for the dispatch value and
   read `linkStage` as the very first statement, before ever referencing
   `arg2`. Scored 37/62 with NO whole-image drift (good sign -- pure
   register-identity residue, not a size mismatch): retail assigns `arg2`
   to `$s0` and the loaded table byte to `$s1` (both need to survive the
   `Get_vtable_Class65650()` call); my build had them swapped. Two follow-up
   attempts trying to fix the order by literally reordering the two
   statements, or introducing a boolean `inRange` computed first, both
   made it WORSE (32/62 and 5/62 respectively) -- the second duplicated
   the `gEntityLinkStageTable` load into two separate `lb` instructions instead of
   being CSE'd back into one, causing a genuine 139789-byte whole-image
   drift.
2. **The actual fix was to stop introducing a second local entirely.**
   Retail's final `slot30` call passes `$s0` -- the SAME register `arg2`
   already lives in -- for the dispatch code, confirmed by `move a1,s0` at
   the call site. So retail reuses the `arg2` PARAMETER itself for the
   0xA/0xB/0xC value (dead by that point, since the caller has already
   forced `arg2 == 4` to reach here) rather than allocating a fresh local.
   Same lever as this round's `Entity__IsNearTarget` residue: mutating the
   parameter in place instead of introducing a same-purpose local resolves
   a register-identity swap, because the extra local was what pushed
   `linkStage`'s pseudo-register ahead of `arg2`'s in GCC's allocation
   order. This single change took it straight to 49/62 with the whole
   top 2/3 of the function byte-exact.
3. **Branch polarity, twice, matching `docs/DECOMPILATION_LEARNINGS.md`'s
   round-14ish entry ("Branch polarity is a real, separate residue from
   branch targets... swap which arm is written first and negate the
   condition").** The remaining residue was exactly this shape at TWO
   nested levels: my `if (linkStage == 0x7F) {...} else { arg2 = 0xA; }`
   compiled with the wrong arm as fallthrough; flipping to
   `if (linkStage != 0x7F) { arg2 = 0xA; } else {...}` (condition negated,
   arms swapped) matched retail's explicit-jump-for-the-short-arm layout.
   Took it to 59/62.
4. The innermost `gEntityEventVideoTable[...] == 0` check had the identical polarity
   residue one level deeper -- flipped to `!= 0` with arms swapped the same
   way, closing it to 62/62.

One new `BasicClassMethods` vtable slot: `slotDC` (`self, s32, s32`, void,
forwarding both arguments straight through).

### Proposed learning

**Branch polarity is not a one-shot fix -- it recurs at every nesting
level independently, and each level needs its own check-and-flip.** This
function needed the identical "negate condition, swap arms, first-written
arm becomes fallthrough" transformation applied twice, once per `if`
level, with a clean match at 59/62 in between that still had one more
polarity flip buried inside it. Do not stop checking for this residue
after fixing the outermost instance.

**A local variable introduced purely to hold "whichever branch's result
survives to a single later use" is itself a register-identity risk, and
this is now the SECOND confirmed instance in one round** (see
`Entity__IsNearTarget`'s report, same session). When a still-live PARAMETER is
available and semantically dead by the point of reuse (its original value
already spent, e.g. forced to a known constant by an earlier guard), reuse
it instead of declaring a fresh local -- retail did exactly that here
(`arg2` recycled for the dispatch code once the caller has forced
`arg2 == 4`), and introducing a same-purpose `code` local instead was what
caused `arg2`/`linkStage` to swap `$s0`/`$s1`.

Round-23 head broadcast's three levers do not apply here (no `s16` locals,
no `sltiu`-gated loop, no `&arr[i+j]` shape) -- reported per the "reply
with the negative answer too" instruction.

## Naming

**Tier B.** Renamed from `func_8005D560` this round (tools/rename.py).
Forwards `(arg1, arg2)` straight through to the base ancestor's `slotDC`,
and on `arg2 == 4` with a positive `gEntityLinkStageTable` value, also
notifies `EntityMethods::slot30` with a link/event-derived code (0xA/0xB/0xC).
"Link" here is not a fresh guess -- it is the SAME vocabulary this function
directly reads (`gEntityLinkStageTable`, the table `Entity__GetLinkStage`
already established, tier A, in an earlier round) via the identical
`moodIndex`-selected row.

## Proposed field names

- `EntityMethods::slot30` -> `notifyParents` -- **tier B.** `tools/
  classtable.py` on `gEntityMethods` shows +0x030 occupied by the already-
  named `BasicClass__NotifyParents` (a shared-ancestor slot, same idiom as
  `Get_vtable_Class65650()`'s table). CROSS-UNIT: called from every one of Entity_b/
  c/d/e/f/g (grep -rn -- '->slot30(' src/Entity_*.c), so proposed here
  rather than applied. Evidence and full writeup in `Entity__SetTargetReached.md`,
  which also dispatches through it.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
