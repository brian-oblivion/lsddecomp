# TodActor__AttachToParent

> Renamed from `Class65650__AttachToParent` on 2026-09-26 (tools/rename.py). Address 0x80065918.

> Renamed from `func_80065918` on 2026-09-24 (tools/rename.py). Address 0x80065918.

**Unit:** TodActor · **Size:** 46 words (0xB8 bytes) · **Status:** MATCHED
(46/46 words, whole-image `./build-and-verify.sh` green)

## What it does

A five-argument function (the fifth passed on the caller's stack, o32 ABI
overflow), gated entirely by `self->unk0C` (the same guard flag that gates
`TodActor__DetachFromParent`'s whole body). When clear, forwards to the shared
intermediate base's `+0x04C` slot, conditionally links via
`self->methods->slot10`, then unconditionally links `other` via
`self->methods->slot13C`:

```c
void TodActor__AttachToParent(TodActor *self, TodActor *other, void *arg2, void *arg3, void *arg4)
{
    D800878D4Methods *base;

    if (self->unk0C == 0) {
        base = GetActorMethods();
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
`TodActorMethods+0x13C`**: the header already carried a comment noting
that offset held `TodActor__LinkPeer` in the compiled table but had never seen
a real dispatch call, so it was left as `pad13C[0x04]`. Cross-checked with
`tools/classtable.py gTodActorMethods` (never by counting, per project policy) —
confirmed `+0x13C -> TodActor__LinkPeer` — and since `TodActor__LinkPeer`'s own
signature is `(TodActor *self, TodActor *other)`, that's now `slot13C`'s
real type, and `other`'s type follows from it (it's not a generic `void *`
argument, it's specifically another `TodActor *`, matching
`TodActor__LinkPeer`'s buddy-link semantics with `self->unk94`).

No residue — matched first attempt. The o32 stack-passed fifth argument
(`arg4`, at `0x38($sp)` after this function's own `-0x28` frame adjustment)
needed no special handling; it just reads as an ordinary parameter.

### Proposed learning

**A `padNNN[0x04]` slot with a comment naming which function occupies it
in the compiled vtable is worth re-checking with `classtable.py` whenever
a new call site is found**, rather than assuming the earlier "not
dispatched through here" note still holds — the type derived from the
now-confirmed target function's own signature (here, `TodActor *other`
instead of a generic `void *`) is strictly more useful than leaving the
slot untyped.

## Naming

Round 75 (charlie), track 3.

- `TodActor__AttachToParent` (was `func_80065918`), tier A. Occupies +0x04C, overriding SceneNode__AttachToParent. While parent (+0x0C) is NULL: chains the base attach with (arg3, arg4) = (parent, offset), links arg2 as a companion if companion2 is empty, then linkPeer(arg1).

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

The occupant of +0x04C takes (self, peer, companion, parent, offset), two more leading arguments than the slot's SceneNode type (self, parent, offset), which it forwards to SceneNode's attachToParent as (arg3, arg4). The unified table keeps the inherited type at +0x04C; Entity__AttachToParent, the one C caller through the slot, casts it to `TodActorAttachToParentFn` (a function-pointer cast emits no code).
