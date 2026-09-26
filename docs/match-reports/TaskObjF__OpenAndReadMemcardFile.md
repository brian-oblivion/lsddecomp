# TaskObjF__OpenAndReadMemcardFile — MATCHED round 75 (echo): 41/41, whole image OK. Lever: MISSING ARGUMENT -- the call is `BuildMemcardPath(pathBuf, self->unkC, suffix)`, forwarding this function's own 3rd parameter, not a 2-argument call.

> Renamed from `func_8004EA38` on 2026-09-24 (tools/rename.py). Address 0x8004ea38.

REVISITED, round 75: MATCHED on the fifth build (first build of the lever); names/types used (the K&R `BuildMemcardPath()` declaration is now a 3-parameter prototype; the 3rd parameter renamed `filterName` -> `suffix`).

## Round 75 (echo)

**Baseline, the preserved NON_MATCHING body rebuilt live exactly as the
unit gave it:** 1/41, `insertions 1 / deletions 1` (positional skeleton
diffs 40; all misalignment after the one missing `move v0,a0`).

**Levers, in order:**

| build | body | score |
| --- | --- | --- |
| 1 | preserved body | 1/41, ins 1 / del 1 |
| 2 | local copy `node = self; ... node->unkC` | 1/41 (no change) |
| 3 | `sel = self->unkC;` named first | 1/41 (no change) |
| 4 | `pb = pathBuf;` named first | 1/41 (no change) |
| 5 | **`BuildMemcardPath(pathBuf, self->unkC, suffix)`** | **41/41, whole image OK** |

**Mechanism.** With three arguments, GCC loads the hard argument registers
in order `a0`, `a1`, `a2`; the `a1` value stays an unforced MEM whose
address is `self`'s pseudo, so `self` is live across the `a0 = sp+0x10`
set, cannot be tied to `$a0`, and local-alloc gives it `$v0` -- retail's
`move v0,a0; lw a1,0xC(v0)` (the scheduler then drops the `addiu a0` into
the `jal` delay slot). The `a2` load is `move a2,a2` on the incoming
parameter and is deleted, which is why retail shows no `$a2` set-up and
why rounds 14-71 read the call as 2-argument. With only two arguments
there is nothing forcing the order and `self` stays in `$a0`.

Also dropped: the `arity-ok` K&R declaration. Both call sites in this unit
now pass three arguments, so `extern void *BuildMemcardPath(void *dest,
s32 selector, void *suffix);` is a real prototype and the whole image
stays byte-exact (TaskObjF__ProbeCardFreeSpace 29/29). `docs/match-reports/BuildMemcardPath.md`
("Why src/class_3bb8c_e.c must declare it unprototyped") is superseded by
this -- flagged for the head, not edited.

```c
s32 TaskObjF__OpenAndReadMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *suffix)
{
    s32 pathBuf[8];
    void *path;
    s32 handle;
    void *buf;

    path = BuildMemcardPath(pathBuf, self->unkC, suffix);
    handle = open(path, 1);
    if (handle == -1) {
        return 0;
    }
    if (destBuf != NULL) {
        buf = BMemPMgrAlloc(0x80);
        read(handle, buf, 0x80);
        strcpy((char *)destBuf, (char *)buf + 4);
        BMemPMgrFree(buf);
    }
    close(handle);
    return 1;
}
```

### Proposed learning

"The callee reads `$aN` but this call site never sets it" is a claim that
the CALLER already holds the value in `$aN` -- i.e. a forwarded parameter
-- far more often than a dead-argument call. Check whether the calling
function's own incoming `$aN` is otherwise unused; if so, pass it. The
tell here was not at the call at all: a lone `move vN,a0` in the prologue
before `a0` is reused for the first argument = an extra argument after it
that GCC loads in order. `arity-ok` annotations where one call site "omits"
an argument deserve this check.

---

Previous title: TaskObjF__OpenAndReadMemcardFile — NON_MATCHING body promoted, round 71. Length: 1 word SHORT (40/41, 0xA0/0xA4). Word-match: 1/41 in-range (live-measured round 71, matches rounds 14/19/37 -- the body is structurally right, see below). First real diff: file 0x3F238 / vram 0x8004EA3C.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class). Restored to
`INCLUDE_ASM` — see "Why restored" below; this is a correct-LENGTH
placeholder, not a correct-BYTES one.

> **ROUND 37 (delta): re-verified only, no new attempt.** Rebuilt this
> EXACT preserved body: reproduces 1/41 in-range, 40 vs 41 words (missing
> redundant move) precisely as recorded, no drift beyond the documented
> gap. Lowest priority on this round's list -- this function has already
> been permuter-searched twice (round 14, ~4600 iterations; round 19,
> ~130,167 iterations), both converging on the identical floor of 5 with
> no zero, which this round's own thesis (prioritize the NEVER-searched
> 38-function queue) explicitly weighs against a third run. Not
> re-searched this round; time spent instead on TaskObjF__TryWriteMemcardSaveFile,
> TaskObjF__BeginSave and TaskObjF__CheckCardStatus, all higher priority on this round's
> own list. Restored to `INCLUDE_ASM`, unchanged.

## What it does

`s32 TaskObjF__OpenAndReadMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *filterName)`. Opens
a path built from `self->unkC` via `BuildMemcardPath`/`func_80050938`; on
success, if `destBuf` is non-NULL, allocates an 0x80-byte scratch buffer,
reads into it via `func_80050928`, copies the string at `buf+4` into
`destBuf` via `strcpy`, and frees the scratch buffer; either way closes
the handle and returns 1 (0 if the open failed). `filterName` (3rd
parameter) is genuinely forwarded by this function's own caller
(`TaskObjF__ProbeMemcardFile`) but never read anywhere in this function's body — the
already-established "unused parameter invisible from the callee's own
disassembly" shape (see `TaskObjF__ProbeMemcardFile`'s report and
DECOMPILATION_LEARNINGS).

## Where it stands

**Every value, branch, and call in the body is correct.** The residue is
address drift caused by ONE MISSING instruction: retail copies `self`
(originally in `$a0`) into `$v0` as its very first real instruction
(`addu $v0, $a0, $zero`) and reads `self->unkC` THROUGH that copy
(`lw $a1, 0xc($v0)`) a few instructions later, rather than reading it
directly through `$a0` before `$a0` gets reused to hold the local
buffer's address. My compiled function is 40 words; retail's is 41 —
confirmed via `objdump -d build/src/class_3bb8c_e.c.o`, which shows my
version reading `self->unkC` via `$a0` directly (skipping the redundant
copy entirely) since `$a0` hasn't been clobbered yet at that point.

This is the project's well-documented "redundant `move` retail emits and
you do not, same register, same value" residue class
(docs/MATCHING-GUIDE.md), now with a fifth confirmed instance —
`TaskObjF__ProbeCardFreeSpace` (matched this round, see its report) has the IDENTICAL
call shape (`BuildMemcardPath(localBuf, self->field, ...)`) and needed the
SAME redundant copy, but it resolved itself once an unrelated arithmetic
computation was reshaped elsewhere in that function, giving GCC a reason
to evacuate `self` from `$a0` on its own. No analogous lever was found
here because this function has no other arithmetic to reshape — the
whole body other than the two calls IS the `BuildMemcardPath`/
`func_80050938` open sequence.

## What was tried (7 attempts)

1. Direct transcription, `path = BuildMemcardPath(pathBuf, self->unkC);` —
   1/41, missing the redundant move.
2. Extract `self` into a locally-declared `Node3bb8cE *obj = self;` used
   for the `->unkC` read — no effect; GCC eliminates the dead copy since
   nothing else uses `obj`.
3. Extract `self->unkC` into its own named local
   (`id = self->unkC; ... BuildMemcardPath(pathBuf, id);`) as a separate
   statement before the call — no effect, same 1/41.
4. A bare `__asm__("")` as the function's first statement — no effect.
5. Swapping the two call arguments' evaluation order in source (not
   meaningfully expressible in C for a 2-argument call with no
   dependency between them; both orderings compile identically here).
6. Ran the permuter (`tools/setup-permuter.sh TaskObjF__OpenAndReadMemcardFile <seed>`) for
   ~280 seconds across ~4600+ iterations with `-j 6 --stop-on-zero`: best
   score reached was 5 (not 0), via `void *new_var; new_var = path; ...
   func_80050938(new_var, 1);` — a read of an UNINITIALIZED local (`path`
   read before its own assignment), which is undefined behaviour and not
   a usable lead. No zero-scoring variant was found in the search budget.
7. Declaring `BuildMemcardPath`'s 2nd argument or the local buffer's size
   differently (tried `s32 pathBuf[8]` vs implicitly larger) — no effect
   on this residue; only affects overall frame size, which already
   matches retail's `-0x40` (confirmed: my compiled prologue is also
   `addiu sp,sp,-0x40`, so the missing word is NOT a frame-size problem,
   purely the missing redundant copy).

## Why restored to `INCLUDE_ASM`

The 40-vs-41-word mismatch means every function AFTER this one in ROM
order (`TaskObjF__FindUnusedMemcardName`, `TaskObjF__CollectExistingMemcardFiles`, `TaskObjF__CheckCardSpace`,
`TaskObjF__ProbeCardFreeSpace`) would drift by 4 bytes if this function's near-miss C
were left in `src/`. Restoring `INCLUDE_ASM` reproduces retail's own
bytes verbatim (correct length), which is what let `TaskObjF__CheckCardSpace` and
`TaskObjF__ProbeCardFreeSpace` (both after this one) be verified and matched cleanly
this round. Per CLAUDE.md: no score short of byte-exact justifies leaving
C in `src/`.

## Preserved near-miss body (`#if 0`)

Reaches 1/41 in-range but at the CORRECT total length once the redundant
move is set aside — i.e. everything else about this transcription is
believed correct.

```c
#if 0
/* stalesyms --fix 2026-09-22: func_800508F8 -> close, func_80050928 -> read, func_80050938 -> open -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
s32 TaskObjF__OpenAndReadMemcardFile(Node3bb8cE *self, u8 *destBuf, u8 *filterName)
{
    s32 pathBuf[8];
    void *path;
    s32 handle;
    void *buf;

    path = BuildMemcardPath(pathBuf, self->unkC);
    handle = open(path, 1);
    if (handle == -1) {
        return 0;
    }
    if (destBuf != NULL) {
        buf = BMemPMgrAlloc(0x80);
        read(handle, buf, 0x80);
        strcpy((char *)destBuf, (char *)buf + 4);
        BMemPMgrFree(buf);
    }
    close(handle);
    return 1;
}
#endif
```

Needs (already declared in `src/class_3bb8c_e.c`, all local to this
unit): `Node3bb8cE`, `BuildMemcardPath`, `func_80050938`, `func_80050928`,
`func_800508F8`, `BMemPMgrAlloc`, `BMemPMgrFree`, `strcpy`.

### Direction NOT tried, with reason

Did not attempt hand-writing a `register`/asm-constraint fix — banned by
CLAUDE.md rule 6 regardless of outcome, and this is explicitly a
same-register-different-count (missing instruction) residue, not a
register-identity swap, so the rule's own test ("does it change WHICH
register holds a value") doesn't even apply here; there is no register to
pin, only an instruction to conjure, which is exactly the class of thing
a barrier or operand-order change cannot do.

### Proposed learning

**The redundant-`move`-of-`self` shape recurs specifically around calls
of the form `helper(localStackBuffer, self->field, ...)`** — both this
function and `TaskObjF__ProbeCardFreeSpace` hit it on the identical call
(`BuildMemcardPath`). It resolved for `TaskObjF__ProbeCardFreeSpace` once other arithmetic
in that function gave the register allocator a reason to move `self`
out of `$a0` early; `TaskObjF__OpenAndReadMemcardFile` has no such arithmetic to lean on,
which may be exactly why it stayed a genuine stall while its sibling
matched. Worth trying on a future instance: if the function has ANY
unrelated computation that can be legitimately reordered to sit between
function entry and the `->field` read, try moving it there before
concluding this is unreachable from C.

## Round 19 (echo): claim re-verified, one manual retry, one bounded
## background permuter run (both negative)

Re-verified the "1/41 best, 40 vs 41 words, missing redundant move"
claim first: rebuilt with the preserved body in place, confirmed
identical to the round-14 description via a fresh `objdump` on the
compiled `.o` (40 words, missing retail's `addu $v0,$a0,$zero` self-copy).

Tried one manual variant not in the original attempt list: indirecting
BOTH `pathBuf`'s address and `self->unkC` through named locals assigned
as separate statements before the call (`pbuf = pathBuf; id = self->unkC;
path = BuildMemcardPath(pbuf, id);`), on the theory that forcing two
explicit materializations right after entry might give the allocator a
reason to evacuate `self` into a scratch register the way `TaskObjF__ProbeCardFreeSpace`
found. Result: **worse and drifted** (39 words, now 2 short instead of
1, with the redundant move still missing) -- reverted immediately.

Set up and ran the permuter as background work (per this round's
instructions: bounded search is not something to wait on). Scaffold
sanity-checked first (`permuter.py --debug`): base score 105
(`Register Differences: 1 (5)`, `Deletions: 1 (100)` -- matches this
report's single-missing-instruction description exactly). Ran
`timeout 600 permuter.py -j 8 --stop-on-zero --best-only` in the
background while doing hand work on other functions in this round's
list; system load was low (uncontended, unlike round 18's five-runner
saturation). **`permuter exit=124`** (bound fired, not an external
kill). **130,167 iterations**, best score reached **5** (down from 105,
via 100 -> 45 -> 5), **no zero found**. This reproduces round 14's own
`~4600`-iteration finding of best-score-5 almost exactly (same score
floor, ~28x more iterations, still no zero) -- strong convergent evidence
the search has found the local floor of whatever this residue's
neighborhood looks like to the permuter's random mutator, though per the
project's standing rule this is phrased as "not closed in 130,167
iterations," not as proof the space is exhausted.

Filing unchanged as STALL at 1/41 (40 vs 41 words), `INCLUDE_ASM`
untouched throughout (both the manual attempt and the permuter run
happened only in a scratch/`permuter-work/` seed, never in `src/`).

### Proposed learning (round 19)

**A second independent permuter run at ~28x the iteration count of the
first, on the same seed, found the exact same score floor (5) as the
first run.** This is a useful cross-check on the "not exhausted, just
under-searched" framing this project insists on: it does not prove the
space is closed, but two runs at very different iteration counts landing
on the identical local minimum is meaningfully stronger evidence of a
real floor than either run alone -- worth treating repeated convergence
to the same nonzero score, across independent runs, as itself informative
even though the project's rule (correctly) forbids calling it
"exhausted."

## Round 71 (delta): NON_MATCHING body promoted, round 71

Promoted the preserved near-miss body (the `#if 0` block above, unchanged
since round 14) into `#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif`
in `src/class_3bb8c_e.c`. This is a **hand-derived** body -- both permuter
runs on record for this function (round 14, ~4600 iterations; round 19,
~130,167 iterations) converged on a best score of 5 (not 0), and the
lowest-scoring variant either run found was a read of an uninitialized
local (undefined behaviour), so nothing from either permuter run is
incorporated into the promoted source.

Live-measured under today's pinned toolchain (`--nop-at-expansion`
included) before writing this entry, per this round's directive: made the
body live C (not `#ifdef`), ran `./build-and-verify.sh` (clean compile,
whole-image SHA1 mismatch as expected -- the 1-word-short length shifts
everything after it in ROM order, which is the ~175KB out-of-range drift
`funcdiff.py` reports and is expected, not a new residue) and
`tools/funcdiff.py TaskObjF__OpenAndReadMemcardFile`. Result: **1/41 words match in-range,
40/41 total length (1 word short)**, objdump confirms 40 instructions in
the compiled body -- identical to the figures already on file since round
14/19/37. No stale-length correction needed; the title line above is
otherwise unchanged in substance. Restored the `#ifdef
NON_MATCHING`/`#else INCLUDE_ASM`/`#endif` wrapper after measuring;
`./build-and-verify.sh` and `tools/check-nonmatching.sh` both green with
the wrapped form in place.

## Naming (round 78, track 3)

`func_8004EA38` -> `TaskObjF__OpenAndReadMemcardFile`. **Tier B.** Private helper called only by `TaskObjF__ProbeMemcardFile`. Builds a memcard path (`BuildMemcardPath(pathBuf, self->cardSlot, suffix)`), opens it, and if `destBuf` is non-NULL reads the first 0x80 bytes and `strcpy`s from offset +4 into `destBuf` (skipping what looks like a 4-byte header field). Mechanics only; what the copied bytes represent to the game (a save's title/comment field, by position) is not confirmed here.
