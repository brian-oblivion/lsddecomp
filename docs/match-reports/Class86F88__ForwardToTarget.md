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
unit's own occupant, `Class86F88__ScrollLeft`, does not itself read past `self`;
the 3-argument shape here comes from this call site, per the project's
established per-call-site-arity convention, not from the occupant's body).
`arg1` is `Class86F88__ForwardToTarget`'s own second parameter, forwarded verbatim and
otherwise unused -- an ordinary "unused-locally, live-at-the-call"
parameter, not something read from `self`.

Matched on the first attempt.

## Naming

Round 75 (bravo, track 3). `func_800523F0` -> `Class86F88__ForwardToTarget`, **tier B**.

Slot +0x060 (`tools/classtable.py gClass86F88Methods`). If `target` (+0x03C) is set, calls its +0x080 with (code, 0x60, 0x60). `target` is whatever Class86F88__AddChildAndSetState stores from its arg3, which TaskObjF__AttachChildA/B pass as their `childC`; TaskObjF__SetChildFlag8 calls the same slot with (code, 0x7F, 0x7F). Called with 0x10 before closing and with 0 on every cursor move/refresh. A sound cue with volumes would fit, but nothing proves it, so the name says only what the code does.

Class86F88, per the round-75 pass, is a scrolling list selector: up to 4 visible rows of 26-character item text, a highlighted cursor row, a horizontal column offset (see the unit header comment of `src/class_3bb8c_k.c`).
