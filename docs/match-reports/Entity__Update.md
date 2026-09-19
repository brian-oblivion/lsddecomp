> Renamed from `func_8005D480` on 2026-09-19 (tools/rename.py). Address 0x8005d480.

# Entity__Update

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
