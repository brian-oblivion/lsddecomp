# Viewport__AddChild — MATCH (33/33 words)

> Renamed from `Unk18Obj__AddChild` on 2026-09-25 (tools/rename.py). Address 0x8003e770.

> Renamed from `func_8003E770` on 2026-09-19 (tools/rename.py). Address 0x8003e770.

**Unit:** code_2cc8c_c · **Size:** 33 instructions

## What it does

`gViewportMethods+0x010`: runs the inherited BasicClass `addChild` on `arg1`, then
discriminates `arg1`'s DYNAMIC CLASS by reading `arg1->methods->header & 0xF`
-- the low nibble of `arg1`'s own vtable header word, the same "header"
concept `tools/classtable.py` labels on every table it dumps -- and stores
`arg1` into `self->unk10` (plus `self->unk30 = arg1->unk14`) when that
nibble is 4, or into `self->unkC` when it is 1; any other value is a no-op.

## The C

```c
void Viewport__AddChild(Unk18Obj *self, GenericObj *arg1)
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
  `Viewport__Viewport`'s ctor as "zeroed, meaning unknown") to `GenericObj *` --
  this function is the first to WRITE a non-zero value into either, and it
  is unambiguously a pointer (`arg1` itself). The ctor's `self->unkC = 0;`/
  `self->unk10 = 0;` assignments are unaffected by the retype (`0` converts
  to a null pointer implicitly either way).
- `Unk18Obj`: new field `unk30` (`s32`), set from `arg1->unk14` on the same
  path as `unk10`.
- `BasicClassMethodsCC8C`: added `slot10` (`BasicClass__AddChild`,
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
often (compare `IntermediateBase__OnNotify`'s own `arg1->target->header & 0xF` dispatch,
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
shape. Mirrored exactly by `Viewport__RemoveChild`.

## Proposed field names

- `Unk18ObjMethods::slot10` -> `addChild` (tier A). Offset `+0x010` matches
  the canonical `BasicClassMethods::addChild` offset exactly, and this
  header already documents it as "inherited BasicClass addChild". NOT
  renamed directly: `code_2cc8c_d.c`'s `Viewport__AttachViewChild` also dispatches
  `m->slot10(self, a1)` on a `Unk18Obj *`, so this field is shared with
  that unit. Head applies by type scope (rename in `Unk18ObjMethods`'s own
  definition, fix the compiler-listed accessors in both `code_2cc8c_c.c`
  and `code_2cc8c_d.c`, oracle).

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__AddChild`. The +0x010 addChild override. Its parameter is `BasicClass *child`; the class-nibble cache names the fields: 4 -> `viewNode` (a Class6B5CC; its +0x014 `coord2` goes into `refView.super`, the GsRVIEW2's super coordinate), 1 -> `drawSystem` (gDrawSystemMethods, DrawSystem). The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
