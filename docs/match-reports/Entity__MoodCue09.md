# Entity__MoodCue09

> Renamed from `func_8005E6F0` on 2026-09-23 (tools/rename.py). Address 0x8005e6f0.

**Unit:** Entity_b · **Size:** 46 words · **Status:** MATCHED (46/46 words,
whole-image build verified byte-exact)

## What it does

Another `gEntityMoodHandlerTable` mood-dispatch handler, `(Entity *this,
EntityMoodHandlerArg *out)`. Sets `out->unk10` from the shared `slot148`
call, then computes `out->unk4 % (this->unk80 / 2)` and sets `out->unk1C =
0xA` when the remainder is zero, then unconditionally calls
`this->methods->slotC4(this, -0x1E, 0)`.

The prompt flagged this function (and `Entity__MoodCue11`) as containing an
`addiu $at, $zero, -0x1` that is NOT the `addiu_at` blocker (`addiu $at,
$at, %lo(...)`) — confirmed: it's part of GCC 2.6.3/maspsx's standard
non-constant `div` expansion (zero-divisor `break 7` check, then an
INT_MIN/-1 overflow `break 6` check against the literal `-1`), the same
idiom `docs/DECOMPILATION_LEARNINGS.md` already documents for `div` by a
non-constant expression. `this->unk80 / 2` is genuinely non-constant at
compile time (a runtime field read), so the compiler cannot fold it to a
shift and must emit the full guarded `div`.

## New field: `Entity::unk80`

`this->unk80` (read here, and independently by `Entity__MoodCue13` matched the
same round and `Entity__MoodCue07`/`Entity__MoodCue11` which remain `INCLUDE_ASM`) is
halved via the standard signed-divide-by-2 idiom (`srl $v1,$v0,31; addu
$v0,$v0,$v1; sra $v0,$v0,1`, i.e. `(x + (unsigned)x>>31) >> 1`), confirming
plain `s32 unk80` in C source as `this->unk80 / 2`.

## Final C

```c
void Entity__MoodCue09(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (out->unk4 % (this->unk80 / 2) == 0) {
        out->unk1C = 0xA;
    }
    this->methods->slotC4(this, -0x1E, 0);
}
```

## Attempt log

Matched on the first attempt.

## Proposed learning

None new beyond the existing `div`-by-non-constant idiom already documented;
this is a second confirming instance (mfhi/remainder rather than mflo/
quotient this time — `out->unk4 % divisor`, not `out->unk4 / divisor`).
