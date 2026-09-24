# Entity__Destructor

> Renamed from `func_8005D1EC` on 2026-09-19 (tools/rename.py). Address 0x8005d1ec.

**Unit:** Entity · **Size:** 35 words · **Status:** MATCHED (35/35 words, whole-image build verified byte-exact)

## What it does

Entity's destructor-side teardown: tears down `this->unk100` and
`this->unk104` (each a `Unk100Obj *`, see `Entity__GetOrCreateUnk100.md`) by calling
their own `slot04` (a per-object dtor-like slot, not `EntityMethods`'), then
calls the shared "BasicClass" ancestor's own `dtor` slot,
`Get_vtable_Class65650()->dtor(this)` (offset `+0x00C` in the shared table — see the
big comment in `Entity.h`).

## Final C

```c
void Entity__Destructor(Entity *this) {
    if (this->unk100 != NULL) {
        this->unk100->methods->slot04(this->unk100);
    }
    if (this->unk104 != NULL) {
        this->unk104->methods->slot04(this->unk104);
    }
    Get_vtable_Class65650()->dtor(this);
}
```

## Attempt log

Matched on the first attempt — a straightforward transliteration of the
disassembly (and of m2c's own output, which was already correct here).

## Proposed learning

None new.

## Naming

**Tier A.** Renamed from `func_8005D1EC` this round (tools/rename.py).
Occupies `EntityMethods` dtor slot +0x00C (`tools/classtable.py`), the
direct counterpart to `Entity__Entity`'s ctor slot +0x008. A destructor's
mechanics (tear down the two cached sub-objects, then the shared ancestor's
own `dtor`) are its purpose.
