# note2pitch2 — MATCHED (64/64 words)

> Renamed from `func_8002E038` on 2026-09-23 (tools/rename.py). Address 0x8002e038.

`code_179d8_l`, vram `0x8002E038`, file offset `0x1E838`. Frameless, 64 words
(0x100 bytes). Sibling in shape to `note2pitch` (same unit, same tail:
div-by-12 magic multiply + `D_8006DAD8[]` table lookup + magnitude-based
shift), but with more front-matter: a struct-array lookup and a div-by-8
half/remainder split feeding the same tail idiom.

## Final C

```c
extern u8 D_8008EA13;
extern u8 D_8008EA18;

typedef struct {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u8 unk6[26];
} D8008E978Entry;

extern D8008E978Entry *D_8008E978;

s32 note2pitch2(s32 a0, s32 a1) {
    s32 origA0;
    s32 idx;
    s32 tblIdx;
    D8008E978Entry *e;
    s32 v0;
    s32 div8;
    u8 a2;
    s16 a3;
    s32 diff;
    s32 q12;
    s16 rem12;
    u16 v1;

    origA0 = a0;
    idx = D_8008EA18 + (D_8008EA13 << 4);
    e = &D_8008E978[idx];
    v0 = (u16)a1 + e->unk5;
    div8 = v0 / 8;
    a3 = div8;
    a2 = 0;
    if (div8 >= 16) {
        a2 = 1;
        a3 = div8 - 16;
    }
    diff = (s16)(a2 + (origA0 + 0x3C - e->unk4));
    q12 = diff / 12;
    rem12 = diff - q12 * 12;
    tblIdx = rem12 * 16;
    tblIdx = tblIdx + a3;
    v1 = D_8006DAD8[tblIdx];
    if ((s16)(q12 - 5) > 0) {
        v1 <<= (s16)(q12 - 5);
    } else if ((s16)(q12 - 5) < 0) {
        v1 = (u16)v1 >> -(s16)(q12 - 5);
    }
    return v1;
}
```

`D8008E978Entry` is a local (this-unit-only) view of the 32-byte-stride
struct array at `D_8008E978` (indexed via `sll ... 5`); only offsets `+4`
and `+5` are read here so the rest is left as anonymous padding — a local
view per the project's multiple-independent-readings convention, not
placed in a shared header.

## Derivation notes (each confirmed against the pinned reproducer pipeline)

This took far more iterations than `note2pitch` because four independent,
non-obvious codegen levers stack on top of each other. All four were found
by bisecting reduced C snippets through
`tools/gcc263/cpp | cc1 | maspsx --expand-div --addiu-at | as`
(per CLAUDE.md's escalate-only-after-reproducing recipe — used here to test
candidate *C*, not to change the toolchain) rather than by reasoning about
GCC internals, and each is worth having on record since none of them are
in `docs/DECOMPILATION_LEARNINGS.md` yet.

1. **Signed vs. unsigned division-by-8 (`sra` vs `srl`) depends on whether a
   later value stays live PAST the divide-and-clamp block, not on the
   divide's own operand types.** `div8 = v0/8;` where `v0 = (u16)a1 +
   e->unk5` is provably non-negative by GCC 2.6.3's own analysis, so in
   isolation the compiler downgrades the standard signed-division-by-power-
   of-2 correction (`if (v0<0) v0+=7;`) to a plain `srl` — even though the
   `if`-guarded `+7` correction code is still (uselessly) emitted. Retail
   uses genuine `sra`. The difference: retail's `origA0` (the original `a0`
   parameter, needed again after the divide for the later `diff`
   computation) has to be preserved across the divide's branch. Once a
   *second* value is kept live across that specific conditional in the
   source, the compiler stops applying the nonneg-shift downgrade and keeps
   `sra`. Confirmed with paired minimal reproducers (`v0/8` alone → `srl`;
   `v0/8` plus an unrelated live value used after → `sra`).
2. **Statement ORDER inside and around the `if` that clamps `div8` into
   `(a2, a3)` changes which of the two locals gets genuinely materialized
   into its own register versus copy-propagated away.** Retail keeps a
   real, distinct `a3` register with an unconditional `move $a3,$a1` before
   the compare and a real `addiu $a3,$a1,-0x10` in the true branch. Writing
   the default assignments as `a2 = 0; a3 = div8;` (a2 first) collapses
   `a3` into an alias of `a1`/`div8` (the compiler proves they're always
   equal and elides the copy) — writing them `a3 = div8; a2 = 0;` (a3
   first) keeps `a3` real. The same order applies inside the `if` body:
   `a2 = 1; a3 = div8 - 16;` (a2 first) matches retail's instruction order
   there; the type of `a3` at that point didn't matter for whether the
   copy survives, only the statement order did.
3. **The `u8`-narrows-a-commutative-add lever from `note2pitch` recurred,
   but this time on `a3`'s WIDTH gating whether the copy-elision above
   happens at all**, not just on operand order. `s32 a3` still collapsed
   into `a1` regardless of statement order; only narrowing `a3` to `s16`
   (matching its later use, sign-extended for the `*16` table-index fusion)
   kept it a distinct, retail-matching register. `u8` also works for the
   register-identity question but reintroduces a spurious `andi $v0,$a3,
   0xff` mask right before the sign-extending `sll/sra` pair that retail
   doesn't have — `s16` avoids that mask entirely since a *signed* 16-bit
   value doesn't need zero-clearing before being treated as signed.
4. **Two independent 16-bit sign-extensions computed back-to-back
   (`rem12*16` and `(s16)a3`, both truncating from a wider value) get
   *scheduled* differently — interleaved (`sll,sll,sra,sra`) versus fully
   sequential (`sll,sra,sll,sra`) — depending on whether they're combined
   in ONE expression or split into separate statements.** Writing
   `D_8006DAD8[rem12*16 + a3]` (one expression) produced an interleaved
   schedule no source-order permutation would undo. Splitting into
   `tblIdx = rem12*16; tblIdx = tblIdx + a3; v1 = D_8006DAD8[tblIdx];`
   (two statements, matching how the value is later reused as `tblIdx`
   rather than only as an anonymous subexpression) produced retail's exact
   fully-sequential order. This is a scheduling-only difference (same
   registers throughout, confirmed instruction-by-instruction) — no
   `__asm__` barrier was needed once the statement split forced it.

### One naming trap hit and reverted while deriving this

Reusing the same local (`idx`) for both the early struct-array index
(`D_8008EA18 + (D_8008EA13<<4)`) and the later table index caused the
*unrelated* `origA0 = a0` copy to be hoisted all the way to the function's
first instruction, shifting the entire compiled body by one word relative
to retail (caught immediately by `funcdiff.py`'s out-of-range byte-count
warning, not by any plausible-looking wrong score). Giving the second index
its own name (`tblIdx`) fixed it. Filed as a caution, not a general lever —
plausibly just liveness-range coincidence rather than a systematic reuse
hazard, but cheap to avoid.

## Result

`build-and-verify.sh` exits 0 (whole-image SHA1 verifies).
`tools/funcdiff.py note2pitch2` reports 64/64 words match.

### Proposed learnings

- A value that must stay live PAST a signed-division-by-power-of-2's
  correction branch can flip GCC 2.6.3's `sra`→`srl` non-negative-range
  downgrade back to genuine `sra`, even when the divide's own operands are
  independently provably non-negative. Worth a name if this recurs — call
  it "cross-branch liveness defeats the nonneg-shift optimization".
- Splitting a two-term index expression into two statements (assign the
  first term to a named local, then add the second) can force GCC's
  scheduler into the same order retail used, when the single-expression
  form produces an interleaved schedule instead. This is a genuine, source-
  reachable fix for an "instruction order only" residue — worth trying
  before reaching for an `__asm__("")` barrier on this class of diff.

## Naming (round 75, runner alpha, FINISHING-PLAN track 3)

Already carries its real name: identified round 71 (track 2, runner bravo)
as `libsnd/vmanager note2pitch2`, fingerprint EXACT masked 1.00 vs the
disc-3.3 reference (position between `libc2/strncmp` and `libsnd/vm_prog`
agrees). Sony symbol; this pass does not rename it further. Matched,
64/64.
