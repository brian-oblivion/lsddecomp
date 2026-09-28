# New_TextRow — MATCHED (31/31 words)

> Renamed from `New_Obj6EAC0` on 2026-09-26 (tools/rename.py). Address 0x800408cc.

> Renamed from `func_800408CC` on 2026-09-18 (tools/rename.py). Address 0x800408cc.

Unit: `src/ScreenWidgets.c`. 3 attempts.

```c
Obj6EAC0Methods *GetTextRowMethods(void);

Unk64Elem *New_TextRow(void *ctx, s32 len, char *name) {
    Obj6EAC0 *self = BMemPMgrAlloc(0xB8);
    if (self != NULL) {
        GetTextRowMethods()->slot08(self, (s32)ctx, len, (s32)name);
        return (Unk64Elem *)self;
    }
    return NULL;
}
```

A `New_X`-shaped allocator: `BMemPMgrAlloc` (the project's generic pool
allocator, already declared in this header) allocates 0xB8 bytes, then
on success dispatches through the DERIVED class table's constructor
slot (`GetTextRowMethods()` returns `&gTextRowMethods`, `slot08` is the ctor,
resolved as `TextRow__TextRow` -- this unit's own hard queue member, still
`INCLUDE_ASM`; calling into it while unmatched is fine per the
established convention).

## A real cross-unit signature collision, corrected

`include/Task.h` already declared
`extern Unk64Elem *New_TextRow(void *ctx, s32 len, char *name);`
from a DIFFERENT unit's caller-side guess (`src/app/Task.c`, two
call sites: `New_TextRow(handle, len, *list)`). That guess is
actually CORRECT at the ABI level -- `handle`/`len`/`*list` forward
straight through to the constructor as raw register values, so typing
them `(void*, s32, char*)` instead of my first cut's generic
`(s32, s32, s32)` changes nothing about the compiled bytes, only the
C-level abstraction. My own first attempt used the generic types and
got `error: conflicting types for 'New_TextRow'` against that
existing declaration -- retyping MY definition to match the EXISTING
one (rather than editing the header) resolved it with zero header
churn. Both `Task.c` call sites still compile and the full
`build-and-verify.sh` stays green.

**Flagging per the round's explicit ask:** this IS the round-13 hazard
("one runner declares a function typed while another matches it with a
different signature") almost recurring -- except caught immediately by
the compiler's own `conflicting types` error rather than silently
merging, because both declaration and definition now live in the same
build. No header edit was needed to resolve it, and the return type
`Unk64Elem *` / cast pattern here is worth reusing if `New_TextRow`
turns out to have further callers.

## The residue and its fix

First cut (single-exit `if(self){ctor} return self;`) scored 30/31:
retail's null-path materialises a literal `move $v0,$zero` in the
`beqz`'s own delay slot, where GCC naturally produces `move $v0,$s0`
instead (since `$s0` already holds `self`, which equals 0 on that exact
path -- a value-correct but differently-SPELLED "surplus mention"
residue, same family as the project's `New_GameApplication`/`strcat`
one-word class). Tried and rejected:

- Restructuring as a genuine early exit
  (`if(self==NULL) return NULL;` before the ctor call) -- this FORCES a
  second real epilogue and costs a whole word (32 vs 31), the
  documented `New_X` "epilogue-merge" stall shape
  (`docs/research/epilogue-merge-residue.md`).
- A separate `result` local, defaulted to `NULL` before the `if` and
  conditionally overwritten -- this keeps ONE epilogue but forces
  `result` to live across the constructor CALL, promoting it to a 4th
  callee-saved register retail does not use (retail's frame only saves
  `$s0`-`$s3`).
- **What worked:** `return (Unk64Elem*)self;` INSIDE the `if`, and
  `return NULL;` AFTER it (not before) -- one shared epilogue, and the
  compiler is free to materialise the null-path's return value as a
  plain `$zero` move directly in the delay slot rather than routing it
  through `self`'s register at all, since the two `return` statements
  are now two independent expressions rather than one shared variable.
  Matches CLAUDE.md/MATCHING-GUIDE's "early exit returning a DIFFERENT
  value: try goto and return both" family, landing on the `return`
  form here (this shape has no loop, so the `GameApplication__RunTitleMenu`-style
  disqualifier for that lever doesn't apply).

### Proposed learning

For the New_X "return-regardless"-looking shape that actually DOES test
the allocation (just not the ctor's return), `return X;` inside the
guard and a SEPARATE `return NULL;` after it (not a shared-variable
default, not a pre-guard early exit) is the form that keeps one
epilogue AND lets the null path use a plain zero-register move. Worth
trying before reaching for the heavier "default value, conditionally
overwritten" idiom whenever the two return values are constants/simple
casts rather than expressions requiring real computation.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800408CC` | `New_TextRow` | B |

**Evidence.** The class's `New_X`-shaped public allocator (matches the
project's established `New_Class` constructor convention): allocates
`0xB8` bytes then dispatches the derived table's own `slot08`
(`TextRow__TextRow`) with `(ctx, len, name)` forwarded as
`Construct`'s `(a1, a2, a3)`. Confirmed the `len`/`a2` argument becomes
`totalChildCount`/`childCount` (a genuine "how many children" parameter,
via `Construct`'s own body) and `name`/`a3` flows through
`TextRow__Reset` into `TextRow__SetText` on a derived
instance -- consistent with "construct an N-character text display and
immediately set its text", the unit's own header-comment hypothesis.
Tier B: the allocate-then-construct mechanics and the parameter-flow
chain are certain; the specific in-game role is this unit's own reading.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `New_Obj6EAC0`: the allocator (0xB8 bytes, the object size), now `TextRow *New_TextRow(void *texture, s32 count, char *text)` calling ctor through GetTextRowMethods(). Its callers make one per string: TaskCore__CreateSlotElements and the item lists (Task, stored as TextRow in TaskCore's slotElements/itemLists, which were the `Unk64Elem` view), ItemList__CreateRows, TitleMenu__CreateSaveTitle, TextEntry__LoadCardResources, ObjM__AdvancePauseSetup ("Pause"). Image byte-identical; the current source is src/ScreenWidgets.c.
