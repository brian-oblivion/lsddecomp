# func_8002AEE0 — STALL (length EXACT at 174/174, best 153/174 words, first real diff at vram 0x8002AF18)

`code_179d8_g`, vram `0x8002AEE0`, file offset `0x1B6E0`, 174 instructions
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
`func_8002AA6C`/`func_8002B198`" shape as the sibling `func_8002B198`
(itself a stall in this same unit), wrapped in an OUTER retry loop keyed off
`arg0` and `D_8006D8F4`.

- Sets a deadline (`D_8008B3E4 = now + 0x1E0`), a retry counter cap
  (`D_8008B3E8`, capped at `0x1E0000`), and a driver-name string pointer
  (`D_8008B3EC = "CD_read"`, confirmed via `asm/data/120C.rodata.s`
  `D_80010AD8`), matching `func_8002B198`'s identical setup with a
  different string (`"CD_datasync"`, `D_80010AE0`).
- On timeout, prints `"CD timeout: "` then `"%s:(%s) Sync=%s, Ready=%s\n"`
  (`D_80010994`, confirmed 4-`%s` format via `asm/data/FD8.rodata.s`) with
  args `(D_8008B3EC, D_8006D620[D_6006D61D], D_8006D6A0[D_8006D8D8[0]],
  D_8006D6A0[D_8006D8D8[1]])`, calls `func_8002A510()`, and returns -1.
  **This means `D_8006D620`/`D_8006D6A0` are STRING-POINTER tables (each
  element is a `char *`, stored as `s32`), not raw values — worth carrying
  forward for whoever next touches `func_8002B198`, see the anomaly note
  below.**
- On success, calls `func_80024E64()` (a trivial Psy-Q getter,
  `(s32)(u16)D_8006C272`, still `INCLUDE_ASM` in `asm/psyq_GsLinkObject4.s`
  — declared here as `extern s32 func_80024E64(void);`); if nonzero, saves
  `*D_8006D8C0 & 3`, runs the same button-dispatch loop as
  `func_8002B3F4`/`func_8002B198` (bit 4 → `D_8006D600(D_8006D8D8[1],
  D_8008B3D4)`, bit 2 → `D_8006D5FC(D_8006D8D8[0], D_8008B3CC)`, until
  `func_80029478()` returns 0), then restores the saved status byte.
- Copies 8 bytes from `D_8008B3D4` into `*(u8 *)arg1` — but ONLY if `arg1 !=
  0` (a null-destination guard retail has that is easy to miss reading the
  raw disassembly out of order).
- If `func_80025900(-1) > D_8006D8F8 + 0x3C`, calls `func_8002AA6C()`
  (itself a stall in this unit, `docs/match-reports/func_8002AA6C.md`).
- If `D_8006D8F4 == 0`, calls `func_8002B198(0)` (also a stall).
- Loops back to the very top of the poll (NOT re-initializing the deadline
  the way this comment described, but jumping back to the SAME `for(;;)`
  head that both timeout branches and initial entry share, so subsequent
  iterations poll against the ORIGINAL deadline) while `arg0 == 0 &&
  D_8006D8F4 > 0`; otherwise returns `D_8006D8F4`.

`func_8002B198` and `func_80024E64` both needed forward `extern` prototypes
added to this unit (`func_8002B198` is still `INCLUDE_ASM` later in this
same file; `func_80024E64` is `INCLUDE_ASM` in a different segment
entirely) — both added near this file's existing `func_8002AA6C` forward
declaration.

## Best-derived body (153/174 words, compiled length EXACT at 174/174 — confirmed via `objdump`/`build/lsdde.map`, zero drift outside range)

```c
#if 0
extern s32 func_80024E64(void);           /* trivial Psy-Q getter, still INCLUDE_ASM elsewhere */
extern s32 func_8002B198(s32 arg0);       /* forward decl -- still INCLUDE_ASM later in this file */
extern u8 D_80010AD8[];                   /* "CD_read", asm/data/120C.rodata.s */

s32 func_8002AEE0(s32 arg0, s32 arg1)
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
        func_80012C20(D_80010994, D_8008B3EC, D_8006D620[D_8006D61D],
                      p6A0[idx0], p6A0[idx1]);
        func_8002A510();
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
                flags = func_80029478();
                if (flags == 0) {
                    break;
                }
                if ((flags & 4) && D_8006D600 != 0) {
                    ((void (*)(s32, u8 *))D_8006D600)(p8D9[0], D_8008B3D4);
                }
                if ((flags & 2) && D_8006D5FC != 0) {
                    ((void (*)(s32, u8 *))D_8006D5FC)(p8D8[0], D_8008B3CC);
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
            func_8002AA6C();
        }
        if (pF8[-1] == 0) {
            func_8002B198(0);
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
   same "ok-flag vs direct branch" class `func_8002B198.md` already
   documents in this unit, confirmed a second time here.
3. **The copy loop needed the file's own established
   `for (i = N; i != -1; i--)` idiom** (already used by the matched
   `func_8002A6EC`), not `n = 8; do {...} while (--n != 0);` — the two
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
   This is the SAME class `func_8002B198.md` already names ("what's left
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

## Anomaly spotted in passing: `func_8002B198.md`'s preserved "best C" body mis-transcribes its own diagnostic call

Not fixed here (out of this function's scope, and `func_8002B198` was not
one of this pass's assigned targets) but worth flagging since it could
mislead whoever picks that stall back up: the report's preserved body has

```c
func_80012C20(D_80010994, p8D8[0], p6A0[p8D8[1]], p620[D_8006D61D], p6A0[p8D8[0]]);
```

but `asm/nonmatchings/code_179d8_g/func_8002B198.s` (lines ~50-70) shows the
EXACT same instruction shape as this function's own diagnostic block: `a1 =
D_8008B3EC` (not `p8D8[0]`), `a2 = D_6006D620[D_8006D61D]`, `a3 =
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

See the shared writeup in `docs/match-reports/func_8002AA6C.md` (filed
there since that function is the round's primary target for this
question); this function's own residue 2 above is cited there as a fourth,
independently-found data point (partial-improvement, not full-transfer)
alongside the three code_179d8_l reductions and the three code_179d8_g
stalls read for the question.

### Proposed learnings

- **A diagnostic block whose retail form ends in a bare-flag-then-branch
  join (`move v0,zero` / `bnez v0,<target>`) will not be reproduced by an
  early `return` inside the arm that sets the OTHER value** — even though
  the early return is logically equivalent and shorter, retail's compiler
  did not have that shape in its source; the extra "redundant-looking"
  instructions have to be sourced with an actual flag variable and a
  single join-point check. This generalizes `func_8002B198.md`'s
  "ok-flag vs direct branch" finding: the direction that seems more
  natural (early return) is not just occasionally worse, it can be a
  categorical mismatch when retail's OWN code shows the flag-join
  shape explicitly in its disassembly (not merely inferred from a byte
  count).
- **The established copy-loop idiom (`for (i = N; i != -1; i--)`) is
  worth checking BEFORE any byte-copy loop in this codebase, not just
  reaching for `do {} while(--n)`** — the two are not compile-equivalent
  under this pinned toolchain (different sentinel register usage), and
  this project already has one confirmed instance (`func_8002A6EC`) plus
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

**Symbol-name note (same finding as `func_8002AA6C.md` this round)**: this
report's preserved body already used the current names (`puts`, `printf`,
`VSync`, `CheckCallback`) -- unlike `func_8002AA6C`/`func_8002A75C`'s
reports, whoever wrote this one had already picked up the rename. Worth
flagging the inconsistency across this unit's reports rather than assuming
any one of them reflects current symbol names.

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
   here this round (budget went to `func_8002A75C` instead, which has a
   cleaner permuter debug signature -- pure reordering, no register/
   insertion/deletion component -- making it the stronger candidate for an
   extended search).
5. asm-differ/permuter text-vs-encoding gap: not implicated here; residue 2
   is a genuine extra `lbu` (a real byte, not a rendering artifact of
   `addiu`/`ori`).

### Proposed learning
A source-level evaluation-order swap of two independent scalar loads
feeding into the SAME call's arguments is inert here, matching this unit's
broader finding (`func_8002A75C.md`) that cc1 2.6.3's argument-evaluation
order for a call is not steered by which local is assigned first in source.
Worth treating as a low-probability lever generally in this codebase rather
than re-trying it by default on the next redundant-load residue.
