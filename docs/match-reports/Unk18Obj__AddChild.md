> Renamed from `func_8003E770` on 2026-09-19 (tools/rename.py). Address 0x8003e770.

# Unk18Obj__AddChild — MATCH (33/33 words)

**Unit:** code_2cc8c_c · **Size:** 33 instructions

## What it does

`D_8006E8E4+0x010`: runs the inherited BasicClass `addChild` on `arg1`, then
discriminates `arg1`'s DYNAMIC CLASS by reading `arg1->methods->header & 0xF`
-- the low nibble of `arg1`'s own vtable header word, the same "header"
concept `tools/classtable.py` labels on every table it dumps -- and stores
`arg1` into `self->unk10` (plus `self->unk30 = arg1->unk14`) when that
nibble is 4, or into `self->unkC` when it is 1; any other value is a no-op.

## The C

```c
void Unk18Obj__AddChild(Unk18Obj *self, GenericObj *arg1)
{
    s32 header;

    Get_vtable_BasicClass()->slot10(self, arg1);
    header = arg1->methods->header & 0xF;
    if (header == 4) {
        self->unk10 = arg1;
        self->unk30 = arg1->unk14;
    } else if (header == 1) {
        self->unkC = arg1;
    }
}
```

Matched on the first build.

## Struct/table knowledge established

- New type `GenericObj`/`GenericObjMethods`: a minimal "any class instance"
  view exposing only the universal vtable header word and one instance
  field (`unk14`) this function reads. `arg1`'s real class is unknown and
  irrelevant here -- this function's own logic only cares about the header
  nibble, not the type it names.
- `Unk18Obj->unkC`/`unk10`: retyped from bare `s32` (established by
  `Unk18Obj__Unk18Obj`'s ctor as "zeroed, meaning unknown") to `GenericObj *` --
  this function is the first to WRITE a non-zero value into either, and it
  is unambiguously a pointer (`arg1` itself). The ctor's `self->unkC = 0;`/
  `self->unk10 = 0;` assignments are unaffected by the retype (`0` converts
  to a null pointer implicitly either way).
- `Unk18Obj`: new field `unk30` (`s32`), set from `arg1->unk14` on the same
  path as `unk10`.
- `BasicClassMethodsCC8C`: added `slot10` (`BasicClass__func_17f98`,
  "addChild").

### Proposed learning

A double dereference of the shape `x->methods->firstWord` (offset 0,0)
combined with an `& 0xF` mask is runtime type identification through the
vtable header nibble, not a struct-field read -- this project's
`classtable.py` already treats table offset 0 as a "header word (id/flags)"
for every class it dumps, and this function is direct proof that the GAME
itself reads that same word the same way at runtime, not just the tooling.
Recognise this shape (`lw v,0(x); lw v,0(v); andi v,v,K`) on sight rather
than deriving it fresh each time it recurs -- it will likely reappear
anywhere a class hierarchy dispatches on "what kind of thing was just added
as a child", which this specific vtable/BasicClass-derived family does
often (compare `Obj86B60__OnNotify`'s own `arg1->target->header & 0xF` dispatch,
a close cousin one level of indirection removed).

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic, only a
  mask and equality comparisons.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**Unk18Obj__AddChild** (renamed from `func_8003E770`, round 55, runner
alpha). Tier A: forwards to `Get_vtable_BasicClass()->addChild` first (base
slot name confirmed, matches canonical `BasicClassMethods::addChild`
`+0x010`), then examines the added child's dynamic class tag
(`arg1->methods->header & 0xF`) to cache a typed reference -- the standard
"override calls base, then does its own bookkeeping" addChild-override
shape. Mirrored exactly by `Unk18Obj__RemoveChild`.
