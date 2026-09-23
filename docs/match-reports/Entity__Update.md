# Entity__Update

> Renamed from `func_8005D480` on 2026-09-19 (tools/rename.py). Address 0x8005d480.

**Unit:** Entity · **Size:** 56 words · **Status:** MATCHED (56/56 words, whole-image build verified byte-exact)

## What it does

A chain of conditional vtable dispatches through this entity's own current
`methods` table (slots `+0x170`, `+0x174`, `+0x178`, `+0x17C`, `+0x180` —
all newly-typed in `EntityMethods` this round), followed by a call into the
shared "BasicClass" ancestor's `slot98` with the function's own two extra
parameters forwarded.

## Final C

```c
void Entity__Update(Entity *this, s32 a1, s32 a2) {
    if (this->methods->slot170(this) != 0) {
        this->methods->slot174(this);
    }
    if (this->methods->slot17C(this) != 0) {
        this->methods->slot180(this);
    }
    this->methods->slot178(this);
    func_80066818()->slot98(this, a1, a2);
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new.

## Naming

**Tier B.** Renamed from `func_8005D480` this round (tools/rename.py). A
chain of five vtable dispatches (two conditional check-then-act pairs plus
one unconditional call) followed by a base-ancestor positional call
forwarding this function's own two arguments -- the shape of a per-tick
"run this entity's behaviour" dispatcher, touching most of this unit's
remaining named `EntityMethods` slots (`activationState`/`deactivationState`,
newly named this round; `slot178`/`slot17C`/`slot180`, still bare). "Update"
describes that shape without asserting what specifically is being updated.
See `## Proposed field names` below for two slots (`slotB8`/`slotBC`) whose
occupants this round's `tools/classtable.py` audit identified while reading
this table, even though neither is called from this unit.

## Proposed field names

- `EntityMethods::slotB8` -> `setVec14` -- **tier A.** `tools/classtable.py`
  on `ENTITY_METHODS` shows +0x0B8 occupied by the already-named
  `BaseObjO__SetVec14`. CROSS-UNIT (called by `func_8005F800`, not yet in
  any Entity_x unit read so far, plus this unit's own header notes a
  same-shape argument at `Entity__IsNearTarget`'s still-`INCLUDE_ASM` call
  site) -- not renamed here, proposed for the head to apply by type scope.
- `EntityMethods::slotBC` -> `addVec14` -- **tier A.** Same audit, +0x0BC
  occupied by `BaseObjO__AddVec14`. Called by `Entity__MoodCue08` (unit not
  identified from this file alone). Proposed, not applied.
