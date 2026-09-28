# TodActor__Reset

> Renamed from `Class65650__Reset` on 2026-09-26 (tools/rename.py). Address 0x80065830.

> Renamed from `TodActor__InitDefaults` on 2026-09-25 (tools/rename.py). Address 0x80065830.

> Renamed from `func_80065830` on 2026-09-24 (tools/rename.py). Address 0x80065830.

**Unit:** TodActor · **Size:** 58 words (0xE8 bytes) · **Status:** MATCHED
(58/58 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x040` (already known from the header's own
comment: "TodActor__Reset, this class's own"). A pure dispatch sequence —
one call through the shared intermediate base class, six calls through
`self`'s own vtable with assorted literal arguments, then a conditional
call to a fixed function if `self->unk68` is set:

```c
void TodActor__Reset(TodActor *self)
{
    D800878D4Methods *base;

    base = GetActorMethods();
    base->slot60(self, 0);
    self->methods->slotF0(self, 1);
    self->methods->slotE4(self, 0x12C);
    self->methods->slot114(self);
    self->methods->slot10C(self, 0x41);
    self->methods->slot130(self);
    self->methods->slot128(self, 0);
    if (self->unk68 != NULL) {
        SceneNode__LinkModel(self, self->unk68->unk20);
    }
}
```

Resolves six new `TodActorMethods` slots (`+0xE4`, `+0xF0`, `+0x10C`,
`+0x114`, `+0x128`, `+0x130` — all carved out of previously-opaque `padNN`
regions) and one new `D800878D4Methods` slot (`+0x060`, alongside the
already-known `+0x038`/`+0x04C`/`+0x050`/`+0x070`). Also adds a real data
field to `Unk68Obj`: `+0x020 s32 unk20`, read directly off the object (not
through its vtable) and forwarded verbatim as `SceneNode__LinkModel`'s second
argument.

## The unconditional-load-in-a-delay-slot detail

Retail's `beqz $v0, END` (testing `self->unk68`) has `lw $a1, 0x20($v0)`
in its delay slot — i.e. the load of `unk68->unk20` executes
UNCONDITIONALLY, even on the path where `self->unk68` is NULL and the
branch is about to skip the call that would use it. This is ordinary
MIPS1 delay-slot scheduling (the compiler proved the load itself has no
side effect worth avoiding) and needed no special handling: writing the
natural `if (self->unk68 != NULL) { SceneNode__LinkModel(self, self->unk68->unk20); }`
reproduced it directly, with GCC choosing on its own to schedule the field
load into the branch's delay slot.

No residue otherwise — every one of the seven calls in this function
matched immediately once its slot was declared with the right argument
shape (all either `(TodActor *)` alone or `(TodActor *, s32)`, no
surprises).

### Proposed learning

None beyond what's already recorded — straightforward once the six new
slots were named from the call sites' own argument registers.

## Naming

Round 75 (charlie), track 3.

- `TodActor__Reset` (was `func_80065830`), tier A. Occupies +0x040, overriding Actor__Reset. Body sets defaults through its own slots: base SetDisplay(0), setUnk64(1), setLastOffsetValue(0x12C) (the 0x12C Actor__Reset writes too), disableTickCallback, selectTickCallback('A'), stopTod, setTod(0), then links mainPart's model to itself.

## Track 4 (2026-09-25, round 85, alpha)

Renamed from `TodActor__InitDefaults`. Override of +0x040, SceneNode's `reset` (Actor's occupant is Actor__Reset, renamed from InitDefaults the same way in round 82), named for its slot: it sets the object's defaults through its own and inherited slots, which is what the slot does in both parents. The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`. Any source block above is the pre-unification spelling; the live body in `src/code_55dd4.c` takes the unified types and slot names, byte-identical.

## Track 7 (round 99, bravo)

`0x12C` is written decimal (300), unnamed: Actor__Reset (ObjMStyleActor) writes
the same default into lastOffsetValue, so a name belongs in include/Actor.h
(proposed to the head, not applied here).
