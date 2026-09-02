# func_8005EBB4

**Unit:** Entity_b · **Size:** 57 words · **Status:** MATCHED (57/57 words,
whole-image build verified byte-exact)

## What it does

Another `D_80089EB0` mood-dispatch handler, `(Entity *this,
EntityMoodHandlerArg *out)`. Sets `out->unk10` from `slot148`, then branches
on `out->unk4`: if zero, sets `out->unk1C = 0xC` and increments
`this->unk44`; otherwise (nonzero), clamps `out->unk4` to `-1` once it grows
past `this->unk80 - 1`. Afterward, unconditionally: if `this->unk44 == 0x24`,
rolls `rand() % 3 == 0` and calls `this->methods->slot30(this, 0xB)` on a hit.

## Final C

```c
void func_8005EBB4(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 == 0) {
        out->unk1C = 0xC;
        this->unk44++;
    } else if (out->unk4 >= this->unk80 - 1) {
        out->unk4 = -1;
    }
    if (this->unk44 == 0x24) {
        if (rand() % 3 == 0) {
            this->methods->slot30(this, 0xB);
        }
    }
}
```

`this->unk80` reuses the field established the same round in
`func_8005E6F0.md`. `rand() % 3` reproduces retail's magic-multiply
sign-corrected remainder exactly, per the `x % N` idiom already confirmed in
`docs/DECOMPILATION_LEARNINGS.md`.

## Attempt log

Two attempts. The first attempt wrote the `if`/`else` with the branches
SWAPPED (`if (out->unk4 != 0) { clamp } else { unk1C/unk44 }`) — same
semantics, but retail lays the physical code out with the `out->unk4 == 0`
body as the fall-through and the `>= this->unk80 - 1` body reached by an
explicit `bnez`/jump-back. Reading the raw disassembly's branch DIRECTION
(`bnez $v1, <clamp-block>` — branches to the clamp block when `out->unk4` is
NONZERO, falls through to the `unk1C`/`unk44++` block when zero) showed the
retail source tests `out->unk4 == 0` FIRST, not `!= 0` first; swapping which
condition is written first in the `if`/`else` matched immediately, with no
other change needed (word length was already correct on attempt 1 — this was
a pure physical-layout swap, not a missing/extra instruction).

## Proposed learning

**For an `if`/`else` where GCC lays the "then" body as the fall-through and
the "else" body as a jump target, the WRITTEN CONDITION determines which
body is which** — swapping to `if (!cond) A else B` (equivalent to `if (cond)
B else A`) changes nothing semantically but flips which physical block is
the fall-through. When a residue is "right content, wrong position, same
register set, same instruction count," check whether the retail branch
TESTS THE OPPOSITE of what was written, not just whether the branch target
label matches.
