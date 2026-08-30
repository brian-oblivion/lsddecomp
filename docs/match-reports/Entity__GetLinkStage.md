# Entity__GetLinkStage

**Unit:** Entity · **Size:** 15 instructions · **Status:** STALLED, class TOOLCHAIN (maspsx `addiu_at` macro-expansion mismatch)

## What it does

```c
s32 v = D_80089EAB[this->moodIndex * 0x10];  /* signed byte */
if (v < 0) {
    return ~v;
}
return v - 1;
```

Same mood-indexed table family as the other three `Entity__Get*` functions.
The branch (`bltz`) genuinely tests the loaded value's sign, taking a
`~v` (bitwise NOT, via `nor $v0,$zero,$v0`) path on negative and `v - 1` on
non-negative — confirmed via `asm-differ` that **both branch targets agree
with retail exactly** (`.L8005D9C4` / `.L8005D9C8` land in the same relative
positions once the load-instruction-count residue below is accounted for).
Per the head's round-2026-08-30-a broadcast on `strcat` — check whether
branch *targets* differ, not just delay-slot filler, before calling
something compiler-internal — this one was checked and the control flow is
byte-correct; the CFG is not in question here.

## The residue

Identical class and mechanism to `Entity__GetEventVideo.md` (full isolated
reproducer there) — the table load is 1 word short, everything else
(including the branch and both return paths) matches:

```
retail:  lui $at,%hi(D_80089EAB) / addiu $at,$at,%lo(D_80089EAB) / addu $at,$at,$v0 / lb $v0,0x0($at)
built:   lui $at,%hi(D_80089EAB) / addu $at,$at,$v0 / lb $v0,%lo(D_80089EAB)($at)
```

## Preserved body

```c
#if 0
s32 Entity__GetLinkStage(Entity *this) {
    s32 v;

    v = D_80089EAB[this->moodIndex * 0x10];
    if (v < 0) {
        return ~v;
    }
    return v - 1;
}
#endif
```

## Proposed learning

See `Entity__GetEventVideo.md` for the full writeup — same toolchain class.
Additionally worth recording: this function is the one place in this batch
with a real branch, and it was the one case where the head's "check branch
targets" rule from the `strcat` adjudication was directly exercised — and it
cleared the check (targets agree), which is what let this be classified
TOOLCHAIN rather than re-attempted as a reshaping problem.
