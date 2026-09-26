# IntermediateBase__Deinit — MATCH (102/102 words)

> Renamed from `Obj86B60__Deinit` on 2026-09-25 (tools/rename.py). Address 0x8003e280.

> Renamed from `func_8003E280` on 2026-09-19 (tools/rename.py). Address 0x8003e280.

**Unit:** code_2cc8c_c · **Size:** 102 instructions (largest in this
round's queue)

## What it does

`gIntermediateBaseMethods+0x048` (and `gTitleMenuMethods`'s own verbatim-inherited `+0x048`, per
`tools/classtable.py`): the teardown counterpart to `IntermediateBase__Init`'s
init/registration -- calls `slot50` (external, unobserved elsewhere),
unconditionally removes the three children `IntermediateBase__Init` added
(`self->unk10`, `self->unkC->unk4`, `self->unkC->unk0`, via the inherited
BasicClass `removeChild`), and, only when `self->unk24 == 0`, also removes
`self->unk10` and `self->unkC->unk0` through `self->unk14`'s/`self->unk18`'s
OWN `removeChild` slots first. Finally, for each of `self->unk10`/`unk14`/
`unk18`, releases it (inherited BasicClass "release", offset +0x004) ONLY IF
it was not the one explicitly supplied via `self->unkC` (the cached
`Obj86B60InitArgs *` from `IntermediateBase__Init`) -- the standard "free only what
you allocated yourself" ownership idiom, mirroring that function's own
"use if given, else allocate" construction.

## The C

```c
void IntermediateBase__Deinit(Obj86B60 *self)
{
    Obj86B60Methods *methods;
    Unk18Obj *obj18;

    methods = self->methods;
    methods->slot50(self);
    obj18 = self->unk18;
    if (self->unk24 == 0) {
        ((Unk14Obj *)self->unk14)->methods->slot14((Unk14Obj *)self->unk14, (void *)self->unk10);
        obj18->methods->slot14(obj18, (void *)self->unk10);
        obj18->methods->slot14(obj18, ((Obj86B60InitArgs *)self->unkC)->unk0);
    }
    methods->slot14(self, (void *)self->unk10);
    methods->slot14(self, ((Obj86B60InitArgs *)self->unkC)->unk4);
    methods->slot14(self, ((Obj86B60InitArgs *)self->unkC)->unk0);
    if (((Obj86B60InitArgs *)self->unkC)->unk10 != obj18) {
        self->unk18 = obj18->methods->slot4(obj18);
    }
    if ((void *)((Obj86B60InitArgs *)self->unkC)->unkC != (void *)self->unk14) {
        self->unk14 = (s32)((Unk14Obj *)self->unk14)->methods->slot4((Unk14Obj *)self->unk14);
    }
    if (((Obj86B60InitArgs *)self->unkC)->unk8 != (void *)self->unk10) {
        self->unk10 = (s32)((Unk10Obj *)self->unk10)->methods->slot4((Unk10Obj *)self->unk10);
    }
}
```

Matched on the first build. Retail reloads `self->unkC` fresh (a plain
`lw`) at every one of the five uses rather than caching it in a register --
matched by NOT introducing a local `Obj86B60InitArgs *args` and instead
repeating the cast at each use site, which is what let this land byte-exact
without a register-pressure iteration this time.

## Struct/table knowledge established

- `Obj86B60Methods`: added `slot14` (inherited BasicClass removeChild) and
  `slot50` (external `TitleMenu__OnDeinit`, unobserved elsewhere).
- `Unk14Obj`/`Unk18Obj`: both confirmed to ALSO expose the inherited
  BasicClass `slot4` (release) and `slot14` (removeChild) -- same universal
  low-offset BasicClass layout every class in this game shares. Both types
  are now fully explained by "generic BasicClass descendant, only ever
  touched through inherited slots" -- neither has a single class-specific
  method observed anywhere in this unit.
- New type `Unk10Obj`/`Unk10ObjMethods`: a THIRD alternate pointer reading
  of `self->unk10` (which stays `s32` in `Obj86B60`, per this header's
  established "keep the general field, cast locally" convention -- see
  `Unk14Obj`'s own comment). Only `slot4` (release) is modelled; this
  function is the only place it is dereferenced as a pointer at all in this
  unit outside `IntermediateBase__Init`'s own (non-dereferencing) forwards.
- This confirms `self->unkC`'s runtime identity across BOTH functions that
  touch it: it is the `Obj86B60InitArgs *` `IntermediateBase__Init` stored there, not
  the unrelated `Obj86B60UnkC *` reading `IntermediateBase__OnState2` uses at the same
  offset from a completely different call path -- consistent with this
  project's established "one struct offset, multiple independent readings by
  different call paths" pattern (`Unk4CObj->unk24`, `Unk64Elem`'s own
  alternate reading), not a conflict.

### Proposed learning

`IntermediateBase__Init`'s "construct with an optional override" idiom
(`field = arg ? arg : helper()`) has a predictable TEARDOWN counterpart one
function away in the same vtable: "release `field` only if
`storedInitArgs->correspondingField != field`". When a match report
documents the construction half of this shape, check the SIBLING vtable slot
immediately following the constructor's own slot (here `slot44` then
`slot48`) before treating the destructor-shaped function as an independent
derivation -- the field-to-arg mapping transfers directly and cost zero
attempts here.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no negation anywhere in
  this function.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk; every access is a single struct dereference or vtable call.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**IntermediateBase__Deinit** (renamed from `func_8003E280`, round 55, runner
alpha). Tier A: exact mirror of `IntermediateBase__Init` -- removes the children
`Init` added (`removeChild`), then releases (`Unk14ObjMethods::release`/
`Unk10ObjMethods::release`, both renamed this round, and
`Unk18ObjMethods::release` (also renamed this round, exclusive --
code_2cc8c_d.c never dispatches this exact slot on a Unk18Obj*)) each of the three helper
objects `Init` may have default-constructed, but ONLY the ones whose
current value still differs from the caller-supplied `initArgs` field --
i.e. only the ones this object actually owns. `Obj86B60Methods::deinit`
(`+0x048`, exclusive to this unit, renamed from `slot48`) IS this function,
confirmed by `tools/classtable.py`/direct table read (see the header's own
"IS" attribution) and independently by `IntermediateBase__Init` calling
`methods->deinit(self)` on its own mode-0 path.

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__Deinit (class prefix). Occupies +0x048, slot `deinit`. Its first call, +0x050, is `onDeinit` (NULL here; Class865C8__OnDeinit, ObjM__TeardownStyle, TaskCore__OnDeinit override it). The three releases go through BasicClass's release on `BasicClass *` fields (the Unk10Obj/Unk14Obj local views are gone).
