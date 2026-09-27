# SePitchBend -- MATCHED (round 73, 112/112, whole-image SHA1 green)

> Renamed from `func_8002E138` on 2026-09-24 (tools/rename.py). Address 0x8002e138.

REVISITED, round 73: MATCHED 112/112, fresh transcription, no permuter; names/types used (params `chan`/`bend`, local 0x34-stride record views, `D_8008D7F4` written as `((u16 *)_svm_sreg_buf)[off + 2]`)

## Round 73 (delta): matched

**Preserved round-33 body rebuilt first, unchanged:** it now compiles to
0x1C8 = **114 words** (2 long, not the 113 on file), funcdiff raw 6/112,
`insertions 10 / deletions 10`, positional skeleton diffs 106.

The four earlier rounds called the leading `move a0,s0` "register
identity, unfixable from C". It was a source-shape problem, and so was
everything else. Every cause below was measured through `cc1 -dr/-dc/-dS`
dumps or an ablation build of the final body. Each ablation reverts that
one item and nothing else:

1. **The extra `move a0,s0` is a QImode pseudo, from `(u8)chan`.** In the
   RTL dump, `(u8)x` expands to `(set (reg:QI 81) (subreg:QI (reg 71)))`
   then `zero_extend`. cse reuses QI reg 81 for every later `(u8)chan`,
   so it stays live and gets its own hard register, and that is the copy.
   `chan & 0xFF` expands to a plain `and:SI` of the SI pseudo, which gives
   retail's `andi $v1,$s0,0xff` with no copy. Ablation: `(u8)chan`
   everywhere gives 113 words.
2. **Shared index before the branch.** `idx = D_8008EA18 + (*p << 4);`
   is computed once before `if (b >= 0)`, and `_svm_tn[idx]` is indexed
   in each arm. Retail computes `$a2` before the `bltz`, puts `sll v0,a2,5`
   in its delay slot, and reloads the `_svm_tn` pointer per arm. Inline
   in each arm, the index is recomputed per arm: 119 words -> 113.
3. **Unsigned bound test.** `(u32)(chan & 0xFF) < 24` gives `sltiu`. The
   and-form alone gives `slti`, because the QI zero-extend was what made
   the value unsigned.
4. **`&D_8008EA13` in a register.** Retail stores to and reloads EA13
   through `$a0` (`lui/addiu a0` + `sb v0,0(a0)` ... `lbu v0,0(a0)`). A
   `u8 *p = &D_8008EA13;` local does exactly that. Ablation without `p`:
   113 words.
5. **Division register roles are set by which values are named.** Arm 1:
   `note = base + prod / 127; fine = prod % 127;` with no `q` local.
   Ablation with a `q` local: 99/112, same length. Arm 2:
   `q = (b * e->unk12) / 127;` with no `prod` local. Ablation with
   `prod`: 106/112, ins/del 1/1.
6. **Tail order: the D7F4 store is really `_svm_sreg_buf[off + 2]`.** With
   `D_8008D7F4[off]` the sched dump shows the `_svm_sreg_dirty` load hoisted
   above the halfword store. The two addresses are `(plus reg symbol)` on
   different symbols, which 2.6.3's dependence test proves disjoint.
   Retail keeps source order and leaves an unfilled load-delay `nop`.
   `((u16 *)_svm_sreg_buf)[off + 2]` puts the constant into the address as
   `(const (plus _svm_sreg_buf 4))`, the test can no longer separate them, and
   the order holds (111 -> 112/112). This also explains `off = chan * 8`:
   it is a u16-element index into the 16-byte per-voice record at
   `_svm_sreg_buf`, the same record `vmNoiseOn2` writes through
   `_svm_sreg_buf`/`D_8008D7F2`. It is not a byte offset.

Build figures (words / raw): a (clean transcription) 119; b (+idx hoist)
113; c (+`& 0xFF`) 111; d (+`(u32)`, EA26 via `(u8)`) 112, 34/112, ins/del
7/7; e (+`p`) 111; h (+arm1/arm2 shapes) 111; **j (+`_svm_sreg_buf[off+2]`)
112/112, SHA1 OK**. Ablations of j: no `p` 113; `q` in arm 1 99/112;
`prod` in arm 2 106/112; `(u8)chan` everywhere 113.

A note on this session's own builds: another runner's helper script
overwrote this runner's `b.sh` in the shared session scratchpad, so some
intermediate reads (e through i) came first from the wrong worktree. Every
figure above was re-measured afterwards through a per-runner copy, and the
match was confirmed with a direct `./build-and-verify.sh`.

### Matched body

`D_8008EA26` is declared `extern s16 D_8008EA26[]` here to agree with the
unit's existing `#ifdef NON_MATCHING` declaration (a scalar `u16` failed
`tools/check-nonmatching.sh` with `conflicting types`); same `sh`, same bytes.

```c
typedef struct {
    u8 unk0;
    u8 pad1[0x34 - 0x1];
} Rec34B_E138;
typedef struct {
    u16 unk0;
    u8 pad2[0x34 - 0x2];
} Rec34H_E138;
extern Rec34B_E138 D_8008D998[];
extern Rec34B_E138 D_8008D99C[];
extern Rec34H_E138 D_8008D994[];
extern s16 D_8008EA26[];
extern u8 _svm_sreg_dirty[];

void SePitchBend(s32 chan, s32 bend) {
    s32 off;
    s32 prod;
    s32 q;
    s32 note;
    s32 fine;
    s16 b;
    s32 idx;
    u8 *p;

    off = (chan & 0xFF) * 8;
    if ((u32)(chan & 0xFF) < 24) {
        p = &D_8008EA13;
        *p = D_8008D998[(chan & 0xFF)].unk0;
        D_8008EA18 = D_8008D99C[(chan & 0xFF)].unk0;
        D_8008EA26[0] = (u8)chan;
        idx = D_8008EA18 + (*p << 4);
        b = bend;
        if (b >= 0) {
            prod = b * _svm_tn[idx].unk13;
            note = D_8008D994[(chan & 0xFF)].unk0 + prod / 127;
            fine = prod % 127;
        } else {
            q = (b * _svm_tn[idx].unk12) / 127;
            note = D_8008D994[(chan & 0xFF)].unk0 + q - 1;
            fine = q + 127;
        }
        ((u16 *)_svm_sreg_buf)[off + 2] = note2pitch2((u16)note, (u16)fine);
        _svm_sreg_dirty[(chan & 0xFF)] |= 4;
    }
}
```

### Proposed learning

**`(u8)x` and `x & 0xFF` are different RTL, and the difference is a
register.** A cast to `u8` goes through a QImode subreg pseudo that cse
reuses, so repeated `(u8)x` keeps a QI pseudo alive, which becomes an
extra `move` right after the callee-saved copy. `x & 0xFF` is an SImode
`and` on the original pseudo. Screen: retail `move sN,aK` followed by
`andi rX,sN,0xff` at every use, while your build has `move aK,sN` or
`andi rX,aK,0xff` first, means write `& 0xFF`. Corollary: a bound test on
the and-form needs an explicit `(u32)` to get `sltiu`.

**A load you cannot keep below a store can come from the store's address
spelling.** When two symbol-indexed accesses on different symbols reorder
in your build and not in retail, write the store as
`((T *)BASE)[i + k]` so the constant sits in the address. 2.6.3's
dependence test then cannot prove the accesses disjoint. Check the splat
symbols first: a `D_X+4` that splat named separately is often a field of
a record at `D_X`.

---

## History (rounds 33-44, superseded by the match above)


**4 words short** (best derivation compiled to 108 words against retail's
112, even after fixing the residues listed below — one more still open).
Raw word-match is not meaningful under that drift. **First real diff is the
second instruction**, vram `0x8002E140`/file `0x1E940` is the last matching
one (`move $s0,$a0`); retail's very next instruction, `andi $v1,$s0,0xFF`,
has an extra `move $a0,$s0` inserted before it in every derivation tried —
confirmed with `tools/asm-differ/diff.py SePitchBend`, not inferred.

`code_179d8_l`, vram `0x8002E138`, file offset `0x1E938`, 112 words
(0x1C0 bytes), real frame (`-0x20`, saves `$s0`,`$s1`,`$ra`). No
`nop_mflo_mfhi` or `gp_rel` hits. Calls `note2pitch2` (matched this round,
`docs/match-reports/note2pitch2.md`) — this function sets up
`D_8008EA13`/`D_8008EA18`/`D_8008EA26` right before calling it, which is the
same trio `note2pitch2` reads.

## What it computes

Channel setup, called with (channel, pitch-ish `a1`). If `chan < 24`: looks
up two per-channel bytes from 52-byte-stride tables (`D_8008D998`,
`D_8008D99C`) into `D_8008EA13`/`D_8008EA18` (also mirrors the raw channel
byte into `D_8008EA26`), recomputes the same `D_8008EA18 + (D_8008EA13<<4)`
index `note2pitch2` uses to reach `_svm_tn[]`, then splits on the sign
of `(s16)a1`: multiplies it by one of two NEW fields on that struct (offset
`0xC`/`0xD` — added to `D8008E978Entry` this round, see below) and divides
the product by **127** (confirmed empirically, see next section), producing
a note-index/fractional-cents pair fed to `note2pitch2(arg0, arg1)`. The
two branches are NOT symmetric: the positive branch's second return value is
the true remainder (`prod - q*127`); the negative branch's is `q + 127` and
its first return value gets an extra `-1` the positive branch doesn't have —
transcribed as-is, both branches confirmed instruction-for-instruction
against the `.s` before writing the C.

**`D8008E978Entry` gained two fields this round**, read only by this
function: offset `0xC` (`unk12`) and `0xD` (`unk13`), both plain bytes. The
struct (originally written for `note2pitch2`, this same file) is safe to
extend in place since it is used only within this unit — no other unit
includes this header (unit-local convention, `common.h` only). Current
shape:

```c
typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[6];
    u8 unk12;
    u8 unk13;
    u8 unk14[18];
} D8008E978Entry;
```

This edit is ALREADY LIVE in `src/code_179d8_l.c` (kept even though this
function stalled, since `note2pitch2` — matched — still compiles
correctly against it and the layout is now-confirmed knowledge for whoever
picks this function back up).

## The divisor-127 magic constant, derived empirically

Retail's second `mult` uses `0x81020409`; `CLAUDE.md`'s reproducer pipeline
was used to identify it (not the hand-derived Hacker's-Delight formula,
which didn't turn up a match in a 5000-divisor search — the constant only
matched a DIRECT compile of `a / 127` through the pinned pipeline):

```c
int probe(int a) { return a / 127; }
```
compiles to `lui 0x8102 / ori 0x409` with `mult, mfhi, addu $v0,$v0,$a0,
sra $v0,$v0,0x6, sra $a0,$a0,0x1f, subu $v0,$v0,$a0` — an exact instruction-
for-instruction match against both of retail's division sites here.

## Residues (both classes already seen this round, in `vmNoiseOn2`)

1. **A spurious extra register copy before the first read of a saved
   register**, same class as `vmNoiseOn2`'s residue 1. Retail reads
   `$s0` directly for `andi $v1,$s0,0xFF` with no preceding copy; every
   phrasing tried here (a separate `u8 chan` local, inlining `(u8)s0`
   everywhere instead, computing the mask before vs. after the shift)
   produces an extra `move $a0,$s0` (or `$a3,$s0`) first. Not resolved —
   consistent with the `vmNoiseOn2` finding that **C variable identity
   does not control physical register assignment** in this compiler; this
   is the second function in the round to hit the same wall, which
   promotes it from anecdote to a pattern worth flagging for the next
   runner (see proposed learning).
2. **A hoisted/reordered store**, same class as `vmNoiseOn2`'s residue
   2. `D_8008EA26 = (u8)s0;` (no dependency on the surrounding
   `D_8008EA13`/`D_8008EA18` stores) moves relative to them without a
   scheduling barrier. A bare `__asm__("")` placed right after the
   `D_8008EA13` store was applied (visible in the preserved body below) and
   it improved the local ordering, but did not close the remaining 4-word
   gap — something else after it still needs a second pass.

## Best-derived body (108/112 words, preserved for the next attempt)

```c
extern u8 D_8008D998[];
extern u8 D_8008D99C[];
extern u8 D_8008D994[];
extern u8 D_8008D7F4[];
extern u16 D_8008EA26;
extern u8 _svm_sreg_dirty[];

void SePitchBend(s32 a0, s32 a1) {
    s32 s0;
    s32 off16;
    s32 idx52;
    s32 idxStruct;
    s32 sa1;
    D8008E978Entry *e;
    u16 dval;
    s32 prod;
    s32 q;
    s32 arg0;
    s32 arg1;
    s32 v0;
    s32 v1;
    u8 *pEA13;

    s0 = a0;
    off16 = (u8)s0 << 3;
    if ((u8)s0 < 24) {
        idx52 = (u8)s0 * 52;
        pEA13 = &D_8008EA13;
        *pEA13 = D_8008D998[idx52];
        __asm__("");
        D_8008EA26 = (u8)s0;
        D_8008EA18 = D_8008D99C[idx52];

        idxStruct = D_8008EA18 + (*pEA13 << 4);
        sa1 = (s16)a1;
        e = &_svm_tn[idxStruct];
        if (sa1 >= 0) {
            prod = sa1 * e->unk13;
            q = prod / 127;
            dval = *(u16 *)(D_8008D994 + idx52);
            arg0 = dval + q;
            arg1 = prod - q * 127;
        } else {
            prod = sa1 * e->unk12;
            q = prod / 127;
            dval = *(u16 *)(D_8008D994 + idx52);
            arg0 = (dval + q) - 1;
            arg1 = q + 127;
        }
        arg0 = (u16)arg0;
        v0 = note2pitch2(arg0, (u16)arg1);
        *(u16 *)(D_8008D7F4 + (off16 << 1)) = v0;
        v1 = (u8)s0;
        v0 = _svm_sreg_dirty[v1];
        v0 |= 4;
        _svm_sreg_dirty[v1] = v0;
    }
}
```

Note: `note2pitch2`'s prototype is already in scope (defined earlier in
this same file, ROM-address order) — no extra declaration needed.

## Attempts

Roughly 8 build/pipeline iterations (well within the 30-attempt cap): the
initial full transcription (close on control flow, the `sltiu`/`slti` type
lesson from `note2pitch2` reapplied to fix an unsigned-compare mismatch),
a pointer-reuse fix for `D_8008EA13` (matches retail's address-register
reuse for the reload later in the function), and the scheduling barrier for
`D_8008EA26`. Restored to `INCLUDE_ASM` per the hard rule.

### Proposed learning

**Two functions in one round (`vmNoiseOn2`, this one) both open with a
spurious extra register-to-register copy before the first use of a value
that must survive a function call (hence lives in a callee-saved
register), and no C-level reshaping removed it in either case.** This is
now a pattern, not a one-off: worth a name (something like "callee-saved
preload residue") so a future runner recognizes it early rather than
re-deriving the negative result. Both instances arose in functions with a
non-trivial `if` gating most of the body and a call to another function
partway through — that shape may be the trigger, but neither instance
pinned down a source-level fix, so treat any future match attempt fitting
that shape as a candidate stall on THIS specific residue before spending a
full budget on it.

## Round 33 update (runner alpha): 108/112 -> 113/112, via the already-validated "duplicate in both arms" idiom

Re-verified the inherited 108/112 body first — it reproduces exactly as
claimed (compiled length 0x1B0 = 108 words), and the realigned
`asm-differ` output confirms residue 1 (the spurious `move a0,s0` at the
very second instruction) at the exact position this report already names.

**Reading further into the realigned diff (past what this report's
original text covers) found a second, independent, and genuinely fixable
residue: the `D8008E978Entry *e = &_svm_tn[idxStruct];` pointer
computation was shared before the `if (sa1 >= 0)`/`else` split, but retail
recomputes the table's base address FRESH inside each arm** (a distinct
4-instruction `lui`/`lw`/`nop`/`addu` sequence appears a second time, deep
inside the `else` arm, with no counterpart in the shared-pointer
compilation). This is the identical "must be textually duplicated in BOTH
arms, not shared after/before the `if`" idiom already validated for
`SpuVmDoAllocate` (fix #3 in that report) and referenced generically in
`SpuVmAlloc`'s report — moving the single
`e = &_svm_tn[idxStruct];` statement to inside EACH branch (so it's
computed twice, once per arm) reproduced retail's double materialization
exactly and moved the function from **108/112 (4 short) to 113/112 (1
LONG)** — a swing of 5 words from one fix, all of it on the SAME residue
class already proven elsewhere in this unit.

**Two follow-up experiments on the remaining 1-word overage, both
negative:**

1. Reordering `off16 = (u8)a0 << 3; s0 = a0;` (computing `off16` from the
   raw parameter register before assigning `s0`, instead of the original
   `s0 = a0; off16 = (u8)s0 << 3;`). Same total length (113), but shifted
   the register-identity residue to a DIFFERENT register pair (`a2`
   instead of the extra `move a0,s0`) and delayed the `sw s0,...` prologue
   store — a different-shaped instance of the same unfixable class, not an
   improvement. Reverted.
2. A bare `__asm__("")` between the `D_8008D7F4[...] = v0;` store and the
   trailing `v1 = (u8)s0;` mask (targeting an apparent instruction-order
   difference in the tail: retail computes the `D_8008D7F4` store address
   before the `_svm_sreg_dirty` mask, my build did the same but scheduled
   slightly differently). Result: **regressed to 114/112** (2 over).
   Reverted.

**What remains, read off the realigned diff, not inferred:** exactly the
residue this report already documents as unfixable-from-C (the extra
`move a0,s0` — a genuine register-identity difference, HARD RULE 6
territory, already tried three ways in this report and confirmed
unmovable by declaration order again this round) plus one minor
instruction-reordering in the trailing `D_8008D7F4`/`_svm_sreg_dirty` sequence
that a barrier could not close (attempt 2 above) and that was not
re-investigated further given the 30-attempt budget is shared across five
queued functions this unit.

**Disposition: restored to `INCLUDE_ASM`.** This is real, measured
progress (108 -> 113, closing 5 of the original 4-short gap and then some)
even though the function is not byte-exact — recorded so the next runner
starts from 113/112 with the duplication fix already applied, rather than
re-deriving it from the stale 108/112 baseline.

```c
extern u8 D_8008D998[];
extern u8 D_8008D99C[];
extern u8 D_8008D994[];
extern u8 D_8008D7F4[];
extern u16 D_8008EA26;
extern u8 _svm_sreg_dirty[];

void SePitchBend(s32 a0, s32 a1) {
    s32 s0;
    s32 off16;
    s32 idx52;
    s32 idxStruct;
    s32 sa1;
    D8008E978Entry *e;
    u16 dval;
    s32 prod;
    s32 q;
    s32 arg0;
    s32 arg1;
    s32 v0;
    s32 v1;
    u8 *pEA13;

    s0 = a0;
    off16 = (u8)s0 << 3;
    if ((u8)s0 < 24) {
        idx52 = (u8)s0 * 52;
        pEA13 = &D_8008EA13;
        *pEA13 = D_8008D998[idx52];
        __asm__("");
        D_8008EA26 = (u8)s0;
        D_8008EA18 = D_8008D99C[idx52];

        idxStruct = D_8008EA18 + (*pEA13 << 4);
        sa1 = (s16)a1;
        if (sa1 >= 0) {
            e = &_svm_tn[idxStruct];
            prod = sa1 * e->unk13;
            q = prod / 127;
            dval = *(u16 *)(D_8008D994 + idx52);
            arg0 = dval + q;
            arg1 = prod - q * 127;
        } else {
            e = &_svm_tn[idxStruct];
            prod = sa1 * e->unk12;
            q = prod / 127;
            dval = *(u16 *)(D_8008D994 + idx52);
            arg0 = (dval + q) - 1;
            arg1 = q + 127;
        }
        arg0 = (u16)arg0;
        v0 = note2pitch2(arg0, (u16)arg1);
        *(u16 *)(D_8008D7F4 + (off16 << 1)) = v0;
        v1 = (u8)s0;
        v0 = _svm_sreg_dirty[v1];
        v0 |= 4;
        _svm_sreg_dirty[v1] = v0;
    }
}
```

### Proposed learning (round 33)

**The "duplicate the shared computation in both `if`/`else` arms instead
of sharing it before the branch" idiom (already named for `SpuVmDoAllocate`)
generalizes beyond an explicit shared STATEMENT to a shared POINTER/ADDRESS
computation feeding field accesses in both arms.** It is now confirmed on
two independent functions in this unit and is worth checking as a matter
of course any time a struct pointer or array-index base is computed once
before an `if`/`else` whose two arms both dereference it — retail is not
guaranteed to hoist it, and GCC 2.6.3 WILL, producing a function that is
short by exactly the size of the hoisted address computation (4-5 words is
the observed range so far).

## Round 35 update (runner bravo): inherited body re-verified real, three new levers tried, all negative

Re-verified the inherited 113/112 body first (per the "compile every inherited
body before trusting it" instruction): it reproduces exactly — `objdump -t`
on `build/src/code_179d8_l.c.o` shows `SePitchBend` at `0x1c4` bytes = 113
words, matching round 33's claim exactly. Confirmed via `asm-differ`'s
realigned diff that the residues are exactly as round 33 described: the
extra `move a0,s0` (later reappearing in a different register depending on
variable naming — see below) at the very second instruction, plus a smaller
tail reordering.

**Three new experiments, all negative, all reverted (`git diff --stat`
clean against the round-33 baseline afterward):**

1. **Moving `off16 = (u8)s0 << 3;` from the top of the function to right
   before its only use** (immediately before
   `*(u16 *)(D_8008D7F4 + (off16 << 1)) = v0;`). Theory: shortening `off16`'s
   live range might change which register the allocator gives it and avoid
   whatever forces the extra copy. Result: **regressed to 110/112** (0x1b8) —
   worse, not better. Reverted.
2. **Splitting the `*pEA13 = D_8008D998[idx52];` store into a temp
   (`ea13val = D_8008D998[idx52]; pEA13 = &D_8008EA13; *pEA13 = ea13val;`)
   and likewise for the `D_8008EA18 = D_8008D99C[idx52];` store**, on the
   theory that reordering when `&D_8008EA13`'s address gets materialized
   relative to the `D_8008D998` read might match retail's instruction order
   (retail computes the `D_8008D998` address FIRST, `&D_8008EA13`'s address
   SECOND; every C phrasing tried computes them in the opposite order,
   because C's `pEA13 = &D_8008EA13;` statement naturally materializes its
   address before the following statement's read). Result: **still
   113/112, byte-for-byte the same total, just one instruction traded for
   another** (`addiu a0,a0,-0x15ed` for a spurious `move t0,a3` at the same
   site) — neutral, not an improvement. This confirms the residue is not a
   simple statement-order artifact: reordering the SOURCE statements does
   not reorder the COMPILED instructions the same way, which is the
   register-identity signature HARD RULE 6 describes, not an
   instruction-scheduling one. Reverted.
3. **A bare `__asm__("")` between the `*(u16 *)(D_8008D7F4 + ...) = v0;`
   store and the `v1 = (u8)s0;` mask**, targeting the tail reordering where
   retail computes `off16<<1` and stores to `D_8008D7F4` BEFORE masking
   `(u8)s0` and reading `_svm_sreg_dirty`, while the compiled body does the
   mask+read first and the shift+store second. Result: **regressed to
   114/112** (0x1c8) — one word worse. Reverted.

**Disposition: no improvement this round; restored to `INCLUDE_ASM`,
confirmed via `git diff --stat` showing no diff against the pre-session
state.** The function is unchanged from round 33's 113/112. Both of this
round's structural residues (the leading register copy and the tail
reorder) resist every lever tried across two rounds now — reordering
source statements, retiming address materialization, and scheduling
barriers all either do nothing or regress. The leading residue continues
to look like straightforward HARD RULE 6 register-identity territory (not
fixable from C, and not a case for `register asm("$N")`); the tail
reorder's resistance to a barrier is consistent with `SeAutoVol` and
`SpuVmAlloc`'s independent round-33 finding that a barrier cannot
prevent a side-effect-free value/address computation from being reordered
across it when there is nothing for the barrier to protect.

**Not tried, for lack of remaining budget relative to this unit's other
four queued functions:** algebraically restructuring the tail's two
independent statement groups into a single expression that forces GCC to
serialize them in source order (e.g. via a sequence point inside one
combined expression, or a dummy volatile read) — untried because it starts
to resemble exactly the "not simply order, but value-numbering" class
`SpuVmAlloc`'s report already showed a barrier cannot reach, and no new
mechanism was identified this round that would behave differently.

### Proposed learning (round 35)

**Reordering C SOURCE statements to match retail's instruction order does
not reliably reorder the COMPILED instructions when the residue is
register-identity, not scheduling** — confirmed directly here: moving the
pointer-address materialization to before vs. after the value it stores
changed WHICH instruction was spurious (an `addiu` became a `move`) without
changing the total count. This is worth stating alongside the existing
"variable identifier choice has no influence on register assignment"
learning (`vmNoiseOn2`'s report) as a corollary: **statement REORDER,
not just naming, is also not a lever for this class** — both are surface
changes the allocator sees through.

## Round 37 update (runner alpha): first permuter search on this function, negative

Per this round's thesis (permuter is the primary lever for the four
never-searched functions in this unit), re-verified the inherited 113/112
body first: `objdump -t` on `build/src/code_179d8_l.c.o` confirms
`SePitchBend` compiles to `0x1c4` bytes = 113 words, matching round 33's
figure exactly.

**`--debug --stack-diffs` sanity check:** base score = **2020** (not the
"one insertion + one deletion, score 200" signature of a clean single-word
residue) — consistent with this report's own description of the leading
residue as a register-identity scramble that cascades through the whole
function's allocation, not an isolated one-instruction gap.
`Stack Differences: 0 (1)` was a real filled-in measurement, not the
unfilled-field false zero CLAUDE.md warns about (the debug output prints
the full penalty breakdown, including a nonzero `Register Differences: 44`
and `Reorderings: 5`).

**Search: `-j 6 --stop-on-zero --best-only --stack-diffs`, bounded to 900s.
`rc` was NOT cleanly captured** — the search was launched via a detached
`nohup ... &` rather than a foreground command whose `$?` could be read on
the very next line, which is exactly the mistake CLAUDE.md warns against
("capture rc on the very next command"). Inferred from the process ending
naturally at the 900s wall-clock boundary (`ps` showed it exit right around
the `timeout` deadline, no earlier) that this is `rc=124` (bound fired), not
an external kill, but this is inference, not a captured exit status — flag
for the next runner: launch as
`nohup bash -c '...; echo "rc=$?" >> log' &` so the log itself carries the
status regardless of which shell later reads it.

**Result: 74597 iterations, ZERO hits at `score = 0`.** Best score reached
was **795** (`permuter-work/SePitchBend/output-795-1`), down from the
2020 baseline, found within the first ~4 minutes; no further improvement in
the remaining ~11 minutes of search.

**The 795 candidate is a NEGATIVE result once translated, not a partial
win — this is the permuter-vs-oracle trap CLAUDE.md names explicitly.** Its
only two changes from the base were (1) an empty `do {} while (0);`
statement at the top of the `else` arm and (2) wrapping the trailing
`*(u16*)(D_8008D7F4+...) = v0; v1 = (u8)s0;` pair in a `do { ... } while
(0);` block. Spliced directly into the real unit (not the isolated
scaffold) and rebuilt: **still 113/112 (0x1c4), no length change**, and
`asm-differ`'s realigned diff shows the residue **relocated, not reduced**
— the `D_8008EA26` store (`andi`/`sh`) that previously appeared once, in
the position matching retail, now appears at a NEW, earlier position with
a duplicated `andi`/`sh` pair, and raw word-match dropped from 4/112 to
3/112. The permuter's own scorer improved (2020 -> 795) on the ISOLATED
scaffold while the same source change made the REAL function's residue
worse against the actual project oracle. Reverted; `git diff --stat` clean
against the pre-session baseline.

**Disposition: unchanged at 113/112 (1 word long).** Not closed in 74597
iterations under shared-machine load (five runners running concurrent
permuter searches across worktrees this round, though each running its own
single search per CLAUDE.md's "one search at a time" rule). The only
zeros this residue class produces are UB/`do-while(0)`-block forms that do
not reproduce off the isolated scaffold — consistent with round 33/35's
finding that this is genuine HARD RULE 6 register-identity territory, now
also confirmed immune to the permuter. Not promoting to
"permuter-exhausted" (a single 15-minute bounded run under contention is
not grounds for a permanent verdict per this round's own instructions) —
record as "not closed in ~74.6k iterations under load; the do-while(0)
grouping idiom is a validated-negative lever for THIS function specifically
(untested elsewhere in the unit)."

### Proposed learning (round 37)

**A `do { ... } while (0);` block wrapping a statement group is a
genuinely different C-level construct from a bare `__asm__("")` barrier —
worth testing generally as a potential fence against GCC 2.6.3's
cross-branch/value-hoisting immunity to `asm("")` (the class `SeAutoVol`
and `SpuVmAlloc` document as resistant to plain barriers) — but on this
function specifically it relocated the residue rather than removing it, so
record the outcome as function-specific, not yet a validated project-wide
idiom.** Also worth a general caution for future permuter rounds on this
unit: the permuter's isolated single-function scaffold does not carry the
same surrounding register-pressure context as the real unit file, so an
improvement in the permuter's own score is not self-evidently an
improvement in the real build — CLAUDE.md already says this, and this
round is a second, independent confirmation of it (after round 18's three
0-for-3 instances) on a *different* residue class (value-hoist/register-
scramble rather than a simple candidate rewrite).

## Round 44 update (runner delta): inherited body re-verified real, not re-attempted further

Re-verified the inherited 113/112 body: `objdump -t` on
`build/src/code_179d8_l.c.o` confirms `SePitchBend` compiles to `0x1c4`
bytes = 113 words, matching rounds 33/35/37's figure exactly -- this is a
genuine, reproducible near-miss, not a stale claim. Given three prior
rounds' worth of levers already tried and confirmed negative (declaration
order, algebraic rewrite, `volatile`, scheduling barriers at two different
sites, and a round-37 permuter search at 74597 iterations), and this
round's budget shared across seven other queued functions in this unit, no
new lever was attempted here. Restored to `INCLUDE_ASM` unchanged.

## Naming (round 75, runner alpha, FINISHING-PLAN track 3)

Already carries its real name: identified round 74 (track 2, runner bravo)
as `libsnd/vmanager SePitchBend` (shape 0.99 vs the disc-3.3 reference,
110w reference vs our 112w, position within the libsnd neighborhood). Sony
symbol; this pass does not rename it further. Matched round 73, 112/112.

## Track 2 (round 86, 2026-09-26, alpha)

The per-field symbols this report names (`D_8008D988`..`D_8008D9BA` at a 0x34 stride, and the twelve game names over +0x1C..+0x33 that earlier rounds gave `D_8008D9A4`..`D_8008D9BA`) are ONE Sony table: libsnd/vmanager.o (disc 3.5) bss puts `_svm_voice` at +0x198 of the block anchored at 0x8008D7F0, so `_svm_voice` = 0x8008D988, 24 voices x 0x34 = 0x4E0 bytes, ending exactly at `_svm_envx_ptr`. The symbols file now carries `_svm_voice` (size:0x4E0); the record type is `include/SvmData.h` (fields by offset only, Sony's rule). Field map: +0x00 `unk00` (was `D_8008D988`), +0x02 `unk02` (`D_8008D98A`), +0x04 `unk04` (`D_8008D98C`), +0x06 `unk06` (`D_8008D98E`), +0x08 `unk08` (`D_8008D990`), +0x0A `unk0A` (`D_8008D992`), +0x0C `unk0C` (`D_8008D994`), +0x0E `unk0E` (`D_8008D996`), +0x10 `unk10` (`D_8008D998`), +0x12 `unk12` (`D_8008D99A`), +0x14 `unk14` (`D_8008D99C`), +0x16 `unk16` (`D_8008D99E`), +0x18 `unk18` (`D_8008D9A0`), +0x1B `unk1B` (`D_8008D9A3`), +0x1C..+0x26 `unk1C`..`unk26` (the SeAutoVol/SetAutoVol ramp: active, step, interval, countdown, accum, limit; `D_8008D9A4`..`D_8008D9AE`), +0x28..+0x32 `unk28`..`unk32` (the SeAutoPan/SetAutoPan ramp, same order; `D_8008D9B0`..`D_8008D9BA`). Preserved bodies in this report keep the per-address `D_` spellings, which still link (except `D_8008D988`, which is now `_svm_voice` itself) (splat keeps them as auto-symbols, since the table lies past the global segment's vram range and splat does not fold them into `_svm_voice`).

`src/` now reads `(u8)_svm_voice[c].unk10`, `(u8)_svm_voice[c].unk14` and `(u16)_svm_voice[c].unk0C` (value casts at the site; the header keeps +0x0C/+0x10/+0x14 `s16`, what most accessors read). Byte-exact. Measured: the address-cast spelling `*(u8 *)&_svm_voice[c].unk10` is NOT equivalent here -- it grows the frame 0x20 -> 0x28 with every other word equal (104/112); spelling +0x0C as `*(u16 *)&...unk0C` on top of that drops to 29/110.

**_svm_sreg_buf / _svm_sreg_dirty (same round).** `D_8008D7F0` (0x180 bytes, 24 voices x 0x10, halfwords at +0x0..+0xA spelled `D_8008D7F0`..`D_8008D7FA` by splat) is Sony's `_svm_sreg_buf` and `D_8008D970` (24 bytes) is `_svm_sreg_dirty`: libsnd/vmanager.o bss +0x000 and +0x180, anchored at 0x8008D7F0. Both are in the symbols file; the record type is `SvmSreg` in `include/SvmData.h` (fields by offset). `src/` keeps the halfword-array cast `((u16 *)_svm_sreg_buf)[off + 2]` (the idx*8 split-scaled-index idiom) and `_svm_sreg_dirty`. Byte-exact.

## Types (round 98, alpha)

This unit's `D8008E978Entry` is now `<libsnd.h>`'s `VagAtr` and `ObjE970` is `VabHdr` (unk4 -> `center`, unk5 -> `shift`, unk12 -> `pbmin`, unk13 -> `pbmax`, ObjE970.unk18 -> `mvol`); preserved bodies above keep the old spellings. See SpuVmAlloc.md, "Unit banner history".
