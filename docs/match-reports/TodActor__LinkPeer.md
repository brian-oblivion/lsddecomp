# TodActor__LinkPeer

> Renamed from `Class65650__LinkPeer` on 2026-09-26 (tools/rename.py). Address 0x80066748.

> Renamed from `func_80066748` on 2026-09-24 (tools/rename.py). Address 0x80066748.

**Unit:** TodActor · **Size:** 26 words (0x68 bytes) · **Status:** MATCHED
(26/26 words, whole-image `./build-and-verify.sh` green)

## What it does

`TodActorMethods` slot `+0x13C`. A symmetric "link" operation between two
instances of this class: if `other` is non-NULL, calls **both** objects'
`slot10` on each other (`other`'s with `self` as the argument, `self`'s with
`other` as the argument), then stashes `other` in `self->unk94`. The
unlink counterpart is `TodActor__UnlinkPeer` (slot `+0x140`), which reverses this
through `slot14`.

```c
void TodActor__LinkPeer(TodActor *self, TodActor *other)
{
    if (other != NULL) {
        other->methods->slot10(other, self);
        self->methods->slot10(self, other);
        self->unk94 = other;
    }
}
```

This corrects `self->unk94`'s type: the first pass (before any function
that touched it beyond zeroing it) guessed `s32`, matching the `sw
$zero, 0x94(...)` in the constructor. It is actually a `TodActor *` —
retyped in `src/world/tod_actor.c`, along with newly typing `slot10`'s
sibling `slot14` (`+0x014`, inherited from `gActorMethods`, "unlink" companion,
see `TodActor__UnlinkPeer`).

Matched on the direct translation, no reshaping.

### Proposed learning

A field zeroed with `sw $zero` in a constructor and never read elsewhere in
the functions attempted so far can still turn out to be a pointer — `sw
$zero` looks identical for a null pointer and an integer 0. Don't commit to
a scalar type until a function that actually *uses* the field (not just
initializes it) is decompiled.

## Naming

Round 75 (charlie), track 3.

- `TodActor__LinkPeer` (was `func_80066748`), tier A. Occupies +0x13C (called by AttachToParent). If `other` is set: other->linkCompanion(self), self->linkCompanion(other), peer = other. A symmetric link, fully evident from the body.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/tod_actor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
