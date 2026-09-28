# CD_cw -- STALL (LENGTH EXACT: built 282/retail 282 words, up from 278/282 this round via two stacked permuter-found levers; 98/282 raw words match per funcdiff -- now a TRUSTWORTHY figure, since length carries zero drift -- so roughly 184 individual words still differ, spread across what looks like a register-allocation cascade from the second lever; see the round-37 addenda for both levers and an honest discussion of the raw-match tradeoff)

> Renamed from `func_80029F10` on 2026-09-23 (tools/rename.py). Address 0x80029f10.

Unit `libcd_bios`. Runner echo, round 26. Carved this round; no prior report exists.

## Signature

```c
s32 CD_cw(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
```

Confirmed by two already-matched call sites in the sibling unit `libcd_bios.c`: `CD_shell` (`CD_cw(1, 0, 0, 0);` and `CD_cw(0x16, D_8006D908, 0, 0)` used as a `while` condition -- i.e. the return value IS a truth value) and `CD_readm` (`CD_cw(9, 0, 0, 0);`). This function is itself called by `CD_sync`... no -- by NOTHING inside this unit; it is the unit's own entry point CALLED BY `CD_shell`/`CD_readm` above, and it itself calls `CD_sync(0, 0)` (blocking wait for CD sync) partway through its own body.

## What this function does

The CD-ROM "command" dispatcher: `arg0` is a command byte (`0`..`0x1B`ish, looked up by name in the shared `D_8006D620[]` string table), `arg1` is an optional parameter (pointer or scalar depending on command), `arg2` is an optional 8-byte output buffer, `arg3` is a "fire-and-forget" flag (skip waiting for completion).

1. If verbosity (`D_8006D608`) is `>= 2`, prints `"%s...\n"` with the command's name.
2. Looks up `D_8006D840[cmd]` -- "does this command need a parameter" -- and if it does and `arg1 == 0`, prints (when verbosity is on) `"%s: no param\n"` and returns `-2`.
3. Calls `CD_sync(0, 0)` -- blocks until CD sync (see that report).
4. If `cmd == 2`, copies 4 bytes from `arg1` into `D_8006D618`.
5. Clears the driver-state byte `D_8006D8D8[0]`, conditionally clears `D_8006D8D9` (gated by `D_8006D740[cmd]`), clears the hardware-register-ish `*D_8006D8C0`, and if `D_8006D740[cmd + 0x40]` is positive, streams that many bytes from `arg1` through `*D_8006D8C8` one at a time.
6. Records the command byte itself into `D_8006D61D` (used by the OTHER two functions' printf as the "current wait's name" selector) and `*D_8006D8C4`.
7. If `arg3 != 0` (fire-and-forget), returns `0` immediately.
8. Otherwise runs the SAME timeout/print/flush-loop idiom as `CD_sync`/`CD_ready` (see those reports), polling `D_8006D8D8[0]` until nonzero, printing `"CD_cw"` on timeout.
9. Once `D_8006D8D8[0]` is nonzero: if it is exactly `2` AND `cmd == 0xE`, snapshots `*(u8*)arg1` into `D_8006D61C`; unconditionally (if `arg2 != 0`) copies the 8-byte `D_8008B3CC` snapshot into `arg2`; returns `-1` if the final state is `5`, else `0`.

## A real, measured finding about `D_8006D840`/`D_6D740` addressing (documented so nobody re-derives it)

`D_8006D840[cmd]` (step 2, the "needs param" table) is referenced through its OWN fresh `%hi`/`%lo(D_8006D840)` relocation -- a genuine, independent global. But the SECOND flag table check (step 5, "does resetting `D_8006D8D9` apply to this command", `D_6D740[cmd]`) and the count used for the streaming loop (`D_6D740[cmd + 0x40]`) are BOTH computed as **pointer arithmetic off `D_8006D740`'s own base plus a literal `0x100` runtime immediate** (`addiu $v0, $v1(=&D_8006D740), 0x100`), never through a fresh `%hi`/`%lo(D_8006D840)` load. Since `0x8006D740 + 0x100 == 0x8006D840` exactly, and `asm/data/57070.data.s` shows `D_8006D740` (32 words), `D_8006D7C0` (32 words), `D_8006D840` (25 words) laid out back-to-back with no gap, `D_6D740[cmd + 0x40]` and `D_6D840[cmd]` are the SAME memory location for the same `cmd` -- but the two are reached via DIFFERENT C-level expressions in retail's own source (one names `D_8006D840` directly, the other computes `D_8006D740 + 0x40` elements over). **Writing this second access as `D_8006D840[cmd]` compiles to a fresh, independent `%hi`/`%lo` load -- the WRONG shape, since retail's actual bytes are a runtime `+0x100` add off an already-live `D_8006D740` base.** The body below gets this right (`D_8006D740[(arg0 & 0xFF) + 0x40]`) and it matches retail's exact instruction sequence at both use sites.

## Why this is a STALL and not a match

**All three of this function's own residues are already-characterized, not-fixable-by-hand classes documented in `docs/DECOMPILATION_LEARNINGS.md` and in this unit's sibling reports (`CD_sync.md`, `CD_ready.md`):**

1. **A missing delay-slot `andi $v0, $s3, 0xFF`** at retail vram `0x80029FF8`, immediately after the `bne $v1, $v0, .L8002A02C` that decides whether `cmd == 2`. Retail fills this branch's delay slot with a re-materialization of `cmd & 0xFF` (unused at the jump target, which immediately recomputes its own fresh `&D_8006D8D8`); this build leaves a plain `move $a0, zero` there instead. This is the delay-slot-fill family DECOMPILATION_LEARNINGS calls out as separate from block-order and explicitly still open: *"the compiler schedules an independent instruction into a delay slot that retail leaves empty or fills differently... that family remains open and is worth its own investigation; do not spend block-order attempts on it."*
2. **A missing MIPS-I load-delay `nop`** inside the 4-byte `D_8006D618` copy loop (retail: `lbu $v0, 0($v1)` / `nop` / `lui $at, ...`; this build's scheduler fills the slot differently and drops the `nop` entirely since nothing in this build's ordering needs it). Tried an explicit incrementing-pointer rewrite of the loop (`src++` each iteration instead of `arg1[i]`, per the project's own "inner loops want INCREMENTING POINTERS" idiom) -- no change; the scheduler's choice here is independent of that idiom.
3. **Two missing `addiu`-materialized-pointer-then-zero-offset-access instructions, each paired with a missing redundant `andi` mask**, at the two "fresh" (post-loop) reads of `D_8006D8D8[0]` -- the `== 2 && cmd == 0xE` tail check and the final `== 5` return check. Retail computes these as a genuine 3-instruction `lui`/`addiu`/`lbu` sequence (a materialized pointer VALUE, then a zero-offset load) plus a redundant `andi ,0xFF`; this build folds the access to a 2-instruction `lui`/`lbu`-with-nonzero-offset (a plain addressing-mode fold, which is STRICTLY equivalent code and the more natural compilation of a bare `D_8006D8D8[0]`). Tried forcing a local `u8 *p = D_8006D8D8; *p` indirection for both spots -- GCC's constant propagator sees the offset is `0` and folds the indirection straight back to the 2-instruction form (no arithmetic to preserve, unlike the loop-internal `state`/`state1` pointers elsewhere in this unit, which DO need separate registers because they cross real `jal`s). Tried a `__asm__("")` barrier between the pointer materialization and the store for the EARLIER `D_8006D8D8[0] = 0` instance specifically -- it moved the store's position (to right after the `D_6D740[cmd]` lookup instead of before it) without adding the missing `addiu`, and cost the function a NET word elsewhere; reverted.

Given three independent, already-catalogued "not worth further hand attempts" residue classes account for essentially the entire remaining 4-word gap (1 + 1 + 2, matching exactly), and the whole rest of the 282-word function -- every branch, every call, every global read in the retail order -- is structurally accounted for, this is being filed as a characterized stall rather than continuing to search for a fourth C-level lever.

One real structural fix IS folded into the body and is worth keeping on record: **the timeout/print/flush-loop section (step 8) needs the SAME `goto timeout3`/`goto success3`/shared `result` block-order as `CD_sync`/`CD_ready`, even though the CALLER-VISIBLE behavior is "return -1 immediately on error, otherwise keep going"** -- i.e. it is NOT sufficient to write `return -1;` directly inside the timeout arm. Retail's actual bytes still materialize `result` at a shared join (`j 2A1F0; li v0,-1` / `2A1EC: move v0,zero` / `2A1F0: bnez v0,epilogue`) even though nothing downstream of that join ever re-reads `result` for a value other than "branch to the epilogue or don't" -- the shared-join SHAPE is present regardless of how trivial the consumer is. Writing `return -1;` inline in the timeout arm compiled 3 words short until this was corrected.

## Body, as reached (278/282 words, near-miss)

```c
/* stalesyms --fix 2026-09-22: func_80012C20 -> printf, func_80024E64 -> CheckCallback, func_80025900 -> VSync, func_80025AE4 -> puts -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
extern s32 D_8006D608;
extern u8 D_8006D61C;
extern u8 D_8006D61D;
extern const char *D_8006D620[];
extern const char *D_8006D6A0[];
extern u8 D_8006D618[4];               /* 4-byte record, written here for cmd == 2 */
extern s32 D_8006D740[];               /* flag table, indexed by cmd; cmd+0x40 reaches the "needs param" table's
                                         * memory (see the addressing note above) -- do NOT re-split this into a
                                         * second D_8006D840[cmd] access for the cmd+0x40 case */
extern s32 D_8006D840[];               /* "does this command need a param" flag table, indexed by cmd; ONLY
                                         * correct as a direct access at its own (non-+0x40) call site */

extern volatile u8 *D_8006D8C0;
extern volatile u8 *D_8006D8C4;
extern volatile u8 *D_8006D8C8;
extern u8 D_8006D8D8[3];
extern u8 D_8006D8D9;

extern s32 D_8008B3E4;
extern s32 D_8008B3E8;
extern const char *D_8008B3EC;
extern u8 D_8008B3CC[];
extern u8 D_8008B3D4[];

extern void (*CD_cbready)(s32 arg0, void *arg1);
extern void (*CD_cbsync)(s32 arg0, void *arg1);

extern s32 VSync(s32 arg0);
extern void puts(const char *arg0);
extern void printf(const char *fmt, ...);
extern void CD_flush(void);
extern s32 CheckCallback(void);
extern s32 getintr(void);
extern s32 CD_sync(s32 arg0, s32 arg1);          /* defined earlier in this unit, ROM order */

extern const char D_80010984[];
extern const char D_80010994[];
extern const char D_80010A20[];        /* "%s...\n" */
extern const char D_80010A28[];        /* "%s: no param\n" */
extern const char D_80010A38[];        /* "CD_cw" */

s32 CD_cw(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    const char **table;
    u8 *state;
    u8 *state1;
    s32 counter;
    s32 result;
    s32 flags;
    u8 savedState;
    u8 *dst;
    const u8 *src;
    s32 i;

    if (D_8006D608 >= 2) {
        printf(D_80010A20, D_8006D620[arg0 & 0xFF]);
    }

    if (D_8006D840[arg0 & 0xFF] != 0 && arg1 == 0) {
        if (D_8006D608 > 0) {
            printf(D_80010A28, D_8006D620[arg0 & 0xFF]);
        }
        return -2;
    }

    CD_sync(0, 0);

    if ((arg0 & 0xFF) == 2) {
        src = (const u8 *)arg1;
        for (i = 0; i < 4; i++) {
            D_8006D618[i] = *src;
            src++;
        }
    }

    D_8006D8D8[0] = 0;

    if (D_8006D740[arg0 & 0xFF] != 0) {
        D_8006D8D9 = 0;
    }

    *D_8006D8C0 = 0;

    if (D_8006D740[(arg0 & 0xFF) + 0x40] > 0) {
        for (i = 0; i < D_8006D740[(arg0 & 0xFF) + 0x40]; i++) {
            *D_8006D8C8 = ((u8 *)arg1)[i];
        }
    }

    D_8006D61D = (u8)arg0;
    *D_8006D8C4 = (u8)arg0;

    if (arg3 != 0) {
        return 0;
    }

    D_8008B3E4 = VSync(-1) + 0x1E0;
    state = D_8006D8D8;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010A38;

    if (*state == 0) {
        table = D_8006D6A0;
        state1 = state + 1;
        do {
            if (D_8008B3E4 < VSync(-1)) {
                goto timeout3;
            }
            counter = D_8008B3E8;
            D_8008B3E8 = counter + 1;
            if (counter <= 0x1E0000) {
                goto success3;
            }
timeout3:
            puts(D_80010984);
            printf(D_80010994, D_8008B3EC, D_8006D620[D_8006D61D],
                          table[state[0]], table[state[1]]);
            CD_flush();
            result = -1;
            goto skip_timeout3;
success3:
            result = 0;
skip_timeout3:
            if (result != 0) {
                return result;
            }
            if (CheckCallback() != 0) {
                savedState = *D_8006D8C0 & 3;
                for (;;) {
                    flags = getintr();
                    if (flags == 0) {
                        break;
                    }
                    if (flags & 4) {
                        if (CD_cbready != NULL) {
                            CD_cbready(*state1, D_8008B3D4);
                        }
                    }
                    if (flags & 2) {
                        if (CD_cbsync != NULL) {
                            CD_cbsync(*state, D_8008B3CC);
                        }
                    }
                }
                *D_8006D8C0 = savedState;
            }
        } while (*state == 0);
    }

    if (D_8006D8D8[0] == 2 && (arg0 & 0xFF) == 0xE) {
        D_8006D61C = *(u8 *)arg1;
    }

    dst = (u8 *)arg2;
    src = D_8008B3CC;
    if (dst != NULL) {
        for (i = 7; i != -1; i--) {
            *dst = *src;
            dst++;
            src++;
        }
    }

    return (D_8006D8D8[0] == 5) ? -1 : 0;
}
```

## What is known independently of this body

- Screened blocker-clean at carve time (round 26). Biggest function in the unit (282 retail words).
- Calling convention confirmed against two ALREADY-MATCHED call sites in `libcd_bios.c` (`CD_shell`, `CD_readm`) -- both pass `0`/constant literals for `arg2`/`arg3` and use the return value as a truth value in one case (`while (CD_cw(0x16, D_8006D908, 0, 0))`), confirming `s32` return, not `void`.
- `D_8006D618` is this unit's own local reading of a symbol `libcd_bios.c` declares as a bare `extern u8 D_8006D618;` (singular) -- this function indexes it `[0..3]`, a genuinely different (but not conflicting, since neither file shares a header) local view. Flagged per the project's multiple-independent-local-views convention.

### Proposed learning

- **When a "does command X need special handling" flag table and its neighbour are laid out back-to-back in `.data`, retail's OWN source can reach the SAME byte through two different C-level paths depending on which call site wrote it** -- one direct symbol reference, one `+0x100`-style pointer arithmetic off a DIFFERENT (but adjacent) symbol's base, in the SAME function. `splat`'s per-address symbol-vs-arithmetic disassembly distinction (a fresh `%hi`/`%lo` relocation vs. a runtime immediate `addiu` off an already-live base) is the only reliable signal for which shape retail used at a given call site; guessing "just use the named symbol everywhere" costs a byte-exact match even when the VALUE read is identical either way.

## Round 35 addendum (echo) -- rebuilt, STILL a stall, same residue

Same stale-symbol-name hazard as this unit's other two reports: the body
above calls `func_80025900`/`func_80025AE4`/`func_80024E64`/`func_80012C20`
under round-26's placeholder names, all four since renamed to
`VSync`/`puts`/`CheckCallback`/`printf`. Rebuilt this round with the four
names corrected (plus the sibling call `CD_sync`, which is this
unit's own already-carved neighbour and was never renamed) and verified
against the whole-image oracle: **reproduces exactly** -- 278/282 words
(4 words short), raw 34/282.

No new lever was tried against this function's own three residues this
round -- they are the delay-slot-fill family (item 1, explicitly called out
in DECOMPILATION_LEARNINGS.md as open and "worth its own investigation; do
not spend block-order attempts on it"), a load-delay `nop` scheduling choice
(item 2, already tried and failed the documented incrementing-pointer
idiom), and a strictly-equivalent addressing-mode fold (item 3, already
tried and failed the local-pointer-indirection idiom) -- all three
independently characterized in the existing report body with the specific
counter-experiments already run. Re-reading the raw `.s` and this round's
rebuild's `asm-differ` output against the report's own three claims found
no discrepancy and no new angle; the residues sit downstream of roughly a
combined 1-word deficit already banked in this unit's two EARLIER functions
(`CD_sync` +1, `CD_ready` -2, net -1 word / -4 bytes before this
function's own code even starts), which shifts every address `asm-differ`
prints for this function and makes its OWN diff noisy with register-rename
artifacts that are pure address-drift noise, not real residues -- exactly
as the existing title already documents. Verdict stands: STALL.

## Round 37 addendum (echo) -- first permuter search finds a real 3-word gain (278/282 -> 281/282)

Rebuilt the round-35 body live first, confirmed it reproduces 278/282
exactly (`build/lsdde.map`: `CD_cw` at the correct retail address
`0x80029f10`, next function `CD_vol` landing 16 bytes/4 words early
at `0x8002a368`). This was this function's **first-ever permuter search**.

`tools/setup-permuter.sh CD_cw <seed>` scaffolded cleanly.
`--debug --stack-diffs`: base score = 2451 (56 stack-difference points, 55
register-difference points, 0 top-level insertions/deletions -- consistent
with this report's own framing of three separate already-characterized
residues rather than one clean isolated diff).

Ran the bounded search (`timeout 900 ... -j 6 --stop-on-zero --best-only
--stack-diffs`), rc captured on the next command: **rc=124** (900s bound),
**57462 iterations**. Unlike `CD_ready`'s search this same round, this
one found real improvement: **best score 1511** (down from base 2451),
saved automatically under `permuter-work/CD_cw/output-1511-1/`.

**The entire mutation, isolated by diffing the saved candidate against the
seed, is one change: the global `D_8006D8D8[3]` gains a `volatile`
qualifier on its `extern` declaration.** Nothing else in the function
differs (the permuter's own brace-style reformatting aside). Translated
this into the real source as `extern volatile u8 D_8006D8D8[3];`, and
correspondingly typed the two local pointers that alias it,
`state`/`state1`, as `volatile u8 *` (required for the assignment
`state = D_8006D8D8;` to type-check without discarding a qualifier).
Rebuilt through the real project pipeline (not the permuter's own scorer)
and re-verified with `funcdiff.py` and `build/lsdde.map`, per CLAUDE.md's
"a permuter score drop is a LEAD, not a RESULT" discipline.

**Result: a genuine 3-word gain.** `build/lsdde.map` now shows
`CD_vol` landing at `0x8002a374` -- only 4 bytes/1 word short of
retail's `0x8002a378`, where it was 16 bytes/4 words short before. `nm`equivalent
word-count confirms: **281/282 words**, i.e. only 1 word remains missing,
down from 4.

**A distraction that cost real time and is worth recording so the next
attempt does not repeat it:** enabling this change made `funcdiff.py`'s
"differs outside this range" byte count go UP (298262 bytes, vs 253209
before), and `build/lsdde.map` showed several unrelated data symbols
(`CD_cbread`, `D_8006D608`, the whole `CD_cbsync.. D_8006D8D9` cluster)
linked 4 bytes earlier than their expected addresses. This LOOKED exactly
like CLAUDE.md's "a struct edit for one function's sake silently breaks a
different, already-matched function" hazard, and cost a real diagnostic
detour (`cmp -l` against `disk/SLPS_015.56`, converting the position,
finding it landed on `jtbl_80010CD8` in a totally unrelated unit,
`libsnd_ssinit_libapi_counter`). **It is not that hazard.** This function is still not
byte-exact (1 word short, not 0), so the whole image downstream of it is
still not byte-exact either -- exactly as it was before this change, just
by a different remaining margin (4 bytes instead of 16). A jump table's
RAW BYTES encode absolute code addresses, so any rodata table anywhere in
the image whose entries point at code living after this function's own
address will show a byte diff purely because those downstream addresses
moved -- this is the ordinary, expected consequence of not being
byte-exact yet, not a new defect. The "outside range" byte count going up
rather than down when the IN-range residue gets smaller is not itself
suspicious once this is understood: it depends on how many absolute-address
references happen to live downstream, not on the magnitude of the shift.
**The correct read, confirmed here, is: check whether the WHOLE-IMAGE
`build-and-verify.sh` state is otherwise sane (it is: `build exit=2`, no
compile errors, and the ONLY source diff is the one deliberate change) and
whether the function's OWN address is where the map says it should be
(`0x80029f10`, always was) -- not whether some symbol 400KB away moved,
which it will, for a completely benign reason, whenever a not-yet-exact
function's total length changes at all, in either direction.**

Used `tools/asm-differ/diff.py` to locate the ONE remaining structural
defect (rather than trust the noisy raw diff, which is full of the
constant-offset drift artifacts described above). Found it exactly where
the ORIGINAL report's item 2 already named it: inside the 4-byte
`D_8006D618` copy loop, retail emits an explicit `nop` immediately after
`lbu $v0, 0($v1)` at the loop's branch-target instruction, which this
build's scheduler omits (filling the slot naturally via the following
independent `lui $at,...` three instructions later instead, which is
STRICTLY EQUIVALENT MIPS-I -- no hazard is actually being violated either
way). This is the exact **delay-slot-fill family** DECOMPILATION_LEARNINGS
already documents as open ("the compiler schedules an independent
instruction into a delay slot that retail leaves empty or fills
differently... do not spend block-order attempts on it"), and the
ORIGINAL report already tried this loop's one sanctioned counter-lever
(the incrementing-pointer idiom) against this exact spot and got no
movement -- the current body already uses that same incrementing-pointer
shape (`src++` per iteration), so nothing new was tried here this round.

**Verdict: STALL, now at 281/282 (1 word short), up from 278/282 -- a
genuine, oracle-verified 3-word gain from a single permuter-found lever.**
The remaining 1-word gap is the same already-characterized,
already-negative-tested residue class this report named before the
search. `INCLUDE_ASM` restored; preserved body in `src/psyq/libcd_bios.c`
updated to this round's improved version. Given the remaining residue is
explicitly the kind DECOMPILATION_LEARNINGS says not to spend block-order
attempts on, a SECOND permuter search seeded from this improved body would
be the next round's most promising next step here, not further hand
attempts -- flagged for the next round rather than run this round, since
this runner's remaining budget went to its other three assigned functions.

### Proposed learning

**A permuter mutation that adds `volatile` to an existing global ARRAY's
extern declaration (not a single pointer cast, as round 33's finding was
originally phrased) is a distinct, real lever from the pointer-cast form
already documented** -- and it can close MULTIPLE independent
instruction-selection residues in one function at once (here, 3 of 4
missing words from a single one-line change), not just the single
load-destination-register effect round 33's original example showed.
Worth checking as a first-class permuter target on this unit's OTHER
volatile-adjacent globals (`D_6006D740`, `D_8006D840`) if either shows a
similar "redundant instruction elided" residue in a future attempt.

**Separately: an "outside range" byte count going UP when an in-range
residue gets SMALLER is not on its own evidence of a new defect.** It
reflects how many absolute-address references anywhere in the whole image
happen to point at code downstream of the still-not-byte-exact function,
which is unrelated to the magnitude of that function's own remaining gap.
Before spending a `cmp -l` detour chasing it, first confirm via
`build/lsdde.map` that the function ITSELF still links at its correct
retail address and that no OTHER source file changed -- if both hold, the
outside-range count is not this change's fault, whichever direction it
moved.

## Round 37, second addendum (echo) -- a follow-up search from the improved body closes the LAST word, at the cost of raw match

Since the residue landscape genuinely changed after the `volatile` lever
(281/282, not 278/282), re-scaffolded the permuter from the IMPROVED body
rather than treating the first search as final. `--debug --stack-diffs`
base score on the new seed: 1506 (vs the original seed's 2451),
consistent with a smaller remaining gap. Ran a second bounded search
(`timeout 900 ... -j 6 --stop-on-zero --best-only --stack-diffs`), rc
captured on the next command: **rc=124** (900s bound), **70560
iterations**. Best score found: **926** (down from 1506), saved under
`permuter-work/CD_cw/output-926-1/`.

**The mutation, isolated by diffing against the seed:** a spurious
assignment inserted as the printf call's 4th argument --
`table[state[1]]` gets assigned into the already-declared (but at that
point still-unused) `src` pointer, with the ASSIGNMENT's value (not `src`
itself) passed to `printf`. `src` is unconditionally overwritten later in
the function (`src = D_8008B3CC;`, in the tail copy) before its value
from this assignment is ever read, so this is a pure dead store with zero
behavioral effect -- its only role is nudging the register allocator.
Translated into idiomatic C as a separate statement immediately before
the call (`src = table[state[1]]; printf(..., src);`) rather than the
permuter's embedded-assignment style; **rebuilt through the real pipeline,
same result exactly** (identical bytes either way), so the separate-
statement form was kept as cleaner and more readable.

**Result, verified via `build/lsdde.map` and `funcdiff.py` (not the
permuter's own score): this function's next-symbol address
(`CD_vol`) now lands EXACTLY at retail's `0x8002a378` -- LENGTH IS
NOW EXACT, 282/282, zero drift into anything downstream.** This is a real,
oracle-confirmed improvement on the metric CLAUDE.md's own diagnostic
chain checks first (address/length), and it means this unit contributes
zero drift to the units that follow it in the image.

**But the raw word-match figure moved the WRONG way: 132/282 -> 98/282.**
With zero remaining length drift, this figure is now fully trustworthy
(unlike every earlier reading in this report's history), so it means
roughly 184 individual words within this function's own window differ
from retail, up from roughly 150 before this lever. Reading the
realigned `asm-differ` output: the earlier state's diffs were
concentrated (one clean load-delay-nop gap plus a little register
renaming); this state's diffs are a much WIDER cascade of register
renames spread through the printf/timeout-loop tail, consistent with the
dead-store nudge changing which physical register the allocator picks for
several nearby values, not just the one it was aimed at. Pure deletion-only
lines (structural gaps, as opposed to same-position value/register
differences) actually DROPPED from 6 to 4 between the two states, so the
function is not literally "more different" in a structural sense -- the
raw-match number is dominated by cosmetic register-identity churn, not by
new missing/extra instructions.

**Judgment call, stated explicitly so the next round can revisit it:**
adopted the length-exact (282/282) body as this round's preserved best,
on the reasoning that (a) LENGTH is the metric this project's own
diagnostic chain checks first and treats as authoritative, independent of
drift, (b) a length-exact function contributes no address contamination
to anything after it, which is a standing project concern, and (c) the
structural (missing/extra instruction) count did not get worse, only the
register-identity churn did. This is NOT the same confidence level as the
`volatile` lever's 3-word gain, which was unambiguous in every measure.
**If a future attempt finds the wider register churn makes further
progress harder rather than easier, reverting to the 281/282 body
(preserved in this report's earlier addendum, and in git history at this
round's commit before this second addendum) is a legitimate and easy
undo** -- it is a strictly simpler, more cleanly-characterized state with
only one remaining residue class.

### Proposed learning (round 37, second addendum)

**A permuter-found dead-store nudge can trade a small, well-characterized
residue for LENGTH exactness at the cost of a much wider but shallower
register-identity cascade, and which of those states is actually "closer"
to a byte-exact match is not decidable from either the permuter's score
or the raw word-match count alone.** Length is drift-free and therefore
authoritative; raw word-match is only trustworthy once length is exact,
but a low trustworthy raw-match figure does not by itself mean the
function regressed -- check whether the STRUCTURAL (missing/extra
instruction) count changed, separately from the register-identity-only
diff count, before deciding whether a length-fixing lever is worth
keeping over a cleaner near-miss with residual drift.

## History (moved from src/libcd_bios.c, comments pass)

A comment inside CD_cw's preserved `#if 0` body, on `D_8006D8D8`, read:

> round 37: permuter-found lever, see match report -- volatile here
> (and on state/state1 below) closes 3 of the 4 missing words

The comment above CD_cw's preserved `#if 0` body read:

> Round 37 (echo): STALL, now 282/282 (LENGTH exact, no drift into
> anything downstream) -- up from 278/282, via two stacked permuter-found
> levers: (1) declaring D_8006D8D8 (and the two local pointers into it,
> `state`/`state1`) `volatile` closed 3 of the original 4 missing words
> (278->281/282); (2) a second search from that improved body found that
> materializing `table[state[1]]` into `src` as its own statement just
> before the timeout printf call (a dead store -- `src` is unconditionally
> overwritten before its value is ever read) closes the last word
> (281->282/282). Raw word-match went DOWN in the process (132/282 ->
> 98/282) even as length became exact -- see the match report's honest
> discussion of why LENGTH is still the right thing to have adopted here.
> Restored to INCLUDE_ASM per project rule.
