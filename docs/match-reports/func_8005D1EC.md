# func_8005D1EC

**Unit:** Entity · **Size:** 35 words · **Status:** MATCHED (35/35 words, whole-image build verified byte-exact)

## What it does

Entity's destructor-side teardown: tears down `this->unk100` and
`this->unk104` (each a `Unk100Obj *`, see `func_8005D108.md`) by calling
their own `slot04` (a per-object dtor-like slot, not `EntityMethods`'), then
calls the shared "BasicClass" ancestor's own `dtor` slot,
`func_80066818()->dtor(this)` (offset `+0x00C` in the shared table — see the
big comment in `Entity.h`).

## Final C

```c
void func_8005D1EC(Entity *this) {
    if (this->unk100 != NULL) {
        this->unk100->methods->slot04(this->unk100);
    }
    if (this->unk104 != NULL) {
        this->unk104->methods->slot04(this->unk104);
    }
    func_80066818()->dtor(this);
}
```

## Attempt log

Matched on the first attempt — a straightforward transliteration of the
disassembly (and of m2c's own output, which was already correct here).

## Proposed learning

None new.
