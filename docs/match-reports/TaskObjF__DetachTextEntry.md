# TaskObjF__DetachTextEntry -- MATCH

> Renamed from `TaskObjF__DetachChildA` on 2026-09-26 (tools/rename.py). Address 0x80050410.

> Renamed from `Class86E00_3bb8c_g__DetachChildA` on 2026-09-23 (tools/rename.py). Address 0x80050410.

> Renamed from `func_80050410` on 2026-09-23 (tools/rename.py). Address 0x80050410.

Unit `TitleMenuTaskObjF`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__DetachTextEntry`: 48/48 words match.

## Source

```c
void TaskObjF__DetachTextEntry(Class86E00_3bb8c_g *self)
{
    if (self->unk68 != 0 && self->unk60 != 0 && self->unk78 != NULL) {
        self->unk78->methods->slot50(self->unk78);
        self->unk78->methods->slot48(self->unk78);
        if (self->unk74 != 0) {
            self->unk78->methods->release(self->unk78);
            self->unk78 = NULL;
        }
    }
}
```

First attempt, byte-exact. `self->unk78` is re-dereferenced fresh before
each of its three uses rather than cached in a local, matching this
project's established "re-dereference; do not cache in a named local"
idiom -- caching it would have forced it into a callee-saved register
across the intervening vtable calls and cost extra instructions. The
three-way `&&` guard compiled directly to retail's three chained `beqz`s
with no reshaping needed, same as its sibling `TaskObjF__AttachTextEntry`.

This is the teardown counterpart to `TaskObjF__AttachTextEntry`'s lazy-init: attach
(`slot10`/`slot44`/`slot4C`) there, detach (`slot50`/`slot48`/`release`)
here, gated by the same one-shot `unk74` flag that init sets.

## Struct changes (additive, `include/class_3bb8c.h`)

None new -- `slot48`/`slot50`/`release` were all already declared while
surveying the unit (`TaskObjF__ReleaseCardIcon`'s report).

### Proposed learning

None new.

## Naming

`TaskObjF__DetachTextEntry` (was `func_80050410`), tier B: the
teardown counterpart of `TaskObjF__AttachTextEntry` -- same guard,
tears down `self->unk78` through the fixed `slot50`/`slot48`/`release`
sequence, one-shot gated by the same `unk74` flag `AttachChildA` sets.

## Track 4 (2026-09-26, round 87)

TaskObjF's childA is now `struct TextEntry *`: slot50/slot48 are TextEntry's detachTarget/releaseCardResources. Zero bytes changed.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__DetachChildA`: the inverse of TaskObjF__AttachTextEntry on `textEntry` (TextEntry detachTarget, releaseCardResources, and release when `ownsWidget`). Called by TaskObjF__OnTextEntryResult through +0x0A0.
