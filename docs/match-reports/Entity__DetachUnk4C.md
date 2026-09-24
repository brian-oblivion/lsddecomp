# Entity__DetachUnk4C

> Renamed from `func_8005D418` on 2026-09-19 (tools/rename.py). Address 0x8005d418.

**Unit:** Entity · **Size:** 26 words · **Status:** MATCHED (26/26 words, whole-image build verified byte-exact)

## What it does

A conditional teardown: if `this->unk0C` (a gate flag; see the parallel
noted in `Entity.h`'s struct comment with `Class65650`/`DreamSys`'s own
shared-base `+0xC` gate) is set, calls this entity's own current
`methods->slot160(this)`, then the shared "BasicClass" ancestor's own
`slot50` (`Get_vtable_Class65650()->slot50(this)`, offset `+0x050` in the shared
table), then clears `this->unk4C`.

## Final C

```c
void Entity__DetachUnk4C(Entity *this) {
    if (this->unk0C != 0) {
        this->methods->slot160(this);
        Get_vtable_Class65650()->slot50(this);
        this->unk4C = 0;
    }
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

This function (along with `Entity__NotifyReset`, `Entity__Update`, `Entity__GetOrCreateUnk100`)
is what established `Get_vtable_Class65650()`'s SHARED-vtable role for this unit —
see the class-framework comment block now at the top of `Entity.h`. Worth
flagging for anyone touching `Entity_b` next: `Get_vtable_Class65650()` (matched,
`code_55dd4.c`) is not Class65650-specific despite living in that unit and
returning `Class65650Methods*` there — it's the common ancestor's vtable
accessor, reused verbatim by Entity. A local, Entity-scoped `BasicClassMethods`
view of the SAME table (rather than importing `Class65650Methods` and
coupling the two units) is in `Entity.h` now, typed only as far as this
unit's own call sites need.

## Naming

**Tier B.** Renamed from `func_8005D418` this round (tools/rename.py). The
exact mirror of `Entity__AttachUnk4C` (clears `this->unk4C` under the same
`this->unk0C` gate, plus a base-ancestor `slot50` call and `methods->slot160`
-- now known to resolve to `Entity__Deactivate`, see the proposed field
names in `Entity__Deactivate.md`). Same caveat as AttachUnk4C: mechanics
established, the unk4C object's own purpose is not.
