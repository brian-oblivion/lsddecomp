# Class65650__OnNotify

> Renamed from `func_80065790` on 2026-09-24 (tools/rename.py). Address 0x80065790.

**Unit:** code_55dd4 · **Size:** 40 words (0xA0 bytes) · **Status:** MATCHED
(40/40 words, whole-image `./build-and-verify.sh` green)

## What it does

Forwards to the shared intermediate base class's `+0x038` slot with all
three of its own arguments, then conditionally dispatches through its own
`slot04` when `arg1` carries a specific type tag, `arg2 == 1`, and
`self->unk60` (the "do I own `unk5C`" guard, already known from
`Class65650__AcquireModelData`/`Class65650__ReleaseModelData`) is clear:

```c
void Class65650__OnNotify(Class65650 *self, TagCheckArg *arg1, s32 arg2)
{
    D800878D4Methods *base;

    base = DreamSys__GetBaseMethods();
    base->slot38(self, arg1, arg2);
    if (arg1->tagged->tag == 0x5F03 && arg2 == 1 && self->unk60 == 0) {
        self->methods->slot04(self);
    }
}
```

This resolves a new `D800878D4Methods` slot (`+0x038`, previously inside
`pad1C[0x34]`, now split into `pad1C[0x1C]` + `slot38` + `pad3C[0x14]`) and
introduces `arg1`'s minimal type: `TagCheckArg` has one field so far,
`+0x000 TaggedObj *tagged`, and `TaggedObj` has one field, `+0x000 u16 tag`
— checked against the magic value `0x5F03`. Neither type is identified
beyond what this function actually touches, per this unit's established
policy for opaque unidentified classes.

No residue — matched on the first attempt once the types above were
declared. The whole function is a straightforward sequence: one `jal` to a
fixed function, one indirect call through the resolved base-class slot, a
three-part `&&` guard (tag check, `arg2` check, `self->unk60` check), and
one more indirect call through `self->methods->slot04` gated by that
guard. `self->methods->slot04` was already typed from `Class65650__OnClass6EF50Notify`'s
report, so no new speculation was needed there.

### Proposed learning

None beyond what's already recorded for this unit — this one was a clean,
single-pass match once the two new field/slot types were named.
