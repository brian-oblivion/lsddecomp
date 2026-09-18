# func_80026CFC -- 3 words long (38 vs 35) -- 1/35 raw word match -- first real diff at vram 0x80026D00

> **HEAD CORRECTION, round 43 (2026-09-15): THE PERMUTER NEGATIVE IN THIS
> REPORT IS AN ARTIFACT AND MUST NOT BE READ AS EVIDENCE.** The
> "150582 iterations, best score 1643 -> 900, never zero" result was produced
> by `tools/setup-permuter.sh`'s generated `compile.sh`, which hardcodes
> `MASPSX_FLAGS` independently of the Makefile and **omits round 42's
> `--gp-symbols=config/gp-symbols.txt` and `--no-nop-mflo-mfhi`**.
>
> That matters for THIS function specifically: its body reads `D_8008A84C`,
> which **is** in `config/gp-symbols.txt`, and its retail asm carries one
> `%gp_rel` reference. Without `--gp-symbols` the permuter's baseline emits an
> absolute access where retail has a gp-relative one, so **at least one word
> could never match no matter what C the permuter produced.** The search could
> not have reached zero; it was not testing the register-allocation
> hypothesis at all. Base score, candidate scores and the iteration count are
> all measured against an unreachable target and none of them is informative.
>
> **The REGISTER-ALLOCATION class itself is NOT withdrawn** -- it rests on the
> hand-built variants and the 3-words-long / 1-of-35 funcdiff figures, which
> came from the real pinned build through `build-and-verify.sh` and are
> unaffected. What is withdrawn is only the claim that a permuter search has
> been tried and failed. **This function has, in effect, never been
> permuter-searched**, so it belongs in Gate 1b's sixth screen as
> never-searched ground rather than as exhausted ground.
>
> Independently confirmed: runner delta hit the same gap the same round on
> `func_8004DCD0`, diagnosed it, and hand-patched its own gitignored
> `compile.sh` -- so delta's 26500-iteration negative IS valid while this one
> is not. Two searches in one round, one trustworthy and one not, separated
> only by whether the runner happened to notice.
>
> The tool is NOT fixed here: changing `MASPSX_FLAGS` is a flag change and
> therefore an operator escalation (CLAUDE.md, "Escalate, do not experiment").
> Escalated in round 43's write-up.


**Unit:** code_171e0 · **Size:** 35 words (retail) · **Status:** STALLED,
class REGISTER-ALLOCATION (round 43, 2026-09-15, runner bravo).

## History

Never attempted before round 43 (round-2026-08-29-a/30-a filed only a
`gp_rel`-blocked stub, no derivation). Round 42 resolved `gp_rel`
project-wide. Round 43 derived the control flow correctly on the first try
(confirmed by the `D_8006D4AC` array in `asm/data/5DB70.data.s` ending in a
`0x00000000` sentinel, exactly matching a "call through the table until a
NULL entry" loop) but could not reproduce retail's REGISTER ALLOCATION
across roughly a dozen structurally-equivalent rewrites plus a 150582-iteration
permuter search (bounded at 600s, `-j 8 --stack-diffs --stop-on-zero
--best-only`) that reduced the permuter's own score from 1643 to 900 but
**did not close it, and did not reach zero in that time under load** --
every candidate below 1000 relied on reading `fn` before its first
assignment (undefined behavior), not a legitimate reshape.

## What it does

Dispatches on `D_8008A84C` to pick one of two "ret" objects
(`GetClass6D4E8Methods()` if `arg0 == 0x13`, else `GetVabDriverMethods()`), stores `arg0`
into `D_8008A84C`, then walks the function-pointer table `D_8006D4AC`
(14 entries + a NULL sentinel, confirmed in `asm/data/5DB70.data.s`),
calling `func_80026D88(val, ret)` before EVERY table read (including the
first, before any table entry is even inspected), and — for every NON-NULL
entry — calling that entry as `val = entry(val)` before advancing to the
next slot and repeating. All 4 callees are cross-unit: `GetClass6D4E8Methods` /
`GetVabDriverMethods` are shared with `func_80026CAC` (see that report), and the
14 table entries are ordinary game functions elsewhere in the image.

## Residue -- this is a genuine register-allocation puzzle, not a control-flow miss

Retail's compiled loop uses exactly **2** callee-saved registers (`$s0` =
the `D_8006D4AC` walk pointer, `$s1` = `ret`) plus `$ra`. The loop-carried
"call argument" value (`val`, first produced by `func_80026C9C()`, then by
each table-entry call) flows **directly through `$a0`/`$v0`**, with no
callee-saved register of its own, EVEN THOUGH it is read and written at a
control-flow JOIN with two predecessors (the initial path and the loop
back-edge):

```
.L80026D3C:
  jal   func_80026C9C
   addu $s1, $v0, $zero      ; s1 = ret (from the EARLIER branch's return,
                              ;   in the delay slot -- BEFORE this call runs)
  j     .L80026D58
   addu $a0, $v0, $zero      ; a0 = func_80026C9C()'s return (val, initial)
.L80026D4C:                  ; loop back-edge target
  jalr  $v0
   addiu $s0, $s0, 0x4       ; entry++ (delay slot)
  addu  $a0, $v0, $zero      ; a0 = the just-called entry's return (val, new)
.L80026D58:                  ; JOIN -- func_80026D88 call site, 2 predecessors
  jal   func_80026D88
   addu $a1, $s1, $zero
  lw    $v0, 0x0($s0)
  bnez  $v0, .L80026D4C
```

Every structurally-equivalent C rewrite tried in round 43 (a `for(;;)` with
`break`, an explicit `goto`-based rewrite matching this exact CFG 1:1, the
same with the formal parameter `arg0` reused as the `val` slot to try to
pin it to `$a0`'s "home" register, dropping the named `fn` variable and
calling through `*entry` directly, and typing `val`/`ret` as
`UnkFlagsObj_171e0 *` instead of `void *`) makes this project's pinned GCC
2.6.3 allocate a THIRD callee-saved register (`$s2`) for `val` and round-trip
it through TWO extra `move` instructions per production/consumption instead
of flowing straight through `$a0`/`$v0` -- consistently **38 words (3 over)**
regardless of which of these was tried. One rewrite (producing `val` in the
`for(...)` loop's own increment clause, i.e.
`for (val = func_80026C9C(); ; val = fn(val)) { ... }`) got closest, at
**38 words** with the extra register shuffle moved to a slightly different
point in the schedule -- still not zero.

**The permuter search (150582 iterations, 600s wall-clock, 8 workers) did not
close this and did not find any candidate near zero.** Its best-scoring
candidates (900 and below) all shared the same defect: they moved
`entry = D_8006D4AC;` inside the loop body and called `fn(val)` as the
ARGUMENT expression to `func_80026D88` before `fn` is ever assigned on the
first iteration -- reading an uninitialized function pointer. That is
undefined behavior, not a legitimate source reshape, and it does not
qualify as a permuter zero (it never reached zero at all). Per
CLAUDE.md/MATCHING-GUIDE.md: **not closed in 150582 iterations under load**,
not "permuter-exhausted" -- the search space is much larger than what 600s
covers and a longer or differently-seeded run might still find something.

## What this is not

- **Not a `gp_rel` residue.** `D_8008A84C`'s own load/store already matches
  (`sw $a0, %gp_rel(D_8008A84C)($gp)` reproduces correctly); the divergence
  is entirely in which registers get allocated in the walk loop.
- **Not a control-flow miss.** The branch targets, the loop shape (fall
  straight into the shared call site before ever testing the first table
  entry), and the table-walk/NULL-sentinel termination all reproduce
  exactly; only the register-to-value binding around the loop's carried
  argument differs.
- **Not fixable by `register T v asm("$N")` or an operand constraint** —
  CLAUDE.md bans exactly this kind of register-identity fix, and this is
  squarely that case (register IDENTITY differs, not instruction order), so
  it is not attempted here.

## Preserved best-effort body (38/35 words, 3 words long)

```c
extern void *D_8006D4AC[];

void func_80026D88(UnkFlagsObj_171e0 *dst, UnkFlagsObj_171e0 *src);

void func_80026CFC(s32 arg0) {
    void *ret;
    void *val;
    void *(*fn)(void *);
    void **entry;

    D_8008A84C = arg0;
    if (arg0 == 0x13) {
        ret = GetClass6D4E8Methods();
    } else {
        ret = GetVabDriverMethods();
    }
    entry = D_8006D4AC;
    for (val = func_80026C9C(); ; val = fn(val)) {
        func_80026D88(val, ret);
        fn = *entry;
        if (fn == NULL) {
            break;
        }
        entry++;
    }
}
```

(`extern s32 D_8008A84C;`, `extern void *GetVabDriverMethods(void);` and
`extern void *GetClass6D4E8Methods(void);` are declared once earlier in this file,
above `func_80026CAC`.)

## Proposed learning

**A loop-carried call ARGUMENT (not accumulator, not index) that changes
every iteration and is produced/consumed exactly once per iteration, at a
control-flow join with two predecessors, is a harder class for this pinned
GCC 2.6.3 than the "constant-argument do-while" idiom that closed
`func_80026F34` in this same unit on the first try.** The difference: when
the loop's call arguments never change across iterations (`func_80026F34`),
each argument gets its own stable callee-saved register once and is done.
When the argument itself IS the previous call's return value
(`func_80026CFC`), this compiler's register allocator — at least across
every C shape tried here — insists on giving it a permanent register too
(3 total: walk pointer, loop-invariant second argument, AND the changing
first argument) rather than retail's 2 (walk pointer, loop-invariant second
argument only, with the changing first argument flowing through
`$a0`/`$v0` directly across the join). If another runner or a future round
hits the identical shape (a `D_XXXXXXXX[]`-table walk that threads a
call's own return value back in as the next call's argument), this report's
permuter seed (`permuter-work/func_80026CFC`, gitignored but reproducible
from this report's preserved body via `tools/setup-permuter.sh`) and its
150582-iteration ceiling are the starting point, not a re-derivation from
scratch.
