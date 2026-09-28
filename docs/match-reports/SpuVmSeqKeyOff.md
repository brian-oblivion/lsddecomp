# SpuVmSeqKeyOff -- STALL (length EXACT at 85/85 words; 73/85 raw word-match; first real diff at word 4 / retail 0x800306AC -- a one-instruction ROTATION of the branch-delay fill, not a shift)

> Renamed from `func_8003069C` on 2026-09-23 (tools/rename.py). Address 0x8003069c.

**Measurement note (both numbers, not conflated):** compiled LENGTH is
exactly retail's 85 words (funcdiff reports no out-of-range drift). The
RAW hex word-match count is 45/85. `tools/asm-differ/diff.py` realigns the
two streams and confirms the first REAL divergence is at word 4 (file
offset 0x20EAC, retail vram 0x800306AC) -- i.e. this is genuine early
register/scheduling residue in the loop's setup, not a shift caused by a
missing/extra instruction. (An earlier draft of this report, before the
`key`-hoisting fix below, scored 41/85 with its first real diff at word 0 --
the improvement moved the residue's START, it did not just improve the
raw count.)

Unit `libsnd_vmanager`, round 23 (2026-09-07). Not a class method. Scans
`D_8008D996[0..spuVmMaxVoice)` for an entry equal to `(s16) p0`; on a match,
runs this unit's "clear channel bits" tail (the same block as
`SsUtKeyOffV`, see that function's report) keyed by the loop index. Called
from `libsnd_decre.c`'s `func_800339AC` as
`SpuVmSeqKeyOff((sa1 << 8) | sa0)`.

## What it is (best-reached body, 45/85 words, correct length, no drift)

```c
#if 0
void SpuVmSeqKeyOff(s32 p0)
{
    s32 key;
    u32 i;

    if (spuVmMaxVoice == 0)
        return;
    key = (s16) p0;
    i = 0;
    do {
        if (D_8008D996[(u8) i].unk0 == key) {
            u16 bankIdx;
            u16 loBit, hiBit;
            u16 old60, old64;

            D_8008EA26 = (u8) i;
            bankIdx = D_8008EA26;
            if (bankIdx < 0x10) {
                loBit = 1 << bankIdx;
                hiBit = 0;
            } else {
                loBit = 0;
                hiBit = 1 << (bankIdx - 0x10);
            }
            D_8008D9A3[bankIdx].unk0 = 0;
            D_8008D98C[bankIdx].unk0 = 0;
            _svm_voice[bankIdx].unk0 = 0;
            old60 = _svm_okof1;
            old64 = _svm_okof2;
            old60 = loBit | old60;
            _svm_okof1 = old60;
            _svm_okon1 = _svm_okon1 & ~old60;
            old64 = hiBit | old64;
            _svm_okof2 = old64;
            _svm_okon2 = _svm_okon2 & ~old64;
        }
        i++;
    } while ((u8) i < spuVmMaxVoice);
}
#endif
```

Needs this unit's shared `_snd_ev_flag`-adjacent globals declared near the
top of `libsnd_vmanager.c` (`D_8008EA26`, `spuVmMaxVoice`, `D_8008D996`/`D_8008D9A3`/
`_svm_voice`/`D_8008D98C`, `_svm_okof1`/`_svm_okof2`, `_svm_okon1`/`_svm_okon2`).

## One CLOSED finding: masking the induction variable is what enables strength reduction to match

The single biggest lever on this function, closing a 4-word/232988-byte
address-drift gap outright (0/85-with-drift -> matches-length-exactly):
retail's loop body **recomputes `i * 0x34` from scratch every iteration**
(`sll`/`addu`/`sll`/`addu`/`sll`, 5 instructions), never strength-reducing it
into a running `+0x34` pointer -- unusual for a -O2 GCC loop with a single
linear array access on the induction variable, which is exactly the shape
GCC's classic loop optimizer normally reduces. Indexing with the bare loop
variable (`D_8008D996[i].unk0`) DOES get strength-reduced by this pinned
toolchain (a running `t0 += 0x34` pointer, computed startup `move t0,zero`,
one `addiu t0,t0,0x34` per iteration) -- 4 words shorter than retail and a
completely different instruction shape. **Indexing with an explicit `(u8)`
mask on the induction variable (`D_8008D996[(u8) i].unk0`) blocks the
reduction and reproduces retail's fresh-multiply-every-iteration shape
exactly**, because the byte truncation breaks GCC 2.6.3's biv/giv linearity
proof (the compiler can no longer show `addr = base + i*52` is a pure
linear function of the iteration count once the index passes through a
narrowing cast). This is a second confirmed instance of "the loop's exit
test masks the counter to a byte" (`(u8) i < spuVmMaxVoice`, already known from
this unit's other loop-bearing functions) but the NEW finding is that the
*array-indexing expression itself* needs the same mask, independently of
the exit test's mask, to reproduce retail's actual codegen -- the two masks
are not redundant from the compiler's point of view even though they are
numerically redundant (i never exceeds 0xFF while the loop runs).

## A second CLOSED finding: compute the key AFTER the zero-count guard, not before

Retail's very first substantive instruction (after the `spuVmMaxVoice`
load/test) is the `beqz` guard itself; the `sll`/`sra` pair that computes
`key = (s16) p0` is scheduled AFTER that guard, only on the path where the
loop will actually run. Declaring `key` and initializing it in the SAME
statement (`s32 key = (s16) p0;`) at the top of the function, before the
guard, computes it UNCONDITIONALLY -- unlike the guard-gated version, GCC
hoists that computation to the very first instruction of the function,
one full word before retail's own `lbu`/`beqz` sequence even starts, and
this measured as a genuine word-0 divergence (41/85, first real diff at
word 0). Splitting the declaration from the initialization and moving the
assignment to AFTER `if (spuVmMaxVoice == 0) return;` reproduces retail's
placement exactly and moved the residue's start from word 0 to word 4 (and
the raw count from 41/85 to 45/85). This is the same-shaped lesson as
`SpuVmSetSeqVol`'s report (a value that COULD be computed early, matching
where it's textually declared, should instead be computed lazily, at the
point retail's control flow first needs it) confirmed a second time in
this same unit.

## The unresolved residue

- **`D_8008EA26`'s address is not hoisted out of the loop.** Retail computes
  `t2 = &D_8008EA26` (a `lui`/`addiu` pair) ONCE, before the loop even
  starts, and every in-loop `sh ..., 0(t2)` reuses it. This build recomputes
  `lui`/`addiu` fresh inside the `if` block on every iteration that takes it
  (the same "materialize %hi/%lo every access" shape `SsUtKeyOffV`'s
  report already covers for a straight-line function -- here it additionally
  fails to hoist out of a LOOP). Tried: caching the address explicitly in a
  `volatile u16 *chanPtr = &D_8008EA26;` local declared before the loop --
  this reintroduced an 8-byte dead stack frame (regressing to 12/85 with
  285KB+ of drift), the same unexplained-frame artifact documented in
  `SsUtSetDetVVol`'s report, apparently triggered here by the combination of
  a loop-carried pointer local plus the `volatile` qualifier. Reverted.
- **`loBit`/`hiBit` land in the wrong scratch registers**, same class,
  same unresolved status, as `SsUtKeyOffV`'s report already documents
  in detail (same source shape, same residue). Not re-investigated
  separately this round since that function's axes (declaration order,
  statement order within each arm) were already shown not to move it.
- A handful of interior words (address 0x20F60, 0x20F88-0x20FC0) are the
  same "which channel-mask half gets which scratch register" residue as
  `SsUtKeyOffV`'s second cluster.

## Axes tried (for the next attempt)

- Loop shape: `for` loop vs. explicit guard + `do`/`while` matching retail's
  own compiled topology -- **no effect** on the residue (both produced
  the wrong, strength-reduced address form until the index mask was added).
- Induction variable declared type: `u32` vs `s32` for the loop counter --
  **no effect**.
- Array-index expression: bare `i` (strength-reduced, wrong) vs. `(u8) i`
  (matches retail's fresh-multiply shape) -- **closed**, see above.
- `D_8008EA26` access: plain global (current, 45/85) vs. an explicit
  `volatile u16 *` cached before the loop (regressed to 12/85 via a
  spurious dead 8-byte frame) -- reverted.
- `key`'s computation site: combined declaration+init before the guard
  (41/85, first real diff at word 0) vs. split declaration/assignment
  with the assignment moved after the `spuVmMaxVoice == 0` guard (45/85,
  first real diff at word 4) -- **closed**, see above.

**Untested axis:** whether the `D_8008EA26` address-hoisting failure and
the `loBit`/`hiBit` register swap are actually the SAME underlying
allocator decision (both are about which values the allocator judges worth
keeping in a persistent register across the loop/branch) rather than two
independent residues -- not distinguished this round for lack of remaining
budget.

### Proposed learning

**A narrowing cast on a loop induction variable is a genuine, source-level
lever against GCC 2.6.3's strength reduction, not just against its
sign-extension choices.** This project already knows `(u8)`/`(u16)`
masking a loop counter changes which COMPARE instruction (`slt` vs `sltu`)
retail used; this round found the same mask, applied to the SAME
counter's use as an ARRAY INDEX (not just the loop-exit test), independently
controls whether the compiler recomputes the element address by
multiplication every iteration or strength-reduces it into a running
pointer -- two different optimizer decisions, both gated by the identical
one-character source difference. Worth checking on any stalled loop in this
unit (or others) whose residue is "extra/missing multiply-by-stride
instructions inside the loop body" before concluding it is a scheduling or
register-identity stall.

## ROUND 31 (runner delta): rebuild confirms both title figures exactly

Rebuilt the preserved body verbatim through the current pinned pipeline.
**Length is exact (85/85 words, no drift) and the raw word-match is
45/85**, matching this report's title precisely, with `funcdiff.py`'s own
diff output reproducing the same divergence pattern (first mismatch at
file offset 0x020EAC, retail vram 0x800306AC = word 4). Did not re-run
the untested axis (whether the `D_8008EA26` hoisting failure and the
`loBit`/`hiBit` swap are one allocator decision or two) this round for
lack of remaining time budget; no new reshape attempted. Restored to
`INCLUDE_ASM`; still a STALL.

## ROUND 32 (runner alpha): rebuild confirms both figures exactly; address-hoisting axis tried and rejected

Rebuilt the preserved body verbatim through the current pinned pipeline.
**Length is exact (85/85 words, no drift) and the raw word-match is
45/85**, matching this report's title precisely.

Tried the untested axis this report's round-23 section flagged (caching
the `D_8008EA26` address so it hoists out of the loop), in a form NOT
already tried and rejected: rather than declaring `volatile u16 *chanPtr =
&D_8008EA26;` BEFORE the loop (which round 23 showed reintroduces an
8-byte dead frame and regresses to 12/85), declared it INSIDE the `if`
block, scoped to the branch that actually uses it, on the theory that a
loop-carried local declared before the loop -- not the pointer idiom
itself -- was what perturbed the frame. **Result: 45/85, byte-identical to
the baseline (same diff offsets, no drift)** -- the address still is not
hoisted out of the loop (still recomputed fresh on every taken iteration),
and this scoping did NOT reintroduce the dead-frame regression either. So
the frame regression specifically needed the pointer to be loop-invariant
(declared outside/before the loop); declaring it inside the branch makes
it neutral -- GCC just re-derives it each time, same as the un-cached form.
Net effect: a safer (non-regressing) rewrite of the same idiom, but no
improvement, confirming the loop-invariant hoisting genuinely requires a
lever not yet found, not just a differently-scoped pointer.

Restored to `INCLUDE_ASM`; `git status --porcelain` empty; whole-image
build re-verified green. Still a STALL, unchanged classification.

### Proposed learning

**Round 23's "reintroduces a dead frame" finding for the `D_8008EA26`
caching idiom was scope-dependent, not idiom-dependent.** Declaring the
same `volatile u16 *` local INSIDE the loop body (scoped to the branch that
uses it) reproduces the un-cached function's exact output (45/85, no
regression) instead of round 23's 12/85-with-drift regression, which only
appeared when the pointer was declared BEFORE the loop (making it loop-
carried). Neither placement achieves the intended hoisting, but the
inside-the-branch placement is strictly safer to experiment with -- worth
using as the starting point for whichever future attempt tries a genuinely
new lever on this residue, rather than the loop-invariant declaration that
round 23 already showed backfires.

## Round 47 (2026-09-16), runner delta -- rebuilt in-tree (third confirmation), then permuter DECLINED on check (b)

**Rebuild-before-trusting-the-score, third time.** Spliced the preserved
body into `src/libsnd_vmanager.c` (with this unit's own local reduced-view
declarations for `spuVmMaxVoice`, `D_8008D996`/`D_8008D9A3`/`_svm_voice`/
`D_8008D98C`, `D_8008EA26`, `_svm_okof1`/`_svm_okof2`, `_svm_okon1`/
`_svm_okon2`, copied from `libsnd_vm_vol_ut_key_ut_keyv.c`'s equivalents per this
project's per-unit reduced-local-view convention) and ran the real oracle:
`build exit=2`, no compile-error grep hits, `funcdiff.py` shows **45/85
raw word-match with NO out-of-range drift warning** -- the compiled length
is exactly retail's 85 words, confirmed independently of the title's own
claim. `tools/asm-differ/diff.py` confirms the first real divergence is
still at file offset `0x20EAC` (word 4, retail `move a3,zero` vs. this
build's `sll v0,a0,0x10` one instruction earlier in program order) --
identical to rounds 31/32's re-verifications and this report's own title.
Restored to `INCLUDE_ASM` immediately after (diffed the restored file
against the pre-splice copy: byte-identical); `./build-and-verify.sh`
confirms `OK: build matches retail SLPS_015.56`.

**Permuter pre-checks -- (a) and (b) run, (c) satisfied by the rebuild
above:**

- **(a) scaffold compiles and scores:** yes.
- **(b) insertion/deletion penalties, `--debug --stack-diffs`:** **NOT**
  near 0/0. Measured: `Insertions: 5 (100)`, `Deletions: 5 (100)`,
  `Reorderings: 1 (60)`, `Register Differences: 30 (5)`, `Stack
  Differences: 0 (1)`, **base score = 1210**. Zero stack differences (the
  frame layout itself is right, consistent with round 23/32's finding that
  the un-cached form doesn't perturb the frame) but very large
  register-difference and insertion/deletion counts -- consistent with
  this report's own two open findings (the `D_8008EA26` address not
  hoisted out of the loop; the `loBit`/`hiBit` register swap) being
  allocator/scheduling decisions across the WHOLE loop body, not a
  localized expression to rewrite.
- **(c) scaffold-vs-real-build agreement:** the in-tree rebuild above
  (45/85, no drift, first diff at word 4) matches this report's own title
  figures exactly and the permuter's debug diff is reading the SAME
  scaffold body, so there is no scaffold/real-build disagreement to flag.

**Verdict: search DECLINED.** With insertions/deletions at 5/5 and 30
register differences on an 85-word function, a source-mutation search
would need to restructure roughly a third of the function's instructions
simultaneously to reach retail -- the same class of gap (WHICH persistent
register holds a hoisted address, whether an address computation is
hoisted out of a loop at all) that `OpenCdFile`/`ReadCdFile` in
`code_179d8_s` also failed check (b) on this same round. Recorded as NOT
SEARCHED (declined on evidence), not as a spent, failed search. The two
already-identified axes (`D_8008EA26` hoisting; `loBit`/`hiBit` register
roles) remain the right ones to attack by hand, per this report's existing
"untested axis" note about whether they are one allocator decision or two.


## Round 56 (2026-09-19), runner bravo -- TWO NEW LEVERS, 45/85 -> 55/85, length still exact

**Rebuild-before-trusting-the-score, fourth time.** Spliced the round-23
body in verbatim: `build exit=2`, no compile-error grep hits, `funcdiff.py`
reports **45/85 with no out-of-range drift warning**, `asm-differ` puts the
first real divergence at 0x20EAC (word 4). Both title figures reproduced
exactly before anything was changed.

Two levers then took it to **55/85, length still exactly 85 words, zero
drift** (`build/lsdde.map` still puts the next symbol, Sony's
`SpuVmSetProgVol`, at its retail 0x800307F0). Ten words, and both are
CLOSED findings -- each reproduces a specific retail instruction sequence
byte-for-byte, not merely a better score.

### CLOSED lever 1 -- a PLAIN pointer local hoists the `lui`/`addiu` address pair out of the loop

This report's oldest open residue was "`D_8008EA26`'s address is not hoisted
out of the loop": retail computes `t2 = &D_8008EA26` once in the preheader
(`lui t2,%hi` / `addiu t2,%lo`) and stores with `sh v0,0(t2)`, while every
build so far emitted a fresh `lui at` / `sh v0,%lo(sym)(at)` pair inside the
loop. Rounds 23 and 32 both tried caching the address in a local and both
failed -- round 23 with a dead 8-byte frame (12/85), round 32 with no effect
at all.

**Both attempts spelled the local `volatile u16 *chanPtr`. The VOLATILE
QUALIFIER ON THE POINTER is what broke the idiom, not the idiom.** Declared
plain and cast through:

```c
u16 *chanPtr;
...
chanPtr = (u16 *) &D_8008EA26;   /* before the loop */
...
*chanPtr = idx;                  /* inside the loop */
```

GCC 2.6.3's loop-invariant motion then hoists the `(set (reg) (symbol_ref))`
into the preheader and the build emits retail's `lui t2,%hi(D_8008EA26)` /
`addiu t2,t2,%lo(D_8008EA26)` / `sh ...,0(t2)` **byte-for-byte**. The global
is still declared `extern volatile u16 D_8008EA26;` (as its matched sibling
`SsUtKeyOff` in `libsnd_vm_vol_ut_key_ut_keyv.c` declares it), so the read-back
`bankIdx = D_8008EA26;` still compiles to a fresh `lui`/`lhu %lo` -- which is
exactly retail's own asymmetry between the store and the reload, and the
thing that made the residue look unexplainable.

On its own this lever left the function ONE WORD LONG (a surviving
`move a0,a3`, below), so it is only half the fix; it is recorded as closed
because the three instructions it targets are now identical to retail.

### CLOSED lever 2 -- an explicit `idx` local kills the redundant `move a0,a3`

With `(u8) i` written inline at both of its uses (the array index and the
`D_8008EA26` store), GCC keeps a SECOND pseudo for the loop-top value of
`i`, initialised `move a0,a3` in the preheader and re-copied in the backedge
delay slot, and every `andi` then reads that copy. Retail has no copy: its
two `andi v0,a3,0xff` read the induction variable directly. Hoisting the
mask into a named local:

```c
u32 idx;
...
do {
    idx = (u8) i;
    if (D_8008D996[idx].unk0 == key) {
        *chanPtr = idx;
        ...
```

removes the copy outright, restores the exact length, and reproduces
retail's loop topology instruction-for-instruction: `andi ...,a3,0xff` in
the preheader, loop body entered one instruction later at 0x800306C8, and
the same `andi` duplicated into the backedge delay slot by the delay-slot
filler's fill-from-target retarget.

**This is the exact opposite of what this unit's other reports recommend.**
`SpuVmSeqKeyOff`'s own round-23 section closed "index with `(u8) i`, not bare
`i`, to block strength reduction" -- that finding still stands, the mask is
still required. What round 56 adds is that the mask must be applied ONCE,
through a named local, not written inline at each use: inline it costs a
loop-carried copy of the induction variable.

### The best body (55/85, length exact 85/85, zero drift)

```c
#if 0
/* file-local reduced view, all already present in libsnd_vm_vol_ut_key_ut_keyv.c under
 * the same names/types -- keep them local to the unit, not in a header */
extern u8 spuVmMaxVoice;
extern volatile u16 D_8008EA26;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 _svm_okon1;
extern u16 _svm_okon2;

typedef struct {
    s16 unk0;
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D996[];
extern Rec34Half _svm_voice[];
extern Rec34Half D_8008D98C[];

typedef struct {
    u8 unk0;
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

void SpuVmSeqKeyOff(s32 p0)
{
    s32 key;
    u32 i;
    u16 *chanPtr;
    u32 idx;

    if (spuVmMaxVoice == 0)
        return;
    i = 0;
    key = (s16) p0;
    chanPtr = (u16 *) &D_8008EA26;
    do {
        idx = (u8) i;
        if (D_8008D996[idx].unk0 == key) {
            u16 bankIdx;
            u16 loBit, hiBit;
            u16 old60, old64;

            *chanPtr = idx;
            bankIdx = D_8008EA26;
            if (bankIdx < 0x10) {
                loBit = 1 << bankIdx;
                hiBit = 0;
            } else {
                loBit = 0;
                hiBit = 1 << (bankIdx - 0x10);
            }
            D_8008D9A3[bankIdx].unk0 = 0;
            D_8008D98C[bankIdx].unk0 = 0;
            _svm_voice[bankIdx].unk0 = 0;
            old60 = _svm_okof1;
            old64 = _svm_okof2;
            old60 = loBit | old60;
            _svm_okof1 = old60;
            _svm_okon1 = _svm_okon1 & ~old60;
            old64 = hiBit | old64;
            _svm_okof2 = old64;
            _svm_okon2 = _svm_okon2 & ~old64;
        }
        i++;
    } while ((u8) i < spuVmMaxVoice);
}
#endif
```

### The residue that remains (30 words), named by class

1. **word 4, the first real diff -- a branch-delay ROTATION, not register
   identity and not instruction selection.** Retail's `beqz v0,end` delay
   slot holds `move a3,zero` (`i = 0`) and the `sll v0,a0,0x10` that starts
   `key = (s16) p0` follows it; the build fills the slot with the `sll` and
   puts `move a3,zero` after it. Both instructions are present, in both
   builds, one word apart -- the scheduler orders the preheader block
   differently, and the delay-slot filler then takes whichever instruction
   ended up first. **Costs 2 words.**
2. **The `idx` register's live range (v0 vs v1) -- one allocator decision
   worth 8 words.** Retail puts `(u8) i` in `v0`, lets `lh v0,0(at)` kill it,
   and REMATERIALISES `andi v0,a3,0xff` in the `bne` delay slot for the
   `sh`; the build keeps `idx` alive in `v1` across the `lh` and leaves the
   `bne` delay slot as a `nop`. That single choice accounts for the whole
   `v0`/`v1` swap through the 0x34-multiply (6 words), the `nop` plus the
   `sh`'s source register (2 words), and the backedge delay slot's register
   (already counted).
3. **`loBit`/`hiBit` land in `a1`/`a2` swapped (4 words) plus the `or`'s
   operand order (1 word)** -- unchanged from this report's earlier rounds.
4. **Tail load/store scheduling (about 9 words).** Retail hoists the
   `_svm_okof2` load into the early slot at 0x80030760 and loads
   `_svm_okon1` lazily at 0x80030788, then finishes the C60/E228 pair before
   starting the C64/E22C pair. The build swaps the two hoisted loads and
   sinks the `_svm_okon1` store past the whole C64/E22C chain.

### Axes tried this round and REJECTED (all measured, all with the lever-1+2 body as the base)

- **Tail statement order, five spellings, ALL BYTE-IDENTICAL (855 permuter
  units, 55/85):** fully sequential C60->E228 then C64->E22C; both `or`s
  computed before either store; `or` operands written the other way round
  (`old60 | loBit` vs `loBit | old60`); `u32` masks. GCC's scheduler picks
  the same order regardless. **The tail residue is not expression-order
  steerable.**
- **`SsUtKeyOff`'s matched tail idiom does NOT transfer here.** Writing
  the tail exactly as that already-matched sibling does
  (`_svm_okof1 = loBit | _svm_okof1; _svm_okon1 &= ~_svm_okof1; ...`, with
  its documented `u32`/`u16` mask type asymmetry) REGRESSES to 47/85. The
  explicit `old60`/`old64` locals are load-bearing here. This is a fourth
  instance of this unit's standing caution that a sibling's idiom needs
  re-verification per function rather than adoption by analogy.
- **All 24 orderings of the four local declarations: byte-identical.**
  Declaration order does not steer register allocation in this function.
- **All 6 orderings of the three preheader statements** (`i = 0`,
  `key = ...`, `chanPtr = ...`): `i`, `key`, `chanPtr` is the unique best;
  every other ordering loses 1-3 words. In particular putting `chanPtr`
  first costs 3.
- **`for (i = 0; (u8) i < spuVmMaxVoice; i++)` in place of the explicit guard
  plus `do`/`while`: sharply worse** -- it reintroduces an 8-byte stack
  frame (`addiu sp,sp,-8`) and drops to 1945 permuter units. Round 23
  recorded this axis as "no effect"; with the round-56 body it is a clear
  regression, so the axis is now CLOSED in favour of the explicit guard.
- **Mixing `idx` and inline `(u8) i`** (either way round) is CSE'd back
  into the single-pseudo form and reproduces the pre-lever-2 output exactly.
  So residue 2 above is NOT reachable by spelling the two occurrences
  differently: GCC 2.6.3 unifies every spelling of `(u8) i`.
- **`u8 idx`** instead of `u32 idx`: regresses (back to the `move a0,a3`
  form).
- **Inverting the `bankIdx < 0x10` arms** (52/85) and **swapping the two
  assignments within each arm** (35/85): both worse. The `loBit`/`hiBit`
  register roles are not arm-order steerable.
- **Three bare `__asm__("")` scheduling-barrier placements** in the tail
  (before the pre-loads, between the E228 and C64 statements, after the
  E228 store): 45/85, 36/85, 36/85 -- all worse. The barrier idiom that is
  load-bearing for this unit's matched `SpuVmGetSeqLVol`/`SpuVmGetSeqRVol` does
  not transfer into this loop body, consistent with `SpuVmSetSeqVol`'s
  round-32 finding about donor register pressure.

### Proposed learning

**1. A `volatile`-qualified POINTER is not the same lever as a `volatile`
GLOBAL, and conflating them cost this function two rounds.** The idiom that
makes GCC 2.6.3 hoist a global's `lui`/`addiu` address pair out of a loop is
a PLAIN pointer local initialised with `&global` before the loop and stored
through inside it. Qualifying that pointer `volatile` defeats the hoist and
can force a spurious stack frame. The global itself stays `volatile` (that is
what forces retail's store-then-reload); the pointer must not be. Worth
trying on any stall whose residue is "retail computes a symbol's address once
before a loop and the build re-materialises `lui`/`%lo` inside it".

**2. Where a narrowing cast on an induction variable is WRITTEN matters as
much as whether it is written.** This report already established that
`(u8) i` as an array index (not just in the loop-exit test) is what blocks
GCC 2.6.3's strength reduction. Round 56 adds the complement: write that same
mask inline at more than one use and GCC materialises a loop-carried COPY of
the induction variable (`move a0,a3` in the preheader and again in the
backedge delay slot) for the body to read, one word longer than retail and
with every `andi` pointed at the copy. Hoisting the mask into a single named
local removes the copy and restores the exact length. Mask once, into a
local; do not repeat the cast.

**3. "First real diff at word N" does not by itself distinguish a register
residue from a two-instruction rotation, and the difference decides whether
the function is worth attacking.** This report has carried "first real diff
at word 4 -- genuine early register/scheduling residue" since round 23, and
three subsequent rounds re-verified the NUMBER without re-reading the
INSTRUCTIONS. Both instructions at word 4 are present in both streams, one
word apart: it is a delay-slot fill order, which is what made the
surrounding preheader worth reshaping at all. Read `asm-differ`'s two
columns, not just the offset it reports.

### Permuter, round 56 -- Gate 3's three checks run, then the search

- **(a) scaffold compiles and scores:** yes.
- **(b) `--debug --stack-diffs`:** `Insertions: 3 (100)`, `Deletions: 3
  (100)`, `Reorderings: 1 (60)`, `Register Differences: 42 (5)`, `Stack
  Differences: 0 (1)`, `Branch Differences: 0 (1)`, **base score = 870**.
  Round 47 declined the search on this check at 5 insertions / 5 deletions /
  30 register differences / base 1210. The round-56 body halves the
  insertion+deletion sum from 10 to 6; the register-difference count rises
  from 30 to 42 precisely BECAUSE more of the function now aligns, so
  differences that were being charged as insertions are charged as register
  differences instead. Zero stack and zero branch differences.
- **(c) scaffold-vs-real-build agreement: AGREE.** The scaffold's debug diff
  shows the same four residue classes listed above, in the same places, as
  the in-tree `asm-differ` run: same `move a3,zero` rotation at word 4, same
  `v0`/`v1` swap through the multiply, same missing rematerialised `andi`
  before the `sh`, same `a1`/`a2` mask swap, same `_svm_okof2`/`_svm_okon1`
  load swap. No scaffold artifact.

**Verdict: SEARCH -- run, and it PAID. See the outcome section below.**


## Round 56 permuter SEARCH OUTCOME -- 55/85 -> 60/85, one candidate translated and verified in-tree

**Search: 491,559 iterations, `-j 6 --stop-on-zero --best-only`, base score
870, wall clock capped with `timeout 2700`; ended on that cap, `rc=124`** (so
it stopped on the clock, not on a zero -- `--stop-on-zero` never fired). Five
candidates below base were kept: 710, 525, 525, 405 and **270**, and the 270
was already standing by iteration ~70,000 -- the remaining ~420,000
iterations improved on it not at all, which is the useful shape of the
negative: the search's whole yield was one mutation, found early.
No orphan workers left behind (swept after the exit).

**The 270 candidate is a real lever and it reproduces in the real tree.** Its
only substantive mutation is to route the value stored into `D_8008EA26`
through a `u16` local instead of through `idx`:

```c
            old60 = i;                 /* u16 local, reused */
            *chanPtr = (u8) old60;     /* instead of: *chanPtr = idx; */
```

That is exactly the residue this report's round-56 section had just recorded
as UNREACHABLE by source shaping -- "GCC 2.6.3 unifies every spelling of
`(u8) i`, so `idx` and an inline `(u8) i` are always the same pseudo". The
permuter found the one spelling that is NOT unified: **going through a
`u16`-typed intermediate first.** `(u8) i` is `(and i 0xff)`; `(u8)(u16) i`
is `(and (and i 0xffff) 0xff)`, and GCC 2.6.3 does not fold the nested mask,
so the two occurrences stay separate pseudos and the `sh`'s operand is
re-materialised after the `lh` exactly as retail does.

**Translated to idiomatic C and measured on the real oracle**, with a named
local instead of reusing `old60`:

```c
            u16 chanVal;
            ...
            chanVal = i;
            *chanPtr = (u8) chanVal;
```

**60/85, length still exactly 85 words, zero drift.** Five spellings of the
same idea were measured in-tree and four give the identical 60/85 output
(fresh `u16` local; fresh `u32` local with an explicit `(u16)` first;
reusing `bankIdx`; reusing `loBit`); only reusing `old64` differs (57/85).
Writing it as a single inline `(u8) (u16) i` does NOT work (back to the
pre-lever form) -- the value has to pass through a VARIABLE.

**A genuine measurement conflict, recorded rather than resolved.** The
permuter's own 270-scoring form (reusing `old60`) scores **57/85** raw --
THREE WORDS WORSE than the `chanVal` form's 60/85 -- while being
structurally closer: it also fixes the tail's `_svm_okon1` store deferral,
leaving the whole remaining residue as a single consistent register
PERMUTATION across the function (`i` in `a2` vs retail's `a3`;
`loBit`/`hiBit` in `a1`/`a3` vs `a2`/`a1`; the tail pair in `a0`/`v1` vs
`v1`/`a0`) plus the word-4 rotation and the two swapped tail loads. The
`chanVal` form matches more individual words but keeps the tail-store
deferral. The body preserved below is the 60/85 one because the title figure
is the raw word-match; **the next attempt should start from the `old60`
form**, because "one register permutation away" is a much better-defined
target than a scatter of individual mismatches.

### The best body (60/85, length exact 85/85, zero drift)

```c
#if 0
/* file-local reduced view -- see the round-56 section above for the full
 * declaration block; only the function body changes here. */
void SpuVmSeqKeyOff(s32 p0)
{
    s32 key;
    u32 i;
    u16 *chanPtr;
    u32 idx;

    if (spuVmMaxVoice == 0)
        return;
    i = 0;
    key = (s16) p0;
    chanPtr = (u16 *) &D_8008EA26;
    do {
        idx = (u8) i;
        if (D_8008D996[idx].unk0 == key) {
            u16 bankIdx;
            u16 loBit, hiBit;
            u16 old60, old64;
            u16 chanVal;

            chanVal = i;
            *chanPtr = (u8) chanVal;
            bankIdx = D_8008EA26;
            if (bankIdx < 0x10) {
                loBit = 1 << bankIdx;
                hiBit = 0;
            } else {
                loBit = 0;
                hiBit = 1 << (bankIdx - 0x10);
            }
            D_8008D9A3[bankIdx].unk0 = 0;
            D_8008D98C[bankIdx].unk0 = 0;
            _svm_voice[bankIdx].unk0 = 0;
            old60 = _svm_okof1;
            old64 = _svm_okof2;
            old60 = loBit | old60;
            _svm_okof1 = old60;
            _svm_okon1 = _svm_okon1 & ~old60;
            old64 = hiBit | old64;
            _svm_okof2 = old64;
            _svm_okon2 = _svm_okon2 & ~old64;
        }
        i++;
    } while ((u8) i < spuVmMaxVoice);
}
#endif
```

The 270-scoring alternative is the same body with `u16 chanVal;` deleted and
the two `chanVal` lines replaced by `old60 = i; *chanPtr = (u8) old60;`
(57/85, length exact).

**The search is SPENT for this function.** Round 47 declined it on check (b);
round 56 ran it after the hand levers had halved the insertion/deletion
count, and it returned a translatable lever on the one residue hand work had
declared closed.

### Proposed learning

**A `(u8) x` and a `(u8)` of a `u16` COPY of `x` are different expressions to
GCC 2.6.3, and that is a usable lever on "this value is kept live when retail
re-materialises it".** The compiler folds every direct spelling of a
narrowing cast into one `(and x mask)` and CSEs them together; routing the
value through a variable of an intermediate width first produces a nested
mask it does not fold, so the two uses stay independent and the allocator is
free to let one die. Reach for it when the residue is "retail recomputes a
masked induction variable after a clobber and the build keeps it in a second
register instead" -- and note it needs a VARIABLE: an inline `(u8)(u16) i`
folds and does nothing.

**Second, about the method rather than the function: "no source form
distinguishes these" is a claim about the spellings you tried.** This
report's own round-56 section stated that conclusion from five hand
variants, all of which were direct re-spellings of the same cast. The
permuter disproved it in the same round, and the disproof was one `u16`
assignment. A negative of the form "GCC unifies every spelling" should be
written as "every spelling TRIED", with the list, which is what makes it
falsifiable by the next runner instead of closing the axis.

## Round 57 (2026-09-19), runner bravo -- TWO NEW LEVERS, 60/85 -> 73/85, length still exact

**Rebuild-before-trusting-the-score, fifth time.** Spliced round 56's 60/85
body (the `chanVal` form) in verbatim: `build exit=2`, no compile-error grep
hits, `funcdiff.py` reports **60/85 with no out-of-range drift warning**
(`0x20E9C-0x20FF0` = 0x154 = exactly 85 words). Round 56's title figure
reproduced exactly before anything was changed. The 57/85 `old60` form --
which round 56's write-up nominated as the better starting point, because its
residue reads as one consistent register permutation rather than a scatter --
also reproduced at 57/85. **Round 56's nomination was right, and it is what
found both levers below**: starting from `old60` is what made it obvious that
WHICH local carries the `D_8008EA26` value is itself a variable.

Two levers then took it to **73/85, length still exactly 85 words, zero
drift** (`build/lsdde.map` still puts the next symbol, Sony's
`SpuVmSetProgVol`, at its retail `0x800307F0`). Thirteen words.

### CLOSED lever 3 -- the chan temp must be `hiBit`, and that is worth 10 words

Round 56 established that the value stored into `D_8008EA26` has to be routed
through a VARIABLE (`(u8) someLocal`, never an inline `(u8) i` or `(u8)(u16) i`),
and measured five spellings: a fresh `u16 chanVal`, a fresh `u32` narrowed
first, reusing `bankIdx`, and reusing `loBit` all gave 60/85; reusing `old64`
gave 57/85. It picked the fresh `chanVal` local and recorded the axis as
explored.

**The one local it did not try is `hiBit`, and that is the only one that
works.** Measured on the round-57 tail (all six re-measured against the
identical base, one build each):

| chan temp | raw match |
| --- | --- |
| **`hiBit`** | **73/85** |
| `loBit` | 63/85 |
| `bankIdx` | 63/85 |
| fresh `u16 chanVal` | 63/85 |
| `old60` | 63/85 |
| `old64` | 63/85 |

```c
            hiBit = i;
            *chanPtr = (u8) hiBit;
```

`hiBit` is special because it is the local assigned in BOTH arms of the
`bankIdx < 0x10` if/else that follows. Using it as the chan temp extends that
pseudo's live range backwards to cover the whole `if` block, and the allocator
then ranks it ahead of `loBit` and of the induction variable. The effect is
not local to the store: it lands **`i` in `$a3` and `loBit`/`hiBit` in
`$a2`/`$a1`, all three matching retail**, where every previous round had
`i` in `$a2` and the mask pair swapped into `$a1`/`$a3`. The whole loop head
(0x20E9C through 0x20F50, the guard, the preheader, the 0x34 multiply, the
`D_8008EA26` store, both arms and the three zero-stores) is now byte-identical
to retail; before this lever the loop head carried five register diffs.

This CLOSES this report's oldest-but-one open residue, "`loBit`/`hiBit` land
in the wrong scratch registers", which had been carried since round 23 and
cross-referenced from `SsUtKeyOffV`'s report as the same unresolved class.

### CLOSED lever 4 -- materialise the `nor` into a local, and write the `or` as a direct global RMW

Round 56 measured five tail spellings and found them byte-identical, and
concluded "the tail residue is not expression-order steerable". **That
conclusion is correct and it is still correct** -- round 57 re-ran the same
axis on the new base and reproduced it exactly (fully sequential
C60->E228 then C64->E22C; both `or`s computed before either store; `or`
operands written the other way round; the compound `&=` form; an explicit
named local for the `_svm_okon1` read -- **all five byte-identical at 69/85**).

What moves the tail is not ORDER, it is which sub-expression gets a name:

1. **Give the complement its own statement.** `_svm_okon1 = _svm_okon1 & ~old60;`
   and `old60 = ~old60; _svm_okon1 = _svm_okon1 & old60;` are not the same
   program to GCC 2.6.3: the second reproduces retail's
   `nor v1,zero,v1` / `and` / `sh` with the E228 store in retail's position,
   the first sinks the `_svm_okon1` store past the entire C64/E22C chain.
   **69/85 -> 72/85, and it removes the last insertion/deletion pair** -- after
   it the two instruction streams align one-for-one for the whole function.
   Applying it to only one of the two chains is worse than both (71 and 70).
2. **Write the `or` as a direct global read-modify-write.**
   `_svm_okof1 = loBit | _svm_okof1;` gives retail's `or v1,a2,v1`;
   `old60 = _svm_okof1; ... old60 = loBit | old60;` gives `or v1,v1,a2`,
   the operands the other way round. **72/85 -> 73/85.** Round 56 recorded
   "GCC canonicalises commutative operands" from the observation that writing
   `old60 | loBit` instead of `loBit | old60` changes nothing. That is true of
   the two operands of one expression and false of the expression's SHAPE: when
   the destination is the same pseudo as one operand, GCC emits that pseudo as
   `rs`; when the destination is the global's own fresh temp, it does not.

```c
            _svm_okof1 = loBit | _svm_okof1;
            old60 = ~_svm_okof1;
            _svm_okon1 = _svm_okon1 & old60;
            _svm_okof2 = hiBit | _svm_okof2;
            old64 = ~_svm_okof2;
            _svm_okon2 = _svm_okon2 & old64;
```

Note this is NOT `SsUtKeyOff`'s tail idiom, which round 56 measured at
47/85 and which folds the complement back into the `&=`; the complement's own
local is what distinguishes them, and it is load-bearing.

### The best body (73/85, length exact 85/85, zero drift)

```c
#if 0
/* file-local reduced view, all already present in libsnd_vm_vol_ut_key_ut_keyv.c under
 * the same names/types -- keep them local to the unit, not in a header */
extern u8 spuVmMaxVoice;
extern volatile u16 D_8008EA26;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 _svm_okon1;
extern u16 _svm_okon2;

typedef struct {
    s16 unk0;
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D996[];
extern Rec34Half _svm_voice[];
extern Rec34Half D_8008D98C[];

typedef struct {
    u8 unk0;
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

void SpuVmSeqKeyOff(s32 p0)
{
    s32 key;
    u32 i;
    u16 *chanPtr;
    u32 idx;

    if (spuVmMaxVoice == 0)
        return;
    i = 0;
    key = (s16) p0;
    chanPtr = (u16 *) &D_8008EA26;
    do {
        idx = (u8) i;
        if (D_8008D996[idx].unk0 == key) {
            u16 bankIdx;
            u16 loBit, hiBit;
            u16 old60, old64;

            hiBit = i;
            *chanPtr = (u8) hiBit;
            bankIdx = D_8008EA26;
            if (bankIdx < 0x10) {
                loBit = 1 << bankIdx;
                hiBit = 0;
            } else {
                loBit = 0;
                hiBit = 1 << (bankIdx - 0x10);
            }
            D_8008D9A3[bankIdx].unk0 = 0;
            D_8008D98C[bankIdx].unk0 = 0;
            _svm_voice[bankIdx].unk0 = 0;
            _svm_okof1 = loBit | _svm_okof1;
            old60 = ~_svm_okof1;
            _svm_okon1 = _svm_okon1 & old60;
            _svm_okof2 = hiBit | _svm_okof2;
            old64 = ~_svm_okof2;
            _svm_okon2 = _svm_okon2 & old64;
        }
        i++;
    } while ((u8) i < spuVmMaxVoice);
}
#endif
```

### The residue that remains (12 words), now only TWO things

Every insertion, deletion and reordering is gone; `asm-differ` aligns the two
streams one-for-one from 0x20EB4 to the end.

1. **The word-4 branch-delay ROTATION -- 2 words, unchanged from round 56.**
   Retail's `beqz v0,end` delay slot holds `move a3,zero` (`i = 0`) and
   `sll v0,a0,0x10` (the start of `key = (s16) p0`) follows; the build fills
   the slot with the `sll` and puts `move a3,zero` after it. Both instructions
   are present in both streams, one word apart. The delay-slot filler takes
   whichever instruction the scheduler put first in the fall-through block, and
   the scheduler prefers the `sll` because it heads a two-instruction chain
   (`sll` -> `sra t1`) where the `move` heads none.
2. **WHICH of `_svm_okof2` and `_svm_okon1` is hoisted into the second early
   load slot -- 2 words directly, 8 more downstream, 10 words in total.**
   Both streams hoist exactly two loads above the `D_8008D98C`/`_svm_voice`
   zero-stores. Retail hoists `_svm_okof1` and `_svm_okof2` and loads
   `_svm_okon1` late at 0x20F88; the build hoists `_svm_okof1` and
   `_svm_okon1` and loads `_svm_okof2` late at the same 0x20F88. The eight
   `$v0`/`$v1`/`$a0` differences through 0x20F9C-0x20FC0 are all downstream of
   that one choice -- whichever value lands in the early slot gets `$a0`.
   Note retail's own read order (C60, C64, E228) is not its USE order
   (C60, E228, C64), while the build's is; the build is scheduling the loads
   in use order and retail is not.

### Axes tried this round and REJECTED (every one measured on the real oracle)

- **Source placement of the `_svm_okof2` read, seven structurally distinct
  spellings, ALL BYTE-IDENTICAL at 73/85:** a dedicated `pre64` local read
  first; reusing `old64` as the preload; preloading both C60 and C64; a fresh
  `n60`/`n64` pseudo for each `or` result; C64 preloaded with C60 left as a
  direct RMW; the compound `&=` form for both E-globals; and the plain base.
  **Residue 2 is not reachable by moving the read.**
- **`__asm__("")` barriers, six placements, every one a regression:** before
  the zero-stores (72, i.e. neutral-to-worse), after them (63), between the
  reads and the RMW block (63), before the `_svm_okon1` statement with C64
  preloaded (58), before it without (59), and two preheader placements that
  changed the length outright (funcdiff refused the score). Round 56 found the
  same for its three tail placements; this is now nine measured placements
  across two rounds and the conclusion is stable -- **the barrier idiom that is
  load-bearing for this unit's matched `SpuVmGetSeqLVol`/`SpuVmGetSeqRVol` does not
  transfer into this loop at any point**, consistent with `SpuVmSetSeqVol`'s
  round-32 donor-register-pressure finding.
- **Reversing the two tail pairs** (C64/E22C chain written before C60/E228):
  45/85.
- **Folding the `or` into the read** (`old60 = _svm_okof1 | loBit;` as one
  statement, then store): 61/85.
- **Reusing `bankIdx` as the `_svm_okon1` read's temp:** 28/85, by far the
  worst variant measured -- it collides with `bankIdx`'s own live range through
  the three zero-stores' index.
- **All six preheader statement orderings re-measured on the new base:**
  `i`, `key`, `chanPtr` is still the unique best (73); `key, i, chanPtr` costs
  1, every other ordering costs 3. Round 56's finding reproduces exactly.
- **`s32 i` instead of `u32 i`:** byte-identical (third confirmation, rounds
  23, 56, 57).
- **`s16 key` instead of `s32 key`:** 70/85.
- **Dropping the `key` local and comparing against `(s16) p0` inline:** 70/85 --
  the sign-extension is still hoisted into the preheader, but a word moves.
- **`idx = 0;` added ahead of `i = 0;`** to try to reorder the preheader:
  byte-identical.

### Proposed learning

**1. When a lever is "route this value through a local", WHICH EXISTING LOCAL
is a second, independent lever, and it can be worth more than the first one.**
Round 56 found the routing lever on this function and measured five candidate
temps; round 57 found that the sixth -- the only one assigned in both arms of
the following if/else -- is worth ten words where all five others are worth
zero. The mechanism is live-range extension: reusing a local that is written
later in the block extends that pseudo backwards over the intervening code and
changes its allocation priority relative to its neighbours. So the enumeration
to run is not "fresh local vs. inline"; it is **every local already in scope,
one build each**. It is six builds on a function like this one, and the report
that skips it records a negative it did not measure.

**2. Giving a sub-expression its own name is a different lever from
reordering statements, and a report can be exactly right about the second
while the first is untouched.** Round 56 measured five tail orderings, found
them byte-identical, and wrote "the tail residue is not expression-order
steerable". Round 57 reproduced all five byte-for-byte -- and then moved the
tail 4 words by splitting `x & ~y` into `t = ~y; x & t;` and by changing
`local = a | local; global = local;` into `global = a | global;`. Neither is a
reordering. **"Not order-steerable" and "not source-steerable" are different
claims**; write the first one and the next runner still has the naming and
RMW-shape axes to try.

**3. GCC 2.6.3 does not canonicalise a commutative operand pair independently
of the destination.** Writing `a | b` versus `b | a` for the same destination
is genuinely byte-identical (round 56 measured this correctly). But
`global = x | global` emits `or rd,x,global` while
`local = global; local = x | local` emits `or rd,local,x` -- same values, same
instruction, operands the other way round. When the residue is one commutative
instruction's operand order, the lever is the destination, not the operands.

## Track 2 (round 86, 2026-09-26, alpha)

This function is still `INCLUDE_ASM` and its C was not touched, but the per-field symbols this report uses (`D_8008D988`..`D_8008D9BA` at a 0x34 stride) are ONE Sony table: libsnd/vmanager.o's `_svm_voice` (0x8008D988, 24 x 0x34 = 0x4E0 bytes), typed in `include/SvmData.h` with fields by offset (`D_8008D98C` is `_svm_voice[i].unk04`, `D_8008D9A3` is `unk1B`, and so on: address minus 0x8008D988). The next attempt should write `_svm_voice[i].unkNN`: in every converted accessor (libsnd_vm_vol_ut_key_ut_keyv/j_c/l/m/p) the struct spelling compiled byte-identically to the separate symbols, and two NON_MATCHING bodies moved closer to retail. The other `D_` spellings in preserved bodies below still link (splat keeps them as auto-symbols); `D_8008D988` itself now reads `_svm_voice` above, since that address is the table's own symbol.
