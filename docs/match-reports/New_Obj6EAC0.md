# New_Obj6EAC0 — MATCHED (31/31 words)

> Renamed from `func_800408CC` on 2026-09-18 (tools/rename.py). Address 0x800408cc.

Unit: `src/code_2cc8c_f.c`. 3 attempts.

```c
Obj6EAC0Methods *Obj6EAC0__GetDerivedMethods(void);

Unk64Elem *New_Obj6EAC0(void *ctx, s32 len, char *name) {
    Obj6EAC0 *self = BMemPMgrAlloc(0xB8);
    if (self != NULL) {
        Obj6EAC0__GetDerivedMethods()->slot08(self, (s32)ctx, len, (s32)name);
        return (Unk64Elem *)self;
    }
    return NULL;
}
```

A `New_X`-shaped allocator: `BMemPMgrAlloc` (the project's generic pool
allocator, already declared in this header) allocates 0xB8 bytes, then
on success dispatches through the DERIVED class table's constructor
slot (`Obj6EAC0__GetDerivedMethods()` returns `&D_8006EB90`, `slot08` is the ctor,
resolved as `Obj6EAC0__Construct` -- this unit's own hard queue member, still
`INCLUDE_ASM`; calling into it while unmatched is fine per the
established convention).

## A real cross-unit signature collision, corrected

`include/code_2cc8c.h` already declared
`extern Unk64Elem *New_Obj6EAC0(void *ctx, s32 len, char *name);`
from a DIFFERENT unit's caller-side guess (`src/code_2cc8c_b.c`, two
call sites: `New_Obj6EAC0(handle, len, *list)`). That guess is
actually CORRECT at the ABI level -- `handle`/`len`/`*list` forward
straight through to the constructor as raw register values, so typing
them `(void*, s32, char*)` instead of my first cut's generic
`(s32, s32, s32)` changes nothing about the compiled bytes, only the
C-level abstraction. My own first attempt used the generic types and
got `error: conflicting types for 'New_Obj6EAC0'` against that
existing declaration -- retyping MY definition to match the EXISTING
one (rather than editing the header) resolved it with zero header
churn. Both `code_2cc8c_b.c` call sites still compile and the full
`build-and-verify.sh` stays green.

**Flagging per the round's explicit ask:** this IS the round-13 hazard
("one runner declares a function typed while another matches it with a
different signature") almost recurring -- except caught immediately by
the compiler's own `conflicting types` error rather than silently
merging, because both declaration and definition now live in the same
build. No header edit was needed to resolve it, and the return type
`Unk64Elem *` / cast pattern here is worth reusing if `New_Obj6EAC0`
turns out to have further callers.

## The residue and its fix

First cut (single-exit `if(self){ctor} return self;`) scored 30/31:
retail's null-path materialises a literal `move $v0,$zero` in the
`beqz`'s own delay slot, where GCC naturally produces `move $v0,$s0`
instead (since `$s0` already holds `self`, which equals 0 on that exact
path -- a value-correct but differently-SPELLED "surplus mention"
residue, same family as the project's `new_class_6d3c8`/`strcat`
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
  form here (this shape has no loop, so the `Class6D3C8__PollGraphRoomStatus`-style
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
| `func_800408CC` | `New_Obj6EAC0` | B |

**Evidence.** The class's `New_X`-shaped public allocator (matches the
project's established `New_Class` constructor convention): allocates
`0xB8` bytes then dispatches the derived table's own `slot08`
(`Obj6EAC0__Construct`) with `(ctx, len, name)` forwarded as
`Construct`'s `(a1, a2, a3)`. Confirmed the `len`/`a2` argument becomes
`totalChildCount`/`childCount` (a genuine "how many children" parameter,
via `Construct`'s own body) and `name`/`a3` flows through
`Obj6EAC0__FinishConstruct` into `Obj6EAC0__SetText` on a derived
instance -- consistent with "construct an N-character text display and
immediately set its text", the unit's own header-comment hypothesis.
Tier B: the allocate-then-construct mechanics and the parameter-flow
chain are certain; the specific in-game role is this unit's own reading.
