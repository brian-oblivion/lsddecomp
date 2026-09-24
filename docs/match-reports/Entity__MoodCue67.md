# Entity__MoodCue67 -- MATCHED (57/57 words)

> Renamed from `func_800620C4` on 2026-09-24 (tools/rename.py). Address 0x800620c4.

Unit: `Entity_e` (round 13). Never touches its `out` argument at all (same
shape as `Entity__MoodCue70` below it in this unit) -- a one-shot "roll dice on
the first tick" gate that sets `unk44 = 0xB`, followed by two independent
`unkFC`-threshold actions guarded by that flag.
`void Entity__MoodCue67(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue67(Entity *this, EntityMoodHandlerArg *out) {
    if (this->unkFC == 0) {
        if (rand() % 3 == 0) {
            this->unk44 = 0xB;
        }
    }
    if (this->unk44 == 0xB) {
        if (this->unkFC == 0x1F6) {
            this->methods->slotCC(this, 0x800, 0);
            Class6B5CC__FaceTarget(this, this->unk94, 1, 0, 0);
        }
        if (this->unkFC >= 0x1F5) {
            this->methods->slotC4(this, -0x200, 0);
        }
    }
}
```

## Derivation notes

- `rand() % 3 == 0` is the magic-constant-3 idiom (`0x55555556`, `mult`,
  `sra 31`+`mfhi`+`subu` sign fix, `sll 1`/`addu` reconstructing `q*3`, then
  compared directly against the dividend rather than materializing the
  remainder) -- verified against the pinned `cc1` (`int f(int x){return
  x%3;}`) byte-for-byte, same technique as `Entity__MoodCue69`/`Entity__MoodCue64`.
- Both post-`unk44==0xB` checks are guarded by the SAME outer `if`, not
  chained/nested against each other -- the `bne` at `.L80062118` branches
  all the way to the function's epilogue (`.L80062194`) when `unk44 != 0xB`,
  skipping both the `unkFC == 0x1F6` call and the `unkFC >= 0x1F5` call.
- `unkFC >= 0x1F5` reads directly off a `slti $v0,$v0,0x1F5` / `bnez` pair
  that skips the call when the `slti` result is 1 (i.e. when `unkFC <
  0x1F5`) -- the call fires on the complementary condition, `>= 0x1F5`.
- `slotCC` (`s32 (*)(Entity*, s32, s32)`, already typed by earlier work in
  this unit) is called here as a bare statement with its return value
  discarded, same as its other known call sites -- no retype pressure.
- `slotC4` (already `void`, see `include/Entity.h`'s slotC4 comment for why
  it must stay that way) called as `(this, -0x200, 0)`.

No new struct knowledge; every field and vtable slot here was already known
from sibling handlers in this unit.

## Naming

Why `MoodCueNN`: the function's address sits in `gEntityMoodHandlerTable` row 67 (`asm/data/79528.data.s`, base 0x80089EB0, stride 0x10; row = (slot address - 0x80089EB0) / 0x10), confirmed by reading `disk/SLPS_015.56` directly rather than trusting address proximity (Entity_d/round 76 measured that row order does not track code address). Tier B: the row-to-function mapping is a compiler fact, not a guess, but which dream object or mood state each row represents is not established -- the row number is kept decimal and zero-padded so the names sort in table order, same convention as Entity_d.
