# Class65650__UnlinkPeer

> Renamed from `func_800667B0` on 2026-09-24 (tools/rename.py). Address 0x800667b0.

**Unit:** code_55dd4 · **Size:** 26 words (0x68 bytes) · **Status:** MATCHED
(26/26 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x140`. The "unlink" mirror of `Class65650__LinkPeer`'s
"link": if `self->unk94` is set, calls both objects' `slot14` on each other
(the companion's with `self`, then `self`'s with a **freshly re-read**
`self->unk94` — retail reloads the field rather than reusing the value
already in a register, so the C mirrors that), then clears
`self->unk94`.

```c
void Class65650__UnlinkPeer(Class65650 *self)
{
    Class65650 *other;

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

None beyond `Class65650__LinkPeer.md`'s (the two are a matched link/unlink pair).
