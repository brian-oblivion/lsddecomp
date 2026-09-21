# Class6D430__Class6D430

> Renamed from `func_80026A50` on 2026-09-18 (tools/rename.py). Address 0x80026a50.

**Unit:** code_171e0 · **Size:** 25 instructions · **Status:** MATCHED (25/25 words, whole-image build verified byte-exact)

## What it does

The constructor for the `D_8006D430` class (its own vtable slot `+0x008`,
per `include/code_171e0.h`'s `Class6D430Methods`). Chains the base
class's constructor first (`Get_vtable_BasicClass()->ctor(this)`), then installs
this class's own vtable pointer (fetched via the already-matched
`GetClass6D430Methods`, which just returns `&D_8006D430`), then zeroes every field
this unit currently knows about.

## Derivation

```
jal   Get_vtable_BasicClass
 addu $s0, $a0, $zero        ; s0 = this
lw    $v0, 0x8($v0)          ; v0 = (base table)->ctor
jalr  $v0                    ; Get_vtable_BasicClass()->ctor(this)
jal   GetClass6D430Methods          ; v0 = &D_8006D430
sw    $v0, 0x0($s0)          ; this->methods = v0
sw    $zero, 0xC($s0)        ; this->unk0C = 0
sw    $zero, 0x10($s0)       ; this->unk10 = 0
sw    $zero, 0x14($s0)       ; this->unk14 = 0
sh    $zero, 0x20($s0)       ; this->unk20 = 0
sh    $zero, 0x22($s0)       ; this->unk22 = 0
sw    $zero, 0x24($s0)       ; this->flags = 0
sh    $zero, 0x28($s0)       ; this->unk28 = 0
sh    $zero, 0x2A($s0)       ; this->unk2A = 0
```

This is where every offset in `Class6D430` beyond `+0x24` (the only
field known before this unit's round) was derived — a straight-line
field-by-field zeroing matches straight-line C with no reordering needed.

## Final C

```c
void Class6D430__Class6D430(Class6D430 *this) {
    Get_vtable_BasicClass()->ctor(this);
    this->methods = (Class6D430Methods *) GetClass6D430Methods();
    this->unk0C = 0;
    this->unk10 = NULL;
    this->unk14 = 0;
    this->unk20 = 0;
    this->unk22 = 0;
    this->flags = 0;
    this->unk28 = 0;
    this->unk2A = 0;
}
```

## Attempt log (abbreviated)

Matched on the second attempt in isolation — the first showed 24/25 in-range
with one call-target (`jal GetClass6D430Methods`) word differing purely from
address drift caused by `Class6D430__AllocBuffer` (below) still being the wrong size
at that point. No change to this function was needed; fixing the drift
source elsewhere resolved it to 25/25.

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable, no branches, single return path (void).
- **loop-invariant hoisting:** not applicable, no loop.
- **prologue store-order barrier:** not applicable, no residue of this class
  was observed.

## Proposed learning

See `Class6D430__AllocBuffer.md` for the real finding from this round: `func_80017B34`
(the allocator) takes **one** argument (`size`), not two. This function's own
"one word off, call-target only" symptom while a sibling function in the same
unit had a genuine size bug is a useful diagnostic pattern worth naming: a
lone call-target-encoding mismatch, with everything else in a function's
window matching, means look for a wrong-sized function *elsewhere in the same
translation unit*, not in the function funcdiff is currently pointing at.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80026A50` | `Class6D430__Class6D430` | A |

**Evidence.** The class's own constructor, `+0x008` slot by the project's
convention: chains `Get_vtable_BasicClass()->ctor`, installs
`GetClass6D430Methods()` as `this->methods`, then zeroes every field this
unit derived. A constructor's mechanics (chain base, install vtable,
initialise fields) ARE its purpose, so tier A by the plan's own rule.
`Class__Class` is the project's constructor-naming convention.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `Class6D430 *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

`Class6D430` is shared with `code_179d8_h.c`/`code_179d8_q.c`
(see `Class6D430__AllocBuffer.md`); every field this constructor zeroes is
therefore checked, and only `flags` (this round's own rename, zero
cross-unit hits) renamed outright. The rest:

| field | proposed name | tier | evidence |
| --- | --- | --- | --- |
| `unk22` | -- (no change proposed) | C | Zeroed here, never read or written anywhere else in this unit. No evidence beyond "exists, width 2 bytes". Left as `unk22` rather than guess. |
| `unk28` | -- (no change proposed) | C | Same as `unk22`: write-only in this unit, no read site found. |
| `unk2A` | -- (no change proposed) | C | Same as `unk22`/`unk28`. |

Not posting these three to the broadcast as renames (there is nothing to
apply); noting them here so the next reader does not re-derive "these are
zeroed and otherwise untouched" from scratch.
