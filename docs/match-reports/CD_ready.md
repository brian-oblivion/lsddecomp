# CD_ready -- STALL (2 words SHORT: built 178/retail 180 words; 17/180 raw words match per funcdiff, unreliable due to length drift; first diff read off asm-differ at file offset 0x1A444 / vram 0x80029C40 -- an INHERITED-drift artifact from the sibling `CD_sync` being 1 word long, not a defect in this function -- the first defect belonging to this function's own body is the missing `andi $a2, $v0, 0xFF` at retail file offset 0x1A640 / vram 0x80029E40)

> Renamed from `func_80029C40` on 2026-09-23 (tools/rename.py). Address 0x80029c40.

Unit `code_179d8_n`. Runner echo, round 26. Carved this round; no prior report exists.

## Signature

```c
s32 CD_ready(s32 arg0, s32 arg1);
```

Confirmed by the already-matched call site `src/code_179d8_b.c:115` (`return CD_ready(arg0, arg1);`).

## What this function does

Sibling of `CD_sync` (see that report for the shared timeout/print/flush-loop idiom, reused verbatim here against `D_8008B3EC = D_80010A14` = `"CD_ready"` instead of `"CD_sync"`). The tail differs: instead of one state byte tested against two values, this function checks TWO INDEPENDENT flag bytes at `D_8006D8D8[2]` and `D_8006D8D8[1]`:

- If `D_8006D8D8[2] != 0`: clear it, optionally copy an 8-byte snapshot (`D_8008B3DC`) into `arg1`, return the ORIGINAL flag value.
- Else if `D_8006D8D8[1] != 0`: clear it, optionally copy a DIFFERENT 8-byte snapshot (`D_8008B3D4`) into `arg1`, return the original flag value.
- Else, loop again if `arg0 == 0`, or return `0`.

## Why this is a STALL and not a match

**This function's OWN residue is exactly two missing instructions, both the identical shape**, and nothing else -- confirmed by grepping the asm-differ output for every `andi` line and finding precisely these two absent on the built side:

```
retail 0x1A640 (vram 0x80029E40):  andi $a2, $v0, 0xFF     -- missing (flag2 == D_8006D8D8[2] check)
retail 0x1A690 (vram 0x80029E90):  andi $a2, $v0, 0xFF     -- missing (flag1 == D_8006D8D8[1] check)
```

In both spots, retail loads the flag byte with `lbu $v0, N($s3)` and then REDUNDANTLY re-masks it with `andi $a2, $v0, 0xFF` before the `beqz`/comparison -- redundant because `lbu` already zero-extends, so the mask can never change the value. GCC in this build correctly proves the mask is a no-op and elides it (`lbu $a2, N($s3)` directly, no `andi`). This is dead-code elimination working AS INTENDED on a genuinely redundant instruction retail happens to keep; it is not something a scheduling barrier can rescue. `docs/DECOMPILATION_LEARNINGS.md`'s decomposition of the `code_179d8_g` "redundant-raw-copy-elision" puzzle names this exact mechanism as its THIRD cause, distinct from block-order: *"Dead-code elimination. `func_8002B94C`'s redundant-looking check is removed by the optimizer BEFORE scheduling runs, which no barrier placement can rescue."* Confirmed empirically here too: neither a `__asm__("")` barrier immediately after the load, nor one after the following store, restores the mask (both left in the body below as inert, since they DID fix a genuine, separate ordering defect one line later -- see below -- and removing them regresses that fix).

Two real fixes ARE folded into the body below and are worth keeping on record:

1. **Same timeout/success block-order fix as `CD_sync`** (`goto timeout`/`goto success`/shared `result`, success placed as a trailing landing pad) -- this function shares that whole preamble verbatim.
2. **The store-then-copy sequence in each flag block needs a `__asm__("")` barrier directly after the flag-clearing store, or GCC sinks the store into the following branch's delay slot instead of leaving it where retail has it (immediately after the load+mask).** Without the barrier: `beqz $a2,skip [delay: move $a1,arg1] ... beqz $a1,ret [delay: STORE]` (store deferred into the SECOND branch's delay slot -- wrong). With the barrier: `beqz $a2,skip [delay:nop] ... STORE ... move $a1,arg1 ... beqz $a1,ret [delay: li $v1,7]` (store immediate, matches retail exactly). This closed a 2-word gap in the `flag1` block (`state2[-1]`) outright and fixed the `flag2` block's ordering too, though `flag2`'s own `beqz` tests `arg1` directly rather than the already-copied `dst`, so it did not need the barrier to reach the right SHAPE -- only to reach the right POSITION for the store; both blocks needed it once verified against the raw `.s`, since positions differ (see the two distinct `if` shapes in the body: `flag2` tests `arg1 == 0` raw, matching retail's `beqz $s4,...`; `flag1` tests the already-materialized `dst == 0`, matching retail's `beqz $a1,...` after `move $a1,$s4` as a real, non-delay-slot instruction). **This IS a genuine ordering residue a barrier fixes, unlike the `andi` mask above -- the two look similar (both "redundant instruction" residues) but are different causes per CLAUDE.md's four-ways-a-score-lies discipline applied at instruction granularity, and only one of the two responds to the same lever.**

## Body, as reached (178/180 words, near-miss)

```c
/* stalesyms --fix 2026-09-22: func_80012C20 -> printf, func_80024E64 -> CheckCallback, func_80025900 -> VSync, func_80025AE4 -> puts -- names retrofitted so this body links as written; the residue it recorded is unverified until rebuilt. */
extern s32 D_8006D608;
extern u8 D_8006D61D;
extern const char *D_8006D620[];
extern const char *D_8006D6A0[];

extern volatile u8 *D_8006D8C0;
extern u8 D_8006D8D8[3];

extern s32 D_8008B3E4;
extern s32 D_8008B3E8;
extern const char *D_8008B3EC;
extern u8 D_8008B3CC[];
extern u8 D_8008B3D4[];
extern u8 D_8008B3DC[];                /* 8-byte record, this function's second flag's snapshot buffer */

extern void (*D_8006D600)(s32 arg0, void *arg1);
extern void (*D_8006D5FC)(s32 arg0, void *arg1);

extern s32 VSync(s32 arg0);
extern void puts(const char *arg0);
extern void printf(const char *fmt, ...);
extern void CD_flush(void);
extern s32 CheckCallback(void);
extern s32 getintr(void);

extern const char D_80010984[];
extern const char D_80010994[];
extern const char D_80010A14[];        /* "CD_ready" */

s32 CD_ready(s32 arg0, s32 arg1)
{
    const char **table;
    u8 *state;
    u8 *state1;
    u8 *state2;
    s32 counter;
    s32 result;
    s32 flags;
    u8 savedState;
    u8 flag1;
    u8 flag2;
    u8 *dst;
    const u8 *src;
    s32 i;

    D_8008B3E4 = VSync(-1) + 0x1E0;
    table = D_8006D6A0;
    state = D_8006D8D8;
    state1 = state + 1;
    state2 = state + 2;
    D_8008B3E8 = 0;
    D_8008B3EC = D_80010A14;

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

        flag2 = *state2;
        if (flag2 == 0) {
            goto checkFlag1;
        }
        *state2 = 0;
        __asm__("");
        src = D_8008B3DC;
        if (arg1 == 0) {
            goto ret2;
        }
        dst = (u8 *)arg1;
        for (i = 7; i != -1; i--) {
            *dst = *src;
            dst++;
            src++;
        }
ret2:
        return flag2;

checkFlag1:
        flag1 = state2[-1];
        if (flag1 == 0) {
            continue;
        }
        state2[-1] = 0;
        __asm__("");
        dst = (u8 *)arg1;
        src = D_8008B3D4;
        if (dst == 0) {
            goto ret1;
        }
        for (i = 7; i != -1; i--) {
            *dst = *src;
            dst++;
            src++;
        }
ret1:
        return flag1;
    } while (arg0 == 0);

    return 0;
}
```

## What is known independently of this body

- Screened blocker-clean at carve time (round 26).
- `D_8008B3DC` is a NEW symbol this unit introduces (not referenced by `code_179d8_g.c` or `code_179d8_b.c`'s own local views) -- an 8-byte scratch buffer parallel to `D_8008B3CC`/`D_8008B3D4`, no dlabel in `asm/data/*.s` (uninitialized/BSS-style, same as those two).
- The two flag bytes (`D_8006D8D8[1]`, `D_8006D8D8[2]`) are read/written ONLY by this function within the unit; `CD_sync` and `CD_cw` only ever touch `D_8006D8D8[0]`/`[1]`.

### Proposed learning

- **Two GCC 2.6.3 residues that both look like "a redundant instruction retail keeps that mine elides" are NOT always the same mechanism, and only one of the two responds to a scheduling barrier.** Here, an `andi` mask after an already-zero-extending `lbu` is dead-code-eliminated (barrier-proof, matches the documented `func_8002B94C` cause) in the SAME function where a store's POSITION relative to a following branch's delay slot IS a genuine order residue a barrier fixes (matches the documented `CD_readsync`/`DreamSys__StepLookOffset` cause). Diagnose each instance against the raw `.s`'s exact instruction sequence rather than assuming a match report's fix for one instance transfers to a superficially similar one three lines later.

## Round 35 addendum (echo) -- rebuilt, STILL a stall, same residue; the cited `func_8002B94C` cause is now suspect but the mechanism re-confirms independently

> **CROSS-REFERENCE CORRECTED (round 40, head).** Round 35 called the cited
> `func_8002B94C` cause "suspect" without resolving why. It resolves cleanly:
> `func_8002B94C` is **`CD_newmedia`**, Sony's code, linked from
> `lib/libcd/iso9660.o` and reclassified in round 34 --
> `src/code_179d8_g.c`'s header comment records the conversion. It was never
> a game stall, so **no source shape ever reached those bytes** and there is
> no "documented `func_8002B94C` cause" to match against. The dead-code
> elimination of an `andi` after an already-zero-extending `lbu` is a real
> GCC 2.6.3 behaviour and this report measured it here independently -- that
> observation stands on its own and is the half worth keeping. What is void
> is only the appeal to `func_8002B94C` as the worked precedent for it, in
> this heading and in the proposed-learning bullet above it.
>
> Note the `CD_readsync` half of that same bullet is the OTHER exit and is
> fine: that one was matched as game C (174/174, round 39), so its precedent
> is real and complete -- go read the C in `src/code_179d8_g.c` rather than
> the report's summary of it.

Same stale-symbol-name hazard as `CD_sync` (see that report's round-35
addendum): the body above calls `func_80025900`/`func_80025AE4`/
`func_80024E64`/`func_80012C20` under round-26's placeholder names, all four
since renamed (`VSync`/`puts`/`CheckCallback`/`printf`). Rebuilt this round
with the four names corrected and verified against the whole-image oracle:
**reproduces exactly** -- 178/180 words (2 words short), raw 17/180,
first-diff location unchanged.

**Worth flagging: the mechanism citation this report leans on
(`func_8002B94C`'s "redundant check removed by the optimizer") is itself now
questionable, independent of this function's own result.** Round 34
converted `func_8002B94C` (`CD_newmedia`) to a linked Sony object
(`lib/libcd/iso9660.o`) -- it was never GCC-compiled game code at all, so
whatever produced its bytes was Sony's own toolchain, not this project's
pinned GCC 2.6.3. Citing it as a confirmed instance of "our GCC's DCE" was
therefore citing the wrong compiler. This does NOT mean the mechanism is
wrong for THIS function, though -- it means the corroborating example was
bad, not that the conclusion was. Re-derived independently against this
function's own live `.s` this round (not by trusting the citation): retail
emits `lbu $v0,N($s3)` then `andi $a2,$v0,0xFF` at both flag reads
(`0x1A640`, `0x1A690`); this build's `asm-differ` output shows the built
code loads the byte DIRECTLY into `$a2` (`lbu $a2,N($s3)`, no `andi`) at
both spots -- i.e. this compiler picks the call/branch-argument register as
the `lbu`'s destination outright rather than loading into `$v0` and then
mask-copying into `$a2`. That is a genuine, redundant-in-value
(`lbu` already zero-extends) but non-redundant-in-REGISTER-CHOICE
instruction retail keeps and this compiler's instruction selection elides.
So the underlying CLASS -- "a byte load's destination register choice can
absorb what looks like a separate masking move" -- holds on independent
re-derivation here; only the round-20 corroborating instance was
mis-attributed to the wrong compiler.

One additional lever tried this round and NEGATIVE: reading both flag bytes
through a `*(volatile u8 *)ptr` cast (testing round 33's "`volatile` on a
local pointer controls whether address computation folds into the memory
instruction" finding, on the chance it also pins the LOAD's destination
register) produced an identical build -- same length, same 17/180 raw score,
same first-diff offset. `volatile` did not touch this residue. Verdict
stands: STALL. This is a register/instruction-selection choice inside `lbu`
itself, not a schedulable ordering question, so the project's two
sanctioned instruments (`__asm__("")` barrier, `volatile`) have both now
been tried against it and both are negative.

## Round 37 addendum (echo) -- first permuter search, negative

Rebuilt the round-35 body live first (per this round's "build the
inherited body before trusting its score" discipline): reproduces
exactly, `build exit=2`, no compile errors, `build/lsdde.map` confirms
`CD_ready` links at the correct retail address `0x80029c40`.
Preserved verbatim, then ran this function's **first-ever permuter
search** -- it was one of the round's identified never-searched near-misses.

`tools/setup-permuter.sh CD_ready <seed>` scaffolded cleanly.
`--debug --stack-diffs` sanity check: **base score = 1265** (13
register-difference lines, 0 insertions/deletions at the top level --
consistent with the two missing `andi` instructions rippling register
choices through the rest of the function via shifted branch
displacements, not a small isolated diff). Matches this report's own
framing, so the scaffold is scoring the right residue.

Ran the bounded search: `timeout 900 ... permuter.py -j 6 --stop-on-zero
--best-only --stack-diffs permuter-work/CD_ready`, rc captured on
the very next command. **rc=124** (the 900-second bound fired; nothing
external killed it). **67179 iterations, and the best score seen across
the entire run never dropped below the starting 1265** -- every candidate
the permuter tried was equal to or worse than the seed. This is a clean,
unambiguous negative: not "no zero found" but "no candidate ever improved
on the base at all," the strongest form of confirmation that this
residue is not source-shape-sensitive under this permuter's mutation set.
Consistent with the round-35 finding that this is a
register/instruction-selection choice GCC's `lbu`-destination-picking
makes below the level any C rewrite can influence -- the permuter mutates
C source, and no C-source mutation moved this at all.

**Verdict: not closed in 67179 iterations under load (rc=124); the
permuter found nothing better than the seed at any point.** Filing as
STALL, unchanged from the round-35 figures. This function has now had
both sanctioned hand-levers (`__asm__("")`, `volatile`) AND a full bounded
permuter search all return negative. Per this round's own instructions,
that is NOT being upgraded to "permuter-exhausted" here on iteration
count alone -- but it is a materially stronger negative than any prior
round reached, and the next round can treat a repeat search on this exact
residue as very unlikely to pay off without a new lever first.

**One more quick hand test, prompted by a cross-function finding, also
negative:** this round's permuter search on this unit's OTHER sibling
`CD_cw` found that declaring the shared global `D_8006D8D8[3]`
(and the local pointers aliasing it) `volatile` closes 3 of that
function's 4 missing words (see `CD_cw.md`'s round-37 addendum).
Since this function also aliases the same global through `state`/
`state1`/`state2`, tried the identical lever here: `extern volatile u8
D_8006D8D8[3];` plus retyping all three local pointers to `volatile u8
*`. **No change whatsoever** -- rebuilt, `build/lsdde.map` shows
`CD_cw` (the next function, still `INCLUDE_ASM`/byte-exact)
landing at the same `0x80029f08`, 2 words short of retail, identical to
the un-modified baseline. This function's residue is a byte-load
DESTINATION-REGISTER choice on VALUE reads of two flag bytes (confirmed
above), not a redundant-instruction-elision on address computation or
staleness the way `CD_cw`'s was -- the two residues only LOOKED
similar ("something about `D_8006D8D8` accesses"), and the lever that
helped one function did nothing for the other. Reverted immediately;
`src/code_179d8_n.c` confirmed back to its committed state
(`build-and-verify.sh` clean, `git status` empty) before moving on.
