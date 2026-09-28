# SpuVmDoAllocate -- STALL (1 word short: 142/143 compiled length; 10/143 raw word-match, badly degraded by the 1-word shift -- see caveat; first real diff at word 0, off=0x01DEA4, vram=0x8002D6A4, asm-differ realigned)

> Renamed from `func_8002D6A4` on 2026-09-24 (tools/rename.py). Address 0x8002d6a4.

Unit `src/psyq/libsnd_vmanager.c` (carved round 24). Size: 143 words (0x23C bytes).
Round 26, runner delta.

**Read the raw-word-match caveat before trusting the 10/143 figure at face
value.** `funcdiff.py` does not realign; with the compiled body still 1 word
short, every instruction after the very first prologue reshuffle lands at
the wrong RAW offset even where the bytes are logically identical, so 10/143
undercounts badly. `tools/asm-differ/diff.py`'s realigned view (used
throughout this report) shows the function is byte-identical or
structurally equivalent across the overwhelming majority of its body; the
real residue is a handful of localized register-allocation differences plus
one still-missing word, detailed below. Do not re-derive a "10/143 means
this is far off" read without looking at the realigned diff first.

## What it is

No arguments, no return value. It:

1. Takes `&D_8008EA28` and derives a pointer one `s16` before it (which
   lands on `D_8008EA26`'s own address) — used later purely as a fixed
   dereference inside a loop, never advanced.
2. Reads `D_8008EA26` once (`chan`), stores `chan << 3` into `D_8008EA28`
   through the pointer from step 1, computes
   `D_8008EA18 + (D_8008EA13 << 4)` (the same channel-index idiom this
   unit's `note2pitch2`/`SpuVmDoAllocate`'s sibling functions use) and
   stashes it in the scalar `D_8008EA2A`, then resets
   `D_8008D98E[chan].unk0` to `0x7FFF` (priority-table reset, same 0x34
   -stride family `SpuVmAlloc` uses).
3. Clears one bit (`1 << D_8008EA26`, read through the pointer from step 1)
   across all 16 words of the `_svm_envx_hist` array — a "clear this channel's
   bit everywhere" sweep.
4. Picks one of two adjacent `u16` fields (`+0xC` or `+0xE`) of a
   0x10-stride table `_svm_pg`, indexed by
   `((s16) D_8008EA24 - 1) / 2`, based on `D_8008EA24 & 1`, and stores the
   result into `D_8008D7F6[D_8008EA28]`.
5. Sets flag bit `0x8` in `_svm_sreg_dirty[D_8008EA26]`.
6. Copies two more fields (`+0x10`, `+0x12`, the second plus `_svm_damper`)
   out of the SAME 0x20-stride `_svm_tn` table `note2pitch2` already
   established in this unit, into `D_8008D7F8[D_8008EA28]` /
   `D_8008D7FA[D_8008EA28]`.
7. Sets flag bits `0x30` in `_svm_sreg_dirty[D_8008EA26]`.

## Struct/global model

```c
extern volatile s16 D_8008EA28;
extern volatile s16 D_8008EA26;
extern s32 _svm_envx_hist[];
extern u8 D_8008EA13;
extern u8 D_8008EA18;

typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D98E[];

extern s16 D_8008EA2A;
extern u16 D_8008EA24;

typedef struct {
    u8 pad0[0xC];
    u16 unkC; /* +0xC */
    u16 unkE; /* +0xE */
} D8008E968RecCE;
extern D8008E968RecCE *_svm_pg;

extern s16 D_8008D7F6[];
extern u8 _svm_sreg_dirty[];

/* Same base pointer as this unit's own note2pitch2 (D8008E978Entry) --
 * extended here with the two halfwords at +0x10/+0x12 this function reads,
 * matching libsnd_cres.c's independent Rec32E978 view of the identical
 * offsets. If this function is picked back up, re-extend note2pitch2's
 * existing D8008E978Entry typedef in place (it is declared once, ahead of
 * both functions, in ROM order) rather than redeclaring it. */
typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[6];
    u8 unk12;
    u8 unk13;
    u8 pad14[2];
    u16 unk16; /* +0x10 */
    u16 unk18; /* +0x12 */
    u8 pad20[0x20 - 20];
} D8008E978Entry;
extern D8008E978Entry *_svm_tn;

extern s16 D_8008D7F8[];
extern s16 D_8008D7FA[];
extern s16 _svm_damper;
```

**`D_8008EA28` and `D_8008EA26` are 2 bytes apart in the linked image, and
the function proves it at compile time.** `t0 = &D_8008EA28 - 1` compiles
to `addiu $t0, $v1, -0x2` reusing `$v1` (D_8008EA28's own materialized
`lui`/`addiu`) — a literal displacement off ANOTHER symbol's base is only
emittable when the SOURCE writes exactly that pointer arithmetic (cc1 has
no knowledge of the two symbols' relative addresses otherwise; see
`SsUtSetDetVVol`'s report in `libsnd_vmanager.c` for the general form of this
discriminator). This does NOT mean the two should be modeled as one
struct/array — `D_8008EA26` is read directly by its OWN symbol at three
other points in this same function, so both a named `D_8008EA26` extern
AND the `&D_8008EA28 - 1` pointer expression must coexist.

**Both are `volatile`, and this matters structurally, not just for one
reload.** Every one of `D_8008EA28`/`D_8008EA26`'s many references in this
function reads fresh from memory even immediately after a previous read of
the same symbol with no intervening write to it — including two full
`D_8008EA13`+`D_8008EA18` recomputations back-to-back separated only by an
unrelated array store, and re-deriving `D_8008EA28*2`'s address twice for
two adjacent but distinct destination arrays. `volatile` reproduces all of
this uniformly; see `libsnd_vmanager.c`'s own note on `D_8008EA26` needing
`volatile` for the identical reason (a re-read the compiler would otherwise
prove redundant and elide).

## Best body reached (142/143 compiled words, 1 short)

```c
void SpuVmDoAllocate(void)
{
    volatile s16 *p28;
    volatile s16 *maskPtr;
    s32 *p;
    s32 i;
    s16 chan;
    s32 idx;
    s32 divRes;
    u16 val;

    p28 = &D_8008EA28;
    maskPtr = p28 - 1;
    chan = D_8008EA26;
    p = _svm_envx_hist;
    *p28 = chan << 3;
    idx = D_8008EA18 + (D_8008EA13 << 4);
    D_8008EA2A = idx;
    D_8008D98E[chan].unk0 = 0x7FFF;

    for (i = 0; i < 16; i++) {
        *p &= ~(1 << *maskPtr);
        p++;
    }

    if (D_8008EA24 & 1) {
        divRes = ((s16) D_8008EA24 - 1) / 2;
        val = _svm_pg[divRes].unkC;
        D_8008D7F6[D_8008EA28] = val;
    } else {
        divRes = ((s16) D_8008EA24 - 1) / 2;
        val = _svm_pg[divRes].unkE;
        D_8008D7F6[D_8008EA28] = val;
    }

    chan = D_8008EA26;
    _svm_sreg_dirty[chan] |= 8;

    idx = D_8008EA18 + (D_8008EA13 << 4);
    D_8008D7F8[D_8008EA28] = _svm_tn[idx].unk16;

    idx = D_8008EA18 + (D_8008EA13 << 4);
    D_8008D7FA[D_8008EA28] = _svm_tn[idx].unk18 + _svm_damper;
    __asm__("");

    chan = D_8008EA26;
    _svm_sreg_dirty[chan] |= 0x30;
}
```

Restored to `INCLUDE_ASM("asm/nonmatchings/libsnd_vmanager", SpuVmDoAllocate);`.
The `D8008E978Entry` typedef was reverted to its ORIGINAL (pre-this-attempt)
shape ahead of `note2pitch2`, which still needs it and is otherwise
unaffected.

## Attempts and what each one fixed (naive form was 130/143; six fixes to 142)

1. **`p28`/`maskPtr` as a genuine pointer pair, not two independent address
   constants.** Writing `maskPtr = &D_8008EA28 - 1;` directly (no `p28`
   variable) computes `t0` via its OWN fresh `lui`/`addiu`, then computes a
   SEPARATE `lui`/`addiu` for the later plain write `D_8008EA28 = ...;` —
   two unrelated address materializations. Retail computes ONE base and
   derives both the pointer AND the write's address from it. Introducing
   `p28` and writing `*p28 = ...;` for the store (instead of the bare
   global name) forced the reuse.
2. **`divRes` must be `s32`, not `s16`.** With `s16 divRes`, GCC inserts an
   extra 2-instruction "narrow to 16 bits with sign extension" step
   (`sll #0xf` / `sra #0x10`) as part of the assignment, because the
   division result is about to be stored into a narrow destination that is
   then used as a wider array index — retail's `sra $v0,$v0,0x1` (the
   division's own final shift) is the LAST instruction of that sequence
   with `s32`.
3. **The per-branch division+lookup+store must be textually duplicated in
   BOTH `if`/`else` arms, not shared after the `if`.** Sharing
   `D_8008D7F6[D_8008EA28] = val;` as one statement after the `if`/`else`
   lets GCC hoist the `D_8008EA28` read and `<<1` out of both arms into a
   single shared computation; retail redoes the read AND the shift inside
   EACH arm independently. This is the general "retail keeps two
   independent materializations" shape from `DECOMPILATION_LEARNINGS.md`'s
   block-order section, just appearing at expression-duplication
   granularity instead of statement-duplication.
4. **`D_8008EA26` must be declared `volatile s16` (not `u16`).** Every
   read of it in this function is an `lh` (signed); a `u16` declaration
   reads via `lhu`. (This is a DIFFERENT independent local view than
   `code_179d8_j.c`/`libsnd_vmanager.c`'s own `volatile u16 D_8008EA26` — both
   are legitimate per the project's per-unit reduced-view convention; only
   the instructions THIS function emits decide THIS unit's view.)
5. **`D_8008EA28` must ALSO be `volatile`.** Without it, GCC proves the
   later re-reads of `D_8008EA28` (by name, after the pointer-store through
   `p28`) are redundant with the value just written and elides them,
   collapsing several of the redundant `lui`/`lh`/`sll #1` sequences retail
   keeps. Marking it volatile restored every one of them across the whole
   function in one change (went from 134 to 140/143).
6. **A bare `__asm__("")` between the LAST `D_8008D7FA[...] = ...;` store
   and the trailing `_svm_sreg_dirty[...] |= 0x30;`.** Without it, GCC schedules
   the (independent, differently-addressed) `D_8008D7FA` store AFTER the
   `|=0x30` store even though it is written first in source — a pure
   instruction-order rearrangement (confirmed: removing the barrier changes
   only which store's four instructions come last, never which register
   holds which value), which is exactly the permitted use of this idiom
   per CLAUDE.md's test. Fixed the tail ordering (140 -> 142/143).

## What is still wrong (the residue)

**One word short, and a small number of narrow-load register-selection
differences that don't change instruction count but do change bytes.**

- **The `chan = D_8008EA26; _svm_sreg_dirty[chan] |= N;` idiom (used twice, for
  the `|=8` and `|=0x30` bit-sets) compiles to an `lhu` immediately followed
  by a manual `sll #0x10`/`sra #0x10` sign-extend pair (2 extra
  instructions each = 4 extra words total), where retail does the
  equivalent in ONE `lh` instruction.** This is a genuine, reproducible
  GCC 2.6.3 quirk specific to a signed `volatile` scalar being used
  (directly OR through an intermediate plain local freshly assigned from
  it) as an array index — every variant tried triggers it identically:
  - Direct `_svm_sreg_dirty[D_8008EA26]` (no intermediate variable): same
    `lhu`+extend pair.
  - `s16 chan = D_8008EA26; _svm_sreg_dirty[chan]`: same pair (the version kept
    in the body above).
  - `s32 chan = D_8008EA26; _svm_sreg_dirty[chan]`: same pair, AND it additionally
    REGRESSES the earlier `chan = D_8008EA26; *p28 = chan << 3;` sequence
    from a clean `lh` to the same `lhu`+extend pattern — so `s32` is
    strictly worse and was reverted.
  - `*(u8 *) &D_8008EA26` (the idiom `libsnd_vmanager.c` documents for
    reading this exact global's low byte through a plain, non-volatile
    pointer, specifically to fold to a compact `lui`+`lbu`): tried at one
    call site in isolation. It did **not** produce the compact form here —
    instead it triggered a cascading REGISTER-ALLOCATION change several
    instructions away (a `sra $a0,$a0,0xf` where a `0x10` was expected,
    in the wholly unrelated `_svm_tn` field-copy block), costing 3 words
    at that one site alone. This is the CLAUDE.md-documented hazard of
    treating a local register-pressure change as free — it is not free,
    the allocator's budget is shared across the whole function — and it is
    evidence AGAINST this being the right idiom for this occurrence, not
    just a wash.
- **Net effect:** these two sites cost 4 words the function doesn't have to
  spare, and something ELSE in the body is 5 words CHEAPER than retail
  elsewhere to net out at "1 short" rather than "4 short" — i.e. this is
  most likely not a single isolated residue but at least two, one of which
  partially cancels the other in the raw instruction count. Whoever picks
  this up should NOT assume fixing the `lh`/`lhu` residue alone closes the
  function; re-measure the total length after any fix here.

**Not tried:** reading `D_8008EA26` through a `volatile s16 *` pointer
variable (rather than the bare global name or a plain local) for these two
specific sites only — by analogy with fix #1 above (`p28`), a genuine
pointer-typed intermediate might behave differently from both the bare
name and a plain-typed copy. This is the natural next lever.

## Proposed learnings

- **A volatile global's `s16`/`u16` declared type must match the retail
  instruction's signedness at EVERY read site independently within a unit
  — there is no single "right" type if the retail disassembly itself mixes
  `lh` and `lhu` for the same symbol.** This function never needed that
  (all reads are `lh`), but the discriminator generalizes: check every
  `lh`/`lhu` of the same volatile symbol before picking one declared type
  and assuming it covers the whole function.
- **A local pointer-arithmetic idiom validated for a shared-vtable-style
  struct field (`SsUtSetDetVVol`'s `&sym - N`) applies just as well to two
  independent SCALAR globals placed 2 bytes apart, with the same
  discriminator (a literal displacement off the OTHER symbol's own
  materialized base).** Worth generalizing the existing writeup beyond
  arrays.
- **An idiom that fixes a residue in one function is not portable to a
  different call site of the SAME symbol without re-measuring the WHOLE
  function's byte count.** `*(u8 *) &D_8008EA26` is confirmed correct and
  necessary in `libsnd_vmanager.c`; applied here it regressed a distant,
  unrelated block by 3 words. Register pressure is global, not local to
  the statement being edited — CLAUDE.md already says this for struct
  edits; this is the same fact for a single-expression idiom swap.

## Round 26 follow-up: the "Not tried" lever, tried (bounded, 5 attempts, negative)

The coordinator asked for a bounded (5-attempt) re-check of the one lever
this report had flagged as untested: reading `D_8008EA26` through a
`volatile s16 *` pointer variable for the two `_svm_sreg_dirty[D_8008EA26]`
sites, by analogy with fix #1 (`p28`). Result: **no improvement, still
142/143.** Five variants tried, all compiling clean:

1. `chanPtr = &D_8008EA26;` immediately before each of the two sites,
   `_svm_sreg_dirty[*chanPtr] |= N;` — **142/143 (neutral)**. Changed the
   instruction MIX at that site from retail's target shape but kept the
   same total: `lhu`+`sll`+`sra` (3 instr, the original bare-symbol form)
   became `addiu`+`lhu` (2 instr) — one instruction cheaper locally, but
   the function's total length didn't move, so something else grew by one
   word to compensate. Confirms these residues are NOT independent, exactly
   as the original report's "1 word short is likely two residues partly
   cancelling" note predicted.
2. Reusing `maskPtr` itself for both later sites (since
   `maskPtr == &D_8008EA26` numerically, being `p28 - 1`) instead of a
   fresh `chanPtr` — **140/143, regressed.**
3. `chanPtr` assigned ONCE near the top (aliased from `maskPtr`) and read
   via `*chanPtr` at both later sites with no reassignment in between —
   **141/143, regressed.**
4. `*(volatile s16 *) &D_8008EA26` inlined directly at each site, no named
   pointer variable at all — **142/143, byte-identical shape to variant 1**
   (same `addiu`+`lhu` pair, same net length).
5. `(s16) D_8008EA26` — a redundant explicit cast to the variable's own
   already-`s16` declared type, on the theory that it might force a
   different front-end code path than the bare reference — **142/143,
   no change** from the ORIGINAL bare-symbol form's count, though this one
   reintroduces the 3-instruction `lhu`+`sll`+`sra` shape rather than
   variant 1/4's 2-instruction `addiu`+`lhu` shape (still not retail's
   2-instruction `lui`+`lh`).

**Why none of these can reach retail's shape, read off the instructions
directly (not inferred):** retail's read is `lui $v1,%hi(D_8008EA26);
lh $v1,%lo(D_8008EA26)($v1)` — a plain, freshly-materialized, SIGNED
2-instruction symbol load with no reuse of any other register. Every
variant above produces one of exactly two OTHER shapes: (a) a bare-symbol
reference that loads UNSIGNED (`lhu`) and then manually sign-extends
(`sll #0x10`/`sra #0x10`, 3 instructions total), or (b) a pointer-typed
reference that computes the address as an `addiu` off some OTHER,
already-live base register rather than its own fresh `lui` (2
instructions, but the wrong 2 — `addiu`+`lhu`, unsigned, not `lui`+`lh`,
signed). Neither the plain-local, s32-local, pointer-variable, nor
inline-cast phrasing reproduces the combination of (freshly-materialized
base) AND (signed load) retail shows together. Something about this
specific `_svm_sreg_dirty[...]` array-index USE of a `volatile s16` scalar
makes GCC 2.6.3 treat the sign-extension and the addressing as mutually
exclusive optimizations in this codebase's exact configuration — pick
either a clean address (unsigned) or a clean sign (bare symbol, unsigned
load + manual extend), never both together the way retail's source
apparently got for free.

**Disposition: closing this lever, not just "not tried" anymore.** Restored
to `INCLUDE_ASM`; `src/psyq/libsnd_vmanager.c` is otherwise unchanged from before
this follow-up (confirmed via `git diff --stat` showing no diff after
reverting). If this function is picked up again, the productive angle is
almost certainly NOT this lh/lhu residue in isolation — per variant 1's
finding, fixing it locally didn't close the function, so whatever is
absorbing that saved word elsewhere in the body is the more load-bearing
unknown. Re-reading the WHOLE function's realigned diff for a genuinely
untried structural axis (not another rephrasing of this same load) is the
next move, not another cast variant.

## Round 35 update (runner bravo): inherited body re-verified real; one genuine byte-level fix found (still 142/143, unchanged total)

Re-verified the inherited 142/143 body first: `objdump -t` on
`build/src/libsnd_vmanager.c.o` confirms `SpuVmDoAllocate` compiles to `0x238`
bytes = 142 words, matching the report's figure exactly.

**Found and fixed one previously-undocumented byte-level mismatch while
reading the realigned `asm-differ` diff fresh (not one of the report's
already-catalogued residues):** at the `if (D_8008EA24 & 1)` branch,
retail emits `blez $v0,<target>` (branch if `<= 0`) where the naive
`if (D_8008EA24 & 1)` phrasing compiles to `beqz $v0,<target>` (branch if
`== 0`). Both are semantically equivalent for the value range `D_8008EA24
& 1` can take (0 or 1), so this was invisible to any word-count
measurement, but it is a genuine instruction-selection mismatch, not a
register-identity one. **Fix: write the condition as
`if ((s16)(D_8008EA24 & 1) > 0)`** — an explicit signed `> 0` comparison —
which reproduces `blez` exactly. Confirmed via `asm-differ`: the `blez`
instruction and its branch target now match retail byte-for-byte at that
site (previously a `r`-tagged register/opcode mismatch line, now clean).

**This did not change the function's total compiled length (still
142/143, 1 word short)** — it is a pure byte-correctness fix layered on
top of the already-known 1-word-short residue, not progress toward closing
the gap itself. Kept in the preserved body below since it is strictly an
improvement (one fewer byte-level mismatch) and costs nothing.

**The primary 1-word-short residue is unchanged from the round-26
follow-up's conclusion** — re-confirmed by reading the realigned diff at
the function's opening (`p28`/`maskPtr`/`chan` sequence): the same
register-identity-flavored scrambling round 26 already tried five
additional variants against and explicitly closed as "not fixable by
naming, aliasing, or casting; needs a genuinely different structural
angle, none identified." No new structural angle was found this round for
that specific residue; the `blez`/`beqz` fix above is independent of it
and was found by re-scanning the whole diff rather than re-attacking the
known residue.

**Disposition: restored to `INCLUDE_ASM`** (still not byte-exact). The
`blez` fix is preserved in the body below for whoever next attempts this
function, alongside all of round 26's already-documented residue analysis,
which remains the primary blocker.

```c
extern volatile s16 D_8008EA28;
extern volatile s16 D_8008EA26;
extern s32 _svm_envx_hist[];
extern u8 D_8008EA13;
extern u8 D_8008EA18;

typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D98E[];

extern s16 D_8008EA2A;
extern u16 D_8008EA24;

typedef struct {
    u8 pad0[0xC];
    u16 unkC; /* +0xC */
    u16 unkE; /* +0xE */
} D8008E968RecCE;
extern D8008E968RecCE *_svm_pg;

extern s16 D_8008D7F6[];
extern u8 _svm_sreg_dirty[];

/* Same base pointer as this unit's own note2pitch2 (D8008E978Entry) --
 * extended here with the two halfwords at +0x10/+0x12 this function reads.
 * If this function is picked back up, re-extend note2pitch2's existing
 * D8008E978Entry typedef in place (declared once, ahead of both functions,
 * in ROM order) rather than redeclaring it -- doing so this round required
 * moving the struct's declaration point earlier in the file, since this
 * function's vram address precedes note2pitch2's. */
typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[6];
    u8 unk12;
    u8 unk13;
    u8 pad14[2];
    u16 unk16; /* +0x10 */
    u16 unk18; /* +0x12 */
    u8 pad20[0x20 - 20];
} D8008E978Entry;
extern D8008E978Entry *_svm_tn;

extern s16 D_8008D7F8[];
extern s16 D_8008D7FA[];
extern s16 _svm_damper;

void SpuVmDoAllocate(void)
{
    volatile s16 *p28;
    volatile s16 *maskPtr;
    s32 *p;
    s32 i;
    s16 chan;
    s32 idx;
    s32 divRes;
    u16 val;

    p28 = &D_8008EA28;
    maskPtr = p28 - 1;
    chan = D_8008EA26;
    p = _svm_envx_hist;
    *p28 = chan << 3;
    idx = D_8008EA18 + (D_8008EA13 << 4);
    D_8008EA2A = idx;
    D_8008D98E[chan].unk0 = 0x7FFF;

    for (i = 0; i < 16; i++) {
        *p &= ~(1 << *maskPtr);
        p++;
    }

    if ((s16)(D_8008EA24 & 1) > 0) {
        divRes = ((s16) D_8008EA24 - 1) / 2;
        val = _svm_pg[divRes].unkC;
        D_8008D7F6[D_8008EA28] = val;
    } else {
        divRes = ((s16) D_8008EA24 - 1) / 2;
        val = _svm_pg[divRes].unkE;
        D_8008D7F6[D_8008EA28] = val;
    }

    chan = D_8008EA26;
    _svm_sreg_dirty[chan] |= 8;

    idx = D_8008EA18 + (D_8008EA13 << 4);
    D_8008D7F8[D_8008EA28] = _svm_tn[idx].unk16;

    idx = D_8008EA18 + (D_8008EA13 << 4);
    D_8008D7FA[D_8008EA28] = _svm_tn[idx].unk18 + _svm_damper;
    __asm__("");

    chan = D_8008EA26;
    _svm_sreg_dirty[chan] |= 0x30;
}
```

### Proposed learning (round 35)

**A word-count-neutral instruction-selection mismatch can hide in a
function whose only known residue is a length gap** — this `blez`-vs-`beqz`
difference cost zero words either way, so it was invisible to every
word-count-based measurement this function had been screened with across
two prior rounds, and only surfaced by re-reading the FULL realigned diff
rather than jumping straight to the known residue's location. Worth a
general habit: even a well-characterized near-miss should get one fresh
full read of `asm-differ`'s output before more attempts, since "the known
residue" and "the only residue" are not the same claim. The specific fix
— an unsigned-truthiness test rewritten as an explicit signed `> 0`
comparison on a value known to be non-negative — reproduces `blez` in
place of `beqz`; worth checking for elsewhere in this unit's remaining
stalls.

## Round 37 update (runner alpha): first permuter search on this function, negative

Re-verified the inherited 142/143 body first (with the round-35 `blez` fix
included): required moving/extending the shared `D8008E978Entry` typedef
earlier in the file (this function precedes `note2pitch2` in ROM order,
and the extended shape — trailing `unk14[18]` split into
`pad14[2]/unk16/unk18/pad20[12]` — is layout-compatible with
`note2pitch2`'s use of only offsets 4/5). Confirmed via `objdump -t`:
`SpuVmDoAllocate` compiles to `0x238` bytes = 142 words, matching the report
exactly. Reverted the relocation immediately afterward since it isn't
needed for anything short of closing the function.

**`--debug --stack-diffs` sanity check:** base score = **5710** — far from
the "one insertion + one deletion, score 200" clean signature, consistent
with this report's own finding that the residue is a diffuse
register-identity/lh-lhu scramble spread across multiple independent call
sites (three prior techniques already confirmed unable to touch it across
rounds 26 and 35). `Stack Differences: 0 (1)` was a real filled measurement.

**Search: `-j 6 --stop-on-zero --best-only --stack-diffs`, bounded to
900s, `rc` captured this time via `nohup bash -c '...; echo "rc=$?" >>
log'`** — but the `echo` still did not appear in the log (checked with
`grep -a 'rc='`, no hit), even though the process had exited cleanly by the
time the wait loop returned. Likely cause: the `timeout`-wrapped permuter
process's own multiprocessing workers hold the log file descriptor open
briefly past the parent's exit (visible in the log's own trailing
`resource_tracker: ... leaked semaphore objects` warning), so the `echo`
race is not fully solved by this wrapping alone. Flag for the next runner:
this needs the `echo` to happen in a script that `wait`s on the actual
permuter PID specifically, not just relies on `bash -c`'s own sequencing.
Inferred (not captured) as `rc=124` (bound fired) from wall-clock timing:
the process ended almost exactly at the 900s mark, matching
`SePitchBend`'s same-shaped exit earlier this round.

**Result: 76056 iterations, ZERO hits at `score = 0`.** Best score reached
was **4245**, down from the 5710 baseline.

**The 4245 candidate is a NEGATIVE result once translated — confirmed by
direct testing, not assumed.** Its only structural change from the base
was hoisting `D_8008EA18 + (D_8008EA13 << 4)` into a new `new_var` local
computed once at the top, then reusing it for the FIRST of the two
`D_8008D7F8`/`D_8008D7FA` index computations while leaving the second one
recomputed fresh. This is precisely the sharing pattern fix #3 in this
report's own "Attempts" section already identified and rejected — the two
sites must each recompute the index independently, not share one. Spliced
directly into the real unit and rebuilt: **144 words (0x240), two words
WORSE than the 142/143 baseline**, not an improvement. Reverted; `git diff
--stat` clean against the pre-session baseline.

**Disposition: unchanged at 142/143 (1 word short).** Not closed in 76056
iterations under shared-machine load (concurrent permuter searches were
running in sibling worktrees this round). The permuter's own scorer
improved (5710 -> 4245) via a change that is a confirmed regression against
the real project oracle — a second instance this round (after
`SePitchBend`) of the isolated-scaffold-score / real-build-score
divergence CLAUDE.md warns about. Not promoting to "permuter-exhausted";
recording as "not closed in ~76k iterations under load, and the only
lower-scoring candidate found reintroduces an already-rejected sharing
pattern."

### Proposed learning (round 37)

**A permuter candidate that LOOKS like a plausible CSE (hoisting a
shared subexpression to one call site) can be exactly the pattern a prior
round's derivation already tried and rejected for THIS function** — worth
checking any permuter candidate's structural change against the function's
own report history before assuming a lower isolated-scaffold score means
anything, not just checking the final word count. This is the same
underlying caution as `SePitchBend`'s round-37 finding (permuter score
improvement does not imply real-build improvement) from a different angle:
here the specific mechanism (re-sharing a deliberately-duplicated
computation) was independently already known-bad from this same function's
own history, which made verifying it cheap and worthwhile rather than
skippable.

## Round 44 update (runner delta): inherited body re-verified real (including the D8008E978Entry struct relocation), not re-attempted further

Re-verified the inherited 142/143 body (with the round-35 `blez` fix and
the round-37 struct-sharing note both intact): `objdump -t` on
`build/src/libsnd_vmanager.c.o` confirms `SpuVmDoAllocate` compiles to `0x238`
bytes = 142 words, matching the report exactly, and `note2pitch2`
(already matched, sharing the same `D8008E978Entry` typedef) is
unaffected at `0x100` bytes = 64 words.

**The shared `D8008E978Entry` typedef this report's round-37 section noted
as "requires moving the struct's declaration point earlier in the file" is
now PERMANENTLY relocated in `src/psyq/libsnd_vmanager.c`**, ahead of
`SpuVmDoAllocate`'s own (still-`INCLUDE_ASM`) slot, with the extended
`unk16`/`unk18` fields this function's derivation needs -- committed
separately this round as a byte-exact, zero-functional-change struct move
so the next attempt at this function does not need to redo that relocation
by hand.

Given three prior rounds' worth of levers already tried and confirmed
negative (five variants in round 26, the `blez` fix and frame
investigation in round 35, algebraic rewrite and a round-37 permuter
search at 76056 iterations), and this round's budget shared across seven
other queued functions, no new lever was attempted here. Restored to
`INCLUDE_ASM` unchanged.

## Naming (round 75, runner alpha, FINISHING-PLAN track 3)

Already carries its real name: identified round 74 (track 2, runner bravo)
as `libsnd/vmanager SpuVmDoAllocate` (shape 0.99 vs the disc-3.3 reference,
141w reference vs our 143w, position within the libsnd neighborhood). Sony
symbol; this pass does not rename it further. The function remains a STALL
(1 word short, see above).

## Track 2 (round 86, 2026-09-26, alpha)

This function is still `INCLUDE_ASM` and its C was not touched, but the per-field symbols this report uses (`D_8008D988`..`D_8008D9BA` at a 0x34 stride) are ONE Sony table: libsnd/vmanager.o's `_svm_voice` (0x8008D988, 24 x 0x34 = 0x4E0 bytes), typed in `include/SvmData.h` with fields by offset (`D_8008D98C` is `_svm_voice[i].unk04`, `D_8008D9A3` is `unk1B`, and so on: address minus 0x8008D988). The next attempt should write `_svm_voice[i].unkNN`: in every converted accessor (libsnd_vm_vol_ut_key_ut_keyv/j_c/l/m/p) the struct spelling compiled byte-identically to the separate symbols, and two NON_MATCHING bodies moved closer to retail. The other `D_` spellings in preserved bodies below still link (splat keeps them as auto-symbols).

## Types (round 98, alpha)

This unit's `D8008E978Entry` is now `<libsnd.h>`'s `VagAtr` and `ObjE970` is `VabHdr` (unk4 -> `center`, unk5 -> `shift`, unk12 -> `pbmin`, unk13 -> `pbmax`, ObjE970.unk18 -> `mvol`); preserved bodies above keep the old spellings. See SpuVmAlloc.md, "Unit banner history".
