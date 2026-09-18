> Renamed from `func_80026A50` on 2026-09-18 (tools/rename.py). Address 0x80026a50.

# Class6D430__Class6D430

**Unit:** code_171e0 · **Size:** 25 instructions · **Status:** MATCHED (25/25 words, whole-image build verified byte-exact)

## What it does

The constructor for the `D_8006D430` class (its own vtable slot `+0x008`,
per `include/code_171e0.h`'s `UnkFlagsObjMethods_171e0`). Chains the base
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

This is where every offset in `UnkFlagsObj_171e0` beyond `+0x24` (the only
field known before this unit's round) was derived — a straight-line
field-by-field zeroing matches straight-line C with no reordering needed.

## Final C

```c
void Class6D430__Class6D430(UnkFlagsObj_171e0 *this) {
    Get_vtable_BasicClass()->ctor(this);
    this->methods = (UnkFlagsObjMethods_171e0 *) GetClass6D430Methods();
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
