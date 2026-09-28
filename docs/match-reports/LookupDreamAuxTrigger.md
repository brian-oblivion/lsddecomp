# LookupDreamAuxTrigger

> Renamed from `func_8005C8AC` on 2026-09-21 (tools/rename.py). Address 0x8005c8ac.

**Unit:** DreamAux · **Size:** 33 words · **Status:** MATCHED round 43
(33/33, byte-exact whole-image build).

## History

Filed BLOCKED in round 2026-08-30-a on one `%gp_rel` reference (to
`gDreamAuxStage`) plus two `addiu_at` indexed loads. Round 42 resolved both
mechanisms. Never actually attempted -- the stub carried no derivation.
Round 43 derived and matched it fresh.

## What it does

A linear search over one of 14 parallel groups (selected by `gDreamAuxStage`,
the same index this unit also uses in `RemapTriggerForDreamColor`/`IsCurrentDreamColor`) for
an entry whose 2-byte `key` matches `*a0`, dispatching the match (or its
absence) into `RemapTriggerForDreamColor`:

```c
typedef struct DreamAuxTriggerEntry {
    s16 key;
    u8 unk2[4];
} DreamAuxTriggerEntry;

extern s8 gDreamAuxTriggerCounts[];
extern DreamAuxTriggerEntry *gDreamAuxTriggerEntries[];

s32 LookupDreamAuxTrigger(s16 *a0)
{
    s32 idx = gDreamAuxStage;
    s32 count = gDreamAuxTriggerCounts[idx];
    DreamAuxTriggerEntry *entry = gDreamAuxTriggerEntries[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        if (*a0 == entry->key) {
            return RemapTriggerForDreamColor((s32)entry, i);
        }
        entry++;
    }
    return 0;
}
```

`gDreamAuxTriggerCounts`/`gDreamAuxTriggerEntries` is a second "count + pointer-to-array" parallel
family in this unit, structurally identical to the already-documented
`gDreamAuxGroupCounts`/`gDreamAuxGroupRecords` (`InitDreamAux`) but a different stride (6 bytes,
not 8) and a different index space (`gDreamAuxStage`, not a loop counter). The
record type is named `DreamAuxTriggerEntry` since `LookupDreamAuxTrigger`'s only
consumer of the match, `RemapTriggerForDreamColor`, is itself part of this unit's
trigger-dispatch cluster (`ProcessDreamAuxTriggerRecord`/`CheckDreamAuxTriggerCondition`/etc.).

## Derivation notes

One attempt short of byte-exact, one fix:

- **First pass (10/33, 129492 bytes of drift, one word too long):**
  `s8 count = gDreamAuxTriggerCounts[idx];` produced a `lb` into `$v0` followed by a
  separate `move $a2, $v0` to get the value into the register the loop bound
  needs to live in across the whole function. Retail's `lb $a2, 0x0($at)`
  loads the byte DIRECTLY into `$a2` with no intermediate register at all.
  Widening the local from `s8` to `s32` (`s32 count = gDreamAuxTriggerCounts[idx];`)
  removed the extra `move` and matched retail's direct-into-`$a2` load.
  Byte-exact immediately after.

This is a different mechanism from the `lbu`-vs-`lb` sign-extend idiom
documented for `IsCurrentDreamColor` earlier this round (that one was about load
INSTRUCTION CHOICE; here the load instruction was already `lb` in both
versions -- the difference was an extra copy into the register a
longer-lived variable needed to occupy). Both point the same direction
though: **for a byte-sized value that stays live across more than its
immediate use (here: both the initial bound check and the whole loop), widen
its C-level local to `s32` rather than leaving it `s8`/`s16`** -- a narrow
local invites the compiler to route the value through a scratch register
first.

## Proposed learning

Add to the corpus: **a byte/half-word value kept alive as a LOOP BOUND (not
just re-read once) should be declared as a wide (`s32`) local, even when the
source array element is narrower.** Symptom when this is missed: one extra
`move` from the load's natural destination register into the loop-bound
register, costing exactly one word and (because it's inside a small
function) enough to look like an unrelated residue rather than a type-width
issue.

## Naming

**LookupDreamAuxTrigger** — tier A. A linear search over
`gDreamAuxTriggerEntries[gDreamAuxStage]` for an entry whose `key` matches
`*a0`, dispatching the match (or its absence) into
`RemapTriggerForDreamColor`. The search IS the function's purpose, so tier
A applies even though it delegates the on-hit adjustment to a sibling.

## Round 100 (alpha): track 7, moved from src/DreamAux.c and include/DreamAux.h

Returns `DreamAuxTriggerEntry *` (was the entry smuggled as s32); a0 ->
`chunkKey`, idx -> `stage`, entry -> `trigger`. DreamAuxTriggerEntry's other
four bytes are named: `dayParity` (CheckTriggerDayParity's byte 2) and
`recordIndices[3]` (FireDreamAuxTriggerEntries' bytes 3..5). Byte-identical.

The header comment, as it stood:

```c
/* A second parallel-group family, same "count + pointer to array" shape as
 * DreamAuxGroupRecord above but a different stride and a different index
 * space: 14 (0xE) groups selected by `gDreamAuxStage` (not a loop index),
 * gDreamAuxTriggerCounts[i] a signed count, gDreamAuxTriggerEntries[i] a pointer to an array of
 * count 6-byte records whose first 2 bytes (`key`, read with `lh`) are the
 * only field LookupDreamAuxTrigger accesses. The remaining 4 bytes are undiscovered
 * from this unit alone. */
```
