# FileResource__FileResource

> Renamed from `Class6D430__Class6D430` on 2026-09-26 (tools/rename.py). Address 0x80026a50.

> Renamed from `func_80026A50` on 2026-09-18 (tools/rename.py). Address 0x80026a50.

**Unit:** code_171e0 · **Size:** 25 instructions · **Status:** MATCHED (25/25 words, whole-image build verified byte-exact)

## What it does

The constructor for the `gFileResourceMethods` class (its own vtable slot `+0x008`,
per `include/code_171e0.h`'s `FileResourceMethods`). Chains the base
class's constructor first (`Get_vtable_BasicClass()->ctor(this)`), then installs
this class's own vtable pointer (fetched via the already-matched
`GetFileResourceMethods`, which just returns `&gFileResourceMethods`), then zeroes every field
this unit currently knows about.

## Derivation

```
jal   Get_vtable_BasicClass
 addu $s0, $a0, $zero        ; s0 = this
lw    $v0, 0x8($v0)          ; v0 = (base table)->ctor
jalr  $v0                    ; Get_vtable_BasicClass()->ctor(this)
jal   GetFileResourceMethods          ; v0 = &gFileResourceMethods
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

This is where every offset in `FileResource` beyond `+0x24` (the only
field known before this unit's round) was derived — a straight-line
field-by-field zeroing matches straight-line C with no reordering needed.

## Final C

```c
void FileResource__FileResource(FileResource *this) {
    Get_vtable_BasicClass()->ctor(this);
    this->methods = (FileResourceMethods *) GetFileResourceMethods();
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
with one call-target (`jal GetFileResourceMethods`) word differing purely from
address drift caused by `FileResource__LoadFile` (below) still being the wrong size
at that point. No change to this function was needed; fixing the drift
source elsewhere resolved it to 25/25.

## Head broadcast levers — applicability

- **goto-vs-return:** not applicable, no branches, single return path (void).
- **loop-invariant hoisting:** not applicable, no loop.
- **prologue store-order barrier:** not applicable, no residue of this class
  was observed.

## Proposed learning

See `FileResource__LoadFile.md` for the real finding from this round: `BMemPMgrAlloc`
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
| `func_80026A50` | `FileResource__FileResource` | A |

**Evidence.** The class's own constructor, `+0x008` slot by the project's
convention: chains `Get_vtable_BasicClass()->ctor`, installs
`GetFileResourceMethods()` as `this->methods`, then zeroes every field this
unit derived. A constructor's mechanics (chain base, install vtable,
initialise fields) ARE its purpose, so tier A by the plan's own rule.
`Class__Class` is the project's constructor-naming convention.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `FileResource *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

`FileResource` is shared with `code_179d8_h.c`/`code_179d8_q.c`
(see `FileResource__LoadFile.md`); every field this constructor zeroes is
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
