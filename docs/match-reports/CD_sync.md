# CD_sync -- STALL (1 word LONG: built 162/retail 161 words; 19/161 raw words match per funcdiff, unreliable due to length drift; first diff read off asm-differ at file offset 0x1A1BC / vram 0x800299BC, `addiu sp,sp,-0x38` vs built `-0x40`; round-37 permuter search ran 102842 iterations, rc=124, best candidate NOT adopted -- see that round's addendum for why)

> Renamed from `func_800299BC` on 2026-09-23 (tools/rename.py). Address 0x800299bc.

Unit `libcd_bios`. Runner echo, round 26. Carved this round; no prior report exists.

## Signature

```c
s32 CD_sync(s32 arg0, s32 arg1);
```

Confirmed by two already-matched call sites: `src/psyq/libcd_bios.c:110` (`return CD_sync(arg0, arg1);`) and `src/psyq/libcd_bios.c`'s own sibling `CD_cw` (`CD_sync(0, 0);`).

## What this function does

Waits for CD-ROM "Sync" status (`D_8008B3EC` is set to `D_80010A0C`, the string `"CD_sync"`). A `do`/`while` loop:

1. Checks a deadline (`D_8008B3E4`, set to `func_80025900(-1) + 0x1E0` before the loop) and a spin counter (`D_8008B3E8`) against a `0x1E0000` threshold. On either timeout condition it prints `"CD timeout: "` + a formatted diagnostic (`"%s:(%s) Sync=%s, Ready=%s\n"`), calls `CD_flush()` (a reset, matched in `libcd_bios.c`), and returns `-1`.
2. Otherwise, if `func_80024E64()` (a busy/status getter) is nonzero, drains a message-flush loop (`getintr()`, still `INCLUDE_ASM` in `libcd_bios.c` but blocker-clean) that dispatches through two global callback pointers, `D_8006D600` and `D_8006D5FC`.
3. Reads the CD-ROM's driver state byte `D_8006D8D8[0]`. If it is `2` or `5`, normalizes it to `2`, optionally copies an 8-byte snapshot (`D_8008B3CC`) into `arg1`, and returns the ORIGINAL state value.
4. Otherwise, loops again if `arg0 == 0` (blocking wait), or returns `0` immediately (single non-blocking poll) if `arg0 != 0`.

## Why this is a STALL and not a match

The C below is length- and structure-complete -- every branch, call, and global access is derived from the retail disassembly, not guessed -- but the compiled body is **1 word (4 bytes) LONGER** than retail's 161. `asm-differ` shows the WHOLE function is structurally identical to retail (same blocks, same branches, same call order) except for one isolated, well-characterized GCC 2.6.3 register-allocation choice:

**GCC hoists the literal `2` (used both in the `state == 2` comparison and the `*state = 2` store a few lines later) into its OWN callee-saved register (`s3`/`s4`/`s5` depending on which variant), computed ONCE before the loop, rather than re-materializing `ori $v0, $zero, 2` at each of the two use sites the way retail does.** This costs 1 extra saved register (2 words: one `sw`, one matching `lw` in the epilogue) but SAVES 2 words (the two `ori`/`li` materializations retail keeps), netting +1 word longer overall. Tried and confirmed NOT fixable by:

- A `goto`-based two-way branch (`if (st==2) goto ready; if (st!=5) continue; ready: *state=2;`) instead of `st==2 || st==5` -- same hoist either way.
- A `switch (st) { case 2: case 5: ... }` -- identical codegen, same hoist.
- A bare `__asm__("")` barrier placed between the load and the comparison -- no effect (the hoist happens at LICM/RTL-cse time, before scheduling; a scheduling barrier does not touch it).

This is the exact class documented in `docs/DECOMPILATION_LEARNINGS.md` under **"One named C variable gets ONE storage location -- retail's transient-rematerialization shape has no C spelling"** (round 20, established on `Entity__MoodCue111`): *"Retail sometimes does something a C author cannot ask for: several independent, transient, scratch-register recomputations of the same value, converging on one physical merge point, with no single variable ever living across the calls."* That entry explicitly says this is **not a toolchain lead, not grounds for raw asm, and not a reason to stop measuring** -- it is a characterized stall. Given the whole rest of the function (161-1=160 of retail's instructions, functionally) already matches in shape, further hand-tuning is very unlikely to close this specific 1-word gap; it is exactly the kind of residue the project's permuter is for.

Two block-order fixes DID matter and are already folded into the body below (documented here so the next attempt doesn't re-discover them):

1. **The timeout/success join must be `goto`-based with the SUCCESS case placed AFTER the timeout/print code, not inlined at the `if`-body.** Writing `if (counter <= LIMIT) { result = 0; goto skip_timeout; }` puts `result=0` in the branch's own delay slot (too short). Retail's shape is `if (counter <= LIMIT) goto success; timeout: ...print...; result = -1; goto skip_timeout; success: result = 0; skip_timeout: if (result != 0) return result;` -- diagnostic block FIRST (as the fallthrough), success as a trailing landing pad reached by a forward branch. This is the same lever documented for `CD_readsync` (round 25, "Block-order / shared-join fallthrough", 61/174 -> 153/174 in one fix). Applying it here took this function from 160/161 (1 word SHORT) to 162/161 (1 word LONG) by fixing an unrelated 2-word gap in this exact join.
2. **`table[state[1]]` inside the printf must be indexed directly off the `state` base pointer (`state[1]`), not through a second cached `state+1` pointer** (`state1`, which IS needed, but ONLY for the flush-loop's `D_8006D600` callback argument). Retail computes `state+1` once (register `s4`) and uses it for exactly one access; the printf's second `%s` argument is `lbu $v0, 1($s2)` -- direct offset off the SAME base used for `state[0]`, not off `s4`. Using `state1[0]` for both compiles the printf's operand fetch through the wrong register and desyncs the whole block's scheduling.

## Body, as reached (162/161 words, near-miss)

```c
/* stalesyms --fix 2026-09-22: func_80012C20 -> printf, func_80024E64 -> CheckCallback, func_80025900 -> VSync, func_80025AE4 -> puts -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
extern s32 D_8006D608;                 /* verbosity level; libcd_bios.c's func_80028CE0 accessor */
extern u8 D_8006D61C;
extern u8 D_8006D61D;                  /* selector into D_8006D620 for the "name" of the current wait */
extern const char *D_8006D620[];       /* string table, selector 0..0x1B -- shared reading, libcd_bios.c */
extern const char *D_8006D6A0[];       /* string table, selector 0..0x6 -- shared reading, libcd_bios.c */

extern volatile u8 *D_8006D8C0;
extern u8 D_8006D8D8[3];

extern s32 D_8008B3E4;                 /* poll deadline, from func_80025900(-1) */
extern s32 D_8008B3E8;                 /* poll spin counter */
extern const char *D_8008B3EC;         /* name of what's being waited for, printed on timeout */
extern u8 D_8008B3CC[];                /* 8-byte record, "Sync" wait's snapshot buffer */
extern u8 D_8008B3D4[];                /* 8-byte record, "Ready" wait's snapshot buffer */

/* This unit's own local view: called through, not just stored-and-compared
 * like libcd_bios.c/_g.c's plain-s32 reading of the same globals. */
extern void (*D_8006D600)(s32 arg0, void *arg1);
extern void (*D_8006D5FC)(s32 arg0, void *arg1);

extern s32 VSync(s32 arg0);                             /* asm/psyq_15d04.s */
extern void puts(const char *arg0);                    /* asm/psyq_15d04.s */
extern void printf(const char *fmt, ...);                /* Psy-Q printf wrapper */
extern void CD_flush(void);                                /* libcd_bios.c, matched */
extern s32 CheckCallback(void);                                 /* asm/psyq_GsLinkObject4.s */
extern s32 getintr(void);                                 /* libcd_bios.c, still INCLUDE_ASM there */

extern const char D_80010984[];        /* "CD timeout: " */
extern const char D_80010994[];        /* "%s:(%s) Sync=%s, Ready=%s\n" */
extern const char D_80010A0C[];        /* "CD_sync" */

s32 CD_sync(s32 arg0, s32 arg1)
{
    const char **table;
    u8 *state;
    u8 *state1;
    s32 counter;
    s32 result;
    s32 flags;
    u8 savedState;
    u8 st;
    u8 *dst;
    const u8 *src;
    s32 i;

    D_8008B3E4 = VSync(-1) + 0x1E0;
    table = D_8006D6A0;
    state = D_8006D8D8;
    state1 = state + 1;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010A0C;

    do {
        if (D_8008B3E4 < VSync(-1)) {
            goto timeout;
        }
        counter = D_8008B3E8;
        D_8008B3E8 = counter + 1;
        if (counter <= 0x1E0000) {
            goto success;
        }
timeout:
        puts(D_80010984);
        printf(D_80010994, D_8008B3EC, D_8006D620[D_8006D61D],
                      table[state[0]], table[state[1]]);
        CD_flush();
        result = -1;
        goto skip_timeout;
success:
        result = 0;
skip_timeout:
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
                    if (D_8006D600 != NULL) {
                        D_8006D600(*state1, D_8008B3D4);
                    }
                }
                if (flags & 2) {
                    if (D_8006D5FC != NULL) {
                        D_8006D5FC(*state, D_8008B3CC);
                    }
                }
            }
            *D_8006D8C0 = savedState;
        }

        st = *state;
        if (st == 2) {
            goto ready;
        }
        if (st != 5) {
            continue;
        }
ready:
        *state = 2;
        dst = (u8 *)arg1;
        src = D_8008B3CC;
        if (dst != NULL) {
            for (i = 7; i != -1; i--) {
                *dst = *src;
                dst++;
                src++;
            }
        }
        return st;
    } while (arg0 == 0);

    return 0;
}
```

## What is known independently of this body

- Screened blocker-clean at carve time (round 26): no `gp_rel`, no forward `mflo`/`mfhi`-before-`mult`/`div`, no `jr $t2` trampoline, no `jtbl_`.
- `D_8008B3E4`/`D_8008B3E8`/`D_8008B3EC` are a poll-deadline/counter/message-pointer trio shared with the unit's other two functions (`CD_ready`, `CD_cw`) -- all three run the identical timeout-and-print idiom against a different `D_8008B3EC` message.
- `D_8006D8D8` is a 3-byte driver-state array (bytes `[0]`, `[1]`, `[2]` each independently meaningful -- `CD_ready` reads all three, this function only `[0]`/`[1]`).
- `getintr`'s return value is a bitmask: bit `0x4` gates a call through `D_8006D600(state[1], D_8008B3D4)`, bit `0x2` gates a call through `D_8006D5FC(state[0], D_8008B3CC)`. `CD_readm` (`libcd_bios.c`, matched) independently documents `D_8006D600` being assigned `cb_read` as a callback, consistent with this reading.
- The format string `D_80010994` is confirmed via `asm/data/FD8.rodata.s`: `"%s:(%s) Sync=%s, Ready=%s\n"` -- 4 `%s`, matching the 4 non-format arguments in the `func_80012C20` call.

### Proposed learning

- **A literal reused between an equality test and a store a few lines later (`if (x == 2) ...; *p = 2;`) is a live trigger for GCC 2.6.3's "hoist the constant into its own saved register across the whole enclosing loop" behavior**, independent of whether the two uses are written as `||`, sequential `goto`s, or a `switch`. Tried all three shapes here; all three hoist identically. Matches the round-20 "transient-rematerialization" finding but had not previously been pinned down to this specific two-site-same-literal trigger.

## Round 35 addendum (echo) -- rebuilt, STILL a stall, same residue

Per CLAUDE.md's "BUILD any inherited/preserved body ONCE before trusting its
recorded score" and round 31's "a preserved body's `jal` targets can go STALE
across an SDK-object round": the body above, as literally written, calls
`func_80025900`/`func_80025AE4`/`CD_flush`(unaffected)/`func_80024E64`/
`getintr`/`func_80012C20` by round-26's placeholder names. Four of
those six are now stale -- SDK-object conversion rounds between 26 and 35
renamed them to their real Sony symbols: `func_80025900` -> `VSync`,
`func_80025AE4` -> `puts`, `func_80024E64` -> `CheckCallback`,
`func_80012C20` -> `printf` (all confirmed via `config/symbols.slps01556.lsdde.txt`
and already reflected in `src/psyq/libcd_bios.c`'s own local declarations for
the same globals). Spliced verbatim with the round-26 names, this body would
not have linked; that was never tested, since round 26's own build always
had all three functions as `INCLUDE_ASM` together after the stall.

Rebuilt with the four names corrected (signatures unchanged, so this is a
pure rename, not a behavior change) and verified against the whole-image
oracle: **reproduces exactly** -- 162/161 words (1 word LONG), first diff
still `addiu sp,sp,-0x38` (retail) vs `-0x40` (built) at the same offset,
same `li s3,0x2` extra callee-saved-register hoist visible in
`asm-differ`'s output at `1a208`. The stale names did not affect the
residue; only the linkage would have failed had this been spliced-and-left
rather than spliced-and-tested.

One additional lever tried this round and NEGATIVE: routing the literal `2`
used at both the `st == 2` compare and the `*state = 2` store through a
single named local (`u8 readyVal = 2;` used at both sites, per round 31's
"defeat 2.6.3's constant canonicalization by routing the constant through an
assignment") does not stop the hoist -- built score if anything got
marginally worse (18/161 raw vs 19/161, same length). This is consistent
with round 33's finding that a GCC 2.6.3 GCSE/value-availability hoist is
immune to source-level rescue "by construction, not by insufficient
placement" -- the mechanism here (retail recomputes the same immediate at
two use sites with nothing live across; this compiler proves one hoisted
copy suffices and keeps it resident in its own register for the whole
enclosing loop) is exactly that class. Verdict stands: STALL, not
progressable by hand C restructuring under the pinned toolchain.

## Round 37 (echo): first-ever permuter search, mixed result -- NOT adopted, but worth recording

This function is listed elsewhere as "already permuter-searched" in this
round's staffing notes, but an exhaustive check of this report and
`docs/PROGRESS.md` found **no prior permuter record for this function at
all** -- no scaffold, no iteration count, no score. Treating that
classification as mistaken and running the search this round, since the
`.venv/bin/python3 tools/decomp-permuter/permuter.py --debug` scaffold
check is cheap and the alternative (skipping a genuinely-unsearched
function because a list said otherwise) throws away real information.

Rebuilt the round-35 body live first (four stale names corrected, as that
addendum already did): reproduces exactly, 162/161 words, `build
exit=2`, no compile errors. `--debug --stack-diffs` base score: 2153.

Ran the bounded search: `timeout 900 ... -j 6 --stop-on-zero --best-only
--stack-diffs`, rc captured on the next command: **rc=124** (900s
bound), **102842 iterations**. Ten distinct improving candidates were
saved (`output-1250-1` through `output-2143-1`); best score **1250**
(down from 2153). No zero.

**Translated the best candidate (1250) and verified through the real
oracle -- the result is genuinely mixed, not a clean win, and was NOT
adopted.** The candidate combines two changes: (1) hoisting
`D_8006D620[D_8006D61D]` into a named local before the timeout `printf`
call, and (2) routing the literal `2` through an assignment embedded in
the `st == 2` comparison itself (`if (st == (new_var2 = 2))`) rather than
declaring it earlier -- a different POSITION for the same
"canonicalize-the-constant" idea round 35 already tried (as a top-level
`u8 readyVal = 2;`) and found negative. Translated idiomatically as
`readyVal = 2;` placed immediately before the `if (st == readyVal)`
check, matching the candidate's position exactly, and rebuilt.

**Result: `build/lsdde.map` shows the next function (`CD_ready`)
landing at `0x80029c3c` -- this build is now 160/161 words, 1 word
SHORT, where the round-35 baseline was 162/161, 1 word LONG.** The
1-word gap did not close; it flipped sign. Raw word-match (now
comparably drift-affected in magnitude either way, just from the
opposite direction) went from 19/161 to 99/161 -- a large apparent
improvement, and `asm-differ` confirms the remaining structural gaps
shrank to essentially one real difference: retail materializes the
literal `2` TWICE (fresh `li v0,0x2` at both the comparison and the
later store), while this candidate's `readyVal` gets ONE register/one
live value reused at both sites -- i.e. this lever pushes the register
allocator in EXACTLY the opposite direction from what retail's own
codegen does here (retail does NOT hoist this constant; it deliberately
re-materializes it), which is consistent with -- not a refutation of --
this report's own three-round finding that this specific hoist is a
GCC-side value-availability decision no C rewrite controls in the wanted
direction.

**Judgment call: NOT adopted.** Unlike `CD_cw`'s sibling result
this same round (where a length-exact state was adopted despite a raw-
match tradeoff, because the structural gap count did not worsen), here
the length gap did not close at all -- it merely changed sign, and the
mechanism achieving the raw-match improvement is pushing AGAINST the
documented root cause rather than around it. Reverted; `src/psyq/libcd_bios.c`
confirmed back to its committed state (`build-and-verify.sh` clean,
`git status` empty) before moving on. Filing as STALL, figures unchanged
from the round-35 addendum (162/161 words LONG, same residue).

### Proposed learning

**"Already permuter-searched" in a staffing list is itself a claim, not a
fact, and costs nothing to re-verify against the actual match report and
`docs/PROGRESS.md` before skipping a function on that basis.** This
function was staffed as already-searched with no supporting record
anywhere; running the search anyway took under 20 minutes total and
produced a real (if ultimately not-adopted) data point. When a report has
no permuter section, treat the function as unsearched regardless of how
it was staffed.

**Separately: a permuter candidate that raises the raw word-match figure
can still be pushing the register allocator in the WRONG direction
relative to the documented root cause, and the two must be checked
independently.** Here the candidate's higher raw-match score came from
making this compiler behave MORE like a single-hoisted-constant compiler
-- the opposite of what closing this specific gap requires (retail wants
the constant re-materialized twice, not hoisted once). A rising raw-match
number is encouraging but not sufficient; read what the remaining
`asm-differ` gap actually consists of before adopting a candidate whose
mechanism contradicts the report's own established diagnosis.
