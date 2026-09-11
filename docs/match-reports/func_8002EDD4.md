# func_8002EDD4 -- STALL: EXACT length (270/270 built words), 267/270 raw word-match, first real diff at file 0x1F69C / vram 0x8002EE9C -- pure REGISTER-IDENTITY residue (a0 vs s0), same instructions, banned to fix by pinning

Unit: `src/code_179d8_m.c`. Round 26 (second pass), runner bravo, incorporating the
HEAD's diagnosis of the "split scaled index" residue (see below).

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/code_179d8_m/func_8002EDD4.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'  -> no hits
```

## Result

`./build-and-verify.sh` GREEN with `INCLUDE_ASM` restored. This round closed
the gap from the first pass's stall (269/270, one word short, wrong shape in
the D_8006DAD4 store block) all the way to **267/270 raw words matching, at
EXACT retail length** — the three remaining differing words are the SAME
three instructions with ONE register swapped (`a0` in retail, `s0` in this
attempt), not a different instruction, count, or control-flow shape. Per
CLAUDE.md's own test ("if removing it changes WHICH REGISTER holds a value,
it is banned"), this is a genuine register-identity residue and not
something to keep reshaping past this point.

## Round 30 (charlie) update: rebuilt, confirmed accurate, permuter run

Re-spliced this exact preserved body into the live unit (using local
typedef renames to route around unrelated duplicate-declaration conflicts
from OTHER still-`INCLUDE_ASM` functions sharing this file, purely a
verification-harness artifact, not a change to the reported C) and rebuilt
from scratch. **All three title figures reconfirmed independently:**
`funcdiff.py` reports exactly **267/270 words match, file 0x1F5D4-0x1FA0C**
(270 words = retail's own length, no drift warning), and
`tools/asm-differ/diff.py` confirms the residue is precisely the three
instructions this report already names (`andi a0,s0,0xff` / `sltiu
v0,a0,0x18` at file offset 0x1F69C, plus the `sb a0,...` store later in the
same block) with `s0` in place of retail's `a0` and nothing else differing
— a clean register-identity-only residue, exactly as classified above.

**One correction found during the rebuild, cosmetic only (does not affect
the score): the preserved body's two `SPU` calls used STALE placeholder
names.** `func_80039228(0)` and `func_80039104(0x20, D_8008DEB0)` were
written when these two library calls had no symbol yet; `asm/nonmatchings/
code_179d8_m/func_8002EDD4.s` itself now names them `_spu_setInTransfer`
and `SpuInitMalloc` (`config/symbols.slps01556.lsdde.txt` lines 287/291,
Psy-Q `libspu`, from the SDK-object-linking work in later rounds). The
preserved body below has been updated to the current names; nothing about
the residue or the score changes.

**Permuter search launched per this round's assignment** (this is the
round's designated permuter target, the closest large near-miss in the
corpus): `tools/setup-permuter.sh func_8002EDD4 <seed>` with the corrected
seed (SDK symbol names fixed, all needed struct/extern declarations
inlined), sanity-checked with `--debug --stack-diffs` first — base score
**15** (3 register differences × 5, zero insertions/deletions/reorderings/
stack/branch differences), matching this report's own classification
exactly. Real search: `permuter.py -j 8 --stack-diffs --stop-on-zero
--best-only`, wrapped in `timeout 1500` (25 minutes), run in the
background while hand work continued on the other five functions. See the
"Permuter result" section at the end of this report for the outcome
(iteration count and exit code recorded there once the bound is reached).

## THE SPLIT-INDEX FIX (HEAD-DIAGNOSED, CONFIRMED CORRECT)

The HEAD identified the root cause of this round's original 1-word
shortfall: retail's early `sll t0,v1,3` computes `idx*8`, and the "missing"
`andi $?,$?,0xffff` mid-block is a mask **on that product**, not on the
index — the earlier attempt's `array[i]` struct-cast indexing folds `idx*16`
into a single `sll #4` and never produces either instruction.

**The fix, applied exactly as prescribed:** declare `u16 woff;` at the loop's
declaration block, assign `woff = (u16) i * 8;` as the loop body's FIRST
statement, and index the six `D_8006DAD4` fields as a flat `s16 *`:

```c
woff = (u16) i * 8;
... (twenty 0x34-stride record zero/init stores, UNCHANGED from before) ...
((s16 *) D_8006DAD4)[woff + 3] = 0x200;   /* +0x6 */
((s16 *) D_8006DAD4)[woff + 2] = 0x1000;  /* +0x4 */
((u16 *) D_8006DAD4)[woff + 4] = 0x80FF;  /* +0x8 -- see note below */
((s16 *) D_8006DAD4)[woff + 0] = 0;       /* +0x0 */
((s16 *) D_8006DAD4)[woff + 1] = 0;       /* +0x2 */
((s16 *) D_8006DAD4)[woff + 5] = 0x4000;  /* +0xA */
```

This reproduced retail's early `sll #3`, the mid-block `andi ...,0xffff` on
the product, and the LATE base-pointer load, **instruction for instruction**
— confirmed via `asm-differ`, the whole 0x34-stride-chain-and-record-init
block through this section now shows zero content differences (only the
downstream register-identity residue noted below). This is the single
biggest structural fix of the round and should be treated as the
established idiom for this record family's D_8006DAD4-array writes
whenever they occur inside a LOOP (see the caveat under
`func_8002E4D8`'s report on the non-loop case, which behaves differently).

**One sub-finding not in the head's message:** the `+0x8` field
(`0x80FF`) needed a `u16 *` cast rather than `s16 *` at its specific store
site — `0x80FF` as a signed 16-bit immediate is `-0x7F01`, and GCC emits
`addiu` (sign-extended immediate load) for a destination typed `s16`, but
`ori` (zero-extended) for `u16` — retail uses `ori`. Same low 16 bits
either way (irrelevant for a halfword store), but a different OPCODE, so a
literal byte-level mismatch. Whenever a halfword literal's top bit is set
(i.e. it reads negative as `s16`), check whether retail uses `ori` or
`addiu`/`li`-as-negative and match the destination's signedness
accordingly — this is now confirmed as a real, byte-visible instruction-
selection lever, not a cosmetic one.

## A second real bug, found and fixed along the way: a redundant outer guard

The unit's very first pass on this function additionally carried a
DUPLICATE branch: an outer `if (D_8008E9D0 != 0) { for (i = 0; i <
D_8008E9D0; i++) ... }` compiles that redundant guard as a SEPARATE `beqz`
immediately followed by the `for` loop's own entry check on the identical
condition — two branches testing the same thing, back to back, where
retail has only one. **Removing the outer `if` and keeping only the `for`
loop's own bound check** eliminates the duplicate `nop`+`beqz` pair (2
words) outright. This is a general lesson: `if (N != 0) { for (i=0; i<N;
...) }` is NOT free in this compiler even though the `if` is logically
redundant with the loop's own zero-trip case — always drop it and let the
loop's own comparison serve as the guard.

## The residual: a register-identity-only mismatch, three words, correct-length function

After both fixes above, exactly one construct remains wrong — and only in
which PHYSICAL REGISTER holds a value, not in shape, count, or control
flow. The MIN clamp (`D_8008E9D0 = min((u8) a0, 0x18)`):

```
retail:  andi a0,s0,0xff / sltiu v0,a0,0x18 / bnez v0,L(store a0) / ...
         ori v0,zero,0x18 / lui at / sb v0,D_8008E9D0(at) / j L2 / nop
       L: lui at / sb a0,D_8008E9D0(at)
       L2: [reload for the loop-guard check]

mine:    andi s0,s0,0xff / sltiu v0,s0,0x18 / bnez v0,L(store s0) / ...
         [same 8 remaining instructions, "s0" everywhere retail has "a0"]
```

Getting the CONTROL FLOW and BRANCH POLARITY here byte-correct took real
work (documented in axes below); once correct, the only remaining
difference is that retail's register allocator puts the masked
min-of-two-values into `$a0`, while every attempt this round puts it into
`$s0` (the register the parameter is saved into across the two early
`jal`s). $s0 is confirmed genuinely DEAD after this point in retail
(`grep '\$s0'` on the `.s` file shows no reads between this instruction and
the epilogue's stack-restore `lw`), so retail's choice to move the value
into `a0` instead of updating `s0` in place is a pure allocator preference,
not forced by any later use. **This is the banned-to-fix class**: CLAUDE.md
explicitly prohibits `register T v asm("$N")` and extended-asm constraints
for exactly this situation, and eight independent C reshapes (below) did
not shift the allocator's choice.

### Axes tried for the min-clamp construct, in order

1. **Bare ternary** `D_8008E9D0 = ((u8)a0<0x18) ? (u8)a0 : 0x18;` — confirmed
   via an ISOLATED reproducer through the pinned pipeline (not just in the
   full function) that this ALWAYS lowers to a "store speculatively, then
   fix up if wrong" shape (unconditional store, reload, compare,
   conditional overwrite) — structurally nothing like retail's single-store
   branch-first shape, regardless of surrounding context. **Ternaries and
   if/else statements get genuinely different lowering strategies in this
   compiler for a min/clamp pattern; do not treat them as interchangeable
   spellings of the same logic.**
2. **`if (count<0x18) store count; else store 0x18;`** (natural order,
   named local): isolated test confirms this gives retail's INSTRUCTION SET
   but the WRONG bit BLOCK ORDER (then-clause first/fallthrough, else via
   jump) — the opposite of retail's actual layout.
3. **Swapped order: `if (count>=0x18) store 0x18; else store count;`**
   (named local): this is the one that reproduces retail's branch shape
   exactly in isolation — GCC internally re-derives the `sltiu`/`bnez` test
   from the ORIGINAL sub-condition regardless of which way the source
   states it, but respects which BLOCK is textually first for fallthrough
   placement. **The "give the else-arm the textually-first position" lesson
   from DECOMPILATION_LEARNINGS's block-order entry generalizes to a plain
   two-way `if`/`else` on a comparison, not just to loop-body arms.**
4. Combined with a named `u8 count` local: introduced an extra `move a0,s0`
   not present in the isolated test — apparently triggered by REGISTER
   PRESSURE specific to the full function (this exact reproducer step, in
   isolation with two preceding calls forcing `s0`, did NOT show the extra
   move; only the full function's additional live ranges did).
5. **Reassigning the parameter itself** (`a0 = (u8) a0;` as its own
   statement, then bare `a0` in both branches) removed the extra `move`
   confirmed via an isolated reproducer with two preceding calls forcing
   `s0` — this matches an isolated single-branch test exactly (`andi
   s0,s0,0xff` in place, no separate move). Applied to the real function:
   removed the extra move, but the reassignment naturally updates the
   parameter's EXISTING home register (`s0`), which is exactly why the
   REMAINING residue is register identity rather than an extra instruction.
6. **`(u32)` cast in the comparison** (`if ((u32) a0 >= 0x18)`) was
   necessary on top of #5 to get `sltiu` (unsigned) rather than `slti`
   (signed) — confirmed via isolated reproducer and then the real build;
   without it the comparison is silently signed, a real (if narrower)
   correctness gap on top of the block-order one.
7. **Named local + `(u32)` cast + reload-based derivation**: regressed to
   271 words (1 too long) — reproduced the extra `move a0,s0` bug from #4
   even with the `u32` cast present, confirming the extra move is
   independent of the signedness fix and tied specifically to using a
   SEPARATE named local rather than reassigning the parameter.
8. **`a0 = a0 & 0xFF;`** in place of `a0 = (u8) a0;`: byte-identical output
   to #5/#6 combined — confirmed the cast and the explicit mask are
   equivalent here, ruling out the CAST SYNTAX itself as a lever.
9. **Moving the `a0 = (u8) a0;` reassignment earlier** (right after the
   `func_80039104` call, before the three zero-fill loops rather than
   immediately before its own `if`): regressed sharply (231/270 words
   match, with cascading drift into the zero-fill loops themselves) —
   textual position of this reassignment matters a great deal and earlier
   is not simply "safer"; the working position is immediately before the
   comparison, not hoisted.
10. **Bare `__asm__("")` scheduling barriers** at two different points
    around the reassignment: no effect on the register choice in either
    placement (confirmed it does not fix this — recorded so the next
    attempt does not re-spend a try here).

Combination #6 (`a0 = (u8) a0; if ((u32) a0 >= 0x18) { D_8008E9D0 = 0x18; }
else { D_8008E9D0 = a0; }`) is the best reached: EXACT 270-word length,
267/270 raw match, and the residual is purely which register (`a0` vs
`s0`) the allocator chose for a value that is otherwise computed, compared,
and stored identically to retail.

### Proposed learnings

1. **`u8 min = a<b ? a : b;` and the equivalent `if`/`else` are NOT
   interchangeable spellings in GCC 2.6.3 — they get fundamentally
   different lowering strategies** (speculative-store-then-fixup for the
   ternary; single-branch-single-store for if/else). When retail's
   disassembly shows a single conditional store with no reload, write an
   `if`/`else`, never a ternary, regardless of which reads more naturally.
2. **For a two-way `if`/`else` on a value comparison, the textually-FIRST
   branch becomes the compiled fallthrough, and GCC will re-derive whichever
   comparison direction it prefers internally** — so match retail's BLOCK
   ORDER (which value is stored via the jump target vs. the fallthrough),
   not retail's literal comparison operator; write whichever `if` condition
   puts the right body first, and let the compiler pick `slt`/`sltu`
   sense on its own.
3. **When a parameter is reused as a scratch temp after being narrowed,
   reassigning the parameter itself (`a0 = (u8) a0;`) is measurably
   different from introducing a same-purpose named local**, even though
   they are semantically identical — the named-local form cost an extra
   `move` in this exact context, twice, on two different tries. Prefer
   reassigning the parameter when the original wide value is never needed
   again.
4. **The `woff`-halfword-index idiom for a small-struct array proven inside
   a loop (this function's D_8006DAD4 fix, confirmed byte-exact) does NOT
   transfer cleanly to the equivalent access OUTSIDE a loop** — see
   `func_8002E4D8`'s report, attempted this same round with the identical
   idiom against a single (non-looping) call site, which reduced but did
   not eliminate its residue and surfaced different register-pressure
   side effects. Loop vs. non-loop context is a real discriminator for
   this idiom, not a detail. **Round 30 adds a THIRD variant**:
   `func_8002EA44.md`'s round-30 update found the idiom REGRESSES a third
   sibling because retail derives the shift from an ALREADY-sign-extended
   working value shared with a neighbouring computation, not from a fresh
   copy of the raw parameter — so "top of function, `s16`, mask deferred to
   use" is not itself the complete recipe; where the shift's INPUT value
   comes from (raw parameter vs. an existing sign-extended temp already
   live at that point) is a per-function fact to check against the
   disassembly, not a constant of the idiom.
5. **A preserved body's `jal` targets can go stale when they were written
   before an SDK-object round gave the target a real symbol.** This
   report's own `func_80039228`/`func_80039104` (round 26) and
   `func_8002F700.md`'s `func_800375E8` (round 26) were both placeholder
   `func_ADDRESS` names at write time; `asm/nonmatchings/.../<func>.s`
   itself now names them `_spu_setInTransfer`/`SpuInitMalloc`/
   `SpuSetNoiseVoice` per `config/symbols.slps01556.lsdde.txt`. The score
   is unaffected either way (same bytes, same call), but the STALE name
   compiles to an `undefined reference` link error the moment the body is
   actually spliced back in for re-verification — which looks exactly like
   a genuine regression until you check whether the current `.s` disassembly
   names the call differently than the report does. Cheap check before
   trusting any "undefined reference" from a re-spliced preserved body:
   `grep 'jal' asm/nonmatchings/<unit>/<func>.s` and compare names against
   the extern declarations the report's own C uses.

## Preserved body (best attempt, 267/270 raw words match at EXACT 270-word length -- residue is register-identity only, banned to fix further)

```c
#if 0
extern void _spu_setInTransfer(s32 a0);
extern void SpuInitMalloc(s32 a0, void *a1);
extern void func_8002F700(void);

extern u8 D_8008DEB0[];
extern s16 D_8008E9FC;
extern s16 D_8008E84C;
extern s16 D_8008E260;
extern s16 D_8008E262;
extern s16 D_8008E230;
extern s16 D_8008E234;
extern s32 D_8008E258;
extern s32 D_8008E25C;
extern u8 D_8008EA40;
extern s16 D_8008E938;

extern Rec34Half D_8008D98A[]; /* value forced to 0x18 at init */

typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34S16;
extern Rec34S16 D_8008D988[];
extern Rec34S16 D_8008D996[];
extern Rec34S16 D_8008D99A[];

extern Rec34Half D_8008D990[];
extern Rec34Half D_8008D98C[];
extern Rec34Half D_8008D98E[];
extern Rec34Half D_8008D998[];
extern Rec34Half D_8008D9A6[];
extern Rec34Half D_8008D9A8[];
extern Rec34Half D_8008D9AA[];
extern Rec34Half D_8008D9AC[];

typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D99C[];

typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D992[]; /* byte field, forced to 0x40 at init */
extern Rec34Byte D_8008D9A3[];
extern Rec34Half D_8008D9A4[];

extern volatile u16 D_8008EA26;
extern u8 D_8008E9D0;
extern s16 D_80090BD0;
extern u8 D_8008EA2C[];
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

typedef struct {
    u8 pad[0x194];
    u16 unk194; /* +0x194 */
    u16 unk196; /* +0x196 */
} ObjDAD4;
extern ObjDAD4 *D_8006DAD4;

void func_8002EDD4(s32 a0) {
    s16 i;

    _spu_setInTransfer(0);
    D_8008E9FC = 0;
    D_8008E84C = 0;
    SpuInitMalloc(0x20, D_8008DEB0);

    for (i = 0; (u16) i < 0xC0; i++) {
        ((u16 *) D_8008D7F0)[(u16) i] = 0;
    }

    for (i = 0; (u16) i < 0x18; i++) {
        D_8008D970[(u16) i] = 0;
    }

    D_80090BD0 = 0;

    for (i = 0; (u16) i < 0x10; i++) {
        D_8008EA2C[(u16) i] = 0;
    }

    a0 = (u8) a0;
    if ((u32) a0 >= 0x18) {
        D_8008E9D0 = 0x18;
    } else {
        D_8008E9D0 = a0;
    }

    for (i = 0; (u16) i < D_8008E9D0; i++) {
        u16 woff;
        u16 chan;
        u16 lowMask;
        u16 highMask;

        /* Halfword-indexed byte-offset for the six D_8006DAD4 fields
         * below: idx*8 computed first (matches retail's very first
         * instruction of the loop body, an early `sll #3` well before
         * the 0x34-stride chain even starts), then indexed as `woff+N`
         * on an `s16 *` (which scales by 2), reaching idx*16 -- the
         * actual per-channel byte stride -- only at the point of use,
         * matching the mid-block `andi ...,0xffff` mask retail's
         * disassembly shows on the PRODUCT and the LATE base-pointer
         * load. This idiom, confirmed byte-exact for this loop, does
         * NOT transfer cleanly to the equivalent non-loop access in
         * func_8002E4D8 -- see that function's report. */
        woff = (u16) i * 8;

        D_8008D98A[(u16) i].unk0 = 0x18;
        D_8008D996[(u16) i].unk0 = -1;
        D_8008D988[(u16) i].unk0 = 0xFF;
        D_8008D9A3[(u16) i].unk0 = 0;
        D_8008D98C[(u16) i].unk0 = 0;
        D_8008D98E[(u16) i].unk0 = 0;
        D_8008D998[(u16) i].unk0 = 0;
        D_8008D99A[(u16) i].unk0 = 0;
        D_8008D99C[(u16) i].unk0 = 0xFF;
        D_8008D990[(u16) i].unk0 = 0;
        D_8008D992[(u16) i].unk0 = 0x40;
        D_8008D9A4[(u16) i].unk0 = 0;
        D_8008D9A6[(u16) i].unk0 = 0;
        D_8008D9A8[(u16) i].unk0 = 0;
        D_8008D9AA[(u16) i].unk0 = 0;
        D_8008D9B0[(u16) i].unk0 = 0;
        D_8008D9B2[(u16) i].unk0 = 0;
        D_8008D9B4[(u16) i].unk0 = 0;
        D_8008D9B6[(u16) i].unk0 = 0;
        D_8008D9B8[(u16) i].unk0 = 0;
        D_8008D9AC[(u16) i].unk0 = 0;

        ((s16 *) D_8006DAD4)[woff + 3] = 0x200;   /* +0x6 */
        ((s16 *) D_8006DAD4)[woff + 2] = 0x1000;  /* +0x4 */
        ((u16 *) D_8006DAD4)[woff + 4] = 0x80FF;  /* +0x8 -- `u16` needed for `ori` vs `addiu` */
        ((s16 *) D_8006DAD4)[woff + 0] = 0;       /* +0x0 */
        ((s16 *) D_8006DAD4)[woff + 1] = 0;       /* +0x2 */
        ((s16 *) D_8006DAD4)[woff + 5] = 0x4000;  /* +0xA */

        /* Scheduling barrier: load-bearing. Without it, GCC hoists the
         * D_8008EA26 write+re-read ABOVE the six D_8006DAD4 stores
         * (still correct VALUE-wise, but 1 word shorter than retail,
         * which keeps this AFTER the stores). Confirmed by direct
         * removal test: 269/270 words without the barrier, 270/270
         * with it. Only changes ORDER, never register identity --
         * permitted per CLAUDE.md's bare-__asm__ exception. */
        __asm__("");
        D_8008EA26 = i;
        chan = D_8008EA26;
        if (chan < 0x10) {
            lowMask = 1 << chan;
            highMask = 0;
        } else {
            lowMask = 0;
            highMask = 1 << (chan - 0x10);
        }

        D_8008D9A3[chan].unk0 = 0;
        D_8008D98C[chan].unk0 = 0;
        D_8008D988[chan].unk0 = 0;
        D_80090C60 |= lowMask;
        D_80090C64 |= highMask;
        D_8008E228 &= ~D_80090C60;
        D_8008E22C &= ~D_80090C64;
    }

    D_8008E260 = 0x3FFF;
    D_8008E262 = 0x3FFF;
    D_8008E228 = 0;
    D_8008E22C = 0;
    D_80090C60 = 0;
    D_8008E230 = 0;
    D_8008E234 = 0;
    D_8008E258 = 0;
    D_8008E25C = 0;
    D_8008EA40 = 0;
    D_8008E8C0 = 0;
    D_8008E938 = 0x80;
    func_8002F700();
}
#endif
```

## Permuter result (round 30, charlie)

**Not closed in 100,458 iterations under load.** `-j 8 --stack-diffs
--stop-on-zero --best-only`, run for the full `timeout 1500` (25 minute)
bound alongside four other live runners' own searches in sibling
worktrees (confirmed via `pgrep -f 'decomp-permuter|permuter.py'` plus a
`readlink /proc/<pid>/cwd` check on every match, both before launch and
after collection — this round had concurrent activity in
`lsddecomp2-wt-echo` and `lsddecomp2-wt-bravo`, neither of which this
search touched or was touched by). The best score seen across the entire
run stayed at the sanity-checked base of **15** (3 register differences,
zero insertions/deletions/reorderings) — never dropped, let alone reached
zero — consistent with this being a genuine allocator-preference residue
rather than a source-shape gap the permuter's mutation set can reach.

**Exit accounting:** the process (PID 994862, the `timeout` wrapper) was
confirmed gone from the process table after the bound elapsed, with zero
survivors under this worktree's path (`lsddecomp2-wt-charlie`) checked by
`cwd`, not by name — the only two other live permuter processes found
system-wide both resolved to different worktrees (`-wt-echo`, `-wt-bravo`)
entirely unrelated to this search. The launching shell had already moved
on to other work by the time the bound elapsed, so the wrapper's literal
numeric exit status was not captured directly; circumstantial evidence
(elapsed time matching the 1500s bound exactly, a `resource_tracker`
"leaked semaphore" warning at the tail of the log — the signature of an
external SIGTERM cutting the multiprocessing pool rather than a clean
`--stop-on-zero` exit, and iterations still actively climbing at the
final logged line with no zero ever printed) is consistent with the
`timeout` bound firing (the 124 case), not with an external kill or a
found zero. Per this round's own instruction to phrase every negative as
"not closed in N iterations under load" rather than upgrading it to
*permuter-exhausted* — this residue remains open for a future attempt,
ideally on an idle box where a much larger iteration budget is cheap,
though the flat, unmoving best-score-15 trace across 100k+ iterations here
is itself weak evidence the mutation space this permuter explores does not
reach retail's specific allocator choice for this construct.

**Translated to a verdict:** this residue is unlikely to be reachable by
permuter search or further hand reshaping under CLAUDE.md's own banned-fix
rule (`register T v asm("$N")` / operand constraints), and the function
should be considered a genuine, durable STALL at 267/270 rather than a
live search target for a future round, absent a new idea about what
actually determines retail's `a0`-vs-`s0` choice here.
