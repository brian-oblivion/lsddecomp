# ApplyMatrixToLVArray -- MATCHED (31/31, round 19)

> Renamed from `func_8001EE98` on 2026-09-17 (tools/rename.py). Address 0x8001ee98.

**Status: MATCHED, whole-image green.** See "Round 19 (echo): MATCHED --
the six-argument dead-code reproducer" at the end of this report for the
winning source and the toolchain reproducer that found it -- this
overturns every verdict below, including this round's own earlier
"unexplained gap" framing, which the head correctly identified as testing
the wrong hypothesis.

**Historical status (kept for context): STALL (unexplained outgoing-arg frame-size gap, 19/31 words)

Unit: `SceneNode` (round 14). A paired-array iteration: `count`
iterations, 0xC bytes/element, calling `func_80015618(fixed, b, a)` once
per element and advancing both `a`/`b` by 0xC each time while `fixed`
stays constant. Called by `SceneNode__RotateLocalVector` (this unit, matched this
round) as `ApplyMatrixToLVArray(dst, dst, 1, buf)`.
`void ApplyMatrixToLVArray(void *a, void *b, s32 count, void *fixed)`.

Blocker screen clean: no `gp_rel`, no `addiu $at,$at,%lo`, no
`mfhi`/`mflo`-adjacent-`mult`/`div` hit.

## Best-reached source (compiles, builds green; restored to `INCLUDE_ASM`
## per project rule -- `SceneNode__RotateLocalVector`'s own call site still resolves and
## scores 29/29 against this function's retail bytes regardless)

```c
void ApplyMatrixToLVArray(void *a, void *b, s32 count, void *fixed) {
    u8 *end;

    end = (u8 *)a + count * 0xC;
    while ((u8 *)a < end) {
        func_80015618(fixed, b, a);
        a = (u8 *)a + 0xC;
        b = (u8 *)b + 0xC;
    }
}
```

Preserved inline (`#if 0`, positioned where it would compile back into
`src/graphics/scene_node.c` in place of the current `INCLUDE_ASM`):

```c
#if 0
void ApplyMatrixToLVArray(void *a, void *b, s32 count, void *fixed) {
    u8 *end;

    end = (u8 *)a + count * 0xC;
    while ((u8 *)a < end) {
        func_80015618(fixed, b, a);
        a = (u8 *)a + 0xC;
        b = (u8 *)b + 0xC;
    }
}
#endif
```

## The residue, precisely

Confirmed via `tools/asm-differ/diff.py`: **every instruction, every
register, and every relative instruction ORDER matches retail exactly.**
The only difference is that every stack offset (and the frame size
itself) is uniformly 8 bytes smaller in my build: retail allocates `0x30`
(48 bytes), mine allocates `0x28` (40). Breaking both frames down by
region:

| region | retail | mine |
| --- | --- | --- |
| outgoing-arg area | 0x18 (24 bytes) | 0x10 (16 bytes) |
| saved regs (`s0,s1,s2,s3,ra`) | 0x14 (20 bytes) | 0x14 (20 bytes, identical) |
| top padding | 4 bytes | 4 bytes (identical) |

The saved-register area and padding are byte-for-byte identical in size;
**the entire 8-byte gap is confined to the outgoing-argument reservation**
-- retail reserves 24 bytes for the call to `func_80015618`, mine reserves
the standard o32 minimum of 16. The call site itself (both in retail's
disassembly and in my build) sets only three argument registers (`a0`,
`a1`, `a2`) before the `jal`, with no visible `sw` to any stack-argument
slot -- there is no OBSERVABLE 4th+ argument being passed.

## What was tried (4 real full builds)

1. **Two named locals (`pa`, `pb`) walking copies of `a`/`b`, plus
   `end`**: 16/31, with BOTH the frame-size gap AND a register-save-ORDER
   difference (retail saves `s0` first, this version saved `s3` first).
2. **Minimal version (source above)**, mutating the `a`/`b` PARAMETERS
   directly instead of introducing separate walking locals: 19/31.
   This fixed the register save ORDER completely (now byte-identical to
   retail, confirmed instruction-by-instruction) but left the 8-byte
   frame gap untouched.
3. **Two dummy unused `s32` locals** added speculatively to test whether
   an unaccounted local might explain the gap: no effect (dead locals are
   eliminated at `-O2` before frame layout, as expected) -- reverted
   immediately, this was a diagnostic probe, not a real fix candidate.

Given the residue is confined entirely to outgoing-argument space for a
call whose visible argument count (3) does not explain 24 reserved bytes
(6 words) rather than the standard 16 (4 words), and no source-level
reshape of THIS function moved it, the most likely explanation is that
`func_80015618`'s TRUE signature takes more arguments than the 3 visible
at this call site (e.g. exactly 6 word-sized arguments, with the extra
ones passed as literal values my reconstruction can't see without
`func_80015618`'s own disassembly) -- but `func_80015618` is in a
different, still-uncarved segment, out of this carve's scope, so that
cannot be confirmed from here.

### Proposed learning

**An 8-byte (or any multiple-of-4) outgoing-argument-area gap with an
otherwise letter-perfect match (same instructions, same registers, same
order) is a signal to check the CALLEE's true arity, not to keep
reshaping the caller.** Every other residue class in this project's
history (redundant moves, register identity, CFG differences) leaves a
trace INSIDE the visible instruction stream; this one is invisible there
by construction -- the caller's own bytes are already perfect, and the
gap only shows up as unexplained reserved stack space. When
`func_80015618` (or whatever uncarved function is the deferred blocker
here) is eventually carved, its own argument count should be checked
against this function's outgoing-arg reservation before re-deriving this
residue from scratch.

## Round 18 (permuter pass, charlie): VERDICT on the ambiguous score, and a scorer-blind-spot finding

**The real, current score is 19/31 -- the title figure, not the "29/29" the
head flagged as ambiguous.** Confirmed directly with the real oracle: the
best-reached source above was dropped into `src/graphics/scene_node.c` in place of
the `INCLUDE_ASM`, and `./build-and-verify.sh` + `funcdiff.py` gave

```
build exit=2   (whole-image check FAILS, as expected for a stall)
ApplyMatrixToLVArray: 19/31 words match (file 0xF698-0xF714)
```

with the diff confirming exactly the report's own analysis: every DIFF line
is a stack-relative immediate off by a uniform 8 (`d0ffbd27`/`d8ffbd27` =
`-0x30` vs `-0x28`, and every saved-register offset shifted the same 8
bytes). Restored to `INCLUDE_ASM` immediately after (build re-verified
green).

**The "29/29" in the prior write-up is `SceneNode__RotateLocalVector` (the CALLER,
already matched, a different, unrelated function) -- not a claim about
this function at all.** A `jal ApplyMatrixToLVArray` instruction encodes only the
callee's symbol address, never its internal bytes, so `SceneNode__RotateLocalVector`
reads as a full match regardless of whether `ApplyMatrixToLVArray` itself is
right, wrong, or still `INCLUDE_ASM`. Running `funcdiff.py SceneNode__RotateLocalVector`
to sanity-check this function's fix would be a category error -- it is
CLAUDE.md's "still-`INCLUDE_ASM`"/stale-signal trap wearing a different
mask: not a stale build and not literal `INCLUDE_ASM`-vs-`INCLUDE_ASM`,
but the same underlying failure of "a green/perfect score that says
nothing about the function you actually changed."

**A second, more consequential instance of the same trap, found while
setting up the permuter for this function: `permuter.py --debug` WITHOUT
`--stack-diffs` reports `base score = 0` (a false full match) for this
exact seed** -- the same source that the real oracle above scores 19/31.
Inspecting `tools/decomp-permuter/src/scorer.py`'s `diff_sameline`: stack
penalties only accumulate when `self.stack_differences` is set (the
`--stack-diffs` CLI flag, off by default and not mentioned anywhere in
`docs/MATCHING-GUIDE.md`'s permuter section); when it's off, a
uniformly-shifted `$sp`-relative offset is invisible to the scorer's field
comparison (`ignore_last_field` is only set when `self.stack_differences`
is true; otherwise the offset field IS compared, but every attempt here
showed identical `addr(sp)` masking in the debug pretty-printer regardless).
Confirmed the fix: re-running with `--stack-diffs` gives
`base score = 90` (`Stack Differences: 80 (1)`, `Register Differences: 2`),
correctly nonzero and consistent with the real 19/31.

**Generalizable finding for this project's permuter workflow:** any
residue that is PURELY a stack-frame/outgoing-arg-size mismatch (this
function's whole class, per the "callee's true arity" theory above) is
INVISIBLE to `permuter.py --debug` unless `--stack-diffs` is passed. A
runner who trusts a bare `--debug` zero on such a function would wrongly
conclude "already matches, nothing to do" and move on without ever
touching the real oracle -- exactly backwards from CLAUDE.md's directive
to distrust suspiciously perfect scores. **Always pass `--stack-diffs`
when debug-basing a function whose report describes a frame-size/
stack-offset residue, and treat a bare (no-`--stack-diffs`) permuter zero
as unproven until corroborated by `build-and-verify.sh` + `funcdiff.py`.**

No further search was run against this function: with `--stack-diffs` the
search space is unchanged from before (still gated on `func_80015618`'s
unknown true arity, out of this carve's scope), so this remains a STALL,
now with its score independently reconfirmed rather than merely asserted.

## Round 19 (echo): found and read `func_80015618`'s own disassembly; the arity theory does not resolve cleanly, and here is why

Per this round's brief ("a report's contamination/exhaustion claim is
about a previous session's attempts, not the function -- verify before
inheriting"), re-ran the real oracle first: `INCLUDE_ASM` still in place,
fresh full build green, `funcdiff.py ApplyMatrixToLVArray` reconfirms **19/31**,
matching this report's title exactly -- no contamination this round.

`func_80015618` is **not actually uncarved** as the prior write-up
assumed -- it lives in `the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s)` (a Psy-Q SDK library segment,
`grep -rln 'glabel func_80015618' asm/` finds only that file), i.e. it is
PSYQ library object code, not a game-code segment awaiting a carve. Its
own disassembly (`the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s):4363-4464`) is a GTE-based vector
transform: reads 5 words at `a0` (loaded via `ctc2` into GTE control
regs 0-4 -- the packed rotation-matrix layout, i.e. `a0` is a rotation
matrix's `m[3][3]` packed as 5 words), reads 3 words at `a1` (put through
a sign/15-bit-magnitude split and two `mvmva` passes -- a fixed-point
range-reduction idiom for a vector too wide for the GTE's 16-bit `IR`
registers), and writes+returns its result through `a2`
(`sw`s to `0/4/8($a2)` then `addu $v0,$a2,$zero` -- this callee itself
follows the "return dst" idiom noted elsewhere in this round's work).
**Only three registers (`a0`,`a1`,`a2`) are ever read** -- there is no
internal use of `a3` or of any stack-passed argument, so the callee's
OWN body gives no positive evidence of a 4th+ argument.

Built an isolated reproducer under the pinned toolchain (per CLAUDE.md's
"escalate, do not experiment" recipe) using this function's own
preserved best-reached source verbatim, declaring `func_80015618` with
exactly the 3 pointer args the call site visibly uses:

```c
extern void func_80015618(void *fixed, void *b, void *a);
void probe(void *a, void *b, int count, void *fixed) {
    unsigned char *end;
    end = (unsigned char *)a + count * 0xC;
    while ((unsigned char *)a < end) {
        func_80015618(fixed, b, a);
        a = (unsigned char *)a + 0xC;
        b = (unsigned char *)b + 0xC;
    }
}
```

run through `cpp | cc1 -mips1 -mcpu=3000 -G0 -O2` directly (no maspsx/as
needed -- cc1's own `.frame` comment already answers the question):
`.frame $sp,40,$31 # vars=0, regs=5/0, args=16, extra=0`. **This exactly
reproduces our current 40-byte/16-byte-args result, not retail's
48-byte/24-byte-args one** -- i.e. a straightforward 3-pointer-arg call,
under the pinned compiler, in complete isolation, computes the SAME
16-byte minimum this unit's own build already produces. This is not a
new residue; it is independent confirmation, from a from-scratch
reproducer rather than this file's own build, that no reshaping of
`ApplyMatrixToLVArray`'s SOURCE can reach retail's 24-byte reservation while the
callee is genuinely a 3-argument call -- the caller's own bytes already
match perfectly aside from this uniform offset, so there is no remaining
degree of freedom on the caller side.

**What this does and does not resolve.** It does NOT identify why
retail's compiler reserved 24 bytes -- that requires knowing
`func_80015618`'s prototype as the ORIGINAL translation unit saw it
(number of formal parameters), which is a property of source text we do
not have (this is precompiled SDK object code) and is NOT recoverable
from the callee's own machine code: a callee's disassembly shows which
registers it happens to READ, never how many words its caller's compiler
believed it was owed. A prototype declaring more parameters than a
given call site passes would be a C89 arity-mismatch error under a
visible ANSI prototype, so if retail's TU really passed only 3 words
(as the callee's behavior is consistent with), the extra reservation is
not explainable by "the call actually had 6 arguments we can't see" --
there would be visible `sw`s to the stack-arg slots before the `jal`,
and retail's own disassembly (quoted in this function's `.s`) has none.

This narrows the space rather than closing it: the 8-byte gap is real,
uniform, and NOT reproducible by any 3-argument call under the pinned
toolchain in isolation, which means it is either (a) an artifact of
`current_function_outgoing_args_size` computation in GCC 2.6.3's mips.c
that depends on something in `ApplyMatrixToLVArray`'s own source shape we have
not yet tried (declaration style, a param count/type quirk, or an
alignment rule triggered by something not present in this minimal
reproducer), or (b) a property of the ORIGINAL call site's visible
argument list that differs from our 3-pointer reconstruction in a way
that produces identical registers/order but a different word count (e.g.
one of the "pointer" arguments genuinely being a 2-word type at the
source level, though nothing in the 3 register values used inside
`func_80015618` requires that). Per CLAUDE.md ("never escalate a
toolchain lead not tried and failed in isolation") this reproducer IS
that isolated failed attempt, and is included here for whoever picks
this up next rather than re-deriving it. Remains a STALL at 19/31,
`INCLUDE_ASM` untouched in `src/`.

### Proposed learning

**A stack-frame/outgoing-arg-size residue that is otherwise byte-perfect
cannot always be resolved by "wait for the callee to be carved."** When
the callee turns out to be precompiled SDK/library object code (as here
-- `func_80015618` lives in `the 0x2258..0x8220 Psy-Q block (now linked from lib/, formerly asm/psyq_2258.s)`, a `psyq_*` segment, not a
game-code segment awaiting a carve), there is no future carve that will
ever supply its source-level prototype; the callee's own disassembly can
rule OUT some arities (by showing which registers it reads) but cannot
positively confirm the ORIGINAL caller-side prototype's word count. Check
whether the deferred function is a `psyq_*` segment before filing "wait
for the carve" as the resolution path -- if it is, the reproducer to run
is an isolated minimal call under the pinned toolchain (as above), and a
clean non-reproduction there is the most this residue class can yield
without the original SDK headers.

## Round 19 (echo): MATCHED -- the six-argument dead-code reproducer

**The head reframed this correctly: the isolated reproducer above tested
the wrong hypothesis.** It confirmed a genuine 3-argument call cannot
reach retail's 24-byte reservation -- true, but that arity was already
known false (the callee's own disassembly reads only `a0`/`a1`/`a2`).
The live question the head posed was: *what six-argument call shape was
in the original source*, given GCC 2.6.3 sizes the outgoing-arg area from
the MAXIMUM argument count over every call expression to a function in
scope, computed during RTL expansion -- which runs BEFORE dead-code
elimination removes an unreachable branch.

Three hypotheses were ranked cheapest-first: (1) an unprototyped call
site with 6 arguments somewhere still live, (2) a second call site the
optimizer removed whose arg-area sizing stuck, (3) a macro expansion.
Tested (1) first in isolation (an unprototyped `func_80015618` called
with 6 arguments) -- reproduced the FRAME size (`args=24`) but NOT the
live call site's own bytes (a real 6-arg call emits real argument-setup
instructions -- `move a3,zero` plus two stack stores for args 5/6 --
which retail's disassembly does not have at all). This ruled out (1) as
literally tested and pointed straight at (2): a call site that sizes the
frame but is never actually reached.

Tested (2) directly under the pinned toolchain (`cpp | cc1 | maspsx |
as`, per CLAUDE.md's "escalate, do not experiment" reproducer recipe):

```c
extern void func_80015618();

void probe(void *a, void *b, int count, void *fixed) {
    unsigned char *end;
    end = (unsigned char *)a + count * 0xC;
    while ((unsigned char *)a < end) {
        func_80015618(fixed, b, a);
        a = (unsigned char *)a + 0xC;
        b = (unsigned char *)b + 0xC;
    }
    if (0) {
        func_80015618(fixed, b, a, 0, 0, 0);
    }
}
```

**This reproduces retail's ENTIRE function byte-for-byte** -- not just
the frame size: total length 31 words (0x7C bytes, matching retail
exactly), the live call site is exactly the same 3 register moves with no
extra setup (the `if (0)` branch emits NO code at all, confirmed via
`objdump` on the assembled `.o`), and every surrounding instruction
(prologue saves, loop guard, loop body, epilogue) lines up 1:1 with
retail's own disassembly, instruction for instruction. `func_80015618`
had to be declared WITHOUT a prototype (`extern void func_80015618();`,
C89's "unspecified argument list" form) -- a real ANSI prototype would
make either call site (3 args vs 6 args to the same declared function) a
compile error; only a K&R-style declaration lets each call site's own
argument list stand on its own.

Translated directly into `src/graphics/scene_node.c` (only the identifier names
changed to match this project's own convention -- `end`/`a`/`b`/`fixed`
already matched):

```c
void ApplyMatrixToLVArray(void *a, void *b, s32 count, void *fixed) {
    u8 *end;

    end = (u8 *)a + count * 0xC;
    while ((u8 *)a < end) {
        func_80015618(fixed, b, a);
        a = (u8 *)a + 0xC;
        b = (u8 *)b + 0xC;
    }
    if (0) {
        func_80015618(fixed, b, a, 0, 0, 0);
    }
}
```

Result: **31/31, `build exit=0`, whole-image `OK: build matches retail
SLPS_015.56`.** Full match, no warnings from the compiler on the
mismatched-arity calls (expected -- that is exactly what an unprototyped
declaration permits).

`include/scene_node.h` updated: `func_80015618`'s declaration changed from
a 3-parameter ANSI prototype to `extern void func_80015618();`, with a
comment explaining why (the two call sites in this same file need
different, incompatible arities). This has no effect on any other unit --
`func_80015618` is declared LOCALLY per-unit throughout this project (a
DIFFERENT 3-parameter prototype already exists in
`include/class_3bb8c.h`, a sibling unit's own independent local view,
untouched by this change) and `include/scene_node.h`'s own declaration is
only ever used by this one call site in `scene_node.c`.

### Proposed learning

**"A frame reserves more outgoing-argument space than any LIVE call site
needs" is not evidence of a missing/uncarved callee's true arity -- it
can be evidence of a DEAD call site to the SAME callee, at a DIFFERENT
arity, still present in the source but eliminated from the compiled
output.** GCC 2.6.3's outgoing-argument-area sizing happens during RTL
expansion, which runs before control-flow-based dead-code elimination;
a call inside a `if (0)`-guarded (or otherwise provably-unreachable, e.g.
compiled-out debug/assert) branch still contributes its argument count to
the frame's sizing pass even though it emits zero instructions. The
diagnostic signature is specific and worth naming: the LIVE call site's
own bytes are already byte-perfect (no missing/extra argument-setup
instructions there), and the gap is PURELY in reserved-but-never-written
stack space -- exactly what this report's original round-14 analysis
already established, which is why the fix was findable at all without
guessing at semantics. When this signature appears, the next reproducer
to try is not "does a wider LIVE call reach this frame size" (it won't,
without also polluting the live call site's own instructions) but "does
an UNREACHABLE call at the wider arity reach this frame size while
leaving the live call site untouched" -- which is what closed it here.

This also generalizes the "never escalate a toolchain lead not tried and
failed in isolation" principle in the other direction: the SAME
reproducer recipe (isolated `cpp|cc1|maspsx|as` pipeline, seconds per
variant) that would have supported an escalation had hypothesis (2) also
failed instead FOUND the real fix -- the recipe is symmetric, useful for
confirming a real toolchain lead and for falsifying one into a genuine
source-level answer.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EE98` -> `ApplyMatrixToLVArray`. Tier A.** Free function,
  complete semantics: `count` iterations of Sony's `ApplyMatrixLV(m, v0,
  v1)` (`v1 = m * v0`, VECTOR in and out) over 0xC-byte elements -- the
  long-vector sibling of `ApplyMatrixToSVArray`. Same reason for the
  `ApplyMatrixTo...` spelling: it must not read as an SDK export.
- **Parameters corrected to `(dst, src, count, m)`** from `(a, b, count,
  fixed)`, per the byte-exact call `ApplyMatrixLV(m, src, dst)` inside the
  loop: the 1st argument is written. Names only; byte-identical. The
  declaration in include/scene_node.h was updated to match, and that is an
  edit to an EXISTING declaration in a shared header -- flagged as such in
  the round summary.
- **The `if (0)` branch is untouched and must stay.** It is what sizes
  retail's 24-byte outgoing-argument area; the derivation is in the MATCHED
  section above and is the reason `ApplyMatrixLV` is declared unprototyped.
  A reader deleting it as dead code breaks the match, so the unit now says
  so at the site as well as here.

## Round 95 (bravo): Sony's declarations

`include/scene_node.h` no longer declares `ApplyMatrixLV` unprototyped; the
three SceneNode units take libgte.h's prototype, `VECTOR *ApplyMatrixLV(MATRIX
*, VECTOR *, VECTOR *)`. The six-argument call in the `if (0)` branch, which
only sizes the outgoing-argument area, now goes through a cast to an
unprototyped function type, `((void (*)())ApplyMatrixLV)(m, src, dst, 0, 0,
0)`. The frame is still 24 bytes; whole-image SHA1 unchanged.


## Round 95 (bravo): moved from include/scene_node.h

The header's banner was rewritten as documentation in round 95; the comments it carried about this function, verbatim:

```c
/* ApplyMatrixToLVArray (this unit, round 14; MATCHED round 19, echo -- see
 * docs/match-reports/ApplyMatrixToLVArray.md): a paired-array iteration sibling
 * to ApplyMatrixToSVArray above -- `count` iterations, 0xC bytes/element (no
 * unaligned-load complication this time, both `a`/`b` are read directly),
 * calling `ApplyMatrixLV(fixed, b, a)` once per element and advancing both
 * `a`/`b` by 0xC each time while `fixed` stays constant across every call.
 * SceneNode__RotateLocalVector (this unit, round 14) calls it as `ApplyMatrixToLVArray(dst, dst,
 * 1, &buf)` where `dst` is SceneNode__RotateLocalVector's own 2nd argument and `buf` is a
 * 0xC-byte stack vector `self->methods`'s own `slot84` just filled in --
 * i.e. `a`/`b` here are the SAME pointer at that one call site, so it does
 * not by itself distinguish their roles. */

/* CHANGED round 50 (charlie), parameter NAMES only -- no type, arity or
 * order change: (a, b, count, fixed) -> (dst, src, count, m). `a` is the
 * WRITE destination and `b` the read source, per the byte-exact call
 * `ApplyMatrixLV(m, src, dst)` inside the loop. The same correction
 * applies to ApplyMatrixToSVArray's declaration above. */
```

## Round 98 (echo): track 7, moved from src/graphics/scene_node.c

The `(u8 *)p + 0xC` byte walks are now `(LongVec3 *)p + 1` element walks: byte-identical. The parameters stay `void *` because src/graphics/viewport_draw.c declares its own `void *` extern of this function while also including scene_node.h, so a typed prototype there would be `conflicting types` in a unit outside this job (proposed to the head). Typed locals copied from the parameters score 25/31 for the same prologue-order reason as ApplyMatrixToSVArray.

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* `dst[i] = m * src[i]` for `count` elements of 0xC bytes each, via Sony's
 * ApplyMatrixLV (`v1 = m * v0`, VECTOR in and out) -- the long-vector
 * sibling of ApplyMatrixToSVArray above, and no unaligned-copy dance
 * needed since both arrays are read and written in place.
 *
 * The `if (0)` call is NOT dead code to delete: retail reserves 24 bytes of
 * outgoing-argument space, six words, where this function's one live call
 * needs four. GCC 2.6.3 sizes that area from every call expression's
 * argument count during RTL expansion, before dead-branch elimination, so
 * the 6-argument call in the unreachable branch is what produces retail's
 * frame. Full derivation in docs/match-reports/ApplyMatrixToLVArray.md;
 * this is also why that call goes through an unprototyped function type. */
```
