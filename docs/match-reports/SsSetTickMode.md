# SsSetTickMode -- MATCH (96/96 words, byte-exact)

> Renamed from `SetSeqTimerMode` on 2026-09-23 (tools/rename.py). Address 0x80032588.

> Renamed from `func_80032588` on 2026-09-23 (tools/rename.py). Address 0x80032588.

Unit: `code_179d8_c_b`. Round 41, runner delta. Owns `jtbl_80010CD8`
(attached rodata, not touched by this round).

Superseded round-23 stall report: this round re-derived from that report's
own preserved 90/96 near-miss body, ran it through a **first-ever permuter
search** (never done before on this unit), and — before the search reached
any useful result — a manual re-read of the outer bound-check's branch
polarity closed the remaining 6 words directly. The permuter search is
recorded below for completeness (it was superseded mid-run, not exhausted).

## What it does

`void SsSetTickMode(s32 a0)`. Two parts, unchanged from the round-23
description:

1. Splits `a0` into a "cmd" value stored in `gSeqTimerRateMode` and a flag in
   `gSeqTimerModeFlag`: if `a0 & 0x1000`, `gSeqTimerModeFlag = 1` and `gSeqTimerRateMode = a0 &
   0xFFF`; else `gSeqTimerModeFlag = 0` and `gSeqTimerRateMode = a0` (the raw value).
2. Reloads `cmd = gSeqTimerRateMode` and dispatches:
   - `cmd >= 6` (signed): `gSeqTickRate = cmd` (raw passthrough).
   - `cmd < 0` (falls out of the unsigned `cmd < 6` recheck): `gSeqTickRate
     = 0x3c`.
   - `0 <= cmd < 6`: a genuine `switch` over `jtbl_80010CD8` (6 dense
     cases), each setting `gSeqTickRate` to a per-case tempo/rate constant,
     and cases 1 and 4 additionally rewriting `gSeqTimerRateMode` based on
     `gVideoMode` (a play-state flag: 0/1/other).

| case | gSeqTickRate | gSeqTimerRateMode side effect |
| --- | --- | --- |
| 0 | `(gVideoMode==1) ? 0x32 : 0x3c` | none |
| 1 | `0x3c` (always) | `(gVideoMode==0) ? 5 : 0x3c` |
| 2 | `0xf0` | none |
| 3 | `0x78` | none |
| 4 | `0x32` (always) | `(gVideoMode==1) ? 5 : 0x32` |
| 5 | `(gVideoMode==1) ? 0x32 : 0x3c` | none |

Round 23's two structural findings both still hold and were not touched
this round:

- **Case bodies are laid out in source order `4, 1, 3, 2, 5, 0`, not
  ascending case-value order.** The jump table (`jtbl_80010CD8`) is still
  indexed by case value as always; only the physical placement of each
  case's code follows this order. Confirmed unchanged in this round's
  final byte-exact build.
- Case 0 and case 5 implement algebraically identical 3-way logic but
  compile to different branch polarities (`bne` vs `beq`) -- this is
  reproduced automatically by writing the two cases with the exact same
  `if`/`else if`/`else` phrasing; no special-casing needed once the outer
  bound-check fix (below) is applied.

## The actual round-23 residue, and why it was mis-scoped

Round 23 attributed the entire 6-word gap (86-90/96) to GCC's `-O2`
tail-merge of the switch's case endings (cases 2 and 3, and the `cmd<0`
default, all reducing to an identical `lui/sw/j/nop` tail that our build
folded into one shared block but retail kept duplicated). That attribution
was **half right and half wrong**:

- Right: GCC's own cross-jump/tail-merge pass genuinely differs in its
  fold decision from a naive switch, and the round-23 negatives (nested
  `switch` vs `if`-chain, typed temp, bare `__asm__("")` barrier) are all
  still valid NEGATIVE results for THAT specific question.
- Wrong (or at least incomplete): round 23 never checked the **outer bound
  checks** (`cmd >= 6` and `(u32)cmd >= 6`) against the actual retail
  disassembly with `asm-differ`. It transcribed them as two sequential
  early returns:

  ```c
  if (cmd >= 6) {
      gSeqTickRate = cmd;
      return;
  }
  if ((u32)cmd >= 6) {
      gSeqTickRate = 0x3c;
      return;
  }
  switch (cmd) { ... }
  ```

  This C is logically correct, but GCC 2.6.3's early-return idiom for
  `if (cond) { body; return; } rest;` places `body` **inline**
  (fallthrough) immediately after the test, and reaches `rest` via a
  branch-away when the test is false. Retail's actual layout is the
  **opposite polarity**: the test's fallthrough (not-taken) case continues
  inline into the next check, and the `gSeqTickRate = cmd; ` passthrough
  store is placed at the very END of the function (`.L800326F8`,
  immediately before the shared epilogue), reached only via a
  branch-when-true. Reading `asm-differ`'s realigned output made this
  visible immediately: the first real diff was not anywhere near the
  switch-case tails round 23 spent its budget on -- it was at the very
  first branch of the function (`beqz` in retail vs `bnez` in our build,
  right after the first `slti`).

## The fix: nest the bound checks under `if (cmd < 6) { ... }` and fall off the end for the passthrough

```c
if (cmd < 6) {
    if ((u32)cmd < 6) {
        switch (cmd) {
        /* ... six cases, unchanged from round 23 ... */
        }
    } else {
        gSeqTickRate = 0x3c;
        return;
    }
}
gSeqTickRate = cmd;
```

This reproduces retail's layout exactly: the passthrough store
(`gSeqTickRate = cmd;`) is now the code that FOLLOWS the whole `if` block in
source order, so GCC places it physically at the end of the function
(reached by branch-when-`cmd>=6`, i.e. exactly `.L800326F8`) instead of
inline. The `(u32)cmd >= 6` case's `0x3c` store also moved from an early
return to an `else` arm, which let GCC's existing tail-merge fold it into
the SAME shared store label the switch's cases 0/5 already used
(`.L800326E8`) -- this is the SAME merge mechanism round 23 already
identified for cases 0/5, just now also covering this arm, and it is
retail's actual behavior, not a bug.

**One trap hit and reverted on the way here:** the round-23 preserved body
carried a bare `__asm__("")` scheduling barrier after cases 2 and 3's
stores (round 23's lever 4, which had narrowed 86->90/96). Combined with
the NEW nested-if nesting, that barrier caused a genuine miscompilation:
`objdump` showed case 3's jump-table target landing on a
`j <case2 tail>` that **skipped over** case 3's own `li v0,0x78` --
i.e. case 3 silently fell through to case 2's dead code with case 3's
store never executed. This was caught by inspecting the raw
`objdump` disassembly directly (word count alone would not have caught
it -- the function was still the wrong length at that point, 93/96, so
the discrepancy was visible as dead code before a jump, not as a
diff-tool artifact). Removing both `__asm__("")` barriers immediately
fixed it and simultaneously produced the exact target length, 96/96.
**Lesson: a bare scheduling barrier that was load-bearing for one control-flow
shape is not guaranteed to still be safe (or even correct) after a later,
unrelated restructuring of the same function -- re-validate it, don't just
carry it forward.**

## Permuter search (first-ever on this unit, superseded mid-run)

Set up per `tools/setup-permuter.sh` against the round-23 90/96 preserved
body (i.e. before the fix above was found):

```
tools/setup-permuter.sh SsSetTickMode <seed with round-23 body>
```

`--debug --stack-diffs` validation against the scaffold matched the
report's own residue shape (base score 1575; visible diffs were exactly
the case-2/case-3 tail-duplication region plus the case-0/case-5 branch
polarity, matching the round-23 prose). The scaffold was trustworthy.

Launched: `PATH=.../permuter-work/bin:$PATH .venv/bin/python3
tools/decomp-permuter/permuter.py -j 4 --stop-on-zero --best-only
permuter-work/SsSetTickMode`, bounded at 900s. **Killed by PID (not
`pkill -f`) partway through, once the manual structural fix above reached
96/96 independently** -- the search was still running against the OLD
(pre-fix) scaffold and could not have found the outer-bound-check
restructuring, since that fix touches code outside the switch the seed's
residue was centered on. No zero was reached before it was stopped;
treat this as **not exhausted, superseded by a manual finding**, not as a
negative permuter result.

## Verification

```
./build-and-verify.sh   ->  build exit=0, "OK: build matches retail SLPS_015.56"
tools/funcdiff.py SsSetTickMode  ->  96/96 words match (file 0x22D88-0x22F08)
```

Whole-image SHA1 passes. This is a genuine match, not a per-function read
against a drifted image.

## Proposed learning

1. **A `-O2` tail-merge residue and an early-return branch-polarity
   mismatch can co-occur in the same function, and fixing only the one
   your predecessor's report emphasized leaves the other as invisible
   residue "inside" what looks like it should have been the whole gap.**
   Round 23's 6-word gap was described entirely in terms of the switch's
   case-tail merging; the real majority of it (3 of 6 words, and the
   FIRST diff in address order) was an unrelated outer-bound-check layout
   question the report never looked at, because its own preserved-body
   listing put the `switch` front and center. **Always run
   `tools/asm-differ/diff.py <func>` on an inherited near-miss body before
   trusting where a stall report says the residue is** -- the report's own
   prose can be right about a real mechanism and still be silent about
   the actual FIRST diff.
2. **`if (cond) { body; return; } rest;` and `if (cond) { rest_wrapped }
   fallthrough_tail;` are NOT interchangeable under GCC 2.6.3's -O2, even
   though they are logically identical for a void function with no other
   exit paths.** The former places `body` inline and `rest` at a distance;
   nesting the "continue processing" code inside the `if` and moving the
   final unconditional statement OUTSIDE and AFTER it flips which side is
   inline. When retail's branch polarity for an early-exit check doesn't
   match a straightforward transcription, try restructuring which side is
   the `if`-body versus the fallthrough tail before assuming the residue
   is a case-body-order or register question.
3. A scheduling barrier (`__asm__("")`) validated as safe for one
   control-flow shape must be re-validated after any later restructuring
   of the same function, including an unrelated outer nesting change --
   here it silently broke a completely different case's stored value
   (case 3 fell through to case 2's dead code) rather than merely
   changing instruction order, which is exactly what the barrier is
   supposed to be restricted to.

## Naming

**Superseded, round 71 (track 2):** the track-3 game name `SetSeqTimerMode`
is replaced by Sony's own name -- fingerprint EXACT masked 1.00 vs
libsnd/ssinit `SsSetTickMode` (disc 3.3), confirming the hypothesis round 69
already recorded below. This is Sony's SDK code, not decompiled game logic;
track 2 names those functions and moves them out of tracks 1/1b/3.

Round 69 (delta), track 3 pass on `code_179d8_c_b`.

| name | tier | evidence |
| --- | --- | --- |
| `SsSetTickMode` (was `func_80032588`) | B | mechanics are fully known (splits an input word into a rate-mode selector and a flag, then picks a tick rate from a 6-way table); the in-game reason a caller picks each mode is not established. |
| `gSeqTimerRateMode` (was `D_8006DCA4`) | B | the "cmd" this function derives from its argument and `_SsStart` (the timer-arming stall) dispatches on for its own device-tag/rate selection -- see that report. |
| `gSeqTimerModeFlag` (was `D_8006DCA8`) | B | the flag bit (`a0 & 0x1000`) extracted alongside the rate mode; `_SsStart`'s `default:` arm returns immediately without touching the timer when this is set, so it gates whether the computed-rate path runs at all. |
| `gVideoMode` (was `D_8006DC98`) | A | assigned directly from Psy-Q's `GetVideoMode()` in `func_8003221C` (sibling unit `code_179d8_c.c`, its own match report). This function only reads it, to choose between the two named tick rates. |
| `gSeqTickRate` (was `D_8009024C`) | B | `src/code_179d8_k.c` (a different, uninvolved unit) independently reads this same global and comments it as "a tick-rate/PPQN-style constant" used in a MIDI Set-Tempo scheduling formula (`GetMetaEvent`/`SetTempo`-style handler) -- two unrelated units converging on the same reading. This function is the one that WRITES it, from the 6-way rate table below. |
| `SEQ_TICKRATE_50`/`_60`/`_120`/`_240` (magic constants `0x32`/`0x3c`/`0x78`/`0xf0`) | B | named by value only (the numbers themselves), not by an NTSC/PAL region claim -- `gVideoMode` (0/1) does select between the 50 and 60 values in three of the six cases, which is suggestive, but this file has no direct evidence pinning which region is which value. |

See `_SsStart.md` for the globals `gSeqTimerId`/`gSeqTimerRateFlag`/
`gSeqTimerStopPending`/`gSeqTimerChainedCallback`, which this function does
not touch.
