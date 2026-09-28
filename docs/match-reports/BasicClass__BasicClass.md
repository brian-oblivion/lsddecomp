# BasicClass__BasicClass

**Unit:** BMemPMgr · **Size:** 13 instructions · **Status:** MATCHED (13/13 words)

## The BasicClass design (established this round; full detail in `include/code_8220.h`'s doc comment)

`BasicClass` is the game's hand-rolled base class, root of the class
framework (`docs/research/class-framework.md`). Vtable is `gBasicClassMethods`
(`BASICCLASS_METHODS`), 14 slots, resolved with `tools/classtable.py
gBasicClassMethods` — matches this unit's carve exactly: 13 of the 14 occupant
functions are named `BasicClass__func_*`/`BasicClass__BasicClass` in this
carve's own 20 functions (the 3 highest slots, `+0x030`/`+0x034`/`+0x038`,
land in `code_8220_b`, next round's carve).

It maintains two singly-linked lists of pool-allocated 8-byte nodes
(`BasicClassListNode { next; BasicClass *value; }`, size confirmed by
`PushBasicClassListNode`'s `BMemPMgrAlloc(0x8)` allocation):

- `children` (`+0x004`): objects added via `addChild`/`removeChild`
  (`BasicClass__AddChild`/`BasicClass__RemoveChild`, own reports). Adding
  one ALSO registers `self` in the child's own `parentRefs` list, by
  dispatching through the CHILD's own vtable slots `+0x020`/`+0x024` — the
  same slots this class's own `addParentRef`/`removeParentRef` occupy — so
  the relationship is bidirectional and works for any BasicClass-family
  object playing "child."
- `parentRefs` (`+0x008`): the back-reference list just described, plain
  push/remove/clear with no notification callback (unlike `children`).

## What this function does

The base constructor, dispatched through the class framework's own slot
`+0x008` convention (docs/research/class-framework.md: "constructors are
called through the method table, base-class constructors included").
Fetches the class's own vtable via `Get_vtable_BasicClass` (a tiny getter, still
`asm/code_8220_b.s`, that just returns `&gBasicClassMethods`), stores it at
`self->methods`, and zeroes both lists.

## The C

```c
void BasicClass__BasicClass(BasicClass *self)
{
    self->methods = Get_vtable_BasicClass();
    self->parentRefs = NULL;
    self->children = NULL;
}
```

## One residue, from collateral drift — not a real defect in this function

First measured at 12/13 words, with the sole difference being the `jal`
TARGET immediate for `Get_vtable_BasicClass` (a call, not a branch — the encoded
absolute address itself differed). This is not a scheduling or codegen
issue in this function at all: `Get_vtable_BasicClass` lives in the still-uncarved
`code_8220_b.s` tail, so its real link address depends on the total size of
everything before it, including `BasicClass__RemoveAllChildren` (this unit's
hardest function, worked on afterward — see its own report). Once
`BasicClass__RemoveAllChildren` reached its correct byte-exact size, this
function's `jal` target resolved correctly with no changes here at all —
confirms DECOMPILATION_LEARNINGS' point that "differs OUTSIDE this range"
warnings mean the CALLING function isn't necessarily the one at fault.

## Field-order note: writes happen in REVERSE offset order

`self->parentRefs` (`+0x008`) is zeroed BEFORE `self->children` (`+0x004`)
in both retail and this C, even though `children` comes first in the
struct and reads more naturally first. Transcribed as observed; not
reordered, since this project's C89 requires an explicit statement order
match anyway and the order shown compiled correctly on the first attempt.

## Provenance

round 11 (2026-09-03), runner delta, unit code_8220 (fresh carve).

## History (moved from include/BasicClass.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * HOW A CLASS IS DECLARED (FINISHING-PLAN track 4; this header is the model).
```
