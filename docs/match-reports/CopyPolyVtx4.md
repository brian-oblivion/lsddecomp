# CopyPolyVtx4 — MATCHED (35/35 words)

> Renamed from `func_8001A4C0` on 2026-09-24 (tools/rename.py). Address 0x8001a4c0.

> **Round 20: closed.** Every section below through "RUNNER PASS, permuter
> round (alpha)" is the round-13/alpha history of a STALL classification that
> is now superseded — not because the register-identity-vs-order analysis was
> wrong, but because it was analysing the wrong SOURCE SHAPE. See "ROUND 20:
> MATCH" at the end for what actually closed it and why the whole prior
> approach could never have reached retail's own instruction sequence.

Unit: `src/code_8220_c.c`. `void CopyPolyVtx4(PolyVtx **dst, PolyVtx **src,
PolyUV4 *uv0, PolyUV4 *uv1, PolyUV4 *uv2, PolyUV4 *uv3)` — calls
`CopyPolyVtx3(dst, src, uv0, uv1, uv2)` (matched round 13, same unit), then
does its own single unaligned 8-byte copy (`src[3]->xy` -> `dst[3]->xy`) and
a 4-byte unaligned copy (`*uv3` -> `dst[3]->uv`). `arg2`/`arg3` (`uv0`/`uv1`)
are unused by this function itself — genuinely just forwarded through to
`CopyPolyVtx3`'s corresponding parameters, per the "unused parameter shows
up in the caller as uninitialised" idiom, except here it is not garbage,
it is a deliberate pass-through.

**Original classification (superseded): same pattern as
DECOMPILATION_LEARNINGS' "Prologue callee-save stores in the wrong ORDER,
same registers and same offsets. Not reachable from C."** The following
attempts were run against a whole-function raw-register `__asm__`
transcription of the tail copy — a HARD RULE 6 violation waiting to happen
(the rule's own text names this exact case: "an awkward unaligned struct
copy" has a C form, unlike a GTE/COP2 store). They were never tried against
plain C using the same idiom that already matched `CopyPolyVtx3` in this
same file, which is what actually closed it. Preserved for the record.

## Best body reached (28/35 words, attempts 1, 4, 7, 10 landed here)

```c
#if 0
void CopyPolyVtx4(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5)
{
    void *p1 = arg1;
    void *p0 = arg0;
    void *p5 = arg5;

    CopyPolyVtx3(p0, p1, arg2, arg3, arg4);

    __asm__ volatile (
        "lw $3, 0xc(%0)\n\t"
        "lw $2, 0xc(%1)\n\t"
        "nop\n\t"
        "lwl $4, 0x3($2)\n\t"
        "lwr $4, 0x0($2)\n\t"
        "lwl $5, 0x7($2)\n\t"
        "lwr $5, 0x4($2)\n\t"
        "swl $4, 0x3($3)\n\t"
        "swr $4, 0x0($3)\n\t"
        "swl $5, 0x7($3)\n\t"
        "swr $5, 0x4($3)\n\t"
        "lw $2, 0xc(%0)\n\t"
        "lwl $3, 0x3(%2)\n\t"
        "lwr $3, 0x0(%2)\n\t"
        "nop\n\t"
        "swl $3, 0x13($2)\n\t"
        "swr $3, 0x10($2)\n\t"
        :
        : "r" (p0), "r" (p1), "r" (p5)
        : "$2", "$3", "$4", "$5", "memory");
}
#endif
```

This scored 28/35 words, build compiles clean (`build exit=0`), no address
drift (compares within `0xACC0-0xACC0+0x8C`, matching retail's declared
`0x8C`-byte length exactly). The 7 differing words (indices 1-7, plus a
partial reshuffle observed in one variant at indices 1-8) are ENTIRELY
confined to the prologue: the `sw`/`lw` pairs that save `$s0`/`$s1`/`$s2`
and load `arg4`/`arg5` off the caller's stack.

## Residue: confirmed register-identity-correct, order-only

Retail's prologue:

```
addiu $sp,$sp,-0x28
sw    $s0,0x18($sp)
addu  $s0,$a0,$zero        ; s0 = arg0
sw    $s1,0x1c($sp)
addu  $s1,$a1,$zero        ; s1 = arg1
lw    $v0,0x38($sp)        ; arg4 (needed only for the call)
sw    $s2,0x20($sp)
lw    $s2,0x3c($sp)        ; s2 = arg5
sw    $ra,0x24($sp)
jal   CopyPolyVtx3
```

My best body's built object (`objdump -d build/src/code_8220_c.c.o`)
confirms the SAME register assignment (`$s0`=arg0, `$s1`=arg1, `$s2`=arg5,
each saved at the SAME stack offsets 0x18/0x1c/0x20) — this took several
attempts to reach (see below) — but orders the four save/load groups
differently: `lw $v0,0x38(sp)` / `sw $s2` / `lw $s2,0x3c(sp)` happen FIRST,
THEN `sw $s1`/`move s1,a1`/`sw $s0`/`move s0,a0`, i.e. the two stack-arg
fetches got hoisted ahead of the two register-to-register moves. Retail
does register moves first, stack fetches second.

## Attempts (10, well under the 30 cap, stopped because the residue matches
a documented dead end)

1. `arg0`/`arg1`/`arg5` referenced directly in the post-call asm operands
   (no locals) — 24/35, WRONG registers (`$s2`=arg0, `$s0`=arg1, `$s1`=
   arg5 — a genuinely different, three-way-rotated assignment, not just
   reordered).
2. Local `p0`/`p1`/`p5` declared in natural parameter order — 25/35,
   `$s1`=arg0/`$s0`=arg1 (still swapped from retail, though closer).
3. `p5`/`p1`/`p0` declared in reverse order — 28/35, but with `$s1`=arg0/
   `$s0`=arg1 still swapped (verified via objdump on this specific variant).
4. `p1`/`p0` declared (that order), no separate `p5` local (used `arg5`
   directly in the operand) — 28/35, and THIS time register identity is
   CORRECT (confirmed via objdump: `$s0`=arg0, `$s1`=arg1, matching retail
   exactly). Order still wrong. **This is the body preserved above.**
5. Reordered `p0`/`p1` (swapped back) — reproduced attempt 2's wrong
   registers again, confirming declaration order of `p0` vs `p1`
   specifically (not `p5`'s presence) controls which of `$s0`/`$s1` each
   lands in.
6. Re-added an explicit `p5` local after `p1`/`p0` (matching attempt 4's
   correct registers, now with all three named) — no change, still 28/35,
   correct registers, same wrong order. Confirms `p5`'s presence/absence
   doesn't affect the order residue once `p0`/`p1`'s order is right.
7. `__asm__ volatile ("" : : : "memory")` between the locals and the call —
   25/35, regressed. The barrier disturbed register COLORING, not just
   order — it forced `p0`/`p1` into different hard registers than the
   unconstrained allocator chose.
8. `__asm__ ("")` (no `volatile`, no clobber) in the same spot — same
   regression as #7.
9. `__asm__ volatile ("")` with fully empty operand/clobber lists, same
   spot — same regression again.
10. The one documented-correct form: a bare `__asm__("")` as the
    function's literal FIRST statement (before any local's initializer, per
    DECOMPILATION_LEARNINGS' "Prologue callee-save stores in the wrong
    order" entry) — declarations moved to the top with initializers
    deferred to keep C89 legal, barrier first. Score UNCHANGED at 28/35 —
    the barrier neither improved nor (this time) broke register identity,
    it just produced a differently-shuffled 7-word mismatch set covering
    the same prologue region. Per the entry's own caveat ("does NOT
    generalise to other residues: three runners tried it elsewhere and
    worsened them"), this is a second confirmed instance of that
    non-generalisation — the lever simply does not reach this residue.

## Why this is filed as a stall, not left INCLUDE_ASM with no explanation

The residue is a same-registers/same-offsets/different-ORDER prologue —
exactly the shape DECOMPILATION_LEARNINGS already documents as "Not
reachable from C" — and the one permitted C-level lever for that class (a
bare `__asm__("")` scheduling barrier) was tried in the position the
learnings entry specifies and did not change the score. No project rule
permits going further: `register T v asm("$N")` or an operand constraint
chosen specifically to pin `$s0`/`$s1`/`$s2` would fix a register identity
that is ALREADY correct, which is not what the rule is for, and the
residue here is not register identity at all — it is instruction order,
which reshaping C statements has not moved after ten structurally distinct
tries.

### Proposed learning

**Second confirmed instance of "prologue callee-save stores in the wrong
order, same registers and same offsets, not reachable from C, and the
`__asm__("")` first-statement barrier does not generalise to it."** Unlike
the original instance (`code_8220_b`, six declaration-order permutations
producing one identical score), this one show a NEW wrinkle worth
recording: getting register IDENTITY correct and getting instruction ORDER
correct are separable sub-problems here, and fixing one did not imply
progress on the other — declaration order of the LOCALS controlled which
hard register each landed in (attempts 2 vs 4/5), while no combination of
declaration order, local-variable presence, or barrier placement moved the
ORDER of the save/load groups once register identity was already correct
(attempts 4, 6, 10 all scored identically 28/35). Whenever a residue like
this is found, verify register identity independently from instruction
order before spending further attempts — an order-only residue with
correct registers is the stronger signal that no more attempts are likely
to help, not a reason to keep trying "just one more" reordering.

## RUNNER PASS, permuter round (alpha, code_8220_c): base confirmed, floor held, two degenerate leads not pursued

`--debug` on this report's preserved body (prologue store-order residue,
28/35 words): base score = **210**, decomposing as 3 reorderings x 60 = 180
plus 6 register differences x 5 = 30 -- pure reordering signature, no
insertion/deletion, matching the report's own "same registers, same
offsets, different ORDER" classification exactly.

Bounded search, `-j 6`, no PERM macros (blind). **Manually interrupted at
iteration 11091** (not a `timeout` self-fire) to comply with a mid-round
instruction to run only one permuter search at a time in this worktree
while the machine was oversubscribed (load average ~50-54/32 measured by
the head) -- this run was killed in favour of the `SubmitPolyF3` search
already in progress. Floor held at **210** for the observed run; no zero.

Two sub-210 candidates were saved (`output-165-1`, `output-180-1`,
gitignored) but both are degenerate syntactic noise on inspection of their
diffs, not genuine reshaping: `output-180-1` wraps the whole body in
`do { ... } while (0);` (semantically identical, no plausible mechanism to
move a prologue store's schedule), and `output-165-1` introduces a
self-assignment `new_var = (new_var = p1);` and reuses `new_var` as a bogus
extra argument to `CopyPolyVtx3` in place of `p1`/`arg4` in a way that
would change which value is passed -- likely wrong, not just reordered.
Neither was oracle-tested (unlike `SubmitPolyF3`'s cached-`head`
candidate, which WAS verified and found to cause address drift -- see that
report); these two didn't warrant the same effort given how clearly
non-substantive their diffs are. Not permuter-exhausted: this is "not
closed in ~11000 iterations of blind, unhinted search," not a proof.

### Proposed learning

A pure REORDER-only residue (asm-differ: zero insertions, zero deletions,
score decomposing entirely into reorder+register penalties) is a distinct
permuter signature from the register-identity-plus-filler class this
unit's other residue-8 family shows (100+100+register). Worth distinguishing
in future permuter triage: this class has no "one instruction is simply
absent" component at all, so a candidate that changes the INSTRUCTION COUNT
(fewer or more) is almost certainly wrong for it, not an improvement --
exactly what both saved sub-210 candidates here turned out to be under
inspection.

## ROUND 20: MATCH (35/35 words) — the tail copy is CopyPolyVtx3's own idiom, one element further

**The mistake in every attempt above: treating this function as needing a
hand-written asm transcription at all.** Re-reading `CopyPolyVtx4`'s own
disassembly side by side with `CopyPolyVtx3`'s (the already-matched sibling
this function calls) shows the tail copy is not a bespoke unaligned-copy
routine — it is `CopyPolyVtx3`'s own 4th-vertex case, done inline by the
caller instead of the callee. `CopyPolyVtx3` copies `dst[i]->xy = src[i]->xy`
and `dst[i]->uv = *uvI` for `i = 0,1,2`; `CopyPolyVtx4`'s own body does
exactly the same two field copies for `i = 3` — `*(arg0+0xC)` and
`*(arg1+0xC)` are simply `dst[3]` and `src[3]` (pointer-array element 3, at
byte offset 3*4 = 0xC), and the second copy target `(*(arg0+0xC))+0x10` is
`dst[3]->uv` (the `PolyVtx` `uv` member is at offset 0x10 — identical layout
to what `CopyPolyVtx3` already established for elements 0-2).

So the function is:

```c
void CopyPolyVtx4(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1,
                   PolyUV4 *uv2, PolyUV4 *uv3) {
    CopyPolyVtx3(dst, src, uv0, uv1, uv2);
    dst[3]->xy = src[3]->xy;
    dst[3]->uv = *uv3;
}
```

`dst[3]->xy = src[3]->xy` and `dst[3]->uv = *uv3` are whole-struct
assignments of the SAME alignment-2, all-`s16` types (`PolyXY8`, `PolyUV4`)
that already make `CopyPolyVtx3` compile to `lwl`/`lwr` + `swl`/`swr` — the
exact instruction shapes retail uses here too. Nothing about this residue was
ever a genuine "no C form exists" case; it was the ordinary struct-copy
idiom, just not recognised as the same idiom as its own sibling call.

**Result: `build exit=0`, `funcdiff.py CopyPolyVtx4` reports 35/35, no
drift warning.** Both the register-identity component (which registers hold
`dst`/`src`/`uv3`) and the instruction-ORDER component (the two stack-arg
fetches vs. the two register-to-register moves in the prologue) that ten
attempts across two rounds could not separate or fix by hand — because they
were fighting a hand-scheduled asm block's own fixed instruction order
against GCC's independently-chosen prologue order for the SURROUNDING
C — both resolved for free the moment GCC was allowed to generate the whole
function's prologue and body together from ordinary C. There was never a
"prologue store order, not reachable from C" residue in this function at
all; that diagnosis was a symptom of comparing GCC's real prologue against a
hand-written asm block's necessarily-different one.

**Header change:** `CopyPolyVtx4`'s declaration in `include/code_8220.h`
retyped from six `void *` parameters to `PolyVtx **dst, PolyVtx **src,
PolyUV4 *uv0, PolyUV4 *uv1, PolyUV4 *uv2, PolyUV4 *uv3` (matching
`CopyPolyVtx3`'s own signature plus the 4th UV). This is a change to an
EXISTING declaration; `include/code_8220.h` is shared by three units this
round but no other runner holds one this round, and the build after the
change is `build exit=0` (whole-image SHA1 match), so no other unit's
compile was disturbed. Call sites in this same file already passed
compatible pointer expressions (implicit-conversion warnings only, no
errors, matching this project's existing tolerance for that class of
warning elsewhere in the corpus).

### Proposed learning

**Before writing a whole-function raw-register `__asm__` transcription for
an "awkward unaligned copy," check whether an ALREADY-MATCHED sibling in the
same unit performs the identical field-level copy — the awkward copy may be
that sibling's own idiom, applied to one more array element, rather than a
genuinely new construct.** This is a sharper, more specific version of
CLAUDE.md's HARD RULE 6 test ("is there a C form?") for the specific case
where the function under test CALLS a sibling that already demonstrates the
C form: don't just ask "can C express this copy" in the abstract, ask "does
a function three lines above already express this exact copy, just with a
different index." The tell, in hindsight: `CopyPolyVtx4`'s own preserved
asm block copied the SAME 8-byte-then-4-byte shape, at the SAME relative
offsets (`+0x0`/`+0x10`), that `CopyPolyVtx3`'s C body already copies for
elements 0-2 — the index literally differs by one array slot. A register-
identity/instruction-order residue that resists ten structurally distinct
manual-declaration-order and barrier-placement attempts is a strong signal
the C shape itself (not just the declaration order within it) is wrong, and
"a whole-function raw asm block was substituted for an ordinary call
sequence + struct copy" is now a second confirmed instance of that shape
error, alongside CLAUDE.md's own `CopyPolyVtx3` precedent from round 13
(the six-line struct-copy rewrite of what had been proposed as a
whole-function raw-register `__asm__`).

## ROUND 20 (runner echo): raw-`__asm__` audit, secondary assignment

Per the coordinator's secondary assignment, audited every `__asm__` block
in `src/code_179d8_c.c`, `src/code_179d8_d.c`, `src/class_16334.c`,
`src/Entity_b.c`, `src/Entity_c.c`, `src/DreamSys.c`, and
`src/code_8220_b.c` (this unit's own family) for the same mistake found
in this function -- a whole-function raw-register transcription standing
in for an idiom ordinary C already expresses via an already-matched
sibling.

**21 blocks total, 21/21 legitimate, zero reworkable.**

- **6 bare `__asm__("");` scheduling barriers**, one each in
  `code_179d8_c.c` (`SetRCnt`), `code_179d8_d.c`, `class_16334.c`,
  `Entity_b.c`, `Entity_c.c`, `DreamSys.c`. Every one is a no-operand,
  no-clobber ordering barrier -- exactly HARD RULE 6's always-permitted
  form ("if it only changes instruction ORDER, it is allowed"), each
  cited in its own surrounding comment or an existing match report as
  fixing a documented reordering residue. None is a register-identity
  fix in disguise (no operand constraints, no register names anywhere
  in any of the six).
- **15 content-bearing `__asm__ volatile (...)` blocks, all in
  `code_8220_b.c`**: `ProjectTriFace` (2), `ProjectQuadFace` (4),
  `TransformAndCullPoly` (1, the large GTE-heavy handwritten function -- rtpt/
  nclip/avsz3 raw `.word` cofunction encodings plus `cfc2`), and the six
  already-matched leaf functions `StoreSxyPolyF3`/`StoreSxyPolyG3`/
  `StoreSxyPolyFT3`/`StoreSxyPolyGT3`/`StoreSxyPolyF4`/`StoreSxyPolyG4` (1-2
  each). **Every single one moves data through `lwc2`/`swc2`/`cfc2` or a
  raw-encoded GTE cofunction op (`rtps`/`rtpt`/`nclip`/`avsz3`) -- real
  COP2 *data* register traffic with no C spelling, exactly the exception
  HARD RULE 6 carves out, not the "awkward-but-expressible" case that was
  actually wrong in `CopyPolyVtx4`.** None of the 15 is an unaligned GPR
  struct copy or any other construct with a plain C form standing in as
  asm -- checked each block's mnemonics individually, not just counted
  them.

**No reworkable instance found anywhere in the audited set.** This
settles the question for these seven files this round: the
`CopyPolyVtx4` mistake (raw asm for a plain-C-expressible copy) does not
recur in any of them. `CopyPolyVtx4` itself, now matched, remains the
only confirmed instance of this mistake found so far project-wide.

## Naming (round 77, alpha)

`func_8001A4C0` -> `CopyPolyVtx4`. **Tier A**: same pure-leaf-copy
reasoning as CopyPolyVtx3, which this function forwards to (elements 0-2)
before doing its own element-3 copy. Parameters unchanged.
