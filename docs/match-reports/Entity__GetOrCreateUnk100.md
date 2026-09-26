# Entity__GetOrCreateUnk100

> Renamed from `func_8005D108` on 2026-09-19 (tools/rename.py). Address 0x8005d108.

**Unit:** Entity · **Size:** 57 words · **Status:** MATCHED (57/57 words, whole-image build verified byte-exact)

## What it does

Lazily creates-or-reuses `this->unk100` (a `Unk100Obj *`, a class instance
of an entirely different, unidentified type that lives behind its own
vtable — see `Unk100Methods` in `Entity.h`), then dispatches three calls
through it: `slot50(sub)`, `slot4C(sub, this, arg2-or-default)`,
`slotD0(sub, arg3)`. Returns the object (new or cached), or NULL if
allocation failed.

`name` defaults to `gEntityDefaultPos` (`{320, 240}`) and `arg2` defaults to
`gEntityDefaultOffset` (`{-100, -100}`) when the caller passes NULL — both plain
2-word data buffers, not strings, in `asm/data/7B3F8.sdata.s`.

## Derivation

```
sub = this->unk100 (cached) OR New_Class6E99C(name-or-default, 0, arg4) (fresh)
if (fresh alloc failed) return NULL
if (fresh) this->unk100 = sub
sub->methods->slot50(sub)
sub->methods->slot4C(sub, this, arg2-or-default)
sub->methods->slotD0(sub, arg3)
return sub
```

## Final C

```c
Unk100Obj *Entity__GetOrCreateUnk100(Entity *this, void *name, void *arg2, void *arg3, s32 arg4) {
    Unk100Obj *cached;
    Unk100Obj *sub;
    Unk100Methods *m;
    void *dispatchArg2;

    cached = this->unk100;
    if (cached == NULL) {
        if (name == NULL) {
            name = gEntityDefaultPos;
        }
        sub = New_Class6E99C(name, 0, arg4);
        if (sub == NULL) {
            return NULL;
        }
        this->unk100 = sub;
    } else {
        sub = cached;
    }
    sub->methods->slot50(sub);
    m = sub->methods;
    dispatchArg2 = arg2;
    if (dispatchArg2 == NULL) {
        dispatchArg2 = gEntityDefaultOffset;
    }
    m->slot4C(sub, this, dispatchArg2);
    sub->methods->slotD0(sub, arg3);
    return sub;
}
```

## Attempt log

This one took several structural iterations, all converging on the same
lesson: **write the C the way the actual control-flow graph shapes it, not
the way the "obviously equivalent" nested-if reads.**

1. `sub = this->unk100; if (sub == NULL) { ...alloc...; this->unk100 = sub; }
   sub->methods->slot50(sub); ...` — matched retail's total size but the
   cached/fresh split was wrong: retail tests the freshly-loaded `$v0`
   (`this->unk100`) directly with `bnez $v0,ALREADY_HAD` and only spills
   into `$s0` inside the branch TARGET (the "already had a value" arm);
   the freshly-allocated path computes straight into `$s0` and reaches the
   shared code via an explicit `j`, skipping the `move $s0,$v0` the other
   arm needs. A single reused `sub` variable made GCC spill into `$s0`
   immediately and unconditionally instead.
2. Introducing a SEPARATE `cached` variable for the initial load (`cached =
   this->unk100; if (cached == NULL) { ... sub = alloc(); ... } else { sub =
   cached; }`) reproduced retail's split exactly — this is the literal
   translation of what m2c's raw output already showed (`temp_v0` vs.
   `var_s0`, with an explicit `goto block_6`), which is worth trusting more
   directly next time rather than "cleaning it up" first.
3. The dispatch tail still needed two more fixes after that: `arg2`'s
   NULL-default was written by reassigning the *parameter itself*
   (`if (arg2 == NULL) arg2 = gEntityDefaultOffset;`), and that block-reused `$s2`
   for both the check and the final value — but retail computes the checked
   value into `$a2` (a temp, distinct from the parameter's home register)
   and leaves `$s2` alone. Introducing a separate `dispatchArg2` local fixed
   the register split, but not the *content*: retail also loads
   `sub->methods` exactly ONCE (right after the `slot50` call, before the
   `arg2` branch) and reuses that value for the `slot4C` dispatch on BOTH
   arms of the branch, whereas the straightforward `sub->methods->slot4C(...)`
   spelling re-derefs `sub->methods` fresh after the branch. Caching that one
   load into a named `Unk100Methods *m` local (assigned right after
   `slot50`, used for the `slot4C` call) got GCC to hoist the load exactly
   where retail has it. `slotD0`'s own `sub->methods->...` reload, right
   after the `slot4C` call (which clobbers `$v0`), was NOT cached — that one
   stayed a fresh dereference in both retail and this code, and matched
   without help.

## Proposed learning

Two generalizable points from this one:

- When a value can come from **two different computations that converge**
  (a cached field vs. a freshly-computed one), model that with **two
  separate C locals** (one per source) rather than one variable that gets
  conditionally (re)assigned in place — even though they're semantically
  identical, GCC 2.6.3 at `-O2` allocates registers differently depending on
  whether the "already have it" case is a *fresh load tested directly* or a
  *reused variable*.
- A method-table pointer (`obj->methods`) read once and used for MULTIPLE
  dispatches across a *branch but not a call* is often hoisted by retail
  into its own temp and reused; a table pointer used again *after an
  intervening call* is reloaded fresh. When a residue is "the same load
  appears twice, once where it should be shared", try caching the vtable
  pointer into a named local at the point retail computes it once, but only
  before the FIRST subsequent call — this project doesn't yet have a name for
  this pattern in `docs/MATCHING-GUIDE.md`'s residue list and it's probably
  worth adding it.

## Naming

**Tier B.** Renamed from `func_8005D108` this round (tools/rename.py). The
MECHANIC is fully established by the body and by its cross-unit callers
(`Entity__MoodCue57`/`Entity__MoodCue85` in Entity_d/Entity_f, per this report and
`Entity.h`): lazily get-or-create `this->unk100`, then dispatch three init
calls through its own vtable, defaulting `name`/`arg2` to two 2-word screen-
coordinate-shaped buffers when NULL. The PURPOSE of the cached `Unk100Obj` is
not established -- every known call site passes `name`/`arg2` as NULL, so
those defaults are never actually exercised in the corpus read so far. Named
after the mechanic (get-or-create) and the field it operates on (`unk100`),
not after a guess about what the object represents.

## Track 4 (2026-09-26, round 87, echo)

`Entity::unk100` is now typed `Class6E99C *` (include/Class6E99C.h); the
local `Unk100Obj`/`Unk100Methods` view and Entity.h's own
`extern Unk100Obj *New_Class6E99C` are deleted. This function's four slot
calls resolve through D_8006E99C (`tools/classtable.py D_8006E99C`) and now
use the unified names: slot50 -> `detachFromParent`
(Class6B5CC__DetachFromParent), slot4C -> `attachToParent`
(BoxFill__AttachToParent, `this` upcast to `Class6B5CC *`), slotD0 ->
`setStep` (Class6E99C__SetStep; `arg3` passed `(s32)`, no code). What the
arguments are, from the occupants: `name` is New_Class6E99C's SIZE (read as
two low halfwords into boxW/boxH; the default gEntityDefaultPos is
{320, 240}), `arg2` is the attach position (BoxFill__AttachToParent's
Pair32E99C; default {-100, -100}, the relative top-left, the same pair
Viewport passes, D_8008A904) and `arg3` the fade step. The parameter names
and the function's own name are Entity's (not renamed here; proposed in
echo's round-87 summary). Image byte-identical.

## Track 4 (2026-09-26, round 88, echo)

The class (id 0x1F234, table `gEntityMethods`) is unified as `Entity` in `include/Entity.h`: a Class65650 subclass whose table and object expand `CLASS65650_SLOTS`/`CLASS65650_FIELDS`. Any source block above is the pre-unification spelling; the live body takes the inherited names (fields `parent`, `coord2`->`tx/ty/tz`, `tick`, `linkTarget`, `state` (was `moodState`), `lastOffsetValue`, `grid` (was `unk4C`), `ticker` (was `companion2`), `arg2` (was `soundCueChannel`), `parts`, `todPlaying`, `peer` (was `target`, cast to the `Unk94Obj` DreamSys view where its own slots are called); slots `reset`, `setDisplay`, `setLightMode`, `setTranslation`/`addTranslation`, `moveLocalZ/X/Y`, `moveLocalZOrFindLink`, `selectTickCallback`, `enableTickCallback`/`disableTickCallback`, `distanceToPeer`, `setTargetReached`, `updateActivationState`/`updateDeactivationState`), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
