# Entity__GetMoodEffect

**Unit:** Entity · **Size:** 6 instructions · **Status:** MATCHED (6/6 words, whole-image build verified byte-exact)

## What it does

`return &D_80089EA4[this->moodIndex * 0x10];` — forms the address of a
16-byte-stride table row selected by `this->moodIndex`, without loading
through it. Same index (`moodIndex`, `Entity.h` offset `+0x98`) as the other
three `Entity__Get*Effect/Stage/Video` functions in this unit, each keyed to
its own table.

## Derivation

```
lw   $v0, 0x98($a0)          ; v0 = this->moodIndex
lui  $v1, %hi(D_80089EA4)
addiu $v1, $v1, %lo(D_80089EA4)
sll  $v0, $v0, 4             ; v0 = moodIndex * 16
jr   $ra
 addu $v0, $v0, $v1           ; return &D_80089EA4[moodIndex*16]
```

## Final C

```c
void *Entity__GetMoodEffect(Entity *this) {
    return &D_80089EA4[this->moodIndex * 0x10];
}
```

`D_80089EA4` is declared `extern u8 D_80089EA4[];` in `Entity.h` (byte-sized
element so the `* 0x10` literal is a direct byte offset, not double-scaled
through `sizeof`).

## Attempt log

Matched on the first attempt. Notably, this function does **not** exhibit
the maspsx `addiu_at` residue that blocks its three siblings
(`Entity__GetUnlockEffect`/`GetLinkStage`/`GetEventVideo`, see
`Entity__GetEventVideo.md`) — it only ever *forms* the table address
(`lui`+`addiu`+`addu`), never loads through it, which routes through a
different maspsx expansion path than the `lb $reg,sym($reg)` load macro that
triggers the bug. This is worth remembering as the dividing line for this
whole table-lookup family: address-only accessors are safe, anything that
dereferences through a runtime-indexed symbol is not.

Note also that before the sibling functions were reverted to `INCLUDE_ASM`,
this function transiently showed a 1-word mismatch (a differing embedded
relocation constant, same instruction) purely from address drift caused by
their now-stalled residue — that was never a bug in this function itself.

## Proposed learning

See `Entity__GetEventVideo.md`. This function is the useful control case
proving the maspsx bug is specific to the *load* macro, not table-lookup
address arithmetic generally.
