# Class86F88__ForwardToTarget -- MATCH

> Renamed from `func_800523F0` on 2026-09-24 (tools/rename.py). Address 0x800523f0.

Unit `class_3bb8c_k`, round 15. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86F88__ForwardToTarget`: 16/16 words match.

## Source

```c
void Class86F88__ForwardToTarget(Class86F88 *self, s32 arg1)
{
    Class86F88 *other = self->unk3C;

    if (other != NULL) {
        other->methods->slot80(other, arg1, 0x60, 0x60);
    }
}
```

## Notes

`self->unk3C` is another instance of this same class (`Class86F88`) --
this function forwards to that OTHER instance's own `slot80` (which this
unit's own occupant, `func_80052498`, does not itself read past `self`;
the 3-argument shape here comes from this call site, per the project's
established per-call-site-arity convention, not from the occupant's body).
`arg1` is `Class86F88__ForwardToTarget`'s own second parameter, forwarded verbatim and
otherwise unused -- an ordinary "unused-locally, live-at-the-call"
parameter, not something read from `self`.

Matched on the first attempt.
