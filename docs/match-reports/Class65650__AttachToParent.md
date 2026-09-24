# Class65650__AttachToParent

> Renamed from `func_80065918` on 2026-09-24 (tools/rename.py). Address 0x80065918.

**Unit:** code_55dd4 · **Size:** 46 words (0xB8 bytes) · **Status:** MATCHED
(46/46 words, whole-image `./build-and-verify.sh` green)

## What it does

A five-argument function (the fifth passed on the caller's stack, o32 ABI
overflow), gated entirely by `self->unk0C` (the same guard flag that gates
`Class65650__DetachFromParent`'s whole body). When clear, forwards to the shared
intermediate base's `+0x04C` slot, conditionally links via
`self->methods->slot10`, then unconditionally links `other` via
`self->methods->slot13C`:

```c
void Class65650__AttachToParent(Class65650 *self, Class65650 *other, void *arg2, void *arg3, void *arg4)
{
    D800878D4Methods *base;

    if (self->unk0C == 0) {
        base = DreamSys__GetBaseMethods();
        base->slot4C(self, arg3, arg4);
        if (arg2 != NULL && self->unk50 == NULL) {
            self->methods->slot10(self, arg2);
        }
        self->methods->slot13C(self, other);
    }
}
```

Resolves `D800878D4Methods+0x04C` (new base-class slot, split out of the
old `pad3C[0x14]`) and, more usefully, **confirms and reactivates
`Class65650Methods+0x13C`**: the header already carried a comment noting
that offset held `Class65650__LinkPeer` in the compiled table but had never seen
a real dispatch call, so it was left as `pad13C[0x04]`. Cross-checked with
`tools/classtable.py gClass65650Methods` (never by counting, per project policy) —
confirmed `+0x13C -> Class65650__LinkPeer` — and since `Class65650__LinkPeer`'s own
signature is `(Class65650 *self, Class65650 *other)`, that's now `slot13C`'s
real type, and `other`'s type follows from it (it's not a generic `void *`
argument, it's specifically another `Class65650 *`, matching
`Class65650__LinkPeer`'s buddy-link semantics with `self->unk94`).

No residue — matched first attempt. The o32 stack-passed fifth argument
(`arg4`, at `0x38($sp)` after this function's own `-0x28` frame adjustment)
needed no special handling; it just reads as an ordinary parameter.

### Proposed learning

**A `padNNN[0x04]` slot with a comment naming which function occupies it
in the compiled vtable is worth re-checking with `classtable.py` whenever
a new call site is found**, rather than assuming the earlier "not
dispatched through here" note still holds — the type derived from the
now-confirmed target function's own signature (here, `Class65650 *other`
instead of a generic `void *`) is strictly more useful than leaving the
slot untyped.

## Naming

Round 75 (charlie), track 3.

- `Class65650__AttachToParent` (was `func_80065918`), tier A. Occupies +0x04C, overriding Class6B5CC__AttachToParent. While parent (+0x0C) is NULL: chains the base attach with (arg3, arg4) = (parent, offset), links arg2 as a companion if companion2 is empty, then linkPeer(arg1).
