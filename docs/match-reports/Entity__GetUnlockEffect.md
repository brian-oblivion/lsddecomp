# Entity__GetUnlockEffect

**Unit:** Entity · **Size:** 14 instructions · **Status:** STALLED, class TOOLCHAIN (maspsx `addiu_at` macro-expansion mismatch)

## What it does

`return (s8)D_80089EA6[this->moodIndex * 0x10] * 1000;` — same mood-indexed
table-lookup family as `Entity__GetMoodEffect`/`Entity__GetLinkStage`/
`Entity__GetEventVideo`. The `* 1000` is a genuine multiply-by-constant,
confirmed structurally correct (matches retail's shift/subtract/shift/add/
shift strength-reduction sequence exactly: `v1*32 - v1 = v1*31`, `*4 = v1*124`,
`+v1 = v1*125`, `*8 = v1*1000`).

## The residue

Identical class and mechanism to `Entity__GetEventVideo.md` (read that report
for the full isolated reproducer and the maspsx source citation) — the table
load itself is 1 word short:

```
retail:  lui $at,%hi(D_80089EA6) / addiu $at,$at,%lo(D_80089EA6) / addu $at,$at,$v0 / lb $v1,0x0($at)
built:   lui $at,%hi(D_80089EA6) / addu $at,$at,$v0 / lb $v1,%lo(D_80089EA6)($at)
```

Every instruction *after* the load (the strength-reduced `* 1000`) matches
retail exactly once the drift from this one missing word is accounted for —
confirmed by rebuilding with this function and `Entity__GetLinkStage`/
`Entity__GetEventVideo` all reverted to `INCLUDE_ASM` (restoring the whole
image to green) and then re-diffing the *other* functions in this unit
cleanly, with zero residue anywhere else in the batch.

## Preserved body

```c
#if 0
s32 Entity__GetUnlockEffect(Entity *this) {
    return D_80089EA6[this->moodIndex * 0x10] * 1000;
}
#endif
```

## Proposed learning

See `Entity__GetEventVideo.md` for the full writeup — same toolchain class,
documented once there to avoid repeating the reproducer three times.
