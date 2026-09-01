# func_8005DA3C

**Unit:** Entity · **Size:** 28 words · **Status:** MATCHED (28/28 words, whole-image build verified byte-exact)

## What it does

A teardown counterpart to `func_8005D9F4` (which sets `this->unkF0 = 1` via
`this->methods->slot60(this, 1)`): calls `slot60(this, 0)`, then two more of
this entity's own current vtable slots (`slot16C`, newly typed this round;
`slot164`, also newly typed), then clears `this->unkF0`.

## Final C

```c
void func_8005DA3C(Entity *this) {
    this->methods->slot60(this, 0);
    this->methods->slot16C(this);
    this->methods->slot164(this, 0);
    this->unkF0 = 0;
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new.
