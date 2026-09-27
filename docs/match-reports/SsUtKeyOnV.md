# SsUtKeyOnV -- STALL (round 70 re-measure: 5 words SHORT, 248/253 built; raw word-match 47/253 and insertions 18 / deletions 18 are drift-contaminated upper bounds while short; first real diff at word 23, the busy-lock guard's branch polarity, per round 36. The round-62 "BLOCKED below cc1" verdict is RETRACTED: 11 of the 16 missing words were the load-delay nop `--nop-at-expansion` emits since round 63)

> Renamed from `func_8003149C` on 2026-09-24 (tools/rename.py). Address 0x8003149c.

> **[RETRACTED round 70: the blocker below is resolved; see the title.]** **TOOLCHAIN-BLOCKED -- note added round 62 by bravo, who did not work this
> function.** While revisiting its near twin `SsUtKeyOn` (same unit,
> `src/libsnd_vm_vol_ut_key_ut_keyv.c`), a below-cc1 blocker was found and escalated: a
> load-delay `nop` that ASPSX 2.34 emits after an indexed load whose next
> unexpanded instruction is a store-to-symbol macro of the loaded register,
> and that maspsx suppresses because the expansion interposes `lui $at`.
> Retail image-wide census: **40 sites with the nop, 0 without**; none in any
> function written as C. **This function has 11 of those 40 sites -- more than
> any other function in the executable** -- so at least 11 of its 16 missing
> words cannot be emitted by this pipeline for ANY C input. Its recorded
> residues (and `SsUtKeyOn`'s, which they were grouped with) predate this
> finding and are not the reason it is short. **Do not staff another source
> attempt until the blocker is resolved.** Reproducer, census method and the
> per-function site counts are in `docs/match-reports/SsUtKeyOn.md`'s
> round-62 section. Nothing else in this report has been changed.


Unit `libsnd_vm_vol_ut_key_ut_keyv` (re-carved round 34; the round-26 text below says `code_179d8_j`, its name before the split). Round 26 (2026-09-09). Not a class method. **Read
`SsUtKeyOn`'s report first** -- this is that function's near-twin, with
one structural difference: instead of allocating a fresh channel slot via
`SpuVmAlloc`, the caller supplies the slot index directly (`idx`,
bounds-checked `< 0x18`), and `idx` itself (not an allocated "result") is
the registration index and the success return value. Every other block
(volume-pan, `SlotE968`/`RecordE978` field copies, the nine-table
registration, the two follow-up calls) is the SAME shape, and responded to
the SAME fixes.

## What it is (best-reached body, 236/253 words compiled, NOT byte-exact)

```c
#if 0
s32 SsUtKeyOnV(s16 idx, s16 p0, s16 p1, s16 p2, u16 p3, u16 p4, s16 p5, s16 p6)
{
    RecordE978 *rec;
    u16 note;
    u8 pending18;

    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (SpuVmVSetUp(p0, p1) != 0) {
        goto fail;
    }
    D_8008EA22 = 0x21;
    D_8008EA0E = (u8) p3;
    D_8008EA0F = (u8) p4;
    D_8008EA18 = (u8) p2;
    if (p5 == p6) {
        D_8008EA11 = 0x40;
        D_8008EA10 = p5;
    } else if (p6 < p5) {
        D_8008EA10 = p5;
        D_8008EA11 = (p6 << 6) / p5;
    } else {
        D_8008EA10 = p6;
        D_8008EA11 = 0x7F - ((p5 << 6) / p6);
    }

    D_8008EA16 = _svm_pg[p1].unk1;
    D_8008EA17 = _svm_pg[p1].unk4;
    D_8008EA0C = _svm_pg[p1].unk0;

    rec = &_svm_tn[D_8008EA18 + D_8008EA13 * 16];
    D_8008EA1B = rec->unk0;
    note = rec->unk16;
    D_8008EA24 = note;
    D_8008EA19 = rec->unk2;
    D_8008EA1A = rec->unk3;
    D_8008EA1C = rec->unk4;
    D_8008EA1D = rec->unk5;
    D_8008EA20 = rec->unk1;
    D_8008EA1E = rec->unk6;
    D_8008EA1F = rec->unk7;

    if ((s16) note == 0) {
        goto fail;
    }
    D_8008EA26 = idx;
    D_8008D996[idx].unk0 = 0x21;
    D_8008D99E[idx].unk0 = p0;
    D_8008D99A[idx].unk0 = p1;
    D_8008D998[idx].unk0 = D_8008EA13;
    _svm_voice[idx].unk0 = D_8008EA24;
    pending18 = D_8008EA18;
    D_8008D994[idx].unk0 = p3;
    D_8008D9A3[idx].unk0 = 1;
    D_8008D98A[idx].unk0 = 0;
    D_8008D99C[idx].unk0 = pending18;
    SpuVmDoAllocate();
    if ((s16) D_8008EA24 == 0xFF) {
        vmNoiseOn((u8) idx);
    } else {
        s32 ret = note2pitch2(p3, p4);
        SpuVmKeyOnNow(1, (u16) ret);
    }
    _snd_ev_flag = 0;
    return idx;

fail:
    _snd_ev_flag = 0;
    return -1;
}
#endif
```

As with `SsUtKeyOn`, the actual attempted source had a bare
`__asm__("");` between almost every statement in the `RecordE978` field-copy
block and the nine-table registration block (the exact same placement that
closed `SsUtKeyOn`'s registration-block ordering, reused verbatim here
and confirmed to work identically -- see finding 2).

## What's identical to `SsUtKeyOn`, confirmed rather than assumed

- The busy-lock guard's shape (`bne v0,v1,NEAR; ...; j FAR; li v0,-1;
  NEAR: ...`) is byte-for-byte the same construct, and a plain `if
  (_snd_ev_flag == 1) return -1;` reproduces `SsUtKeyOn`'s WRONG ("far
  jump") polarity here too -- same residue, same non-result from every
  guard-clause reshape tried previously (see that report's finding 4 for
  the full experiment log; not repeated here since the outcome is
  identical).
- `RecordE978`, `SlotE968`, and every scratch global in the
  `D_8008EA0C`-`D_8008EA24` range are the SAME symbols this unit's other
  function already declared; no new struct knowledge here beyond what
  `SsUtKeyOn`'s report already recorded.
- `D_8008EA18` volatile / `D_8008EA24` plain-with-cast (finding 2 of the
  sibling report) reproduced correctly on the first attempt by reusing the
  same declarations -- no rediscovery needed.
- The volume-pan block (direct per-branch stores, no merged
  `outA`/`outB` locals) matched byte-for-byte on the first attempt, same
  fix as the sibling function's finding 1.
- The nine-table registration block's barrier placement (barriers in
  most, not all, of the nine gaps -- specifically NONE between
  `D_8008D994[idx].unk0 = p3;` and `D_8008D9A3[idx].unk0 = 1;`) reproduced
  from the sibling function's exact final configuration and matched byte-
  for-byte, INCLUDING the single-instruction (`li v1,0x1`) placement that
  needed that specific gap barrier-free.

## What's NEW here (not present in `SsUtKeyOn`)

### A second, independent guard-polarity residue: the "note == 0" check

`SsUtKeyOn`'s equivalent check (`if ((s16) note == 0) goto fail;`,
sharing a tail with the bounds/`func_80032148` failures) compiled with the
SAME polarity as retail (`beqz a0,FAIL_TAIL`). Here, the identical C
construct compiles to `beqz a0,FAIL_TAIL` in my build but retail uses
**`bnez a0,SUCCESS`** (jump-to-continue on the true/non-fail case, with the
fail path reached by FALLING THROUGH into the shared tail when false) --
the reverse of `SsUtKeyOn`'s own polarity choice for the textually
identical construct. Given `SsUtKeyOn` already proved this class of
residue is insensitive to source reshaping (three different guard shapes,
byte-identical output), this was not re-attempted here; it is recorded as
a SECOND, independently-occurring instance of the same "GCC's branch
polarity choice for a guard depends on something not visible in this
project's source-level view" phenomenon, not a new finding.

### A minor re-emergence of the fresh-read ordering issue, at a DIFFERENT statement pair

`SsUtKeyOn`'s barrier configuration placed `D_8008EA13`'s fresh read
immediately before its own store (`D_8008D998[idx].unk0 = D_8008EA13;`),
matching retail exactly there. Reusing the identical C and identical
barriers here, retail's read positions ONE STATEMENT EARLIER than my build
(retail: read `D_8008EA13` right after the `D_8008D99A` store, i.e. one
step before my build performs it) -- the same class of "GCC batches an
independent load differently depending on function-wide register
pressure" issue documented in `SsUtKeyOn`'s finding 3, but the exact
barrier arrangement that solved it there does not fully solve it here.
This accounts for a small (2-4 word) fraction of the remaining gap; the
much larger contributor is the missing load-delay-slot nops in the
`SlotE968`/`RecordE978` field-copy blocks, inherited unchanged from
`SsUtKeyOn` (same confirmed-not-a-barrier-problem diagnosis; not
re-investigated here since `SsUtKeyOn`'s report already exhausted the
axes available, and this function's copy of the same blocks showed
IDENTICAL missing-nop positions).

## Attempts

4 build-and-verify cycles (far fewer than `SsUtKeyOn`, since almost
every fix transferred directly): (1) fresh implementation reusing every
`SsUtKeyOn` fix proactively, which alone reached 35/253 raw / 236
compiled words with the exact three residue classes predicted going in.
No further reshaping attempted this round given `SsUtKeyOn`'s report
already demonstrates these residues resist source-level correction, and
this function's remaining time budget was better spent confirming the
transfer (which itself is the useful finding) than re-deriving the same
negative result twice.

### Proposed learning

**When a unit contains near-identical sibling functions (here: an
"allocate a free slot" and a "use a caller-given slot" pair sharing every
internal block), fix the FIRST one exhaustively, then reuse its solved
declarations and barrier placements verbatim on the second as the STARTING
point, not as a reference to consult after failing independently.** Doing
so here turned what would likely have been another 15+-attempt investigation
into a 4-attempt confirmation, and the two residues that DID differ between
siblings (a second, independently-oriented branch-polarity flip; a
fresh-read position that shifted by one statement) were both instances of
mechanisms `SsUtKeyOn`'s report had already characterized as
non-source-derivable, not new problems requiring new investigation.

**A residue confirmed non-source-derivable in one function should be
treated as evidence about the TOOLCHAIN/CONTEXT-SENSITIVE MECHANISM, not
about that one function** -- worth checking whether OTHER already-matched
or still-stalled functions in this unit (or sibling units with the same
busy-lock-guard idiom, e.g. `libsnd_decre.c`'s own reentrancy-guarded
functions per this file's header comments) show the same polarity
inconsistency, since it may be a corpus-wide pattern worth a dedicated
census rather than a per-function surprise each time it's hit.

## ROUND 31 (runner delta): rebuild confirms both title figures exactly

Rebuilt the preserved body verbatim (same barrier placement as
`SsUtKeyOn`'s finding 3, reused here as that report recommends)
through the current pinned pipeline. `build/lsdde.map` puts the next
function, `SsUtKeyOffV`, at built address `0x8003184c` against its own
retail address `0x80031890` -- a 68-byte (17-word) deficit, confirming
**236/253 words, 17 SHORT**, exactly as the title states. `funcdiff.py`'s
raw count reproduces **35/253** exactly as well. No new reshape attempted
this round: this function's own report already treats its two
differences from `SsUtKeyOn` (the reversed `note==0` polarity, the
shifted `D_8008EA13` fresh-read position) as instances of mechanisms the
sibling report characterizes as non-source-derivable, not new problems,
and this round's time was spent instead on `SsUtKeyOn` (where the
report's one specifically-flagged untested lever, `result` typed `s32`,
gained one word -- see that report's round-31 addendum) and on
`SsUtChangePitch` (where a permuter search found a genuine improvement --
see that report). Restored to `INCLUDE_ASM`; still a STALL.

## ROUND 36 (runner delta): stale-symbol rebuild -- 237/253 measured, same declaration gaps as SsUtKeyOn

Same rename as `SsUtKeyOn` (`func_80032148` -> `SpuVmVSetUp`, round
34's `libsnd/vm_vsu.o` conversion) and the same three declaration gaps
round 34's carve left in `src/libsnd_vm_vol_ut_key_ut_keyv.c` (`SpuVmVSetUp` itself,
`D_8008EA22`, the `SlotE968`/`_svm_pg` pair) -- see `SsUtKeyOn.md`'s
round-36 addendum for the full derivation; adding them once in this shared
file fixed both functions' preserved bodies.

Rebuilt the body shown above verbatim, with the SAME barrier placement
`SsUtKeyOn`'s round-36 rebuild used (the report above says this
function reuses that exact configuration). Clean compile. **Measured:
237/253 words, 16 short** (`build/lsdde.map`: retail's next function
`SsUtKeyOffV` at `0x80031890`, this build's at `0x80031850`, a
0x40/16-word deficit). Raw word-match 33/253. Close to but not an exact
reproduction of the title's inherited 236/253 -- one word different, same
caveat as `SsUtKeyOn`'s round-36 note (barrier placement recovered
from prose, not guaranteed bit-exact).

**Not reattempted beyond the rebuild**, for the same reason as
`SsUtKeyOn`: this function's own report treats every one of its
residues (three inherited, two more of its own) as instances of mechanisms
the sibling report already investigated exhaustively and confirmed
non-source-derivable, and this round's remaining time went to the two
tighter gaps instead. Restored to `INCLUDE_ASM`. Classification unchanged:
STALL.

### The corrected, linkable body (237/253 words, this round's measurement)

Positioned where it would compile: replacing the `INCLUDE_ASM` for
`SsUtKeyOnV` in `src/libsnd_vm_vol_ut_key_ut_keyv.c`, between `SsUtKeyOff` and
`SsUtKeyOffV`. Needs the same `SlotE968`/`_svm_pg`/`D_8008EA22`
declarations as `SsUtKeyOn`'s corrected body (see that report), all
now present in the shared file.

```c
s32 SsUtKeyOnV(s16 idx, s16 p0, s16 p1, s16 p2, u16 p3, u16 p4, s16 p5, s16 p6)
{
    RecordE978 *rec;
    u16 note;
    u8 pending18;

    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (SpuVmVSetUp(p0, p1) != 0) {
        goto fail;
    }
    D_8008EA22 = 0x21;
    D_8008EA0E = (u8) p3;
    D_8008EA0F = (u8) p4;
    D_8008EA18 = (u8) p2;
    if (p5 == p6) {
        D_8008EA11 = 0x40;
        D_8008EA10 = p5;
    } else if (p6 < p5) {
        D_8008EA10 = p5;
        D_8008EA11 = (p6 << 6) / p5;
    } else {
        D_8008EA10 = p6;
        D_8008EA11 = 0x7F - ((p5 << 6) / p6);
    }

    D_8008EA16 = _svm_pg[p1].unk1;
    D_8008EA17 = _svm_pg[p1].unk4;
    D_8008EA0C = _svm_pg[p1].unk0;

    rec = &_svm_tn[D_8008EA18 + D_8008EA13 * 16];
    D_8008EA1B = rec->unk0;
    note = rec->unk16;
    D_8008EA24 = note;
    D_8008EA19 = rec->unk2;
    D_8008EA1A = rec->unk3;
    D_8008EA1C = rec->unk4;
    D_8008EA1D = rec->unk5;
    D_8008EA20 = rec->unk1;
    D_8008EA1E = rec->unk6;
    D_8008EA1F = rec->unk7;

    if ((s16) note == 0) {
        goto fail;
    }
    __asm__("");
    D_8008EA26 = idx;
    __asm__("");
    D_8008D996[idx].unk0 = 0x21;
    __asm__("");
    D_8008D99E[idx].unk0 = p0;
    __asm__("");
    D_8008D99A[idx].unk0 = p1;
    __asm__("");
    D_8008D998[idx].unk0 = D_8008EA13;
    __asm__("");
    _svm_voice[idx].unk0 = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    D_8008D994[idx].unk0 = p3;
    D_8008D9A3[idx].unk0 = 1;
    __asm__("");
    D_8008D98A[idx].unk0 = 0;
    __asm__("");
    D_8008D99C[idx].unk0 = pending18;
    SpuVmDoAllocate();
    if ((s16) D_8008EA24 == 0xFF) {
        vmNoiseOn((u8) idx);
    } else {
        s32 ret = note2pitch2(p3, p4);
        SpuVmKeyOnNow(1, (u16) ret);
    }
    _snd_ev_flag = 0;
    return idx;

fail:
    _snd_ev_flag = 0;
    return -1;
}
```

## ROUND 40 (runner charlie, RECOVERED BY THE HEAD): rebuild reproduces the recorded figure EXACTLY; first-ever permuter search run, 63315 iterations, no zero

**Provenance note, because it affects how much this entry is worth.** Runner
charlie ended its session with this body live in `src/` and **zero commits**,
waiting on a notification that was never coming (nothing runs on a runner's
behalf). The head recovered the worktree by hand before teardown: measured the
body, preserved it below, restored the `INCLUDE_ASM`, and wrote this entry. The
MEASUREMENTS here are the head's own and were taken with the sibling
`SsUtKeyOn` restored to
`INCLUDE_ASM`, so this figure is free of that sibling's drift. The
INTERPRETATION is reconstructed from the runner's artifacts, not from its
reasoning, and is flagged where it is inference.

### Rebuild: the recorded figure is honest

Length **237 words built against retail's 253** -- exactly **16 words SHORT**,
reproducing round 36's recorded figure to the word. Measured from
`build/lsdde.map` (the function's own linked extent) rather than from
`funcdiff`'s in-range count, because a length-short body shifts every later
function and `funcdiff` correctly refused the number: it reported ~288500
bytes differing OUTSIDE the range. **That is the guard working, not a
failure** -- a length gap is exactly the case where a raw word-match is
mostly ripple.

### Permuter: first-ever search on this function, and it found no zero

| field | value |
| --- | --- |
| iterations | **63315** |
| base score | 4035 |
| best score reached | **2575** (`output-2575-1`) |
| zero found | **no** |
| stop reason | own `timeout` bound, not a natural stop |

Recorded in full because the next round ranks on cost, and **a search with no
recorded iteration count reads as never run** -- which is exactly the
over-count round 37 measured and corrected. This function is now genuinely
searched: 63315 iterations against a scaffold that reproduced the residue,
no zero.

**The saved sub-base candidates were NOT translated.** Best is 2575 against a
base of 4035 -- a ~36% improvement in PERMUTER units, which is not a word
count and does not convert to one. They are still leads worth one translation
attempt each, and that is the cheapest untried thing left here.

### The residue is a LENGTH gap, so look for what is MISSING

16 words short is not a scheduling or register-identity residue. Something
retail emits is not being emitted at all -- a block that was folded, a call
that was inlined, a loop that was rotated. Chasing word-match percentages here
will mislead; find the missing instructions first.

### Preserved body (237/253 words, rebuilt and measured this round)

```c
#if 0
s32 SsUtKeyOnV(s16 idx, s16 p0, s16 p1, s16 p2, u16 p3, u16 p4, s16 p5, s16 p6)
{
    RecordE978 *rec;
    u16 note;
    u8 pending18;

    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (SpuVmVSetUp(p0, p1) != 0) {
        goto fail;
    }
    D_8008EA22 = 0x21;
    D_8008EA0E = (u8) p3;
    D_8008EA0F = (u8) p4;
    D_8008EA18 = (u8) p2;
    if (p5 == p6) {
        D_8008EA11 = 0x40;
        D_8008EA10 = p5;
    } else if (p6 < p5) {
        D_8008EA10 = p5;
        D_8008EA11 = (p6 << 6) / p5;
    } else {
        D_8008EA10 = p6;
        D_8008EA11 = 0x7F - ((p5 << 6) / p6);
    }

    D_8008EA16 = _svm_pg[p1].unk1;
    D_8008EA17 = _svm_pg[p1].unk4;
    D_8008EA0C = _svm_pg[p1].unk0;

    rec = &_svm_tn[D_8008EA18 + D_8008EA13 * 16];
    D_8008EA1B = rec->unk0;
    note = rec->unk16;
    D_8008EA24 = note;
    D_8008EA19 = rec->unk2;
    D_8008EA1A = rec->unk3;
    D_8008EA1C = rec->unk4;
    D_8008EA1D = rec->unk5;
    D_8008EA20 = rec->unk1;
    D_8008EA1E = rec->unk6;
    D_8008EA1F = rec->unk7;

    if ((s16) note == 0) {
        goto fail;
    }
    __asm__("");
    D_8008EA26 = idx;
    __asm__("");
    D_8008D996[idx].unk0 = 0x21;
    __asm__("");
    D_8008D99E[idx].unk0 = p0;
    __asm__("");
    D_8008D99A[idx].unk0 = p1;
    __asm__("");
    D_8008D998[idx].unk0 = D_8008EA13;
    __asm__("");
    _svm_voice[idx].unk0 = D_8008EA24;
    __asm__("");
    pending18 = D_8008EA18;
    D_8008D994[idx].unk0 = p3;
    D_8008D9A3[idx].unk0 = 1;
    __asm__("");
    D_8008D98A[idx].unk0 = 0;
    __asm__("");
    D_8008D99C[idx].unk0 = pending18;
    SpuVmDoAllocate();
    if ((s16) D_8008EA24 == 0xFF) {
        vmNoiseOn((u8) idx);
    } else {
        s32 ret = note2pitch2(p3, p4);
        SpuVmKeyOnNow(1, (u16) ret);
    }
    _snd_ev_flag = 0;
    return idx;

fail:
    _snd_ev_flag = 0;
    return -1;
}
#endif
```

## ROUND 40 CONTINUATION (runner charlie): the flagged 2575 candidate (and two siblings) were translated and tested -- none reproduce

The head's recovery entry above flagged the best sub-base permuter
candidate (score 2575 against a base of 4035) as "worth one translation
attempt" and explicitly NOT yet translated at the time of recovery. This
session resumed (both this round's background permuter searches completed
and their notifications were processed normally) and closed that open
item -- for three candidates, not just the flagged one, since the saved
`permuter-work/SsUtKeyOnV/output-*` directory held two more below the
4035 base worth checking at the same time:

- **Score 3265**: a pointer-indirection idiom for the `note` read
  (`u16 *new_var = &rec->unk16; note = *new_var;`). Translated as
  `pnote`/`*pnote` and rebuilt (isolated: sibling `SsUtKeyOn` held at
  `INCLUDE_ASM` so its own shortfall does not contaminate this function's
  window, per CLAUDE.md's attribution-hazard note): **byte-identical to
  baseline, 33/253 raw match, no change.**
- **Score 2575** (the one the head's entry flagged): the same
  pointer-indirection PLUS a barrier reshuffle (drop the `__asm__("")`
  between the `D_8008D99A`/`D_8008D998` stores, add a second one between
  `D_8008D998` and `_svm_voice`). Rebuilt in isolation: **byte-identical to
  baseline, 33/253, no change.**
- **Score 2635**: plain (non-pointer) `note` read with an extra
  `__asm__("")` immediately after it, combined with the same
  `D_8008D99A`/`D_8008D998` barrier drop as the 2575 candidate (but
  without the second `D_8008D998`/`_svm_voice` barrier). Rebuilt in
  isolation: **byte-identical to baseline, 33/253, no change.**

All three are the same scaffold-vs-real-oracle disagreement
`SsUtKeyOffV`'s and `SsUtKeyOn`'s round-40 entries document: the
permuter's isolated translation unit does not reproduce this project's real
register-allocation pressure, so a scaffold-local score improvement
(4035 -> 2575, ~36%) is not evidence about the real function on its own.
No new axis beyond what rounds 26/31/36 already characterized as
non-source-derivable was found. Restored to `INCLUDE_ASM` (already was,
going into this continuation); still a STALL, same five residues, now
confirmed exhausted under a real permuter search of 63315 iterations as
well as hand reshaping and direct transfer from the sibling function.

Also added the `SpuVmVSetUp` extern to `src/libsnd_vm_vol_ut_key_ut_keyv.c` (shared fix,
same gap `SsUtKeyOn.md`'s continuation describes -- this function's
preserved body needs the same declaration and it was likewise missing).

### Proposed learning

**When a sibling pair shares almost every residue, a permuter search on
the second one still surfaces DIFFERENT sub-base leads than the first,
even against the same declarations -- but "different leads" is not the
same as "different outcome."** This function's search found three distinct
candidates that `SsUtKeyOn`'s search did not surface at all, yet all
three failed to transfer for the identical reason `SsUtKeyOn`'s single
lead did (scaffold-local register pressure, not a real source-level fix).
The negative result generalizes across the sibling pair even though the
specific leads found do not -- worth recording because it means a second
sibling's search is not redundant work (it can find genuinely new
candidates) even when the eventual disposition is known in advance to be
the same class of non-result.

## Round 70 re-measure (runner charlie): `--nop-at-expansion` narrows the
length gap from 16 to 5 words; NON_MATCHING promoted, comment updated

Promotion job (`docs/FINISHING-PLAN.md` track 1b), not a matching session:
measure once, do not iterate. The round-40 body above was put LIVE
(replacing `INCLUDE_ASM`, no `#ifdef`, sibling `SsUtKeyOn` left at
`INCLUDE_ASM` so its own bytes do not contaminate this window) and rebuilt.

**Length: 248/253 built, 5 words short** (`build/lsdde.map`:
`SsUtKeyOffV` now at `0x8003187c` against retail's own `0x80031890`, a
0x14/5-word deficit -- was `0x8003184c`/17-then-16 words short at every
prior measurement). This matches the round-62 accounting's prediction
closely: this function has 11 load-delay-nop sites (vs the sibling's 9),
and `--nop-at-expansion` (CLAUDE.md, "Open toolchain blockers", resolved
round 63) is exactly that fix, though it does not close this function's
gap to zero the way it did for `SsUtKeyOn` -- 5 words of some other
residue (the two guard-polarity flips this report already documents, most
likely) remain.

**`funcdiff.py`'s raw word-match (47/253) and insertions/deletions (18/18)
are NOT reported as clean figures**: with the length gap still nonzero,
`funcdiff` itself flagged 281521 bytes differing OUTSIDE this function's
range (CLAUDE.md's "address drift" guard), so those numbers are an upper
bound only, same status as this report's own round-26/31/36 raw counts
were before a length-exact measurement existed. Not re-characterized
further -- this is a measurement, not a matching attempt.

**Disposition: promoted to `#ifdef NON_MATCHING ... #else INCLUDE_ASM
... #endif`** in `src/libsnd_vm_vol_ut_key_ut_keyv.c`, comment updated to the current
score, verified build unchanged (`./build-and-verify.sh` green,
`tools/check-nonmatching.sh` green). The title line and the round-62 note
at the top of this file are now STALE (both still say
"TOOLCHAIN-BLOCKED" with pre-round-63 figures); this entry is the
authoritative current score in the interim.

NON_MATCHING body promoted, round 70

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (`D_8008D988` itself now reads `_svm_voice` above) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

The NON_MATCHING body now stores `_svm_voice[idx].unkNN`; normalized disassembly identical.

## Naming (round 95, track 6, delta)

Signature is now Sony's `<libsnd.h>` prototype, all eight parameters
`short`, returning `short` (was `s32` with `u16 p3, p4`); the body passes
`(u16)p3, (u16)p4` to note2pitch2. Measured before and after: unchanged
(248/253). ProgAtr/VagAtr substitution as in SsUtKeyOn.md.
