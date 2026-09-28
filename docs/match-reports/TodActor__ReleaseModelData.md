# TodActor__ReleaseModelData

> Renamed from `Class65650__ReleaseModelData` on 2026-09-26 (tools/rename.py). Address 0x80065cec.

> Renamed from `func_80065CEC` on 2026-09-24 (tools/rename.py). Address 0x80065cec.

**Unit:** TodActor · **Size:** 30 words (0x78 bytes) · **Status:** MATCHED
(30/30 words, whole-image `./build-and-verify.sh` green)

## What it does

The teardown for the `+0x5C` sub-object, dispatched from `TodActor__TeardownModelData`
(`slot_teardown5C`). Tears down the `+0x70`/`+0x74` arrays first (through
`slot_teardown70`, `+0x104`, i.e. `TodActor__TeardownParts`), then either releases
`self->unk5C` through its own vtable slot `+0x004` (if `self->unk60`
records that this instance owns it) or simply clears it to `NULL`
(if it was only borrowed — see `TodActor__AcquireModelData`'s report for the
borrow/allocate distinction that sets `unk60`).

```c
void TodActor__ReleaseModelData(TodActor *self)
{
    Unk5CObj *result;

    self->methods->slot_teardown70(self);
    if (self->unk60 != 0) {
        result = self->unk5C->methods->slot4(self->unk5C);
    } else {
        result = NULL;
    }
    self->unk5C = result;
}
```

## The residue: a shared store needs a shared local, not two inline stores

First attempt wrote the obvious per-branch form —
`self->unk5C = self->unk5C->methods->slot4(...)` in the `if`, `self->unk5C
= NULL` in the `else`. That scored 28/30: retail's `else` branch stores via
`$v0` (`addu $v0,$zero,$zero` in the branch's own delay slot, then `sw
$v0, 0x5C($s0)` at the merge label), while my direct `else` stored the
literal straight from `$zero` with no `$v0` involved. Both branches in
retail converge on **one physical `sw` instruction** at the merge label —
which only happens when the C source computes the value into a shared
local first and performs **one** assignment to the field after the `if`,
not two separate field-assignment statements. Restructuring to `result =
...; result = NULL; ...; self->unk5C = result;` closed it immediately, no
further attempts needed.

### Proposed learning

When an `if`/`else` residue is specifically "the `else` arm stores a literal
directly instead of routing it through the same register the `if` arm
used", the source is very likely computing into a **shared local variable**
with the actual field/return write happening **once, after** the
conditional — not two independent per-branch assignments to the same
lvalue. This is the same family as the `New_X` `goto`-vs-`return` levers
(one shared write site reached two ways) but for a plain assignment rather
than a function return.

## Naming

Round 75 (charlie), track 3.

- `TodActor__ReleaseModelData` (was `func_80065CEC`), tier A. teardownParts (+0x104), then if ownsModelData stores modelData->release() (gModelDataMethods +0x004, FileResource__Release) back into modelData, else NULL.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/tod_actor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
