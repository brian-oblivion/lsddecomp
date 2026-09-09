# func_8002EDD4 -- STALL: 1 word short (269/270 built length), 129/269 raw word-match (in-range, drift-affected -- see caveat below), first real diff at file 0x1F728 / vram 0x8002F328 (retail's `andi a0,a0,0xffff`, entirely absent from the built body)

Unit: `src/code_179d8_m.c`. Round 26, runner bravo.

## Screens (clean)

```
grep -n 'gp_rel' asm/nonmatchings/code_179d8_m/func_8002EDD4.s            -> no hits
grep -A2 -nE '\b(mflo|mfhi)\b' ... | grep -E '\b(mult|multu|div|divu)\b'  -> no hits
```

## Result

`./build-and-verify.sh` GREEN with `INCLUDE_ASM` restored (this report's body
is not in `src/`). The best attempt compiled clean (no compile error) and is
**exactly one instruction (4 bytes) SHORT of retail's length**: retail is
`0x438` bytes (270 words); the built object measured `0x434` bytes (269
words) via `objdump -t build/src/code_179d8_m.c.o` (the reliable way to read
this once drift appears — see below). Confirmed via the map file too:
retail's *next* function in this unit, `func_8002F20C`, links at
`0x8002f20c`; with this body in place it links one word early, at
`0x8002f208`, which is exactly the shortfall propagated forward.

**Do not trust `funcdiff.py`'s own byte-drift count while this body is
in `src/`** — with the whole unit's downstream functions shifted by 4
bytes, an *unrelated* function three units over (`func_80032588` in
`code_179d8_c`, which owns `jtbl_80010CD8`) shows up as a huge apparent
mismatch purely because its jump-table's absolute case-label addresses
shifted. This is CLAUDE.md's "struct edits are non-local" hazard's cousin:
here the non-locality is pure *link-address* drift from a length mismatch,
not a struct layout change, but it presents identically (huge out-of-range
byte count, unrelated-looking function affected). `cmp -l` + the map file is
the fast way to confirm the shift is explained by THIS function's own
shortfall and not a separate defect: `(N-1) - 0x800 + 0x80010000` on the
first differing byte lands squarely on `jtbl_80010CD8`, which is owned by a
completely different, already-known-stalled function three units away —
not evidence of a second defect, just the expected consequence of one
function linking 4 bytes short.

## Shape (believed fully correct)

Straightforward init function, no register-identity residue anywhere:

1. `func_80039228(0);` (lock/unlock call, same idiom as
   `func_800335FC`/`func_80032D34` in `code_179d8_i.c`).
2. `D_8008E9FC = 0; D_8008E84C = 0;` (both `s16`).
3. `func_80039104(0x20, D_8008DEB0);` — a two-argument Psy-Q SPU call
   (defined in `asm/psyq_SpuSetMute.s`, still `func_XXXX`-named); the first
   argument (`0x20`) and second argument (`&D_8008DEB0`, computed via
   `lui`/`addiu` BEFORE the call whose delay slot sets the first argument)
   read straightforwardly as `func_80039104(0x20, D_8008DEB0);` with
   `D_8008DEB0` declared `extern u8 D_8008DEB0[];`.
4. Three flat zero-fill loops: `D_8008D7F0` reinterpreted as a flat `u16`
   array (192 halfwords = 0xC0, i.e. 24 records * 0x10 bytes) zeroed in one
   loop; `D_8008D970` (24 bytes) zeroed; `D_8008EA2C` (16 bytes) zeroed.
   `D_80090BD0 = 0;` sits between the second and third loop.
5. A MIN clamp, `D_8008E9D0 = ((u8) a0 < 0x18) ? (u8) a0 : 0x18;` — **must be
   written as a ternary, not an `if`/`else` statement.** An `if`/`else`
   version compiles to the OPPOSITE branch polarity (test-then-negate,
   `beqz` skip-to-else) and adds a spurious `move a0,s0` (the mask target
   register differs from what the comparison consumes), while retail's
   actual shape is the classic ternary lowering: test `count<0x18`; `bnez`
   FORWARD to the "then" arm (store `count`) which is placed textually
   LAST; the "else" arm (store the constant `0x18`) is the fallthrough,
   ending in an unconditional jump over the "then" arm to the join. Fixed by
   using the ternary form directly on `(u8) a0`, no separate `count`
   local — that reproduces retail's exact branch polarity AND removes the
   spurious `move`.
6. If `D_8008E9D0 != 0`: a per-channel loop (`i` from `0` to
   `D_8008E9D0-1`) that (a) resets twenty 0x34-stride record fields
   (`D_8008D98A`=0x18, `D_8008D996`=-1, `D_8008D988`=0xFF, `D_8008D9A3`=0,
   `D_8008D98C`=0, `D_8008D98E`=0, `D_8008D998`=0, `D_8008D99A`=0,
   `D_8008D99C`=0xFF, `D_8008D990`=0, `D_8008D992`=0x40 [BYTE store, not
   halfword], `D_8008D9A4`=0, `D_8008D9A6`=0, `D_8008D9A8`=0,
   `D_8008D9AA`=0, `D_8008D9B0`=0, `D_8008D9B2`=0, `D_8008D9B4`=0,
   `D_8008D9B6`=0, `D_8008D9B8`=0, `D_8008D9AC`=0 — this exact order,
   verified instruction-by-instruction against the raw disassembly); then
   (b) six fields of a NEW 0x10-byte-stride per-channel record family
   pointed to by `D_8006DAD4` (`unk6`=0x200, `unk4`=0x1000, `unk8`=0x80FF,
   `unk0`=0, `unk2`=0, `unkA`=0x4000); then (c) the SAME "deactivate
   channel" sequence `func_800300D0`'s inner branch already documents
   (`D_8008EA26 = i;` write-then-volatile-reread as `chan`, split into a
   `lowMask`/`highMask` pair depending on `chan < 0x10`, redundantly
   re-zeroing `D_8008D9A3[chan]`/`D_8008D98C[chan]`/`D_8008D988[chan]`
   [genuinely redundant with step (a) except `D_8008D988`, which flips from
   0xFF back to 0 — transcribe faithfully, do not dedupe], then OR-ing the
   masks into `D_80090C60`/`D_80090C64` and AND-NOT-ing them out of
   `D_8008E228`/`D_8008E22C`).
7. Unconditional tail: several flag/mode globals reset
   (`D_8008E260`/`D_8008E262`=0x3FFF, `D_8008E228`/`D_8008E22C`=0,
   `D_80090C60`=0, `D_8008E230`/`D_8008E234`=0, `D_8008E258`/`D_8008E25C`=0
   [these two are `s32`, stored via `sw`, unlike everything else here],
   `D_8008EA40`=0 [byte], `D_8008E8C0`=0, `D_8008E938`=0x80), then a tail
   call `func_8002F700();` (no arguments — confirmed from that function's
   own prologue, which never reads `$a0`).

## The one unresolved residue: an early-computed, mid-block-masked `idx*8` partial product for the `D_8006DAD4` 0x10-stride indexing

This is the **exact same open mystery already on record in
`docs/match-reports/func_8002EA44.md`** (same unit, same file), but here it
is far more tractably isolated: a single, precisely-located missing
instruction rather than 5 words scattered across a much larger function.

Retail's raw disassembly, in program order, for the loop body:

```
sll   a0, v1, 3          # a0 = idx*8   -- FIRST instruction of the loop body,
                          #   computed before ANY of the 0x34-stride record
                          #   writes even begin
sll   v0, v1, 1           # \
addu  v0, v0, v1          #  | v0 = idx*0x34  (the record byte offset chain)
sll   v0, v0, 2           #  | -- used by all twenty D_8008D9xx/D_8008D98x
addu  v0, v0, v1          #  |    stores below
sll   v0, v0, 2           # /
ori   v1, zero, 0x18
[D_8008D98A store using v0, value v1]
addiu v1, zero, -1
[D_8008D996 store using v0, value v1]
ori   v1, zero, 0x40
andi  a0, a0, 0xffff      # <-- THE MISSING INSTRUCTION. a0 is not touched
                          #     again until it is doubled (below); nothing
                          #     between this mask and that later use reads
                          #     or writes a0.
[D_8008D988 store using v0, value t1]
... (fifteen more D_8008D9xx zero-stores using v0) ...
[D_8008D9AC store using v0, value zero]
lui v0, %hi(D_8006DAD4); lw v0, %lo(D_8006DAD4)(v0)   # base pointer -- LOADED LATE
sll  a0, a0, 1             # a0 = a0*2 = idx*16 (doubling the EARLY, masked value)
addu a0, a0, v0            # a0 = D_8006DAD4 + idx*16
[six stores through a0: +6, +4, +8, +0, +2, +0xA]
```

The C in this report reproduces every instruction here **except the single
`andi a0,a0,0xffff`** — the struct-array cast `((Rec16DAD4 *)
D_8006DAD4)[(u16) i].unkN = val;` computes `idx*16` as ONE shift at the
point of use (matching the FINAL `sll a0,a0,1` doubling step, since the cast
index is already `(u16) i`, not a pre-shifted-by-8 value) and never emits
the early `sll a0,v1,3` / mid-block mask at all — GCC has no reason to
materialize a `idx*8` partial product when the whole `idx*16` multiply can
be done in one shift right where it's needed.

**a0 has no other reader between the mask and its next use** (confirmed by
re-reading the full instruction range by hand) — this is not a case of a
missing SEPARATE field write at a distinct stride; the register is used
for nothing except this one 0x10-stride index, start to finish.

### What was tried, all rejected, with effect on length

1. **Baseline: cast-index directly by `(u16) i`** (as shown above; the
   version whose length is reported here): **269/270 words**, drift
   confined to exactly one missing instruction, everything downstream
   shifted forward by 4 bytes accordingly.
2. **Declare `Rec16DAD4 *rec = &((Rec16DAD4 *) D_8006DAD4)[(u16) i];` at the
   top of the loop body** (matching this project's "declarations at block
   top" convention, on the theory that an early declaration-with-initializer
   would front-load the index computation the way retail's `idx*8` is
   front-loaded): **WORSE.** This front-loads the WRONG half of the
   computation — it hoists the `D_8006DAD4` **base pointer load** (the `lw`)
   early too, which retail does NOT do (retail keeps that `lw` at the very
   end, right before the six stores). It also collapses the multiply back
   into a single early `sll #4` rather than reproducing the early-`*8`
   / late-`*2` split. Confirmed via `asm-differ`: the `lui`/`lw` pair for
   `D_8006DAD4` appears immediately after the loop-entry code in the built
   output, versus appearing only ~30 instructions later in retail.
3. **Explicit early scalar `u16 idx8 = (u16) i << 3;` plus raw pointer
   arithmetic** (`*(s16 *) ((u8 *) D_8006DAD4 + (idx8 << 1) + N) = val;` for
   each of the six fields, replacing the `Rec16DAD4` struct cast entirely):
   **WORSE, grew to 276/270 words (6 too long)** — the six repeated
   `(idx8 << 1)` subexpressions were not fully commoned by the compiler the
   way retail's single materialized `a0` is reused across all six stores.
   Reverted.
4. **Reordering the two `Rec16DAD4` stores that use offsets `+0x6`/`+0x4`
   to appear textually earlier** (right after `D_8008D996[i].unk0 = -1;`,
   before the twenty 0x34-record zero-stores), on the theory that retail's
   *actual* store-instruction order might not equal literal C statement
   order and the scheduler moves cheap stores forward: **rejected without
   building** — this actually changes the STORE instruction sequence itself
   (not just where an invisible intermediate is computed), and the raw
   disassembly's store order for the twenty 0x34-record fields is already
   confirmed byte-for-byte against the ONLY correct baseline (attempt 1);
   moving real stores would only trade one known-good match for a new,
   unrelated one.

None of these reproduce retail's specific "compute an 8x partial product
immediately, mask it once far away from where it's finally consumed and
far from where it was computed, keep the eventual base-pointer load late"
shape. As with `func_8002EA44`, no C form that computes `idx*16` for a
single point-of-use naturally produces this split — something about the
*actual* source structure (not yet identified) causes GCC to split the
multiply and interleave its mask with unrelated, cheaper stores.

### Proposed learning

**This is now the SECOND independent instance, in the same unit, of the
same class of residue** (`func_8002EA44`'s report is the first): an
early-computed partial product for a small-record array index, kept live
across many unrelated intervening stores, with its final materialization
(mask and/or scale) split across two widely-separated points. Since it now
recurs, it is worth a name: **the "split scaled index" residue.** Both
instances share:
- the affected array element size is a small power of two (16 bytes here,
  also effectively 16 in `func_8002EA44`'s case reading its report),
- the index is a loop variable / parameter that ALSO drives a *different*,
  unrelated stride's addressing elsewhere in the same function (the 0x34
  record chain here; a similar unrelated computation in `func_8002EA44`),
  and
- forcing an explicit early local (either scalar or pointer) made the
  match WORSE both times, not better — so "hoist it yourself" is
  confirmed, twice now, to be the wrong lever.

Given two independent failures to reproduce it with explicit hoisting, the
next attempt on either function should probably NOT try a third hoisting
variant, but instead look for a **third, distinct statement in the source**
that uses `idx*8` (not `idx*16`) directly and has simply not been
identified yet — a genuine missing field write, rather than a scheduling
artifact, would explain the "early, independent, never re-read until far
later" pattern far more naturally than any deliberate GCC scheduling
behavior would. This function is a much better candidate for that search
than `func_8002EA44` (the residue is a single, precisely bounded
instruction here, not 5 words scattered through a much larger function),
so it is the one to hand a future attempt.

## Preserved body (best attempt, 269/270 built words -- 1 word short, structurally correct except for the single residue documented above)

```c
#if 0
extern void func_80039228(s32 a0);
extern void func_80039104(s32 a0, void *a1);
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

/* New 0x34-stride record fields this function is the first to touch,
 * plus several already declared further down this file, moved/repeated
 * here (identical extern declarations, legal to repeat) because this
 * function's ROM-order position is ahead of their original declaration
 * point. */
extern Rec34Half D_8008D98A[]; /* value forced to 0x18 at init */

/* Same 0x34-stride channel-configuration record family, s16-field view
 * -- matches code_179d8_j.c's own Rec34D994 shape. Declared here (ahead
 * of its other users further down this file) because this function
 * needs D_8008D988/D_8008D996/D_8008D99A before their later declaration
 * point; the later, second declaration point only adds the field not
 * needed here (D_8008D994/D_8008D99E). */
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

/* Same 0x34-stride record family, UNSIGNED 16-bit view. */
typedef struct {
    u16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34U16;
extern Rec34U16 D_8008D99C[];

/* Same 0x34-stride record family, plain BYTE field view -- matches
 * code_179d8_j.c's Rec34Byte shape. */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D992[]; /* byte field, forced to 0x40 at init */
extern Rec34Byte D_8008D9A3[];
extern Rec34Half D_8008D9A4[];

/* "Currently selected channel" scratch global -- same volatile idiom
 * documented at this symbol's other declaration site in this file. */
extern volatile u16 D_8008EA26;

/* Loop bound / threshold, read fresh each call. */
extern u8 D_8008E9D0;

extern s16 D_80090BD0;
extern u8 D_8008EA2C[];

extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

/* Pointer to a "current object". Two independent local views of the
 * same D_8006DAD4 pointer coexist in this file (this project's
 * multiple-independent-local-views convention): a single struct with
 * fields at +0x194/+0x196 (func_8002F2A4), and here, an array of 24
 * 0x10-byte per-channel records -- consistent with +0x194 sitting just
 * past a 24 * 0x10 = 0x180-byte array. */
typedef struct {
    u8 pad[0x194];
    u16 unk194; /* +0x194 */
    u16 unk196; /* +0x196 */
} ObjDAD4;
extern ObjDAD4 *D_8006DAD4;

typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 */
    s16 unk6; /* +0x6 */
    s16 unk8; /* +0x8 */
    s16 unkA; /* +0xA */
    u8 padC[0x10 - 0xC];
} Rec16DAD4;

void func_8002EDD4(s32 a0) {
    s16 i;

    func_80039228(0);
    D_8008E9FC = 0;
    D_8008E84C = 0;
    func_80039104(0x20, D_8008DEB0);

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

    D_8008E9D0 = ((u8) a0 < 0x18) ? (u8) a0 : 0x18;

    if (D_8008E9D0 != 0) {
        for (i = 0; (u16) i < D_8008E9D0; i++) {
            u16 chan;
            u16 lowMask;
            u16 highMask;

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

            ((Rec16DAD4 *) D_8006DAD4)[(u16) i].unk6 = 0x200;
            ((Rec16DAD4 *) D_8006DAD4)[(u16) i].unk4 = 0x1000;
            ((Rec16DAD4 *) D_8006DAD4)[(u16) i].unk8 = 0x80FF;
            ((Rec16DAD4 *) D_8006DAD4)[(u16) i].unk0 = 0;
            ((Rec16DAD4 *) D_8006DAD4)[(u16) i].unk2 = 0;
            ((Rec16DAD4 *) D_8006DAD4)[(u16) i].unkA = 0x4000;

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
