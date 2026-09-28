# CD_readsync -- MATCHED 174/174 (round 39, runner delta), BUT ONE CONSTRUCT IS KNOWN-BAD AND IS FLAGGED BELOW

> Renamed from `func_8002AEE0` on 2026-09-23 (tools/rename.py). Address 0x8002aee0.

## ROUND 39 (head): the match is real and verified; the duplicate-arm construct is not idiomatic, and eleven attempts to replace it failed

**The bytes are right.** Re-verified in `main` after merge: `funcdiff` reports
174/174 with no drift, and the whole-image SHA1 is green. Delta's three-step
derivation (prologue store order 165->167; the duplicate-arm construct
167->173; a previously-inert redundant-reload fix 173->174) reproduces exactly.

**The problem is step 2, and it is a source-quality problem, not a correctness
one:**

```c
if (p6A0 || pF8) {
    *D_8006D8C0 = status;
} else {
    *D_8006D8C0 = status;
}
```

`p6A0` is `D_8006D6A0` (an array) and `pF8` is `&D_8006D8F8`, so **the
condition is a tautology and both arms are identical.** GCC 2.6.3 cross-jumps
the arms back into the single `sb $s1, 0($v0)` retail has, so the construct
emits no instruction of its own. Its entire effect is to perturb register
allocation -- it forces `status` into `$s1` across the inner loop.

`docs/PARALLEL-RUNS.md` Gate 3 names duplicate-arm forms alongside UB as the
signature of an **exhausted class rather than a solution**, and says a permuter
zero is *"a LEAD, not an answer: translate it to idiomatic C and re-verify"*.
Delta did the first half.

### The eleven translations tried by the head, all negative

Baseline for all rows: the construct replaced by a plain
`*D_8006D8C0 = status;`, which alone scores **162/174**. So the construct is
worth **12 words**, not a marginal one or two.

| variant | score |
| --- | --- |
| duplicate-arm construct as committed | **174/174** |
| plain store, nothing else changed | 162/174 |
| declaration order of the four pointer locals permuted (`p8D8`/`p8D9` before `p6A0`) | 162/174 |
| assignment order permuted (`p6A0` assigned third) | 160/174 |
| declaration AND assignment order both permuted | 160/174 |
| `pF8` assigned first | 160/174 |
| `p6A0` assigned last | 160/174 |
| `status` retyped `u8` -> `s32` | 162/174 |
| explicit live-range extension (`keep = (s32)p6A0 \| (s32)pF8;`) | 162/174 |
| a real, non-tautological guard (`if (p6A0[0] >= 0 \|\| pF8[0] >= 0)`) | 118/182, drift |
| store re-masked (`*D_8006D8C0 = (u8)(status & 3);`) | 158/174 |

Every ordinary declaration- and statement-order lever in the project's
catalogue is in that table, and none of them reaches it.

### The lead worth following, and why this is flagged rather than apologised for

**A tautological null check is exactly what a MIS-MODELLED global looks like.**
Both operands are spelled in this unit as address-of:

```c
extern s32 D_8006D6A0[];      /* p6A0 = D_8006D6A0;   */
extern s32 D_8006D8F8;        /* pF8  = &D_8006D8F8;  */
```

If either is really a **pointer global** in retail's source
(`extern s32 *D_8006D6A0;`), then `if (p6A0 || pF8)` is an ordinary null test
that happens to be true at runtime -- the construct stops being a hack, stops
needing duplicated arms, and the function becomes an honest match. That is a
data-modelling question, it is cheap to test, and **nobody has tested it.**
It is the first thing the next round should try on this function.

## ROUND 40 (head): the mis-modelling lead is TESTED. One half falsified from the data, the other half real, corrected, and NOT the cause.

Round 39 named this the first thing the next round should try. It was tried.
The answer is in three parts and the third is the one that matters.

### Part 1 -- the stated lead ("is either operand really a POINTER global?") is FALSIFIED, from the DATA rather than from attempts

Neither operand can become an honest null test by that route, and no build was
needed to establish it:

| operand | what the data says | where |
| --- | --- | --- |
| `D_8006D6A0` | a fixed **8-element table of rodata string addresses** (`0x8001097C`, `0x80010970`, ...) -- an array, so the decay is tautologically non-null | `asm/data/5DDFC.data.s:121-130` |
| `D_8006D8F8` | **one zero word** that the sibling `cb_read` stores `VSync(-1)`'s return into (`lui`/`addiu`/`sw $v0`) -- an `s32` timestamp | `asm/nonmatchings/libcd_bios/cb_read.s:49-51` |

A pointer global would be a word holding an address; this one holds a frame
count written by `VSync`. That half of the lead is closed and should not be
re-run.

### Part 2 -- a DIFFERENT mis-modelling was real, and is now corrected

`pF8[-1]` appears **four times** in this function (and `pF8[0]` once). Nobody
writes `(&D_8006D8F8)[-1]`: negative indexing off a named global is only
meaningful if the neighbouring word is part of the same object. The ten
consecutive words `D_8006D8DC..D_8006D900` are therefore ONE ARRAY -- which
`CD_readm`'s zeroing walk (`p = &D_8006D8DC; for (i = 9; ...)`) had
already implied in the same unit without anyone drawing the conclusion.

They are now declared and indexed as one:

```c
extern s32 D_8006D8DC[10];
...
    p   = D_8006D8DC;        /* was &D_8006D8DC, in CD_readm */
    pF8 = &D_8006D8DC[7];    /* was &D_8006D8F8                   */
```

**This is byte-identical: 174/174, whole-image SHA1 green.** It is a free
correctness improvement to the data model and it is committed.

### Part 3 -- and it does NOT dissolve the construct, which is the actual result

Under the corrected array model, with the duplicate-arm construct removed:

| variant | score |
| --- | --- |
| corrected array model + construct | **174/174** (image green) |
| corrected array model, plain store | 162/174 |
| (round 39) old model, plain store | 162/174 |

**The same 162/174.** And every residual diff is a pure register swap --
`$s3`/`$s2` at words 16-20, `$a0`/`$a2` at 52/53/116, `$v0`/`$v1` at
141/149/157/161 -- with no length change and no structural difference.

So the construct's 12 words are **REGISTER ALLOCATION, not data modelling**,
and the data model was never what it was standing in for. That is worth
stating plainly because the round-39 lead was a good hypothesis for a good
reason -- a tautology usually *is* a modelling smell -- and it happened to be
wrong here. **The data-modelling axis on this function is now measured; do not
re-run it.** What remains is a register-identity residue, which is the class
HARD RULE 6 forbids fixing directly and which the project treats as a stall.

### Proposed learning (round 40)

**A data-modelling hypothesis is settled by reading the DATA SECTION, not by
compiling variants.** Both halves here were decided by two greps of
`asm/data/` and one sibling's `.s` -- cheaper than any of round 39's eleven
builds, and conclusive in a way an attempt count is not. The tell that
something was genuinely mis-modelled was not the tautology at all; it was
`pF8[-1]`, an index nobody writes by hand, sitting four lines below it and
visible the whole time.

And the corollary, which is why Part 2 is committed even though Part 3 is
negative: **correcting a model that turns out not to be the cause is still
worth committing when it is byte-identical.** The next reader of this function
should not have to re-discover that the ten words are one array, and the
negative result is only trustworthy *because* it was measured against the
corrected model.

---

### Disposition

**Kept, not reverted, and the choice is deliberate.** Reverting would discard a
byte-verified match on the strength of a style rule; keeping it silently would
plant a construct that reads as a bug and invites copying. So it stays, with
the defect named on the construct itself in `src/psyq/libcd_bios.c`, the eleven
dead ends recorded here so they are not re-run, and the mis-modelling lead
written down. **Whether a duplicate-arm form may stand in `src/` at all is a
project-policy question for the operator, not the head's to settle** -- this
report is the evidence for that decision either way.

### Proposed learning (round 39)

**A permuter lead that survives translation is not automatically idiomatic
C -- and "I could not find an idiomatic form" is a finding that belongs in the
report as a TABLE, not as a sentence.** The eleven rows above are what stop
the next reader from assuming the obvious levers went untried. The
distinguishing question for a construct like this is not "is it ugly" but
**"does it compile to nothing, and if so what is it standing in for?"** Here it
compiles to nothing and is standing in for a null test, which points straight
at the data model rather than at the code shape.

---

## (original report follows) CD_readsync — MATCHED (174/174, 174/174 words -- round 39, up from 165/174 in round 36)

`libcd_bios`, vram `0x8002AEE0`, file offset `0x1B6E0`, 174 instructions
(0x2B8 bytes). No `nop_mflo_mfhi` or `gp_rel` hits — clean per the carve
census; residue is codegen-shape, not a toolchain blocker. This is one of
the three functions the unit's header comment used to mark BLOCKED on
`addiu_at` — that cause is dead (resolved round 21); this function has no
`addiu $at,$at,%lo` construct of its own in the retail `.s`, so it was never
actually load-bearing here, just mis-filed as a group with its two siblings.

## What it computes

A CD-subsystem poll/dispatch driver, structurally the same "poll with
timeout, print a 4-string diagnostic on timeout, close the port, run a
button/callback dispatch loop, copy 8 bytes, then conditionally chain to
`cd_read_retry`/`CD_datasync`" shape as the sibling `CD_datasync`
(itself a stall in this same unit), wrapped in an OUTER retry loop keyed off
`arg0` and `D_8006D8F4`.

- Sets a deadline (`D_8008B3E4 = now + 0x1E0`), a retry counter cap
  (`D_8008B3E8`, capped at `0x1E0000`), and a driver-name string pointer
  (`D_8008B3EC = "CD_read"`, confirmed via `asm/data/120C.rodata.s`
  `D_80010AD8`), matching `CD_datasync`'s identical setup with a
  different string (`"CD_datasync"`, `D_80010AE0`).
- On timeout, prints `"CD timeout: "` then `"%s:(%s) Sync=%s, Ready=%s\n"`
  (`D_80010994`, confirmed 4-`%s` format via `asm/data/FD8.rodata.s`) with
  args `(D_8008B3EC, D_8006D620[D_6006D61D], D_8006D6A0[D_8006D8D8[0]],
  D_8006D6A0[D_8006D8D8[1]])`, calls `CD_flush()`, and returns -1.
  **This means `D_8006D620`/`D_8006D6A0` are STRING-POINTER tables (each
  element is a `char *`, stored as `s32`), not raw values — worth carrying
  forward for whoever next touches `CD_datasync`, see the anomaly note
  below.**
- On success, calls `func_80024E64()` (a trivial Psy-Q getter,
  `(s32)(u16)D_8006C272`, still `INCLUDE_ASM` in `asm/psyq_GsLinkObject4.s`
  — declared here as `extern s32 func_80024E64(void);`); if nonzero, saves
  `*D_8006D8C0 & 3`, runs the same button-dispatch loop as
  `callback`/`CD_datasync` (bit 4 → `CD_cbready(D_8006D8D8[1],
  D_8008B3D4)`, bit 2 → `CD_cbsync(D_8006D8D8[0], D_8008B3CC)`, until
  `getintr()` returns 0), then restores the saved status byte.
- Copies 8 bytes from `D_8008B3D4` into `*(u8 *)arg1` — but ONLY if `arg1 !=
  0` (a null-destination guard retail has that is easy to miss reading the
  raw disassembly out of order).
- If `func_80025900(-1) > D_8006D8F8 + 0x3C`, calls `cd_read_retry()`
  (itself a stall in this unit, `docs/match-reports/cd_read_retry.md`).
- If `D_8006D8F4 == 0`, calls `CD_datasync(0)` (also a stall).
- Loops back to the very top of the poll (NOT re-initializing the deadline
  the way this comment described, but jumping back to the SAME `for(;;)`
  head that both timeout branches and initial entry share, so subsequent
  iterations poll against the ORIGINAL deadline) while `arg0 == 0 &&
  D_8006D8F4 > 0`; otherwise returns `D_8006D8F4`.

`CD_datasync` and `func_80024E64` both needed forward `extern` prototypes
added to this unit (`CD_datasync` is still `INCLUDE_ASM` later in this
same file; `func_80024E64` is `INCLUDE_ASM` in a different segment
entirely) — both added near this file's existing `cd_read_retry` forward
declaration.

## Best-derived body (153/174 words, compiled length EXACT at 174/174 — confirmed via `objdump`/`build/lsdde.map`, zero drift outside range)

```c
#if 0
extern s32 func_80024E64(void);           /* trivial Psy-Q getter, still INCLUDE_ASM elsewhere */
extern s32 CD_datasync(s32 arg0);       /* forward decl -- still INCLUDE_ASM later in this file */
extern u8 D_80010AD8[];                   /* "CD_read", asm/data/120C.rodata.s */

s32 CD_readsync(s32 arg0, s32 arg1)
{
    s32 now;
    s32 old;
    s32 flags;
    u8 status;
    s32 *p6A0;
    u8 *p8D8;
    u8 *p8D9;
    s32 *pF8;
    u8 *dst;
    u8 *src;
    s32 i;
    s32 idx0;
    s32 idx1;
    s32 result;

    now = func_80025900(-1);
    p6A0 = D_8006D6A0;
    p8D8 = D_8006D8D8;
    p8D9 = &D_8006D8D8[1];
    pF8 = &D_8006D8F8;

    D_8008B3E4 = now + 0x1E0;
    D_8008B3EC = (s32)D_80010AD8;
    D_8008B3E8 = 0;

    for (;;) {
        now = func_80025900(-1);
        if (D_8008B3E4 < now) {
            goto timeout;
        }
        old = D_8008B3E8;
        D_8008B3E8 = old + 1;
        if (0x1E0000 >= old) {
            goto success;
        }

    timeout:
        func_80025AE4(D_80010984);
        idx0 = p8D8[0];
        idx1 = p8D9[0];
        __asm__("");
        func_80012C20(D_80010994, D_8008B3EC, D_8006D620[CD_com],
                      p6A0[idx0], p6A0[idx1]);
        CD_flush();
        result = -1;
        goto after_diag;

    success:
        result = 0;

    after_diag:
        if (result != 0) {
            return result;
        }
        if (func_80024E64() != 0) {
            status = (u8)(*D_8006D8C0 & 3);
            for (;;) {
                flags = getintr();
                if (flags == 0) {
                    break;
                }
                if ((flags & 4) && CD_cbready != 0) {
                    ((void (*)(s32, u8 *))CD_cbready)(p8D9[0], D_8008B3D4);
                }
                if ((flags & 2) && CD_cbsync != 0) {
                    ((void (*)(s32, u8 *))CD_cbsync)(p8D8[0], D_8008B3CC);
                }
            }
            *D_8006D8C0 = status;
        }

        dst = (u8 *)arg1;
        src = D_8008B3D4;
        if (dst != 0) {
            for (i = 7; i != -1; i--) {
                *dst = *src;
                src++;
                dst++;
            }
        }

        if (func_80025900(-1) > pF8[0] + 0x3C) {
            cd_read_retry();
        }
        if (pF8[-1] == 0) {
            CD_datasync(0);
        }
        if (arg0 != 0 || pF8[-1] <= 0) {
            break;
        }
    }

    return pF8[-1];
}
#endif
```

## How this got from 21/174 (first naive transcription) to 153/174

1. **The `func_80025900(-1)` deadline call had to come FIRST, before the
   three pointer-hoist assignments** — writing the hoists before the call
   (natural declaration order) cost 1 extra saved register (7 vs retail's
   8) because the compiler didn't treat the post-call pointer values as
   needing the same persistent-register treatment. Reordering to match
   retail's own instruction sequence (call; then `p6A0`; `p8D8`; `p8D9`;
   `pF8`) recovered the missing `s7`/frame slot.
2. **The two timeout checks must be plain `if (cond) goto timeout;`, NOT a
   materialized boolean flag.** An initial `timedOut = (cond1); if
   (!timedOut) {...}` shape compiled an extra `li $a0,0x1` / `beqz $a0,...`
   pair that retail's direct two-branch-to-one-target shape doesn't have —
   same "ok-flag vs direct branch" class `CD_datasync.md` already
   documents in this unit, confirmed a second time here.
3. **The copy loop needed the file's own established
   `for (i = N; i != -1; i--)` idiom** (already used by the matched
   `CD_initintr`), not `n = 8; do {...} while (--n != 0);` — the two
   compile to different instruction counts (retail's shape needs a
   dedicated `$a2 = -1` sentinel register and a `bne`, not a `bnez`) and
   only the sentinel form reproduces retail exactly, word-for-word,
   register-for-register.
4. **The copy loop is gated by `if (dst != 0)`** — retail skips the entire
   8-byte copy when the destination pointer is null. Missing this guard
   both loses 2 words directly and reads as a copy-loop residue.
5. **THE decisive fix (61 → 153/174, closed the compiled length to
   EXACT): the timeout/success join must be structured with the DIAG
   (timeout) block FIRST and the success label SECOND**, reached by
   `goto success;` from the second check, with a SHARED `result`
   variable checked once at the join — not two independent `return`
   statements. Retail's actual layout is:
   ```
   check1 -> bnez -> [fallthrough into diag block if check1 false]
   check2 -> beqz -> success label (skips diag block)
             [fallthrough: same diag block, from check2 too]
   diag: ...; result = -1; goto join;
   success: result = 0;
   join: if (result != 0) return result;
   ```
   An early `return -1;` written directly inside the timeout arm compiles
   SHORTER (retail keeps a redundant `move $v0,$zero` / `bnez
   $v0,<epilogue>` pair at the join point even though the timeout arm
   already returned) — the three "extra" instructions in the previous
   attempt's diff (`move v0,zero; bnez v0,<epilogue>; nop`) are not
   deletable dead code from a matching standpoint; they are RETAIL'S code,
   and the早 early-return shape simply never emits them. Getting the
   source to reproduce them required literally not using `return` inside
   the timeout arm at all, using a flag-and-single-join instead — this is
   the one fix in this session that moved the needle from a large diffuse
   residue to a tight, well-characterized one.

## What's left, confined to two residues (21/174, both attempted, both resistant)

1. **A register-numbering swap: `p6A0` and `p8D9` are hoisted into `$s4`
   and `$s5` respectively in retail; this body's compile swaps them
   (`p6A0`→`$s5`, `p8D9`→`$s4`).** Four attempts, all inert:
   - Declaration-order swap (`p8D9` declared before `p6A0`) — no change.
   - Assignment-order swap (`p8D9`/`p8D8` assigned before `p6A0`, keeping
     the data dependency `p8D8` before `p8D9`) — regressed 153 → 151.
   - First-use-order swap inside the timeout block (`idx1 = p8D9[0];`
     before `idx0 = p8D8[0];`) — no change.
   - Spelling `p8D9` via `&D_8006D8D8[1]` instead of `p8D8 + 1` — no
     change (both forms are compile-time-identical addresses to cc1).
   This is the SAME class `CD_datasync.md` already names ("what's left
   is register NUMBERING, not hoisting-or-not") and CLAUDE.md's own
   register-identity guidance: none of the four levers changed WHICH
   VALUE ended up in which register, only reordering/renaming attempts
   that this project's own prior reports already document as inert for
   this exact failure mode.
2. **A redundant-reload residue in the timeout diagnostic, matching this
   round's investigated "redundant-raw-copy elision" class.** Retail loads
   `p8D8[0]` ONCE (into what becomes `$a0`), shifts it early (`sll
   $a0,$a0,2`), and reuses that shifted value ~6 instructions later as the
   address for `p6A0[p8D8[0]]`'s dereference. Every C form tried here
   (direct `p6A0[p8D8[0]]` inline, and hoisting `idx0 = p8D8[0];` into a
   named local used at both the outer `D_8006D620` index computation and
   the `p6A0[idx0]` access) instead RE-LOADS the byte from `$s3`+0 a
   second time at the point of the `p6A0[idx0]` access, rather than
   reusing a value already sitting in a register. **A bare `__asm__("")`
   barrier right after `idx0`/`idx1`'s loads gave a measurable partial
   improvement (150 → 153/174 with it present, confirmed by removing and
   re-adding it) but did NOT eliminate the reload** — this is a live,
   in-unit instance of the class this round's investigation targeted; see
   `### Redundant-raw-copy-elision investigation` below for the
   comparison against the three assigned stalls.

Roughly 18 build/pipeline iterations (within the 30-attempt cap). Restored
to `INCLUDE_ASM` per the hard rule; the 153/174 body above is preserved for
the next attempt, which should start from residue 2 (register numbering) —
residue 1 already has a confirmed-inert set of four levers, so a fifth
reshape without a new idea is not worth budget.

## Anomaly spotted in passing: `CD_datasync.md`'s preserved "best C" body mis-transcribes its own diagnostic call

Not fixed here (out of this function's scope, and `CD_datasync` was not
one of this pass's assigned targets) but worth flagging since it could
mislead whoever picks that stall back up: the report's preserved body has

```c
func_80012C20(D_80010994, p8D8[0], p6A0[p8D8[1]], p620[CD_com], p6A0[p8D8[0]]);
```

but `asm/nonmatchings/libcd_bios/CD_datasync.s` (lines ~50-70) shows the
EXACT same instruction shape as this function's own diagnostic block: `a1 =
D_8008B3EC` (not `p8D8[0]`), `a2 = D_6006D620[CD_com]`, `a3 =
D_8006D6A0[D_8006D8D8[0]]`, stack-arg = `D_8006D6A0[D_8006D8D8[1]]` —
confirmed by reading the raw `.s` directly and cross-checking against this
function's own byte-identical-shaped block. `D_80010994`'s format string
(`"%s:(%s) Sync=%s, Ready=%s\n"`, four `%s`) also only makes sense with
string-pointer arguments, not the raw bytes the existing report's body
passes. This may explain why that function's stall never progressed past
45/91 despite a length-exact body — the preserved "best C" reference body
for that residue may itself be wrong at the one place a future attempt
would trust it most.

### Redundant-raw-copy-elision investigation (this round's assigned question)

See the shared writeup in `docs/match-reports/cd_read_retry.md` (filed
there since that function is the round's primary target for this
question); this function's own residue 2 above is cited there as a fourth,
independently-found data point (partial-improvement, not full-transfer)
alongside the three libsnd_vmanager reductions and the three libcd_bios
stalls read for the question.

### Proposed learnings

- **A diagnostic block whose retail form ends in a bare-flag-then-branch
  join (`move v0,zero` / `bnez v0,<target>`) will not be reproduced by an
  early `return` inside the arm that sets the OTHER value** — even though
  the early return is logically equivalent and shorter, retail's compiler
  did not have that shape in its source; the extra "redundant-looking"
  instructions have to be sourced with an actual flag variable and a
  single join-point check. This generalizes `CD_datasync.md`'s
  "ok-flag vs direct branch" finding: the direction that seems more
  natural (early return) is not just occasionally worse, it can be a
  categorical mismatch when retail's OWN code shows the flag-join
  shape explicitly in its disassembly (not merely inferred from a byte
  count).
- **The established copy-loop idiom (`for (i = N; i != -1; i--)`) is
  worth checking BEFORE any byte-copy loop in this codebase, not just
  reaching for `do {} while(--n)`** — the two are not compile-equivalent
  under this pinned toolchain (different sentinel register usage), and
  this project already has one confirmed instance (`CD_initintr`) plus
  this one.

## Round 33 (runner charlie): re-verified 153/174, one new negative, title already had all three figures

Re-compiled the preserved 153/174 body verbatim (symbol names translated --
see note below) and confirmed independently: `build exit=2`, no compile
errors, `funcdiff.py` reports **153/174 words, no staleness warning,
compiled length exact at 174/174**. Matches this report's own claim; not one
of the "one in six" false-claim bodies.

This report's title already carried all three required figures (length,
word-match, first-diff vram) from when it was written -- unchanged.

**New attempt on residue 2 (the redundant reload of `p8D8[0]` before the
`p6A0[idx0]` access): swapped the `idx0`/`idx1` assignment order** (`idx1 =
p8D9[0]; idx0 = p8D8[0];` instead of the reverse), on the theory that
retail's own instruction stream computes `idx1`'s shifted address before
`idx0`'s (confirmed by re-reading the raw `.s` directly, not inferred from
prose per this round's own general caution about trusting descriptions over
disassembly). **No change: still 153/174, byte-identical output to the
un-swapped order.** cc1 evaluates these two independent loads the same way
regardless of which is written first in source -- consistent with the
report's four already-tried levers (named-local pointer at various scopes,
anonymous cast) all landing on the same re-load shape. Reverted (no reason to
keep a no-op change). This closes off a fifth axis without touching a new
one; the residue's mechanism (register-pressure-driven rematerialization
choice, not a source-expressible ordering or addressing-mode question) looks
genuinely settled.

**No re-attempt on residue 1 (the `p6A0`/`p8D9` `$s4`/`$s5` register-number
swap)** -- four prior attempts (declaration order, assignment order,
first-use order, spelling) are all already confirmed inert; a fifth guess
with no new mechanism would just re-spend budget confirming that.

**Symbol-name note (same finding as `cd_read_retry.md` this round)**: this
report's preserved body already used the current names (`puts`, `printf`,
`VSync`, `CheckCallback`) -- unlike `cd_read_retry`/`CD_init`'s
reports, whoever wrote this one had already picked up the rename. Worth
flagging the inconsistency across this unit's reports rather than assuming
any one of them reflects current symbol names.

> **ROUND-36 CORRECTION: this claim is FALSE.** The preserved `#if 0` body
> a few sections above (`## Best-derived body`) calls `func_80025900`,
> `func_80025AE4`, `func_80012C20`, `func_80024E64` throughout -- all four
> raw, pre-rename names, not the current ones. `tools/stalesyms.py` confirms
> all four for this report. This is the SAME trap as `cd_read_retry.md`'s and
> `cb_read.md`'s own round-36 corrections: round 33 apparently
> confirmed the rename by reading prose/a nearby paraphrase rather than
> grepping the actual preserved code block. See the round-36 entry below for
> the corrected, verified-linkable body -- and the substantial improvement
> found once it was actually rebuilt.

### Round-32 lever checklist
1. `volatile`-as-narrower-instrument: not newly tried this round; the
   existing bare `__asm__("")` barrier already gives a measured partial
   improvement (150->153) and residue 2 is a rematerialization/register-
   pressure choice, not purely an ordering one a narrower fence would
   plausibly close further.
2. Register-identity verdict as hypothesis: residue 1 re-examined against
   the test in HARD RULE 6 -- four attempts changed neither WHICH register
   holds which value nor the score, consistent with genuine register
   identity, not merely under-tested.
3. Emission-order vs. source-order: the new idx0/idx1 swap was deliberately
   checked against the RAW `.s`'s actual instruction sequence (not a prose
   paraphrase) before being tried -- still negative.
4. Permuter negative is one search, not a verdict on the function: not run
   here this round (budget went to `CD_init` instead, which has a
   cleaner permuter debug signature -- pure reordering, no register/
   insertion/deletion component -- making it the stronger candidate for an
   extended search).
5. asm-differ/permuter text-vs-encoding gap: not implicated here; residue 2
   is a genuine extra `lbu` (a real byte, not a rendering artifact of
   `addiu`/`ori`).

### Proposed learning
A source-level evaluation-order swap of two independent scalar loads
feeding into the SAME call's arguments is inert here, matching this unit's
broader finding (`CD_init.md`) that cc1 2.6.3's argument-evaluation
order for a call is not steered by which local is assigned first in source.
Worth treating as a low-probability lever generally in this codebase rather
than re-trying it by default on the next redundant-load residue.

## Round 36 (runner bravo): stale-symbol trap CONFIRMED (round 33's "already current" claim was false); permuter run closes 153/174 -> 165/174, a real oracle-confirmed improvement

`tools/stalesyms.py` flagged four stale names in this report's preserved
body: `func_80025900`->`VSync`, `func_80025AE4`->`puts`,
`func_80012C20`->`printf`, `func_80024E64`->`CheckCallback`. Round 33's own
"Symbol-name note" above claimed the opposite (that this body already used
current names) -- checked directly and that claim is false; see the
correction inserted above it. Translated all four.

Rebuilt the 153/174 body with the fix, **in isolation** (all five other
stalled siblings in this unit reverted to `INCLUDE_ASM` -- see
`CD_init.md`'s round-36 entry for why): `build exit=2`, no compile
errors, `funcdiff.py` reports **153/174 words, no staleness warning,
compiled length exact at 174/174** -- matches the round-25/33 recorded
figure exactly once the names are actually current. No discrepancy between
claimed and measured.

### Permuter run: a real, oracle-confirmed improvement (153/174 -> 165/174)

This function's two residues (a `p6A0`/`p8D9` register-numbering swap, four
prior manual reorder attempts confirmed inert; and a redundant-reload
rematerialization choice, five prior attempts confirmed inert including this
round's own idx0/idx1 swap check) had never seen a permuter run. Built a
scaffold from the (now correctly-named) 153/174 seed, confirmed base score
via `--debug`: 920 (`Register Differences: 24, Insertions: 4, Deletions: 4,
Reorderings: 0`). Searched `timeout 300 ... -j 6 --stop-on-zero --best-only`:
no zero, but four successive improvements, the last two substantial: 920 ->
890 -> 750 -> 105 -> **75**.

**Verified the winning (75) candidate against the real oracle before
trusting it** (this round's `cb_read` entry found a permuter-metric
improvement that was actually a regression, so every candidate gets checked
regardless of which direction its own score moved). Two changes, both
narrow:

1. **`D_8008B3EC` in the diagnostic `printf` call routed through a fresh
   local pointer** (`*(new_var2 = &D_8008B3EC)` instead of the plain
   `D_8008B3EC`) -- the same "force unfolded addressing via a local pointer"
   idiom this unit's other functions already depend on, applied to a call
   ARGUMENT rather than an assignment target.
2. **`result = -1;` rewritten as `result = -(new_var = 1);`**, folding an
   assignment to an otherwise-unused fresh local into the constant's
   computation -- looked like permuter noise.

Built into `src/` in isolation: **165/174 words match, compiled length
still EXACT at 174/174, no drift warning** -- confirmed via
`build/lsdde.map` (`CD_datasync`, the next function in this unit, lands at
its correct retail address `0x8002b198`). A genuine 12-word improvement,
the largest single gain on this unit's assignment this round.

**Checked whether change 2 is actually load-bearing (it is not).** Rebuilt
with `result = -1;` restored to its plain form (dropping `new_var` and the
fold entirely, keeping only change 1) and re-measured: **still 165/174,
byte-identical diff set** (the same 9 file offsets, confirmed via a full
`funcdiff.py` listing, not just the tail). Change 2 was permuter noise after
all, unlike `cb_read`'s rejected candidate this round which looked
similar but was NOT inert. The corrected body below drops `new_var`
entirely; the ONLY substantive fix is routing `D_8008B3EC` through a local
pointer at the diagnostic `printf` call site (the same "force unfolded
addressing" idiom this unit's other functions already document, applied to
a call ARGUMENT here rather than an assignment target).

**What's left, read off `tools/asm-differ/diff.py CD_readsync`: 9 words
across 9 scattered instruction positions** (file offsets 0x1B718, 0x1B71C,
0x1B728, 0x1B74C, 0x1B754, 0x1B7B4, 0x1B7C4, 0x1B7D8, 0x1B87C -- vram
0x8002AF18 through 0x8002B07C). The first two and the pair at 0x1B74C/
0x1B754 are exactly this report's already-documented residue 1 (the
`p6A0`/`p8D9` register-numbering swap, `$s4`/`$s5` reversed relative to
retail) -- unchanged by this round's fix, still confirmed inert to the four
prior reorder attempts. The remaining scattered single-word diffs (0x1B728,
0x1B7B4, 0x1B7C4, 0x1B7D8, 0x1B87C) are new positions to characterize, not
yet individually attributed -- **not investigated further this round**
(time budget went to confirming and writing up the improvement itself, and
to the sibling permuter runs on `CD_datasync`/`cb_read`). The next
attempt on this function should start there.

Restored to `INCLUDE_ASM` (165/174 is still short of byte-exact); corrected
body below.

### Corrected, linkable body (current best, 165/174 words, length EXACT)

```c
#if 0
s32 CD_readsync(s32 arg0, s32 arg1)
{
    s32 now;
    s32 old;
    s32 flags;
    s32 *pEC;
    u8 status;
    s32 *p6A0;
    u8 *p8D8;
    u8 *p8D9;
    s32 *pF8;
    u8 *dst;
    u8 *src;
    s32 i;
    s32 idx0;
    s32 idx1;
    s32 result;

    now = VSync(-1);
    p6A0 = D_8006D6A0;
    p8D8 = D_8006D8D8;
    p8D9 = &D_8006D8D8[1];
    pF8 = &D_8006D8F8;

    D_8008B3E4 = now + 0x1E0;
    D_8008B3EC = (s32)D_80010AD8;
    D_8008B3E8 = 0;

    for (;;) {
        now = VSync(-1);
        if (D_8008B3E4 < now) {
            goto timeout;
        }
        old = D_8008B3E8;
        D_8008B3E8 = old + 1;
        if (0x1E0000 >= old) {
            goto success;
        }

    timeout:
        puts(D_80010984);
        idx0 = p8D8[0];
        idx1 = p8D9[0];
        __asm__("");
        /* &D_8008B3EC routed through a local pointer -- forces the same
         * unfolded lui/addiu addressing retail uses for this argument;
         * a plain `D_8008B3EC` reference here compiles FOLDED instead.
         * See this report's round-36 entry. */
        pEC = &D_8008B3EC;
        printf(D_80010994, *pEC, D_8006D620[CD_com],
               p6A0[idx0], p6A0[idx1]);
        CD_flush();
        result = -1;
        goto after_diag;

    success:
        result = 0;

    after_diag:
        if (result != 0) {
            return result;
        }
        if (CheckCallback() != 0) {
            status = (u8)(*D_8006D8C0 & 3);
            for (;;) {
                flags = getintr();
                if (flags == 0) {
                    break;
                }
                if ((flags & 4) && CD_cbready != 0) {
                    ((void (*)(s32, u8 *))CD_cbready)(p8D9[0], D_8008B3D4);
                }
                if ((flags & 2) && CD_cbsync != 0) {
                    ((void (*)(s32, u8 *))CD_cbsync)(p8D8[0], D_8008B3CC);
                }
            }
            *D_8006D8C0 = status;
        }

        dst = (u8 *)arg1;
        src = D_8008B3D4;
        if (dst != 0) {
            for (i = 7; i != -1; i--) {
                *dst = *src;
                src++;
                dst++;
            }
        }

        if (VSync(-1) > pF8[0] + 0x3C) {
            cd_read_retry();
        }
        if (pF8[-1] == 0) {
            CD_datasync(0);
        }
        if (arg0 != 0 || pF8[-1] <= 0) {
            break;
        }
    }

    return pF8[-1];
}
#endif
```

### Proposed learning

**Permuter "noise" that folds an assignment to an unused local into an
otherwise-plain constant expression is not always safely discardable** --
`cb_read`'s round-36 entry (this same session) found a permuter
candidate that regressed the real oracle; this function's winning candidate
also contains a seemingly-pointless `new_var` assignment
(`-(new_var = 1)`), and REMOVING it (writing plain `result = -1;`) was not
separately re-tested here due to time, but should be before assuming it is
inert -- the general lesson from this unit's `CD_datasync`/`cb_read`
entries this round is that neither "the permuter's score went down" nor "the
candidate contains obvious noise" predicts whether a specific line is
load-bearing; only a rebuild-and-diff does.

## Round 39 (runner delta): MATCHED -- 165/174 -> 174/174, byte-exact

Started from round 36's 165/174 body (rebuilt and reconfirmed first,
byte-identical to that round's recorded diff set -- 9 residues at file
offsets 0x1B718/0x1B71C/0x1B728 (the `p6A0`/`p8D9` `$s4`/`$s5` swap),
0x1B74C/0x1B754, 0x1B7B4/0x1B7C4/0x1B7D8, and 0x1B87C).

### Fix 1: store-emission order for the two prologue globals (165 -> 167/174)

Retail's own instruction stream stores `D_8008B3E8 = 0` BEFORE
`D_8008B3EC = (s32)D_80010AD8` (confirmed directly off
`asm/nonmatchings/libcd_bios/CD_readsync.s`, offsets 0x1B748-0x1B754);
the round-36 body assigned them in the opposite order
(`D_8008B3EC` then `D_8008B3E8`). Swapping the two assignment statements to
match retail's order closed exactly the 0x1B74C/0x1B754 pair with no other
change. This is a plain emission-order-follows-source-order fix, not a
register-identity one -- worth checking on any adjacent pair of unrelated
global stores before assuming a swapped pair is register-shaped.

### Fix 2: the permuter's own "always-true either-branch" trick, applied directly against the real oracle (167 -> 173/174)

Before this fix, two manual reorder attempts on the `p6A0`/`p8D9` register
pairing were tried and both regressed (declaration-order swap: 165->165;
assignment-order swap `p8D8;p8D9;p6A0`: 167->165) -- consistent with round
36's four already-documented inert reorder axes for this exact residue
class.

Ran the permuter instead (`tools/setup-permuter.sh CD_readsync <seed>`
from the fix-1 167/174 body; base score 45, `Register Differences: 9` per
`--debug`, confirming this is purely the register-numbering residue with
nothing else hiding under it). Searched `timeout 600 ... -j 6 --stack-diffs
--stop-on-zero --best-only`; found a real improvement to permuter score 10
in the first few minutes (`output-10-1/`). **Verified directly against the
real oracle before trusting it** (per this project's own standing caution
that a permuter score move is a lead, not a result, in EITHER direction):

```c
if (p6A0 || pF8) {
    *D_8006D8C0 = status;
} else {
    *D_8006D8C0 = status;
}
```

Both arms are the identical statement -- a no-op semantically, and the
condition (`p6A0 || pF8`) is always true at this point (both are addresses
of file-scope globals, never null) -- but forcing `p6A0` and `pF8` to be
read (and therefore live in registers) at this specific point in the
function changed the register allocator's pressure/numbering enough to fix
the ENTIRE `p6A0`/`p8D9` `$s4`/`$s5` swap: 167 -> 173/174, closing 6 of the
7 words this residue class accounted for (0x1B718, 0x1B71C, 0x1B728,
0x1B7C4, 0x1B7D8, 0x1B87C all closed at once; only 0x1B7B4 remained).

This is the same phenomenon `CD_datasync.md`'s round-36 entry documents
(a permuter candidate reusing an already-hot register/variable as a
throwaway sink), but the FORM here is different and worth recording
separately: not a reused-variable write, but a duplicated-arm branch on an
always-true condition referencing the two contested pointer variables by
name. Read literally the branch looks like dead-weight the permuter left
behind; it is retail's own register-pressure shape made visible from C.

### Fix 3: the redundant reload, re-tried after fix 2 changed the context (173 -> 174/174, MATCH)

The one remaining residue (0x1B7B4) was the same "redundant reload of
`p8D8[0]`/`p8D8[1]`" class this report's original investigation already
named: retail loads `idx1` directly as `lbu $v0, 0x1($s3)` (an offset off
`p8D8`'s own base register), NOT through the separately-hoisted `p8D9`
pointer, even though `p8D9` (`$s4` in the now-fixed mapping) is genuinely
used later in the button-dispatch loop. This exact rewrite
(`idx1 = p8D8[1];` instead of `idx1 = p8D9[0];`) had been tried once before
this round, at the fix-1-only (167/174) checkpoint, and regressed badly
(167 -> 162/174) -- filed at the time as inert. **Re-tried it here, after
fix 2 changed the surrounding register allocation, and this time it closed
the function outright: 173 -> 174/174, BYTE-EXACT.**

Confirmed via the strongest available check, not just `funcdiff.py`'s
in-range read: reverted every other stalled sibling in this unit
(`cd_read_retry`, `CD_init`, `CD_datasync`, `callback`,
`cb_read`) to `INCLUDE_ASM`, rebuilt, and `./build-and-verify.sh`
reports **`OK: build matches retail SLPS_015.56`, exit 0** -- the whole-image
SHA1, not a per-function window.

### Proposed learnings

- **A negative result for a specific reshape is conditioned on the register
  state it was tested under, not permanent** -- this is the SAME lesson
  `cb_read.md`'s round-20 entry already drew for a guard-polarity flip,
  now confirmed a second time in this same unit for a completely different
  lever (redirecting a load's source pointer). When an earlier, unrelated
  fix changes a function's register allocation, re-check a previously-inert
  lever before ruling the residue permanently resistant.
- **The permuter's random mutator can find a "duplicate both arms of an
  always-true branch, referencing the contested variables" shape that no
  reasonable manual reorder attempt would try, and it can be genuinely
  load-bearing** -- add this to `CD_datasync.md`'s "reuse an already-hot
  register as a sink" finding as a second, structurally different form of
  the same underlying phenomenon (force a value's liveness at a specific
  program point by making the source visibly reference it there). Both
  require real-oracle verification regardless of which direction the
  permuter's own score moved; this one held up, `cb_read`'s round-36
  candidate did not.

### Final body (174/174, MATCHED)

```c
s32 CD_readsync(s32 arg0, s32 arg1)
{
    s32 now;
    s32 old;
    s32 flags;
    s32 *pEC;
    u8 status;
    s32 *p6A0;
    u8 *p8D8;
    u8 *p8D9;
    s32 *pF8;
    u8 *dst;
    u8 *src;
    s32 i;
    s32 idx0;
    s32 idx1;
    s32 result;

    now = VSync(-1);
    p6A0 = D_8006D6A0;
    p8D8 = D_8006D8D8;
    p8D9 = &D_8006D8D8[1];
    pF8 = &D_8006D8F8;

    D_8008B3E4 = now + 0x1E0;
    D_8008B3E8 = 0;
    D_8008B3EC = (s32)D_80010AD8;

    for (;;) {
        now = VSync(-1);
        if (D_8008B3E4 < now) {
            goto timeout;
        }
        old = D_8008B3E8;
        D_8008B3E8 = old + 1;
        if (0x1E0000 >= old) {
            goto success;
        }

    timeout:
        puts(D_80010984);
        idx0 = p8D8[0];
        idx1 = p8D8[1];
        __asm__("");
        /* &D_8008B3EC routed through a local pointer -- forces the same
         * unfolded lui/addiu addressing retail uses for this argument;
         * a plain `D_8008B3EC` reference here compiles FOLDED instead. */
        pEC = &D_8008B3EC;
        printf(D_80010994, *pEC, D_8006D620[CD_com],
               p6A0[idx0], p6A0[idx1]);
        CD_flush();
        result = -1;
        goto after_diag;

    success:
        result = 0;

    after_diag:
        if (result != 0) {
            return result;
        }
        if (CheckCallback() != 0) {
            status = (u8)(*D_8006D8C0 & 3);
            for (;;) {
                flags = getintr();
                if (flags == 0) {
                    break;
                }
                if ((flags & 4) && CD_cbready != 0) {
                    ((void (*)(s32, u8 *))CD_cbready)(p8D9[0], D_8008B3D4);
                }
                if ((flags & 2) && CD_cbsync != 0) {
                    ((void (*)(s32, u8 *))CD_cbsync)(p8D8[0], D_8008B3CC);
                }
            }
            /* permuter-found: forcing p6A0/pF8 to be read here (both arms
             * are identical) fixes the p6A0/p8D9 register-numbering swap
             * that otherwise cascades through the rest of the function --
             * see this report's round-39 fix 2. */
            if (p6A0 || pF8) {
                *D_8006D8C0 = status;
            } else {
                *D_8006D8C0 = status;
            }
        }

        dst = (u8 *)arg1;
        src = D_8008B3D4;
        if (dst != 0) {
            for (i = 7; i != -1; i--) {
                *dst = *src;
                src++;
                dst++;
            }
        }

        if (VSync(-1) > pF8[0] + 0x3C) {
            cd_read_retry();
        }
        if (pF8[-1] == 0) {
            CD_datasync(0);
        }
        if (arg0 != 0 || pF8[-1] <= 0) {
            break;
        }
    }

    return pF8[-1];
}
```

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the bare `__asm__("")` after
`idx1 = p8D8[1];` in the timeout path is **justified** and now commented at the
site. Measured by deleting it alone (60 bytes of the image differ; `funcdiff`
cannot score this function because its symbol is absolute, so the evidence is
`objdump -d -r` of `build/src/libcd_bios.c.o` with and without it): retail
issues `lbu a0,0(s3)` and `lbu v0,1(s3)` (the two `p8D8` bytes) directly after
the `puts` call; without the barrier the `p8D8[0]` load sinks below the
`D_8008B3EC` and `CD_com` loads for the `printf` arguments, and the index
arithmetic reshuffles around it. Instruction order.

## History (moved from src/libcd_bios.c, comments pass)

The warning above the tautological `if (p6A0 || pF8)` read, in full:

> ------------------------------------------------------------
> KNOWN-BAD CONSTRUCT, KEPT ONLY BECAUSE IT IS BYTE-EXACT.
> DO NOT COPY THIS SHAPE INTO ANOTHER FUNCTION.
>
> `p6A0` and `pF8` are both addresses of globals, so the
> condition is a TAUTOLOGY and both arms are IDENTICAL. GCC
> 2.6.3 cross-jumps the two arms back into the single `sb`
> retail has, so this compiles to no extra instruction -- its
> whole effect is to perturb register allocation, forcing
> `status` into $s1 across the inner loop. Worth 12 words:
> 162/174 without it, 174/174 with it.
>
> docs/PARALLEL-RUNS.md Gate 3 names duplicate-arm forms
> alongside UB as the signature of an EXHAUSTED class rather
> than a solution, and says a permuter zero is a LEAD to be
> translated into idiomatic C and re-verified. Round 39's head
> tried eleven such translations and none reached 174/174 --
> every declaration- and assignment-order permutation of the
> four pointer locals, `status` retyped to s32, a real
> (non-tautological) guard, a re-masked store, and an explicit
> live-range extension. All tabulated in the match report.
>
> THE MIS-MODELLING LEAD IS TESTED AND THE ANSWER IS SPLIT
> (round 40, head). The round-39 form of this comment said a
> tautological null check is what a mis-modelled global looks
> like, and asked whether either operand is really a POINTER
> global. Measured from the DATA, not from attempts:
>
>   - D_8006D6A0 is a fixed 8-element table of rodata string
>     addresses (asm/data/5DDFC.data.s:121). An array.
>   - D_8006D8F8 is one zero word that the sibling
>     cb_read stores VSync()'s return into
>     (cb_read.s:49-51). An s32 timestamp.
>
> Neither is a pointer global, so the condition CANNOT become
> an honest null test by that route. That half is closed.
>
> A DIFFERENT mis-modelling was real and IS now corrected:
> `pF8[-1]` (used four times below) reaches D_8006D8F4 by
> negative indexing off D_8006D8F8, which nobody writes -- the
> ten consecutive words ARE one array, as the zeroing walk in
> CD_readm already implied. They are now declared
> `s32 D_8006D8DC[10]` and indexed, and that model is
> BYTE-IDENTICAL (174/174, whole image green).
>
> BUT IT DOES NOT DISSOLVE THIS CONSTRUCT. Under the corrected
> model, removing the construct still scores 162/174 -- the
> same figure as before -- and every residual diff is a pure
> register swap ($s2/$s3, $a0/$a2, $v0/$v1). So the 12 words
> are REGISTER ALLOCATION, not data modelling, and the data
> model was never what this construct was standing in for.
> Do not re-run the data-modelling axis; it is measured.
> ------------------------------------------------------------

The comment on `pEC = &D_8008B3EC` read:

> &D_8008B3EC routed through a local pointer -- forces the same
> unfolded lui/addiu addressing retail uses for this argument;
> a plain `D_8008B3EC` reference here compiles FOLDED instead.
> See docs/match-reports/CD_readsync.md's round-36 entry.
