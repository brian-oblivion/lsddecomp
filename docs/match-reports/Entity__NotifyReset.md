> Renamed from `func_8005D658` on 2026-09-19 (tools/rename.py). Address 0x8005d658.

# Entity__NotifyReset

**Unit:** Entity · **Size:** 31 words · **Status:** MATCHED (31/31 words, whole-image build verified byte-exact)

## What it does

Calls the shared "BasicClass" ancestor's `slotE0(this, a1, a2)`, then, if
`a2 == 4`, also calls this entity's own current `methods->slot160(this)` —
the same `slot160` that `Entity__DetachUnk4C` and `Entity__UpdateDeactivationState` also dispatch
through.

## Final C

```c
void Entity__NotifyReset(Entity *this, s32 a1, s32 a2) {
    func_80066818()->slotE0(this, a1, a2);
    if (a2 == 4) {
        this->methods->slot160(this);
    }
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new.

## Naming

**Tier B.** Renamed from `func_8005D658` this round (tools/rename.py).
Forwards `(a1, a2)` to the base ancestor's `slotE0`, and on `a2 == 4` also
fires `EntityMethods::slot160` -- confirmed this round via
`tools/classtable.py` to resolve to `Entity__Deactivate` for a base Entity
(see `Entity__Deactivate.md`'s proposed field name for the slot itself).
"Reset" is deliberately weaker than "Deactivate": this function's own
evidence is only "forwards an event code, and on code 4 also deactivates",
not what the forwarded event fundamentally represents.
