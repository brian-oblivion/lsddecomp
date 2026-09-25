# New_Class6D4E8 -- MATCHED (20/20 words)

> Renamed from `new_class_6d4e8` on 2026-09-25 (tools/rename.py). Address 0x800271d8.

Unit `code_179d8_o`, round 26 (2026-09-09). The unit's "new" function: allocate
an instance and dispatch to its constructor through the class's own method
table.

## Class-table finding (name is a hypothesis, resolution is not)

The FirecatFG name is drawn from `D_8006D4E8`, this class's own 29-slot method
table. Per CLAUDE.md, that name is a hypothesis, not evidence -- resolved
instead with `tools/classtable.py 0x8006D4E8 --vs 0x8006B58C` (comparing
against `D_8006B58C`, the 14-slot table for the project's "BasicClass"
hierarchy, whose constructor `BasicClass__BasicClass` sits at `+0x008`).
`D_8006D4E8` overrides exactly three slots relative to `D_8006B58C`
(`+0x004`, `+0x008`, `+0x00C` -- `DestroyChained`, `Class6D4E8__Class6D4E8`,
`Class6D4E8__Destroy`), inherits `+0x010..+0x038` verbatim (identical
`BasicClass__func_*` addresses in both tables), and adds new slots from
`+0x040` up that `D_8006B58C` doesn't have at all (including this unit's own
`Class6D4E8__NoOpSlot40`). Since `+0x008` is confirmed as the constructor slot in the
BASE table, `New_Class6D4E8` allocating and then calling through that same
slot on ITS OWN table (which resolves to `Class6D4E8__Class6D4E8`, this unit's next
function) is a genuine "allocate + construct" pair -- confirmed by the table
lookup, not assumed from the name.

## What it is

```c
Obj6D4E8 *New_Class6D4E8(void)
{
    Obj6D4E8 *self;

    self = BMemPMgrAlloc(0x2C);
    if (self != NULL) {
        GetClass6D4E8Methods()->ctor(self);
        return self;
    }
    return NULL;
}
```

`BMemPMgrAlloc` is the project's already-established Psy-Q allocator
(`extern void *BMemPMgrAlloc(s32 size);`, same signature used throughout the
codebase). `GetClass6D4E8Methods` (still `INCLUDE_ASM` in the `code_179d8`
remainder) returns this class's own table, `&D_8006D4E8`, typed here as
`Obj6D4E8Methods *` (a local view -- see the unit header comment and the
sibling reports for `Class6D4E8__Class6D4E8`/`Class6D4E8__Destroy`, which establish the
struct's other slots).

## A residue worth recording: return-statement PLACEMENT, not phrasing

The first attempt used the more compact `if (self != NULL) { ctor(self); }
return self;` (one trailing return). It compiled to the WRONG VALUE in one
delay slot (`move $v0, s0` where retail has `move $v0, zero` in the `beqz`'s
delay slot -- same instruction count, 19/20 words matched, but one operand
differed) since both are semantically `0` when the branch is taken, but
retail's compiled shape prefers materializing the literal there.

Restructuring to return `self` INSIDE the `if` and `NULL` as the trailing
statement, i.e. **failure trailing, success inside the guard** (the body
above), reached byte-exact on the same attempt count. A different ordering
tried first -- `if (self == NULL) { return NULL; } ...; return self;`
(failure FIRST, success trailing) -- regressed badly (22/20, two extra words:
a `j` plus a duplicated partial epilogue), because GCC's cross-jump pass did
not merge that arrangement's two return points into retail's single shared
epilogue. Both variants have two `return` statements; only the one matching
retail's PLACEMENT (which one is written first) merges correctly. Consistent
with the project's round-24 finding that a shared return block's placement
follows the FIRST return in source order -- here more specifically, WHICH
return is first also decides whether the merge happens at all for this tiny
a function.

### Proposed learning

- For a two-exit function this small, if the naive single-`return`-at-bottom
  form gets the right LENGTH but a wrong delay-slot-fill VALUE, try swapping
  which of the two logical outcomes is written as the trailing statement
  before concluding the residue is unfixable -- cheap to test on a 20-word
  function, and it closed this one on the very next attempt.
