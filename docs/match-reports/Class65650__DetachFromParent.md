# Class65650__DetachFromParent

> Renamed from `func_800659D0` on 2026-09-24 (tools/rename.py). Address 0x800659d0.

**Unit:** code_55dd4 · **Size:** 35 words (0x8C bytes) · **Status:** MATCHED
(35/35 words, whole-image `./build-and-verify.sh` green)

## What it does

Guarded by a new field carved out of the first pass's `unk04` padding,
`self->unk0C` (offset `+0x0C`, in the BasicClass/intermediate-class region):
if set, unlinks the `+0x94` companion (`self->methods->slot140`, i.e.
`Class65650__UnlinkPeer`, already matched separately), then, if a *second* companion
pointer `self->unk50` (offset `+0x50`, likewise newly carved out) is set,
unlinks it too via the class's `slot14` (`+0x014`, the same "unlink"
inherited slot `Class65650__UnlinkPeer` itself uses on `unk94`), and finally calls
the base class's own `+0x050` slot on `self`.

```c
void Class65650__DetachFromParent(Class65650 *self)
{
    if (self->unk0C != 0) {
        self->methods->slot140(self);
        if (self->unk50 != NULL) {
            self->methods->slot14(self, self->unk50);
        }
        GetActorMethods()->slot50(self);
    }
}
```

Matched on the direct translation, no reshaping.

`self->unk0C`'s name and gating role echo `DreamSys`'s own `unk_0xC` gate
field (`include/DreamSys.h`) — both classes share the same intermediate
base `gActorMethods`, so a coincidence at the same low offset is plausible,
but nothing here proves the two fields are the same base-class field
rather than each subclass's own; noted, not claimed.

Adds `slot50` (`+0x050`) to `D800878D4Methods` and `slot140` (`+0x140`,
`Class65650__UnlinkPeer`) to `Class65650Methods`.

### Proposed learning

None new; confirms the `unk94`/`slot10`/`slot14` "companion link" shape
generalizes to a second, independent link slot (`unk50`) on the same
class, unlinked through the identical `slot14`.

## Naming

Round 75 (charlie), track 3.

- `Class65650__DetachFromParent` (was `func_800659D0`), tier A. Occupies +0x050, overriding SceneNode__DetachFromParent. While parent is set: unlinkPeer, unlink companion2 if set, chain the base detach. The mirror of AttachToParent.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gClass65650Methods`) is unified as `Class65650` in `include/Class65650.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `Class65650Methods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
