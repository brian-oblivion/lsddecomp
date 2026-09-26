# IntermediateBase__Init — MATCH (93/93 words)

> Renamed from `Obj86B60__Init` on 2026-09-25 (tools/rename.py). Address 0x8003e10c.

> Renamed from `func_8003E10C` on 2026-09-19 (tools/rename.py). Address 0x8003e10c.

**Unit:** code_2cc8c_c · **Size:** 93 instructions

## What it does

`gIntermediateBaseMethods+0x044` (the "IntermediateBase" table's own slot44, and
`gClass86B60Methods`'s own `+0x044` occupant per `tools/classtable.py`): an
init/registration routine. Fills three fields with "use the init-args field
if set, else derive from a helper call" (`self->unk10`/`unk14`/`unk18`),
stashes the init-args pointer itself into `self->unkC`, registers three
children through the inherited BasicClass `addChild` (`self->methods->
slot10`), forwards `(self,0,0,0)` to `slot4C`, records `arg2` into
`self->unk24`, and -- only when `arg2 == 0` -- runs three more registration
calls (two through the just-constructed `self->unk18` object, one through
`self->unk14` reinterpreted as a pointer) before dispatching `slot60(self,2)`
and `slot48(self)` (`IntermediateBase__Deinit`, the very next function in this queue).

## The C

```c
void IntermediateBase__Init(Obj86B60 *self, Obj86B60InitArgs *arg1, s32 arg2)
{
    Obj86B60Methods *methods;
    Unk18Obj *obj18;

    methods = self->methods;
    if (arg1->unk8 != NULL) {
        self->unk10 = (s32)arg1->unk8;
    } else {
        self->unk10 = (s32)New_FrameClock();
    }
    if (arg1->unkC != NULL) {
        self->unk14 = (s32)arg1->unkC;
    } else {
        self->unk14 = (s32)New_LightRig();
    }
    if (arg1->unk10 != NULL) {
        self->unk18 = arg1->unk10;
    } else {
        self->unk18 = New_Viewport();
    }
    self->unkC = (Obj86B60UnkC *)arg1;
    obj18 = self->unk18;
    methods->slot10(self, arg1->unk0);
    methods->slot10(self, arg1->unk4);
    methods->slot10(self, (void *)self->unk10);
    methods->slot4C(self, 0, 0, 0);
    self->unk24 = arg2;
    if (arg2 == 0) {
        obj18->methods->slot10(obj18, arg1->unk0);
        obj18->methods->slot10(obj18, (void *)self->unk10);
        ((Unk14Obj *)self->unk14)->methods->slot10((Unk14Obj *)self->unk14, (void *)self->unk10);
        methods->slot60(self, 2);
        methods->slot48(self);
    }
}
```

## Residue chased: a missing 6th callee-saved register

First attempt built and linked (build exit=0) but scored 7/93 with a
178502-byte OUTSIDE-range diff -- textbook address drift, one word short.
`tools/asm-differ/diff.py IntermediateBase__Init` showed retail saving 5 registers
(`s0..s4`, frame `-0x28`) against my first cut's 4 (`s0..s3`, frame `-0x20`):
the classic "one fewer live-across-call value than retail" register
shortfall this project's learnings describe.

The missing value was `self->unk18`, read directly (as `self->unk18->
methods->slot10(...)`, twice) instead of through a local. Retail computes it
into a register (`lw $s2, 0x18($s0)`) once, right after `self->unkC = arg1`
in program order -- scheduled early enough to land ahead of the first
`addChild` call rather than at its first logical use, ordinary -O2
instruction scheduling once the value has to live in a register anyway.
Adding `Unk18Obj *obj18 = self->unk18;` at that point (matching retail's
instruction position, not the value's first *use*) closed it to 93/93 on
the next build.

## Struct/table knowledge established

- `Obj86B60InitArgs` (this function's 2nd parameter): `unk0`/`unk4` are
  child pointers forwarded to `addChild`; `unk8`/`unkC`/`unk10` are the
  three optional overrides for `self->unk10`/`unk14`/`unk18`.
- `Obj86B60.unk10`/`unk14`: BOTH were previously modelled as opaque
  generic words (`Viewport__RemoveAllChildren`/`TaskCore__BeginElementScroll` in a sibling unit). This
  function CONFIRMS both are pointer-valued in this unit's own reading too
  -- `unk10` is forwarded as an `addChild`-style child and as `Unk14Obj::
  slot10`'s 2nd arg; `unk14` is dispatched through as `Unk14Obj *`. Kept
  `s32` in the struct (the more general reading) per this header's
  established "keep the general field, cast locally" convention -- see
  `Unk14Obj`'s own header comment.
- `Obj86B60.unk18`: newly typed `Unk18Obj *` (was unobserved padding).
  Constructed either from `arg1->unk10` or from this unit's own New_X
  allocator `New_Viewport` (queued later this round).
- `Obj86B60.unk24`: new field, `s32`, set to `arg2`; also the gate for this
  function's second half.
- `Unk18Obj`/`Unk18ObjMethods`, `Unk14Obj`/`Unk14ObjMethods`: new minimal
  types, one dispatch slot (`slot10`) each, both OBSERVED only by this
  function.
- `Obj86B60Methods`: added `slot10` (inherited BasicClass `addChild`),
  `slot4C` (external `TaskCore__OnInit`), `slot48` (`IntermediateBase__Deinit`, next in
  this queue).

### Proposed learning

Add a THIRD instance to DECOMPILATION_LEARNINGS' "explicit intermediate
element pointer" family, but note it points the OPPOSITE direction from
that entry's usual lesson: that entry is about NOT caching (`self->field`
read fresh at each use avoids a spurious promotion). Here NOT caching was
the bug -- retail DOES cache `self->unk18` into a register across two later
calls, and my char-for-char-faithful "just dereference the field again"
reading under-counted retail's register pressure by one. **When a residue is
"one word short, everything after shifts" (address drift) AND the diff shows
retail using one MORE callee-saved register than your build, look for a
struct field dereferenced through the SAME expression at 2+ call sites later
in the function and hoist it into a local, even if nothing else in the
function's shape suggests caching was warranted.** This is the reciprocal
check to "do not cache a `this->field` across an intervening vtable call" --
sometimes retail's source did cache, and only the register count in the diff
tells you which.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`, `nor`/`addiu` pair):** does not apply --
  this function has no negation of any kind.
- **Lever 2 (N independently-incrementing walkers, dual-based-type
  pointers):** does not apply -- no array walk in this function at all,
  every access is a single struct dereference or a vtable call.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. 2 attempts (address
drift on the first, closed on the second by hoisting `self->unk18` into a
local at the position retail's own instruction schedule implied).

## Naming

**IntermediateBase__Init** (renamed from `func_8003E10C`, round 55, runner alpha).
Tier A: mechanics fully known and coherent -- takes an `Obj86B60InitArgs *`
and a mode flag, resolves three helper-object fields (`self->unk10`,
`self->unk14`, `self->viewport`) from the caller-supplied args or a default
helper, retains the args pointer itself (`self->initArgs = (Obj86B60UnkC *)arg1`
-- the field's own new name, see `## Proposed field names`), registers
children through the inherited `addChild` slot, and (on mode 0) performs
extra registration and calls `deinit` -- the standard "construct with
caller-overridable defaults" idiom this project uses elsewhere. Paired with
`IntermediateBase__Deinit` as the mirror-image teardown (see that report).

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__Init (class prefix). Occupies +0x044, slot `init(self, IntermediateBaseInitArgs *args, s32 mode)`, typed s32 because the callers use the result: Class6D3C8__RunPollTask returns `task->methods->slot44(task, extra, 0)` and Class6D3C8__PollStatusObj switches on it; the overrides Class86668__Init and TaskCore__Init call this base and return a field (eventCode, +0x038). This occupant itself returns nothing. Obj86B60InitArgs is IntermediateBaseInitArgs; its five fields and +0x010/+0x014/+0x018 are only ever added, removed or released through BasicClass slots here, so they are `BasicClass *` (casts dropped). +0x04C, called after the children are added, is `onInit` (NULL here; Class865C8__OnInit, ObjM__InitStyleAndWorld, TaskCore__OnInit). The name is kept, tier B: with mode 0 the body also runs setState(2) and deinit, so "Init" says less than it does.

## Track 4 (2026-09-26, round 86, delta)

Class 0x14 (was D_8006EFAC) unified as LightRig in `include/LightRig.h`; its NULL-args fallback is `self->unk14 = (BasicClass *)New_LightRig();`, the allocator's prototype now coming from include/LightRig.h (was `void *New_LightRig(void)` in include/code_2cc8c.h). `unk14` stays IntermediateBase's `BasicClass *`; a pointer cast emits no code; image byte-identical.

## Track 4 (2026-09-26, round 88, delta: FrameClock)

`New_D8006EF50` is now `New_FrameClock` (include/FrameClock.h), returning `FrameClock *`; the store into `unk10` (`BasicClass *`, field unchanged) upcasts it. Byte-identical.
