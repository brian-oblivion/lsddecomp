# GridCell__GridCell

> Renamed from `Class86AA0__Class86AA0` on 2026-09-26 (tools/rename.py). Address 0x8004d3dc.

> Renamed from `func_8004D3DC` on 2026-09-22 (tools/rename.py). Address 0x8004d3dc.

**Unit:** title_menu · **Size:** 20 words · **Status:** MATCHED (20/20)

## What it does

The constructor (`ctor`, slot +0x008) for `GridCell`. Chains to a base
ctor (fetched via `GetSceneNodeMethods(self)`), installs this class's own vtable,
then zeroes three of its own fields (`unk34` u16, `unk36` u16, `unk38`
s32) directly -- unlike NodeGuardedViewport__NodeGuardedViewport's sibling ctor, there is no
post-construct hook call here (the retail instruction stream ends right
after the zero-stores).

## The C

```c
void GridCell__GridCell(GridCell *self)
{
    GetSceneNodeMethods(self)->ctor(self);
    self->methods = GetGridCellMethods();
    self->unk34 = 0;
    self->unk36 = 0;
    self->unk38 = 0;
}
```

## Notes on GetSceneNodeMethods's declared arity

`GetSceneNodeMethods` is already declared elsewhere in the codebase
(`include/dream_day.h`) with a two-argument signature,
`void *GetSceneNodeMethods(StageMap *self, s32 arg1)`. This unit's own call
site never sets up a second argument register (`$a1`) before the `jal` --
the instruction immediately after is a plain `lw` on the return value, not
an `addu $a1, ...` -- so it is declared here, file-locally, as single-
argument: `extern BaseCtorTable_3bb8c_c *GetSceneNodeMethods(void *self);`. This
is safe: each translation unit gets its own extern prototype for a given
external symbol in this project (no shared declaration is enforced across
units), and the only thing that has to be right for THIS unit's codegen to
match is what THIS call site's own register usage requires.

## Proposed learning

> **Head correction, round 9.** The advice below is right about codegen and
> wrong about the function, and the difference matters. Corrected version
> first; the runner's original is kept under it because its practical half is
> what produced the match.

**Match your own call site — but a disagreement between two units'
declarations of one symbol means nobody has established the real signature
yet, and settling it is one `cat` away: read the CALLEE.**

Here that read settles it flatly. `GetSceneNodeMethods`'s entire body
(`asm/SceneNode.s`) is:

```
lui   $v0, %hi(gSceneNodeMethods)
addiu $v0, $v0, %lo(gSceneNodeMethods)
jr    $ra
 nop
```

It reads **neither `$a0` nor `$a1`**. It takes **no arguments** and returns
`&gSceneNodeMethods` — the plain no-parameter vtable getter already documented in
`docs/research/class-framework.md`, the same shape as `GetGameApplicationMethods`. So the
2-argument declaration in `src/world/dream_day.c` and the 1-argument declaration
in `include/class_3bb8c.h` are **both wrong about the function**, and both are
**right about their own call site**, and both units are byte-exact.

**That is the actual finding, and it is more interesting than an arity
mismatch:** retail's own source called one zero-argument getter with two
arguments from one file and one argument from another. That is what C89 does
when no prototype is in scope — the call passes whatever is written and
nothing checks it — so this is direct evidence about how the original was
organised, not an inconsistency to tidy up.

**Consequences, now annotated at both declaration sites so nobody undoes
them:**

- Do NOT reconcile the two declarations, and do NOT reduce either to `(void)`.
  The declared arg list is what makes the caller emit its argument setup;
  changing it changes the bytes and breaks the match.
- When you meet a helper like this, read the callee before writing a
  signature. If it ignores its arguments, expect the call sites to disagree
  and expect each to need its own local declaration.

### The runner's original wording (superseded)

> When a project-wide helper (a base-ctor getter, an allocator, etc.) is
> called with a smaller argument list at one site than another unit already
> declares for it, trust the instruction stream at YOUR call site over the
> other unit's declaration -- match arity to observed register setup, not to
> consistency with a sibling file's extern prototype for the same symbol.

The practical instruction is sound. What it left out is that a divergence is a
signal the real signature is unknown, and that the callee settles it cheaply —
without which two wrong declarations sit in the tree looking like a resolved
question.

## Naming

**GridCell__GridCell** -- tier A. Canonical ctor (`Class__Class`
convention): chains a base ctor (`GetSceneNodeMethods`), installs this
class's own vtable, zeroes three of its own fields. Same shape and
evidence class as `NodeGuardedViewport__NodeGuardedViewport`.

## Track 4 (2026-09-26, round 88, alpha)

GridCell is unified in `include/grid_cell.h` and expands
SCENENODE_FIELDS: the three fields this ctor zeroes are SceneNode's, so
`unk36` -> `flags36` and `unk38` (s32) -> `nextInCell` (void *, written as
`NULL`; StageMap__DispatchToRectCells walks it as a pointer). The store is
`sw $zero` either way; image byte-identical. GridCell has no own fields:
New_GridCell allocates 0x3C bytes, shorter than SceneNode's 0x44.

## Track 6 (2026-09-26, round 92, bravo): the class is named GridCell

`python3 tools/renametype.py Class86AA0 GridCell` (the class, its table
`gClass86AA0Methods` -> `gGridCellMethods`, the getter, `New_`, the five
methods, and the `onClass86AA0LinkCommand` slot/methods of Actor and Entity,
which are the handlers for a link command from this class). **Tier B.**

Evidence, all from the code:

- the only `New_GridCell` call sites are StageMap__StageMap's: one
  per element as `cellParent`, and 0x668 / 4 = 410 per element as `cells`,
  attached to the cellParent on a 0x800-unit lattice (row stride 20, the
  grid's `gridCells`); StageMap__Finalize releases them;
- StageMap__PopulateSlotCells fills each cell from the element's
  placement records (TMD linked with GsLinkObject4, coord2 translation and
  y rotation, `flags36`), and chains the overflow cells (index 400 on) off
  a lattice cell through `nextInCell`;
- the two cell walks, StageMap__DispatchToRectCells (NotifyGridCell) and
  Actor__ScanGridWindow, visit a cell and its `nextInCell` chain.

Tier B, not A: the element's `cellParent` is also a GridCell (the root of
its cells, never given a model), and what a cell is in the game (a map
tile, presumably) is not shown by the code. The earlier banner kept the
table-address name because the link dispatch alone says how it links, not
what it is; the construction and fill sites above are what name it.
