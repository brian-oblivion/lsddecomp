# Actor__Actor -- MATCHED (28/28 words)

> Renamed from `BaseObjO__BaseObjO` on 2026-09-25 (tools/rename.py). Address 0x80057044.

> Renamed from `func_80057044` on 2026-09-18 (tools/rename.py). Address 0x80057044.

Unit: `class_3bb8c_o` (round 17). The constructor of the shared
intermediate base class this unit implements from here onward -- chains to
the base-class ctor via the fixed `GetSceneNodeMethods()` table, installs this
class's own vtable, zeroes three fields and dispatches its own `slot40`.

## Final source

```c
BaseObjO *Actor__Actor(BaseObjO *self) {
    if (GetSceneNodeMethods()->ctor(self) == NULL) {
        goto fail;
    }
    self->methods = GetActorMethods();
    self->unk44 = 0;
    self->unk4C = NULL;
    self->unk50 = NULL;
    self->methods->slot40(self);
    return self;
fail:
    return NULL;
}
```

## Class identification (this file's central finding)

This function IS `D800878D4Methods::ctor` (`code_55dd4.h`, Class65650's own
reading of the shared base) and simultaneously IS `vtable_DreamSys`'s
implicit base-construction step -- confirmed three ways:

1. `GetActorMethods()` (Class65650's own getter, already declared in
   `code_55dd4.h` returning `D800878D4Methods *`) is called here to
   install `self->methods`, i.e. this function is establishing the exact
   table `GetActorMethods` returns.
2. The functions this unit defines right after this one --
   `Actor__AddChild`/`Actor__RemoveChild`/`Actor__RemoveAllChildren` -- occupy exactly
   `D800878D4Methods`'s `+0x010`/`+0x014`/`+0x018` slots, and
   `Actor__AddChild`/`Actor__RemoveChild` are independently named at
   `vtable_DreamSys`'s `+0x010`/`+0x014` in `DreamSys.h` too (as
   "the SAME shared base class... slot10/slot14, the link/unlink pair").
3. `GetActorMethods()->ctor(self)` returning NULL on failure and `self` on
   success matches this function's OWN return convention exactly (a
   self-consistent constructor chain).

Neither sibling header is this unit's to edit (see CLAUDE.md's header
discipline and the file banner), so this unit keeps its own local reading,
`BaseObjO`/`BaseObjOMethods`, named distinctly from both.

## Derivation

- **`GetSceneNodeMethods()` needed a NON-VOID `ctor`, forcing a fresh local
  table type rather than reusing `class_3bb8c.h`'s existing
  `BaseCtorTableB_3bb8c_c`.** That header's own `ctor` field is typed
  `void (*ctor)(void *self)` (established from a DIFFERENT unit's call site
  that discards the return), but THIS call site checks
  `GetSceneNodeMethods()->ctor(self) == NULL` -- an outright arity/return-type
  conflict at the same slot, same shape as the project's other
  `GetSceneNodeMethods` per-call-site-typing precedent already written up in
  that very header (see its own long comment on why the arg list "is what
  THIS call site's bytes need", not the callee's true signature). Declared
  a fresh local `FixedBaseTable` here instead of touching the shared
  header. `code_d294.h`'s OWN independent view (`SceneNodeMethods::ctor`,
  `void *(*ctor)(void *self)`) already needed the same non-void return for
  the same reason, confirming this is not a one-off.
- **`goto fail; ... fail: return NULL;`, not `if (cond) return NULL;`.**
  Per the documented "early exit returning a DIFFERENT value" lever: with a
  plain `if`/`return`, retail would need an explicit `j` to reach the
  shared epilogue (the return-NULL path and the return-self path would be
  laid out separately); with `goto`, the exit value is stolen into the
  branch's own delay slot and both paths converge on ONE epilogue, costing
  one fewer word. Confirmed by direct A/B: `if`/`return` scored 27/28 with
  exactly one spurious `j`/extra-`nop` pair; `goto` closed it to 28/28.
- **The reload of `self->methods` after the store** (`sw v0,0(s0)` then
  `lw v0,0(s0)` before the `slot40` dispatch) falls out naturally from
  writing `self->methods->slot40(self)` as an ordinary field access after
  `self->methods = GetActorMethods();` -- no caching needed, no barrier
  needed.

### Proposed learning

- **`GetSceneNodeMethods`'s per-call-site-typing precedent is not unique to
  `class_3bb8c.h`/`code_d294.h`/`class_3ac78.c`.** A fourth, independent
  local reading (`FixedBaseTable` here) needed the SAME non-void-vs-void
  fork on the SAME slot (`ctor`, offset `+0x008`) for the identical reason
  (this call site's return value is checked). Worth noting because it
  shows the fork is a structural property of the symbol (many real
  occupants across many classes share this one dispatch stub, and at least
  two of them use their return value while at least one doesn't), not an
  artifact of one unit's local confusion.

## Naming

**`Actor__Actor` -- tier A.** The raw `Class__Class` constructor form:
takes an already-allocated `self`, chains to the true root
(`GetSceneNodeMethods()->ctor`), installs `self->methods`, zeroes three fields
and dispatches `slot40`. The report's own "Class identification" section
establishes THREE independent ways that this is the shared intermediate
base's own constructor, not `Class65650`'s: (1) it installs the exact
table `GetActorMethods()` returns, (2) the functions defined right after it
in this unit occupy that same table's `+0x010`/`+0x014`/`+0x018` slots and
are independently named at the identical offsets in two sibling headers'
own local views (`DreamSys.h`, `code_55dd4.h`), (3) `code_55dd4.c`'s real
`Class65650` constructor (`Class65650__Class65650`) calls THIS function
through `base->ctor(self)` to chain to it first, then immediately
overwrites `self->methods` with `Class65650`'s own, more specific table --
i.e. `Class65650` derives from this class, it is not this class.

## Track 4 (2026-09-25, round 82, delta)

Renamed from `BaseObjO__BaseObjO`. Occupant of +0x008 in gActorMethods: the ctor, named for its slot. Chains SceneNode's ctor, zeroes state/grid/ticker, calls reset. The class (id 0x34, table `gActorMethods`, formerly `D_800878D4`) is unified as `Actor` in `include/Actor.h`: a SceneNode subclass and the base of Class65650/Entity, DreamSys and Class876FC. Any source block above is the pre-unification spelling; the live body in `src/class_3bb8c_o.c` takes the unified types and field/slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
