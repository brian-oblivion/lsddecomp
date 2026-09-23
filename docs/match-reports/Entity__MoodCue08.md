# Entity__MoodCue08

> Renamed from `func_8005E694` on 2026-09-23 (tools/rename.py). Address 0x8005e694.

**Unit:** Entity_b · **Size:** 23 words · **Status:** MATCHED (23/23 words,
whole-image build verified byte-exact)

## What it does

A short two-call dispatcher, not itself a `gEntityMoodHandlerTable` table entry (no
`EntityMoodHandlerArg` argument): `this->methods->slot48(this, 1,
D_80089DF0)` followed by `this->methods->slotBC(this, D_80089D78)`. Both
`D_80089DD8`/`D_80089DF0`/`D_80089D78` are opaque data blobs only ever
address-taken (never dereferenced) by this unit's functions, so they are
declared as plain `u8[]` in `Entity_b.c`.

`slot48` is shared with `Entity__MoodCue17` (same table offset, same literal `1`
first argument) — see that report for why it is typed `s32`-returning rather
than `void`.

## Final C

```c
void Entity__MoodCue08(Entity *this) {
    this->methods->slot48(this, 1, D_80089DF0);
    this->methods->slotBC(this, D_80089D78);
}
```

## Attempt log

Matched on the first attempt once written after `Entity__MoodCue17` had already
established `slot48`'s signature and after the whole-unit address drift from
earlier residues was resolved.

## Proposed learning

None new — straightforward two-call body, no residue.
