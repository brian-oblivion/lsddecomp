# TodActor__SelectTickCallback

> Renamed from `Class65650__SelectTickCallback` on 2026-09-26 (tools/rename.py). Address 0x800660bc.

> Renamed from `func_800660BC` on 2026-09-24 (tools/rename.py). Address 0x800660bc.

**Unit:** TodActor · **Size:** 32 words (0x80 bytes) · **Status:** MATCHED
(32/32 words, whole-image `./build-and-verify.sh` green)

## What it does

A 3-way dispatch on the low byte of `value`, storing a **pointer value**
(never calling it) from one of three vtable slots into `self->unk78`, a
new field carved out of what the first pass left as opaque `unk78`
padding:

```c
void TodActor__SelectTickCallback(TodActor *self, s32 value)
{
    switch ((u8)value) {
    case 0x41:
        self->unk78 = self->methods->slot118;
        break;
    case 0x42:
        self->unk78 = self->methods->slot11C;
        break;
    case 0x43:
        self->unk78 = self->methods->slot120;
        break;
    }
}
```

`slot118`/`slot11C`/`slot120` are typed as plain `void *` in
`src/world/tod_actor.c`, not function pointers — this function only ever
takes their *address* out of the vtable and stores it, it never `jalr`s
through them, so there is no evidence here for their call signature (a
future function that actually invokes `self->unk78` would be the place to
type them properly).

The three-case switch on `0x41`/`0x42`/`0x43` compiles as a binary
comparison tree pivoting on the middle value (`0x42` first, then `< 0x43`
to choose between the `0x41` and `0x43` subtrees) rather than a jump
table — GCC 2.6.3's ordinary lowering for three sparse, numerically close
case values. A plain `switch` reproduced this exact tree with no
reshaping needed.

### Proposed learning

When a function only ever *copies* a vtable slot's pointer value rather
than calling it, don't force a function-pointer type onto that slot for
this call site's sake — a plain `void *` avoids implying a signature no
evidence yet supports, and callers that actually invoke it later can retype
it then.

## Naming

Round 75 (charlie), track 3.

- `TodActor__SelectTickCallback` (was `func_800660BC`), tier A. Occupies +0x10C. Stores the VALUE of slot +0x118/+0x11C/+0x120 (TickCallbackA/B/C) in tickCallback for selector 0x41/0x42/0x43 ('A'/'B'/'C'); anything else leaves it. Callers: InitDefaults ('A'), Entity__Reset ('B', which Entity overrides with Entity__TickSoundCue).

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/tod_actor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
