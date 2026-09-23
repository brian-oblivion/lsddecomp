# func_8005FEF8

**Unit:** Entity_c · **Size:** 33 words · **Status:** MATCHED (33/33 words,
whole-image build verified byte-exact)

## What it does

`(Entity *this, EntityMoodHandlerArg *out) -> void`. Another mood-dispatch
handler, same family as `Entity_b`'s `Entity__MoodCue14`/`Entity__MoodCue09`/etc,
but with the `slot148` opener made CONDITIONAL rather than unconditional:

```c
void func_8005FEF8(Entity *this, EntityMoodHandlerArg *out) {
    if (out->unk4 % 120 == 0) {
        out->unk10 = this->methods->slot148(this);
        out->unk1C = 1;
    }
}
```

The magic-multiply constant `0x88888889` at shift 6, reconstructed by
retail's own multiply-back (`*16` via `sll 4` minus itself = `*15`, then
`*8` via `sll 3` = `*120`), is division by 120 -- `out->unk4 % 120`
reproduces it directly, same idiom as `Entity_b`'s `Entity__MoodCue07` (divisor
90) and `Entity__MoodCue00`/`Entity__MoodCue13` (divisor 10/3).

## Attempt log

Matched on the first attempt.

## Proposed learning

None new -- confirms the mood-dispatch handler family extends into
`Entity_c` with the same idioms `Entity_b` established, including a new
variant (conditional rather than unconditional `slot148` opener).
