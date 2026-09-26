# TodActor__AcquireModelData

> Renamed from `Class65650__AcquireModelData` on 2026-09-26 (tools/rename.py). Address 0x80065c5c.

> Renamed from `func_80065C5C` on 2026-09-24 (tools/rename.py). Address 0x80065c5c.

**Unit:** code_55dd4 · **Size:** 36 words (0x90 bytes) · **Status:** MATCHED
(36/36 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x0F4`'s implementation, dispatched from
`TodActor__SetupModelData` (`slot_setup5C`). Sets up `self->unk5C`, borrowing an
existing sub-object from the constructor's `arg1` if it already has one at
its own `+0x00C` field, or allocating a fresh one via `New_ModelData`
otherwise — recording which case happened in `self->unk60` (the guard flag
`TodActor__ReleaseModelData`'s teardown already reads). If the result is still `NULL`
(allocation failed), tears itself back down via `TodActor__ReleaseModelData` and
reports failure (`1`); otherwise dispatches to the class's own `+0x100`
slot and returns *its* result as the final success/failure code.

```c
s32 TodActor__AcquireModelData(TodActor *self, UnkArg1Obj *other)
{
    if (other->unk0C != NULL) {
        self->unk5C = other->unk0C;
        self->unk60 = 0;
    } else {
        self->unk5C = New_ModelData(other);
        self->unk60 = 1;
    }
    if (self->unk5C == NULL) {
        goto fail;
    }
    return self->methods->slot100(self);
fail:
    TodActor__ReleaseModelData(self);
    return 1;
}
```

Adds `UnkArg1Obj` (`include/code_55dd4.h`) for the constructor's `arg1`,
typed only at its `+0x00C` field (a `Unk5CObj *`, borrowed or freshly
allocated), and retypes the `arg1` parameter all the way from
`TodActor__TodActor` through `slot_setup5C`/`TodActor__SetupModelData` to here as
`UnkArg1Obj *` instead of the generic `void *` the first pass used
(implicit `void *` -> `UnkArg1Obj *` conversions at the two call sites
needed no changes). Also adds `New_ModelData`'s prototype (from
`asm/psyq_memset.s`, itself an allocator wrapping `BMemPMgrAlloc`) and
`slot100` (`+0x100`, called on success, its own return value threaded
straight through).

## The residue: `goto` direction matters, and it is not what `if`/`else` naturally produces

A first attempt wrote the natural `if (self->unk5C == NULL) { fail-body }
return self->methods->slot100(self);` (no `goto`) and scored 22/36 with a
length change (`build differs OUTSIDE this range`). The two-branch guard
compiled with **`beqz`, jumping to the fail path, use-path as the
fallthrough** — but retail's actual instruction is **`bnez`, jumping
forward to the use-path**, with the *fail* path as the fallthrough, at a
completely different offset. Rewriting with `if (...) goto use; <fail>;
use: <use>;` (the mirror-image `goto`, matching retail's own branch
polarity) produced **the exact same wrong output** — GCC 2.6.3 canonicalized
both spellings to the same layout here, unlike the `New_X` `goto` cases
where spelling changed the result.

What actually worked: keeping the **fail path physically last in the
source**, reached by `goto`, with the **use path inline immediately after
the guard** (no `goto` needed for it at all):

```c
if (self->unk5C == NULL) {
    goto fail;
}
return self->methods->slot100(self);
fail:
    ...
```

This is not just "add a `goto`" — the earlier `goto use; ...; use:` attempt
also added a `goto` and did not work. The lever is specifically **which
branch is the fallthrough and which is the explicit jump target placed at
the end of the function**, and only one of the two placements (fail last)
matched.

### Proposed learning

When a two-arm guard's `goto`-vs-plain-`if` residue doesn't respond to the
"obvious" `goto` rewrite (jump to whichever arm reads more naturally as
the "early exit"), try the **other** direction too — `goto` the arm that
is NOT the early exit, placing it last in the function. GCC 2.6.3 will
happily canonicalize an `if (cond) goto L; body; L:` back to whatever
layout it already preferred if both spellings describe the same graph
with no distinguishing feature (here: same fallthrough/jump split either
way) — so the fix isn't "use goto", it's "put the block that should be a
jump target physically last in the source", which sometimes only works in
one of the two possible label placements.

## Naming

Round 75 (charlie), track 3.

- `TodActor__AcquireModelData` (was `func_80065C5C`), tier A. Borrows arg1->modelData (+0x0C) with ownsModelData = 0, or makes one with New_ModelData(arg1) with ownsModelData = 1. On success returns setupParts (+0x100); on NULL calls ReleaseModelData and returns 1.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`. code_55dd4.c includes it, and include/code_55dd4.h's local `extern Unk5CObj *New_ModelData(UnkArg1Obj *arg)` is deleted. The call reads `self->modelData = (Unk5CObj *)New_ModelData((struct Src6F240 *)other)`, pointer casts with no code. The field's type, Unk5CObj (a view of ModelData), belongs to TodActor and is left for that class's unification. Image byte-identical.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
