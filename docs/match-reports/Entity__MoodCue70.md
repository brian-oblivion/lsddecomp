# Entity__MoodCue70 -- MATCHED (45/45 words)

> Renamed from `func_800624BC` on 2026-09-24 (tools/rename.py). Address 0x800624bc.

Unit: `Entity_e` (round 12). Three independent, unrelated checks on
`this->unkFC` in sequence; `out` (the `EntityMoodHandlerArg *` parameter)
is unused entirely -- confirmed by the disassembly never touching `$a1`.
`void Entity__MoodCue70(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue70(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (rand() & 1) {
            this->unk44 = 0xB;
        }
    }
    if (this->unkFC == 0x12C) {
        this->methods->slot44(this, 0, ROTATION_YAW_PLUS180);
    }
    if (this->unkFC < 0x258) {
        this->methods->slotC4(this, this->unk44 == 0 ? -0x100 : 0x100, 0);
    }
}
```

## Derivation notes

Straightforward on the first two branches. The third branch's ternary was
the interesting part: the disassembly is

```
lw v0,0x44(s0)      ; this->unk44
lw v1,0(s0)          ; this->methods
bnez v0,L80062548       ; if unk44 != 0, keep the delay-slot value
 ori a1,zero,0x100          ; delay: a1=0x100
addiu a1,zero,-0x100          ; fallthrough (unk44==0): overwrite a1=-0x100
L80062548: ... slotC4(this, a1, 0)
```

which reads directly as `this->unk44 != 0 ? 0x100 : -0x100` -- and that
was my first-pass C. It compiled to the *semantically identical* value
(same result for both cases of `unk44`) but with **the branch polarity
inverted and the two literal-setting instructions swapped**: `beqz` instead
of `bnez`, with `-0x100` in the delay slot and `0x100` on the fallthrough.
A same-size, semantically-correct-but-byte-different residue -- exactly the
"redundant move"/operand-order class in `docs/MATCHING-GUIDE.md`, just for
a ternary's branch sense rather than a redundant move.

Fixed by writing the mathematically equivalent but syntactically flipped
`this->unk44 == 0 ? -0x100 : 0x100` -- same value in both cases, but this
phrasing made GCC choose the branch polarity (`beqz`... no, ended up
matching retail's `bnez`) and literal ordering that matches retail exactly.

### Proposed learning

**A ternary's `cond ? A : B` vs `!cond ? B : A` are value-equivalent but not
byte-equivalent.** When a ternary residue shows a same-size branch-polarity
inversion (`bne`/`beq` swapped) with the two arms' *values* otherwise
matching, try flipping the comparison operator and swapping the arms
(`x != 0 ? A : B` &harr; `x == 0 ? B : A`) before reaching for anything
heavier -- it's a one-line, zero-risk edit and resolved this residue and
`Entity__MoodCue66`'s in this same round.
