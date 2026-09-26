# New_CdDriver -- MATCHED (20/20 words)

> Renamed from `New_Class6D4E8` on 2026-09-26 (tools/rename.py). Address 0x800271d8.

> Renamed from `new_class_6d4e8` on 2026-09-25 (tools/rename.py). Address 0x800271d8.

Unit `code_179d8_o`, round 26 (2026-09-09). The unit's "new" function: allocate
an instance and dispatch to its constructor through the class's own method
table.

## Class-table finding (name is a hypothesis, resolution is not)

The FirecatFG name is drawn from `gCdDriverMethods`, this class's own 29-slot method
table. Per CLAUDE.md, that name is a hypothesis, not evidence -- resolved
instead with `tools/classtable.py 0x8006D4E8 --vs 0x8006B58C` (comparing
against `D_8006B58C`, the 14-slot table for the project's "BasicClass"
hierarchy, whose constructor `BasicClass__BasicClass` sits at `+0x008`).
`gCdDriverMethods` overrides exactly three slots relative to `D_8006B58C`
(`+0x004`, `+0x008`, `+0x00C` -- `FileResource__Release`, `CdDriver__CdDriver`,
`CdDriver__Finalize`), inherits `+0x010..+0x038` verbatim (identical
`BasicClass__func_*` addresses in both tables), and adds new slots from
`+0x040` up that `D_8006B58C` doesn't have at all (including this unit's own
`CdDriver__NoOpSlot40`). Since `+0x008` is confirmed as the constructor slot in the
BASE table, `New_CdDriver` allocating and then calling through that same
slot on ITS OWN table (which resolves to `CdDriver__CdDriver`, this unit's next
function) is a genuine "allocate + construct" pair -- confirmed by the table
lookup, not assumed from the name.

## What it is

```c
Obj6D4E8 *New_CdDriver(void)
{
    Obj6D4E8 *self;

    self = BMemPMgrAlloc(0x2C);
    if (self != NULL) {
        GetCdDriverMethods()->ctor(self);
        return self;
    }
    return NULL;
}
```

`BMemPMgrAlloc` is the project's already-established Psy-Q allocator
(`extern void *BMemPMgrAlloc(s32 size);`, same signature used throughout the
codebase). `GetCdDriverMethods` (still `INCLUDE_ASM` in the `code_179d8`
remainder) returns this class's own table, `&gCdDriverMethods`, typed here as
`Obj6D4E8Methods *` (a local view -- see the unit header comment and the
sibling reports for `CdDriver__CdDriver`/`CdDriver__Finalize`, which establish the
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

## Naming

Round 79 (delta).

- **`New_CdDriver`** (was FirecatFG's `new_class_6d4e8`) -- **tier A**.
  Body alone: `BMemPMgrAlloc(0x2C)`, and on success dispatches
  `GetCdDriverMethods()->ctor` (table +0x008, which classtable.py
  resolves to `CdDriver__CdDriver`) on the new block, returning it, or
  NULL. The inherited hypothesis is confirmed; only the spelling changes to
  the project's `New_Class` convention. No direct caller in `asm/` or `src/`
  (grep for the name and for `800271D8`), so the caller is not known; the
  name rests on the body.
- `0x2C` is now `CLASS6D4E8_SIZE`, the allocation size. The unit's
  `Class6D4E8` struct is a partial view and is NOT claimed to be complete,
  which is why this is a constant and not `sizeof`.
- Local types renamed to the tree's class prefix: `Obj6D4E8` ->
  `Class6D4E8`, `Obj6D4E8Methods` -> `Class6D4E8Methods` (the prefix every
  other method of this class already carries in code_179d8_q.c).


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all FileResource's (the driver runs on its clients' objects; FileResource's +0x018/+0x01C were named pos/size for it). Byte-identical. `New_Class6D4E8` -> `New_CdDriver` by rename.py.
