# Entity__GetLinkStage

> **HEAD ADJUDICATION, round 2026-08-30-a.** The diagnosis in this report is
> CORRECT and the head reproduced it independently from scratch. It is now
> written up project-wide in **`docs/research/addiu-at-blocker.md`**, with an
> isolated reproducer and a corpus census: retail uses the unfolded (`addiu_at`)
> form for **502 of 502** runtime-indexed global accesses across 39 files, and
> the folded form **zero** times. There is no counterexample anywhere in the
> executable.
>
> **One correction to the proposed remedy.** Repinning `--aspsx-version` to 2.29
> is not surgical and should not be presented as the fix. `config_for_aspsx_version`
> flips **four** flags below 2.30, not one — `addiu_at` plus three nop-insertion
> rules (`nop_at_expansion`, `nop_mflo_mfhi`, `nop_lw_lw`) that affect constructs
> throughout the image, including inside the 57 functions that currently match.
> maspsx exposes no `--addiu-at` flag, so the behaviour cannot be enabled alone
> without patching maspsx. This is the same shape as the already-rejected `-G`
> experiment. See the research document; the ruling is the operator's.


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
