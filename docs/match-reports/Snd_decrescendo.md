# Snd_decrescendo — STALL (re-derived round 58; register-identity residue)

> Renamed from `func_80033C90` on 2026-09-23 (tools/rename.py). Address 0x80033c90.

**Two bodies were measured and BOTH are preserved below, because on this
function the two title figures rank them in opposite orders.** Read this
before quoting a number.

| | body A (preserved as the one to build on) | body B (best raw score) |
| --- | --- | --- |
| **length** | **2 words SHORT** — 200 built vs 202 retail | **1 word SHORT** — 201 built vs 202 |
| **raw word-match** | **21/202** | **46/202** |
| `regs` / `vars` (cc1 `.frame`) | **6/0** (= retail) / 16 | 5/0 / 16 |
| `off` (=172*slot) in a call-saved reg | yes, `$s1` — **retail's own register** | no, stays in `$v0` |
| negHandler's `unk90` clear | **retail's 5-word `lw 0($s2)` + `addu` form** | 20-word full recompute (not retail's) |

**Body A is the closer body and body B's 46/202 is largely an ALIGNMENT
artifact.** Body A is 2 words short, so from the first call onward its words
sit 2 slots off retail's indices; body B is 1 short. The function's tail holds
three ~20-word address-recompute blocks that are almost entirely `$v0`/`$v1`
arithmetic — register-independent, so they match on whichever body happens to
be aligned. Counted in the prologue alone (words 0-28), where alignment is not
yet in play, body A matches **15** words and body B **10**. Body B is behind on
every structural measure and needs the negHandler lever re-applied. Do not
"improve" this report by promoting body B.

**First real diff (both bodies):** word 0 — the prologue.
Retail `addiu $sp,$sp,-0x40`, built `addiu $sp,$sp,-0x38`. Read off
`tools/asm-differ/diff.py Snd_decrescendo`, not inferred.

**On the pre-round-58 title, and why "length exact" and "M/N" must be stated
separately.** It read "Length: exact, 202/202 words" and "Raw word-match:
18/202". The first is a statement about the RETAIL RANGE (202 words at
0x24490-0x247B8) and says nothing about any body. The body that title recorded
builds to **180** words — 22 SHORT — with a wrong branch target. The two
measurements read identically and are not the same thing: `202/202` was the
range, and no body has ever been length-exact here.

REVISITED, round 58: STALL, re-derived from the .s; body rewritten (180 -> 200 of 202 words, structurally exact, residue is callee-saved register identity); names/types not relevant (unit has not passed track 3; trigger was stale title)

## What the function does (round 58 reading, derived from the .s)

A per-slot fade/ramp tick on the `_ss_score[screen][slot]` record.
`unk98` is a tick countdown, `unk42` the period/step, `unk3E` an enable gate,
`unk40` the remaining step count, `unk78`/`unk7A` the live output pair.

Each call decrements `unk98`. Then:

- **`unk42 > 0`** — step by `-1`, but only on ticks where `unk98 % unk42 == 0`
  (so `unk42` is a PERIOD). Off-period ticks skip everything including the
  end-of-ramp check and go straight to the final `SpuVmGetSeqVol` readback.
- **`unk42 <= 0`** — step by `unk42` itself every tick (so a negative `unk42`
  is a per-tick RATE rather than a period). Here `unk42` is RE-READ from the
  struct after the `SpuVmGetSeqVol` call, while the `unk40` update a few
  instructions earlier uses the register cached before it.

Either way `unk40` is stepped first; if it goes negative the ramp is over.
Otherwise `SpuVmGetSeqVol` reads the current pair, each half is tested against
the step (`== 0` for the `-1` step, `< -unk42` for the `+unk42` step), and
either the stepped pair is written back with `SpuVmSetSeqVol` or the ramp is
killed. Killing it means `SpuVmSetSeqVol(key, 0, 0, 0)` plus
`_ss_score[..][..].unk90 &= ~0x20` — bit 5 of `unk90` is the "this slot is
ramping" flag, and every exit path that is not an off-period tick re-checks
`unk98 == 0 || unk40 == 0` and clears it there too.

The key passed to both callees is `(s16)(screen | (slot << 8))`, consistent
with `libsnd_vmanager.c`'s established `SpuVmGetSeqVol(s32 p0, s16 *, s16 *)`.

## Claims of the previous report: what round 58 CONFIRMED

- The `_ss_score[(s16)a0][(s16)a1]` indexing, the 172-byte stride, and the
  field offsets `unk3E/unk40/unk42/unk78/unk7A/unk90/unk98`. All correct.
- `p` is a CACHED local. `p->unk98` is read after two calls with no reload of
  `_ss_score[..]`, so the row load is not being redone — that only happens if
  the pointer is in a variable.
- The three `unk90 &= ~0x20` sites are written as the FULL
  `_ss_score[a0][a1]` expression, not through `p` — retail recomputes the
  address at each, which `p->unk90` would not do.
- `unk42` really is re-read from memory in the `<= 0` arm after the call while
  the `unk40` update uses the cached register.
- The residue really is allocation, not logic.

## Claims of the previous report: what round 58 FALSIFIED

1. **"`callReal` ... `goto tailFinal`" is wrong — it is `goto tailCheck`.**
   The shared call block ends `j .L80033F0C` (0x80033E54, encoding
   `08 00 cf c3` -> 0x80033F0C), and `.L80033F0C` is the `lw $v0,0x98($s0)`
   end-of-ramp check, not the `.L80033F78` readback. The preserved body
   therefore skipped the end-of-ramp check on the commonest path. A
   control-flow error, not a codegen detail.

2. **The block ORDER was wrong.** Retail lays the trailer out as
   call -> clear -> negHandler -> tailCheck -> tailFinal. The preserved body
   had negHandler -> call -> clear. GCC 2.6.3 emits labelled blocks in source
   order, so this alone moves every block after the first call.

3. **`unk40` is `s16`, not `u16`.** See the unit comment. The `u16` reading
   is what produced the `ori $zero,0xFFFF`-in-a-callee-saved-register shape
   the old report attributed to allocation pressure.

4. **"the signature `void Snd_decrescendo(s32, s32)` is already fixed by that
   caller's own extern declaration" — there is no such caller and no such
   declaration.** `func_80033738` became `libsnd/sscall` (`SsSeqCalledTbyT`)
   in round 34, in this very file's own header comment. `grep -rn
   Snd_decrescendo src/ include/ config/` finds NOTHING outside this unit. The
   parameter types were free the whole time, and `s16` parameters build 1 word
   closer than `s32`-plus-casts. **A "fixed by the caller" claim has a
   lifetime: it dies when the caller leaves the game count.**

5. **"`s16 c42`"** — retail's single `lh $a2,0x42($s0)` needs `s32`. With
   `s16`, cc1 loads `lhu` and adds a `sll 16`/`sra 16` pair, because the value
   feeds a 16-bit store whose truncation makes the load's signedness dead.

6. **The old report's variant 2 ("pre-declared `s16 sa0`/`sa1` gives frame
   `-0x40`") is not reproducible as a lever.** Every 2026-09-19 variant sits
   at `vars=16` -> frame 0x38. `-0x40` in that variant came with 8/202 and a
   different body; it was a coincidence of pressure, not an axis.

## The levers that moved it (180 -> 200 words)

In order of size, each measured on its own:

| change | words built | raw | note |
| --- | --- | --- | --- |
| inherited body | 180 | 18/202 | 22 short, wrong branch target |
| block order + `callReal -> tailCheck` | 184 | 21/202 | control flow now retail's |
| `unk40` retyped `u16 -> s16` | 184 | — | de-CSE'd the 0xFFFF constant |
| `s16` parameters | 184 | 22/202 | `move $a3,$a0` appears |
| **cached row pointer used at negHandler only** | **203** | 35/202 | **breaks the tail merge** |
| `c42` retyped `s16 -> s32` | 201 | 43/202 | `lh $a2,0x42($s0)` |
| explicit `off` local reused at negHandler | 200 | 19/202 | `regs=6/0`, retail's count |
| **`do { } while (0)` around the if/else** | 200 | **21/202** | **puts `off` in `$s1`, retail's register** |

**The load-bearing one is the fifth.** Retail's `negHandler` clears `unk90`
with `lw $v1,0x0($s2); addu $v1,$s1,$v1` — five words — while the other two
clear sites recompute the whole address (lui/addiu + the 8-instruction ×172
multiply), twenty words each. Writing all three as the same expression makes
cc1's jump optimiser TAIL-MERGE negHandler into the clear handler, because
after allocation the two blocks are `rtx_renumbered_equal_p`. One block
vanishes and the body is ~20 words short — which is most of the inherited
body's 22-word shortfall, and the old report never identified it. Giving
negHandler the cached row pointer makes the two blocks textually different,
the merge does not happen, and the body jumps from 184 to 203 words in one
build.

**The asymmetry the old report called "a register's lifetime" is right about
the mechanism and wrong about what to do with it.** It is not something to
reproduce by restructuring the whole function; it is one source expression
(`(*row)[slot]` at negHandler, `_ss_score[screen][slot]` at the other two).

## The residue: callee-saved register IDENTITY

The built body and retail agree instruction-for-instruction. What differs is
the permutation:

| value | retail | built |
| --- | --- | --- |
| `p` | `$s0` | `$s0` |
| `off` = 172*slot | `$s1` | `$s1` — **correct** |
| `row` = `&_ss_score[screen]` | `$s2` | `$s4` |
| `slot` | `$s3` | `$s2` |
| `screen` | `$s4` | `$s3` |

Both allocate exactly five call-saved registers plus `$ra` (`regs=6/0`), both
in the same order within the prologue's `sw` block — the ORDER of the saves
follows the assignment, so the prologue differs only by register number.
Retail gives the two short-lived derived addresses the LOW numbers and the two
parameters the HIGH ones; cc1 here does the opposite.

**One lever DID move it, and it is bravo's round-58 `do { } while (0)` find.**
A `do { } while (0)` around the `if (c42 > 0) { ... } else { ... }` group moves
`off` from `$s3` to `$s1`, retail's own register, and takes the body from
19/202 to 21/202. cc1 2.6.3's loop pass treats it as a real loop construct and
that reorders global allocation; **a plain `{ }` brace block in the identical
place was measured and changes NOTHING** (19/202, byte-identical output), which
independently confirms bravo's claim on a second function.

The lever saturates at one placement. Measured and all inert: a second
`do-while` around the clear/neg trailer, around negHandler alone, around
`tailCheck` alone, around the if-arm alone, and around the whole body — every
one 200 words / 21/202, except the whole-body wrap which is worse (9/202).

Eleven further source shapes were measured against the remaining three-way
permutation and NONE moved it: declaration order `row`/`off` (both ways),
declaring the out-params first, `off` as `u32`, `s32` parameters with `(s16)`
casts, `off` folded into the `p` expression, `row[0]` vs `*row` in the `p`
expression, a late-assigned `p`, and `row` declared last. The residual
permutation — `row` needs to move up two priority places past `slot` and
`screen` — is stable across all of them.

Per **CLAUDE.md HARD RULE 6 this is a STALL**: a register-identity mismatch is
not to be closed with `register T v asm("$N")` or an operand constraint, and
the GTE exception does not apply — every instruction here has an ordinary C
spelling.

## The frame: `vars=24` vs `vars=16`

Retail `addiu $sp,$sp,-0x40`, built `-0x38`. cc1's own `.frame` comment gives
the decomposition directly and is the fastest way to read a frame mismatch:

```
.frame  $sp,64,$31   # vars= 24, regs= 6/0, args= 16     <- retail's shape
.frame  $sp,56,$31   # vars= 16, regs= 6/0, args= 16     <- built
```

`frame = vars + 4*regs + args`, `args` is 16 (the o32 outgoing-argument area,
4 args max here), and `regs` now MATCHES at 6. So the whole 8-byte gap is
`vars`, and `vars` is `get_frame_size()` — declared locals PLUS reload's spill
slots, which are counted even when reload later stops using them. Measured on
a synthetic ramp through the pinned pipeline, `vars` grows in 8-byte steps
with live-value pressure while the generated code stores nothing to the new
space. The function's only address-taken locals are the two `s16` out-params
at `0x10`/`0x12`; the other 20 bytes of retail's local area are never
touched by any instruction in the function.

**So the frame size is a PRESSURE symptom, not a declaration count**, and it
is the same residue as the register permutation, read off a different
instruction. Adding locals to chase 0x40 would be fitting the symptom.

## Permuter

Gate 3, all three checks run 2026-09-19, on body A:

1. **scaffold compiles and scores** — YES, `base score = 2551`.
2. **`--debug --stack-diffs`** — Insertions 9, Deletions 11, Stack
   Differences 96. Net `-2` insns, which is the real tree's `200 vs 202`.
3. **agreement — run the BYTES way, per bravo's round-58 correction.**
   `objdump -d` of `permuter-work/Snd_decrescendo/base.o` against
   `build/src/libsnd_decre.c.o` built from the same body: 201 disassembly
   lines each, `diff` **EMPTY** — not even branch targets differ, because
   here the function is the whole object's `.text`. **AGREE.**

   ```sh
   OD=tools/binutils/bin/mipsel-linux-gnu-objdump
   for o in build/src/libsnd_decre.c.o permuter-work/Snd_decrescendo/base.o; do
       $OD -d $o | sed -n '/<Snd_decrescendo>:/,/^$/p' | sed 's/^ *[0-9a-f]*:\t//'
   done   # ... diff the two
   ```

   **My first pass at check 3 was NOT this check** — it was a reasoning-level
   "the signatures agree" (scaffold net `-2` vs the real build's 2-short). It
   happened to reach the same verdict, but it could not have detected a
   scaffold artifact, which is the only thing check 3 exists to detect. Bravo
   root-caused the same substitution in the reports for `StageMap__SplitFootprintRect` and
   `IsPointOutOfBounds`, where it had blocked searches for 39 rounds. Confirmed
   here on a third function: do the bytes.

**Search: ONE bounded run, NEGATIVE.** `-j 6 --stop-on-zero --best-only`,
**232743 iterations**, ended on its own 1500s `timeout` bound (`rc=124`), **no
zero**. Best score 1475 against base 2551; candidates were written at 2340,
2255, 1710, 1510 and 1475.

**The best candidate is REJECTED as a semantic change, not adopted.** Its only
substantive mutation is an inserted `goto tailCheck;` at the end of the
`c42 > 0` arm, which BYPASSES the shared `SpuVmSetSeqVol(pk, lo, hi, 0)` call
that both arms fall into — it scores better by deleting the write-back the
function exists to perform. That is the ordinary permuter failure mode (it
mutates freely and only ever scores against the target), and it is why a
scorer number is a lead and never an answer. Nothing in the 1475/1510/1710
candidates was translatable.

**SCOPE OF THIS NEGATIVE, and it is narrower than it looks.** The scaffold was
seeded from the body BEFORE the `do { } while (0)` lever was found — its
`base score = 2551` is the 19/202 body, not body A's 21/202. Per Gate 3, a
negative is a verdict about the BODY it was measured on, so **this run says
nothing about body A.** A future round re-running the search should re-seed
from body A and re-run checks 2 and 3 first; that is a legitimately unspent
search, not a repeat of this one.

## Preserved bodies

Both need `SsScore` from `include/SsScore.h` (`unk40` is **`s16`** there,
as these bodies require) and these declarations, all already present in
`src/libsnd_decre.c`:

```c
extern s32 SpuVmSetSeqVol(s16 a0, u16 a1, u16 a2, s32 a3);
extern s32 SpuVmGetSeqVol(s32 p0, s16 *out1, s16 *out2);
#include "SsScore.h" /* SsScore, extern SsScore *_ss_score[] */
```

### Body A — 200/202 words, 21/202, `regs=6/0`. THE ONE TO BUILD ON.

Live in `src/libsnd_decre.c` inside `#if 0`. Block-for-block and
instruction-for-instruction retail; the only differences are the three-way
callee-saved permutation and the 8-byte `vars` gap.

```c
#if 0
void Snd_decrescendo(s16 a0, s16 a1)
{
    SsScore **row = &_ss_score[a0];
    s32 off = a1 * sizeof(SsScore);
    SsScore *p = (SsScore *)((u8 *)*row + off);
    s32 c42 = p->unk42;
    s32 cnt = p->unk98 - 1;
    s16 pk;
    u16 sp10, sp12;
    u16 lo, hi;

    p->unk98 = cnt;
    do {
    if (c42 > 0) {
        if ((u32)cnt % (u32)c42 != 0) {
            goto tailFinal;
        }
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 -= 1;
        if (p->unk40 < 0) {
            goto negHandler;
        }
        pk = (s16)(a0 | (a1 << 8));
        SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
        if (sp10 == 0) {
            goto clearHandler;
        }
        if (sp12 == 0) {
            goto clearHandler;
        }
        lo = sp10 + (u16)-1;
        hi = sp12 + (u16)-1;
    } else {
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 += c42;
        if (p->unk40 < 0) {
            goto negHandler;
        }
        pk = (s16)(a0 | (a1 << 8));
        SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
        if ((s32)sp10 < -(s32)p->unk42) {
            goto clearHandler;
        }
        if ((s32)sp12 < -(s32)p->unk42) {
            goto clearHandler;
        }
        lo = sp10 + p->unk42;
        hi = sp12 + p->unk42;
    }
    } while (0);
    SpuVmSetSeqVol(pk, lo, hi, 0);
    goto tailCheck;

clearHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    _ss_score[a0][a1].unk90 &= ~0x20;
    goto tailCheck;

negHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    ((SsScore *)((u8 *)*row + off))->unk90 &= ~0x20;

tailCheck:
    if (p->unk98 == 0 || p->unk40 == 0) {
        _ss_score[a0][a1].unk90 &= ~0x20;
    }

tailFinal:
    SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), &p->unk78, &p->unk7A);
}
#endif
```

### Body B — 201/202 words, 46/202, `regs=5/0`. Higher raw score, worse body.

Body A with the one negHandler line changed back from
`((SsScore *)((u8 *)*row + off))->unk90 &= ~0x20;` to
`(*row)[a1].unk90 &= ~0x20;` — i.e. dropping the explicit `off` reuse. That
costs the call-saved `off` register (`regs` falls to 5, away from retail's 6)
and makes negHandler recompute the whole address instead of using retail's
5-word form, but leaves the body 1 word short instead of 2, which realigns the
tail and roughly doubles the raw score. Recorded so the 46/202 is not lost and
so nobody re-derives it as a discovery.

```c
#if 0
void Snd_decrescendo(s16 a0, s16 a1)
{
    SsScore **row = &_ss_score[a0];
    s32 off = a1 * sizeof(SsScore);
    SsScore *p = (SsScore *)((u8 *)*row + off);
    s32 c42 = p->unk42;
    s32 cnt = p->unk98 - 1;
    s16 pk;
    u16 sp10, sp12;
    u16 lo, hi;

    p->unk98 = cnt;
    do {
    if (c42 > 0) {
        if ((u32)cnt % (u32)c42 != 0) {
            goto tailFinal;
        }
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 -= 1;
        if (p->unk40 < 0) {
            goto negHandler;
        }
        pk = (s16)(a0 | (a1 << 8));
        SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
        if (sp10 == 0) {
            goto clearHandler;
        }
        if (sp12 == 0) {
            goto clearHandler;
        }
        lo = sp10 + (u16)-1;
        hi = sp12 + (u16)-1;
    } else {
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 += c42;
        if (p->unk40 < 0) {
            goto negHandler;
        }
        pk = (s16)(a0 | (a1 << 8));
        SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
        if ((s32)sp10 < -(s32)p->unk42) {
            goto clearHandler;
        }
        if ((s32)sp12 < -(s32)p->unk42) {
            goto clearHandler;
        }
        lo = sp10 + p->unk42;
        hi = sp12 + p->unk42;
    }
    } while (0);
    SpuVmSetSeqVol(pk, lo, hi, 0);
    goto tailCheck;

clearHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    _ss_score[a0][a1].unk90 &= ~0x20;
    goto tailCheck;

negHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    (*row)[a1].unk90 &= ~0x20;

tailCheck:
    if (p->unk98 == 0 || p->unk40 == 0) {
        _ss_score[a0][a1].unk90 &= ~0x20;
    }

tailFinal:
    SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), &p->unk78, &p->unk7A);
}
#endif
```

**Body A is a strong track-1b NON_MATCHING candidate**: hand-derived, no
permuter content, block-for-block retail, and it documents the algorithm far
better than the disassembly does.

### Proposed learning

**A near-miss body that is SHORTER than retail by about the size of one
repeated block has probably been tail-merged, and the fix is a source-level
one at exactly one of the merge's inputs.** GCC 2.6.3's `jump_optimize` runs
cross-jumping AFTER register allocation and compares insns with
`rtx_renumbered_equal_p`, so two textually identical C blocks that happen to
get identical allocations collapse into one. Retail keeps them separate
whenever the surrounding CSE made their code differ — typically because one
copy is reached BEFORE some register is repurposed and so can reuse a cached
subexpression the other cannot. The tell is in the `.s` and is cheap to read:
the same C statement appearing once in a short form using callee-saved
registers and again in a long recompute form. Give the short site the cached
spelling (a `row`/offset local) and the long sites the full expression; the
merge stops and the missing block reappears. This is the SAME observation the
round-13-era report filed as "a register's lifetime, not source phrasing" and
then declined to act on, which is why it is restated here as an action.

**Second, smaller:** read a frame-size mismatch off cc1's `.frame` comment
(`vars=`/`regs=`/`args=`) before theorising about locals. It splits the gap
into "wrong number of callee-saved registers" (a real allocation difference
worth chasing) and "wrong `vars`" (spill-slot pressure, generally downstream
of the allocation and not independently fixable). Here it said `regs` matched
and the whole 8 bytes was `vars`, which retired a whole class of candidate
edits in one build.

**Third: `do { } while (0)` is a register-allocation lever, not only a
scheduling one.** Runner bravo found in this same round that a
`do { } while (0)` around a statement group is a real loop construct to cc1
2.6.3 while a plain `{ }` is not, and used it to close delay-slot residues on
`StageMap__SplitFootprintRect`. Confirmed here on a second function AND on a different kind
of residue: it re-ranked GLOBAL REGISTER ALLOCATION, moving `172*slot` into
`$s1` — retail's own register. **Run the brace-block control**: the same wrap
written `{ ... }` produced byte-identical output, which is what turns the
observation into a mechanism. Two scope limits, both measured here: it
saturates (a second `do-while` anywhere else was inert), and wrapping the
whole function body is worse than not using it at all.

**Fourth, and this one is about how the figures are recorded, not about the
compiler: "length exact" and "M/N words match" are different measurements that
read identically in a title, and on this function they rank two bodies in
OPPOSITE orders.** The pre-round-58 title said "length exact, 202/202 words"
and "18/202" — the `202/202` was the retail RANGE, not any body, and the body
it described was 22 words short. Then within round 58, body B scores more than
twice body A's raw word-match while being behind on every structural measure,
because being 1 word short rather than 2 realigns a tail full of
register-independent arithmetic. So: state the built body's length and the
raw match as two separate figures, never one; and when a body's length is
wrong, treat the raw word-match as a weak signal and compare word COUNT plus
the instruction listing instead.

## Round 97: the record is Sony's SsScore

The unit's local `Entry90902E8` (a 0xAC-byte view of `_ss_score[a][s]`) is
deleted; the unit includes `include/SsScore.h`. Evidence: Snd_decrescendo is
Sony's (pinned in `config/psyq-objects.ld`), only libsnd reads `_ss_score`,
and 0xAC is SS_SEQ_TABSIZ. Every offset the function's disassembly touches
(`+0x3E` lh, `+0x40` lhu/sh read-modify-write and lh sign test, `+0x42` lh,
`+0x78`/`+0x7A` by address, `+0x90` lw/sw, `+0x98` lw/sw) is already a
field of SsScore with the same width, so SsScore.h did not change. The local
view's `unk2B`, `unk44`, `unk4A`, `unk70`, `unk8C`, `unkA0` and
`unkA4` had no reader in this function and were dropped, not moved. The
`lhu` at `+0x40` is GCC's load for an `s16` read-modify-write, not
evidence of `u16`; the preserved bodies were written against `s16`. The SDK
check: `libsnd/decre` ships as `Snd_decrescendo` on 3.0 (text 0x474) and
3.3 (0x4B0) and as `_SsSndDecrescendo` on 3.5/3.6 (0x2AC); retail's is 0x328,
so none links. Zero bytes changed.

## Unit history (moved from the code_179d8_i.c banner, round 97)

```text
/*
 * code_179d8_i -- what is LEFT of functions 220..237 of the original
 * code_179d8 monolith after round 34 gave fourteen of its sixteen functions
 * back to Sony.  Now 0x24490..0x247B8 (vram 0x80033C90..0x80033FB8), a
 * ONE-function unit holding Snd_decrescendo alone.
 *
 * ROUND 34 (2026-09-12): 0x2397C..0x24490 is TEN linked `libsnd` objects
 * (all Psy-Q 3.3) covering ELEVEN functions, every one of which had been
 * MATCHED as C:
 *   0x8003317C SsUtGetVabHdr         libsnd/ut_gvh    (already carried Sony's name)
 *   0x80033260 SsUtGetVagAtr         libsnd/ut_gva    (was func_80033260)
 *   0x800334A0 SsSetMVol             libsnd/scsmvol
 *   0x800334F0 SsUtGetProgAtr        libsnd/ut_gpa    (was func_800334F0)
 *   0x800335FC SsVabTransBody        libsnd/vs_vtb
 *   0x800336CC SsSetMute             libsnd/scsmute
 *   0x8003370C SsVabTransCompleted   libsnd/vs_vtc
 *   0x80033738 SsSeqCalledTbyT       libsnd/sscall    (157w)
 *   0x800339AC Snd_pause             libsnd/pause
 *   0x80033A4C Snd_nextpause         libsnd/pause
 *   0x80033AB0 Snd_tempo             libsnd/tempo     (120w)
 * Their C is DELETED, not commented out.  Reclassifying eleven matched
 * functions out of the game count is the correction CLAUDE.md asks for, not a
 * regression -- they were Sony library code the whole time.  Do not write C
 * for any of them again; `python3 tools/sdkstalls.py` and
 * `.venv/bin/python3 tools/psyq_sdk.py coverage` are the evidence.
 *
 * A pure PREFIX trim, so the unit kept its name and the `c` line simply moved
 * to 0x24490.
 *
 * A SECOND RUN in the same round then took the other end.  `libsnd/replay` and
 * `libsnd/vs_vab` (0x247B8..0x2490C) are `Snd_replay`, `SsVabClose` and
 * `SsVabOpen` -- func_80033FB8, func_80034020 and func_800340B0, all three
 * previously MATCHED as C and all three now deleted from here.  That run sat
 * in the MIDDLE of what the prefix trim had left, so the slice became
 * [c][o][o][c] and the tail half became the one-function unit
 * `src/code_179d8_i_b.c` (Snd_play).  Nothing moved with it: this unit
 * never owned a rodata attach.
 *
 * `libsnd/pause` is taken from the 3.3 disc ON PURPOSE: 3.5/3.6 split that
 * module into `pause` (0xA0) + `npause` (0x64), which is the same two
 * functions but does not tile as one object.  `runs` says the same thing as
 * "libsnd/pause supersedes libsnd/npause".
 *
 * A STANDING NOTE THIS FILE CARRIED FOR SEVERAL ROUNDS IS NOW RESOLVED.  The
 * func_80033260 comment said it could not be renamed to `SsUtGetVagAtr`
 * because the name had no `= 0x8003....;` alias in
 * config/symbols.slps01556.lsdde.txt, so INCLUDE_ASM'd callers in
 * code_179d8_k.c and code_179d8_e.c carried a literal `jal func_80033260`,
 * and "config/ is not this unit's to edit".  An SDK-object conversion edits
 * exactly that file: Sony's names for all eleven are now in the symbols file
 * and `make extract` rewrote every caller's `.s`.  The rename was mechanical.
 *
 * ROUND 33 CORRECTION, KEPT BECAUSE THE LESSON OUTLIVES ITS EXAMPLE.  This
 * file's carve-time census certified func_80032D34 as "ordinary large fresh
 * ground, not blocked" -- correctly, against both live blocker screens -- and
 * it was Sony's `SsVabOpenHeadWithMode` all along; a 232-line derivation went
 * into it.  A screen measures the obstruction it was built for and says
 * nothing about the ones it was not, and a carve-time census recorded as a
 * DIRECTIVE outlives the thing it was measured against.  Round 34 is the same
 * finding at eleven times the scale: every function above passed every blocker
 * screen and every one was unmatchable by construction.  Screen with
 * `python3 tools/nearmiss.py` (which runs `sdkstalls.py` for you); do not
 * trust a transcribed census, this comment included.
 *
 * Owns no rodata: zero `jtbl_` in its disassembly, and the yaml's rodata slot
 * list names no `.rodata, code_179d8_i` line.  Both of the old
 * code_179d8_tail's jump tables went to code_179d8_k.
 *
 * Keep every function in strict ROM-address order.
 */

#include "common.h"

/* A 172 (0xAC)-byte record; _ss_score is an array of pointers to arrays of
 * these, indexed [screen][slot]-style by two signed 16-bit indices. This is
 * a reduced LOCAL view -- only the fields this unit's functions touch are
 * named. See code_179d8_f.c's own Entry90902E8 for a fuller layout of the
 * same array; each unit keeps its own independent reading, per project
 * convention (multiple local views of one struct are expected here). */
typedef struct {
    u8 pad0[0x2B];
    u8 unk2B;
    u8 pad2C[0x3E - 0x2C];
    s16 unk3E;
    s16 unk40;
    s16 unk42;
    s16 unk44;
    u8 pad46[0x4A - 0x46];
    s16 unk4A;
    u8 pad4C[0x70 - 0x4C];
    s16 unk70;
    u8 pad72[0x78 - 0x72];
    s16 unk78;
    s16 unk7A;
    u8 pad7C[0x8C - 0x7C];
    u32 unk8C;
    s32 unk90;
    u8 pad94[0x98 - 0x94];
    s32 unk98;
    u8 pad9C[0xA0 - 0x9C];
    s32 unkA0;
    u32 unkA4;
    u8 padA8[0xAC - 0xA8];
} Entry90902E8;

extern Entry90902E8 *_ss_score[];

/* unk3E, unk40, unk42, unk78, unk7A, unk98 added to Entry90902E8 above,
 * in place of existing padding -- no existing field's offset changed.
 * unk40 is loaded with `lhu` (declared u16) but sign-checked via an
 * explicit `(s16)` cast at every comparison site -- matches retail's
 * `sll 16`/`bltz` idiom for checking a 16-bit value's sign without a
 * plain `lh`.
 *
 * STALL -- see docs/match-reports/Snd_decrescendo.md for the full
 * algorithm derivation (correct, byte-verified block-by-block against
 * the asm) and the best C body reached (18/202 words, first diff at
 * word 1 -- the prologue's own `-0x40` vs `-0x38` frame size). The
 * residue is register/stack allocation, not logic. */
```
