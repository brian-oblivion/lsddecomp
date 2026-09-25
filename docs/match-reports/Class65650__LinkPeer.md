# Class65650__LinkPeer

> Renamed from `func_80066748` on 2026-09-24 (tools/rename.py). Address 0x80066748.

**Unit:** code_55dd4 · **Size:** 26 words (0x68 bytes) · **Status:** MATCHED
(26/26 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x13C`. A symmetric "link" operation between two
instances of this class: if `other` is non-NULL, calls **both** objects'
`slot10` on each other (`other`'s with `self` as the argument, `self`'s with
`other` as the argument), then stashes `other` in `self->unk94`. The
unlink counterpart is `Class65650__UnlinkPeer` (slot `+0x140`), which reverses this
through `slot14`.

```c
void Class65650__LinkPeer(Class65650 *self, Class65650 *other)
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
$zero, 0x94(...)` in the constructor. It is actually a `Class65650 *` —
retyped in `include/code_55dd4.h`, along with newly typing `slot10`'s
sibling `slot14` (`+0x014`, inherited from `gActorMethods`, "unlink" companion,
see `Class65650__UnlinkPeer`).

Matched on the direct translation, no reshaping.

### Proposed learning

A field zeroed with `sw $zero` in a constructor and never read elsewhere in
the functions attempted so far can still turn out to be a pointer — `sw
$zero` looks identical for a null pointer and an integer 0. Don't commit to
a scalar type until a function that actually *uses* the field (not just
initializes it) is decompiled.

## Naming

Round 75 (charlie), track 3.

- `Class65650__LinkPeer` (was `func_80066748`), tier A. Occupies +0x13C (called by AttachToParent). If `other` is set: other->linkCompanion(self), self->linkCompanion(other), peer = other. A symmetric link, fully evident from the body.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
