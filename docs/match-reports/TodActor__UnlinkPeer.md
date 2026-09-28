# TodActor__UnlinkPeer

> Renamed from `Class65650__UnlinkPeer` on 2026-09-26 (tools/rename.py). Address 0x800667b0.

> Renamed from `func_800667B0` on 2026-09-24 (tools/rename.py). Address 0x800667b0.

**Unit:** TodActor · **Size:** 26 words (0x68 bytes) · **Status:** MATCHED
(26/26 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x140`. The "unlink" mirror of `TodActor__LinkPeer`'s
"link": if `self->unk94` is set, calls both objects' `slot14` on each other
(the companion's with `self`, then `self`'s with a **freshly re-read**
`self->unk94` — retail reloads the field rather than reusing the value
already in a register, so the C mirrors that), then clears
`self->unk94`.

```c
void TodActor__UnlinkPeer(TodActor *self)
{
    TodActor *other;

    other = self->unk94;
    if (other != NULL) {
        other->methods->slot14(other, self);
        self->methods->slot14(self, self->unk94);
        self->unk94 = NULL;
    }
}
```

Matched on the direct translation, including the reload (writing
`self->methods->slot14(self, other);` with the cached local instead would
still be semantically correct but is not what retail's bytes show — not
tested since the as-derived form matched immediately).

### Proposed learning

None beyond `TodActor__LinkPeer.md`'s (the two are a matched link/unlink pair).

## Naming

Round 75 (charlie), track 3.

- `TodActor__UnlinkPeer` (was `func_800667B0`), tier A. Occupies +0x140 (called by DetachFromParent). The mirror of LinkPeer: mutual unlinkCompanion, peer = NULL.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/tod_actor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
