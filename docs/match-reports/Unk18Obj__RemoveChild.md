> Renamed from `func_8003E7F4` on 2026-09-19 (tools/rename.py). Address 0x8003e7f4.

# Unk18Obj__RemoveChild — MATCH (32/32 words)

**Unit:** code_2cc8c_c · **Size:** 32 instructions

## What it does

`D_8006E8E4+0x014`: the exact teardown counterpart to `Unk18Obj__AddChild`
(`+0x010`) -- same `arg1->methods->header & 0xF` class-header dispatch,
clearing whichever of `self->unk10`/`unk30` or `self->unkC` that function
would have set, then unconditionally running the inherited BasicClass
`removeChild` on `arg1`.

## The C

```c
void Unk18Obj__RemoveChild(Unk18Obj *self, GenericObj *arg1)
{
    s32 header;

    header = arg1->methods->header & 0xF;
    if (header == 4) {
        self->unk30 = 0;
        self->unk10 = NULL;
    } else if (header == 1) {
        self->unkC = NULL;
    }
    Get_vtable_BasicClass()->slot14(self, arg1);
}
```

Matched on the first build -- direct transfer of `Unk18Obj__AddChild`'s own
field-to-header-value mapping, zero attempts spent re-deriving it, per that
report's own "Proposed learning" about reading ctor/dtor-adjacent slots
together.

## Struct/table knowledge established

- `BasicClassMethodsCC8C`: added `slot14` (`BasicClass__func_17ff0`,
  "removeChild").
- No new fields -- this function only confirms (never contradicts)
  `Unk18Obj__AddChild`'s `unk10`/`unk30`/`unkC` derivation, by clearing exactly
  what that function sets.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build. Last function in this round's queue -- see `Obj86B60__OnNotify.md`
and `New_Unk18Obj.md` for the two shared-table classtable dumps
(`gIntermediateBaseMethods`, `D_8006E8E4`) that resolved this and every other function in
this round except the register-identity stall `Obj86B60__NotifyParents`.

## Naming

**Unk18Obj__RemoveChild** (renamed from `func_8003E7F4`, round 55, runner
alpha). Tier A: exact mirror of `Unk18Obj__AddChild` -- clears the same
tag-keyed cached reference, THEN forwards to
`Get_vtable_BasicClass()->removeChild` (matches canonical
`BasicClassMethods::removeChild` `+0x014`).

## Proposed field names

- `Unk18ObjMethods::slot14` -> `removeChild` (tier A). Offset `+0x014`
  matches the canonical `BasicClassMethods::removeChild` offset exactly,
  and this header already documents it as "inherited BasicClass
  removeChild". NOT renamed directly: `code_2cc8c_d.c`'s `func_8003EB84`
  also dispatches `self->methods->slot14(self, self->unk10)` on a
  `Unk18Obj *`, so this field is shared with that unit. Head applies by
  type scope (rename in `Unk18ObjMethods`'s own definition, fix the
  compiler-listed accessors in both `code_2cc8c_c.c` and
  `code_2cc8c_d.c`, oracle).
