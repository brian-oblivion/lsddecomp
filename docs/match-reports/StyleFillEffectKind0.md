# StyleFillEffectKind0 -- MATCHED 99/99 (round 75), lever: the parameter is the walking pointer (no `arr = arg0` copy)

REVISITED, round 75: MATCHED 99/99; names/types used (return type `void **`, matching siblings StyleFillEffectKind1/2/3).

## Round 75 (bravo): MATCHED on the second build

**Preserved body rebuilt first**, verbatim from the `#if 0` block, live in
place of the INCLUDE_ASM: `build exit=2`, no compile-error grep hits, 93/99,
`insertions 2 / deletions 2 (positional skeleton diffs 6)`.

**The recorded diagnosis was wrong, and reading the diff showed it.** All six
differing words are the first six after `addiu $sp`: the three
`sw $sN` / `move $sN,$aN` pairs of the prologue. The REGISTERS were already
right (built also had `$s2=arg0`, `$s5=arg1`, `$s4=arg2`); only the ORDER of
the three argument copies differed -- built `a1,a2,a0`, retail `a0,a1,a2`.
Every other word, including the loop and epilogue, was byte-identical. So
"whole-function 3-way register rotation" (rounds 47/48, and the Check-3
"AGREE" + 90k-iteration permuter search built on it) described a prologue
ORDER residue, not register identity.

**Lever (build 2):** drop the `arr = (void **) arg0;` local and walk `arg0`
itself, exactly as the already-matched sibling `StyleFillEffectKind1` does:

```c
void **StyleFillEffectKind0(void **arg0, s32 arg1, void *arg2) {
    ...
    for (i = 0; i < arg1; i++) {
        fp(arg2, (void *) t3);
        *arg0 = New_Class876FC((void *) 0, &D_8008E0A4, (void *) gStyleCueSelf, arg2);
        arg0++;
    }
    return arg0;
}
```

With the copy, arg0's move into its saved register became the `arr` pseudo's
initialisation and was emitted after the other parameters' copies; with the
parameter itself as the accumulator, the three copies come out in parameter
order. The return type became `void **` (the caller's forward declaration in
`StyleBuildEffectSlots` updated and its `(void **)` cast dropped; byte-neutral).
Whole image `OK: build matches retail`, `tools/check-nonmatching.sh` green.

2 builds total. No permuter, no barrier.

### Proposed learning

**Read which words differ before naming the residue.** A diff confined to
the prologue's `sw $sN` / `move $sN,$aN` pairs with the same register set on
both sides is an ORDER difference in the parameter copies, not a register
rotation. Tell: retail copies `$a0,$a1,$a2` in order, built copies arg0
LAST. Cause: `local = argN;` then walking the local instead of the
parameter. Fix: use the parameter as the accumulator. (Sibling of this
round's "parameter reused as accumulator" match.)


> Renamed from `func_80054DA4` on 2026-09-23 (tools/rename.py). Address 0x80054da4.

## Round 48 (alpha): re-measured, Check 3 AGREE, permuter searched (900s, no zero)

**Re-measured at 93/99, not round 47's recorded 87/99**, same preserved body
verbatim -- rebuilt fresh (`build exit=2`, no compile-error grep hits, no
out-of-range drift) and `funcdiff.py` reads 93/99. The word-match figure
moved; the mechanism (a 3-way `$s2`/`$s5`/`$s4` register rotation across
`arg0`/`arg1`/`arg2`) is unchanged and still the entire residue. Filing this
as a correction, not a new finding -- the earlier 87/99 may have been a
transcription slip or measured against a slightly different environment;
either way 93/99 is what this round's pinned pipeline reproduces.

**Check 3, both checks run:**

- Built the permuter scaffold (`tools/setup-permuter.sh`), ran `--debug
  --stack-diffs`: `Stack Differences: 24 (1)`, `Register Differences: 9 (5)`,
  `Reorderings: 0`, `Insertions: 0`, `Deletions: 0` -- base score 69, a PURE
  register/stack-colour residue with no structural insertions or deletions,
  matching the "whole-function register rotation" description exactly.
- Rebuilt the same body in-tree (swapped into `src/`, ran
  `build-and-verify.sh` + `funcdiff.py`): 93/99, no out-of-range drift, and
  the raw word diffs are pure register-field substitutions at the same
  addresses the scaffold flags (retail `sw $s2,0x20($sp)` vs built
  `sw $s5,0x2c($sp)`, etc.).
- **AGREE**: scaffold and in-tree rebuild show the identical signature (pure
  register/stack colour, zero ins/del). Per this round's Check-3 guidance,
  this is exactly the "meaningful search" case.

**Permuter search run**: `-j 6 --stop-on-zero --best-only`, bounded at 900s.
**Result: not closed.** 90,031 iterations completed within the wall-clock
bound; best score plateaued at 45 (down from the base-line 69) and never
reached 0 -- candidates at score 45 and 30 were written
(`permuter-work/StyleFillEffectKind0/output-{30,45}-*`) but none is a zero. The
wrapping shell that was to append `permuter rc=$?` was reaped before that
line was written (a harness artifact, not a search anomaly); the log's
growth stopped at iteration 90031 in step with the 900s wall-clock and the
worker pool's own normal-shutdown warning (`resource_tracker: ... leaked
semaphore objects`), which is what `timeout`'s SIGTERM produces on a clean
exit -- treating this as **rc=124-equivalent (bound fired), not exhausted.**

A 3-way register ROTATION (not a 2-variable swap) may simply be a larger
perturbation than decomp-permuter's default move set reaches efficiently in
under 90k iterations -- contrast with `TickStyle` (this unit, round 47),
a 2-variable-class residue closed in 4 iterations. Worth a longer bound or a
hinted search in a future round if this function is revisited; not
re-attempted here (machine discipline: one search per function this round,
budget spent).

## Round 47 (bravo). Cold fresh, never worked before. Length matches retail
exactly (0x18C bytes / 99 words both sides, confirmed via `build/lsdde.map`:
`StyleFillEffectKind0` to `StyleFillEffectKind1` is exactly `0x18C`). First real diff off
`asm-differ`: word 2 (`0x0455A8`/vram `0x80054DA8`), retail `sw $s2,0x20($sp)`
vs built `sw $s5,0x2c($sp)`.

## Signature (recovered with confidence)

```c
void *StyleFillEffectKind0(void *arg0, s32 arg1, void *arg2);
```

Already forward-declared this way at the call site in `StyleBuildEffectSlots`
(matched, this unit, earlier round): `filled = (void **)
StyleFillEffectKind0(gStyleEffectSlots, val, arg0);`. `arg1` is the loop bound, `arg0` is
the output array walked and returned one slot advanced (the same
"array-fill, return next slot" idiom as `StyleFillEffectKind1`/`StyleFillEffectKind3`/
`StyleFillEffectKind2`), `arg2` is passed through unchanged to every callee.

## What the function does (recovered from the raw disassembly)

```c
extern u8 D_800871C8[];
extern s32 D_80087328[];
extern u8 *D_8008E0B4;
extern s32 D_8008E0BC;
extern u8 D_8008E0A4[];
extern void SetupStyleSpawnParamsA(void *arg0, void *arg1);   /* this unit, cold */
extern void SetupStyleSpawnParamsB(void *arg0, void *arg1);   /* this unit, cold,
                                                         signature widened */
extern void *New_Class876FC(void *arg0, void *arg1, void *arg2, void *arg3);

void *StyleFillEffectKind0(void *arg0, s32 arg1, void *arg2) {
    void **arr;
    s32 i;
    s32 t3;
    void (*fp)(void *, void *);

    arr = (void **) arg0;
    D_8008E0BC = rand() % 7;
    D_8008E0B4 = (u8 *) D_800871C8 + ((u32) rand() % 5) * 12;
    t3 = (u32) rand() % 5;
    if (t3 != 0) {
        t3 = D_80087328[t3];
    }
    fp = SetupStyleSpawnParamsB;
    if (gStyleCounter % 7 != 0) {
        fp = SetupStyleSpawnParamsA;
    }
    for (i = 0; i < arg1; i++) {
        fp(arg2, (void *) t3);
        *arr = New_Class876FC((void *) 0, D_8008E0A4, (void *) gStyleCueSelf, arg2);
        arr++;
    }
    return (void *) arr;
}
```

Every value confirmed directly off the raw bytes:

- `rand() % 7` uses the signed reciprocal `0x92492493`/`sra 2` (the standard
  GCC signed-divide-by-7 idiom); `rand() % 5` (twice) uses the unsigned
  reciprocal `0xCCCCCCCD`/`srl 2`.
- The `D_80087328` table lookup is a **word**-stride array (`sll v0,s1,2`
  before the `lw`), and the guard `beqz s1,...` skips the lookup only when
  the `rand()%5` remainder is exactly 0 -- matching the `if (t3 != 0)`
  reassignment shape.
- **A function pointer, not a branch, dispatches the per-iteration call.**
  `s3` is reused: first as the `%7` magic constant, then unconditionally
  loaded with `&SetupStyleSpawnParamsB` (filling the `mult`'s latency slot for free),
  then conditionally overwritten to `&SetupStyleSpawnParamsA` if
  `gStyleCounter % 7 != 0`. This is the **same shared-dispatch idiom
  `TickStyle` uses via `ObjAB4C::slotE8`**, except here the two
  candidates are plain functions (not vtable slots), selected by a modulo
  test rather than a self-object's own state.
- `SetupStyleSpawnParamsB` is called through this pointer with **two live argument
  registers** (`a0=arg2`, `a1=t3`) even though its OWN body (round 46's
  derivation, unrelated to this call) never references either -- the
  standard "already-matched/derived signature can be too narrow" situation.
  Widened its signature from `void SetupStyleSpawnParamsB(void)` to
  `void SetupStyleSpawnParamsB(void *arg0, void *arg1)` (dead params, zero cost in
  the callee, confirmed: rebuilding `SetupStyleSpawnParamsB`'s own preserved body
  under the wider signature still reproduces its recorded 25/87 score
  exactly -- see `docs/match-reports/SetupStyleSpawnParamsB.md`).

## The stall: `arg0`/`arg1`/`arg2` land in swapped saved registers

Retail: `$s2=arg0`, `$s5=arg1`, `$s4=arg2` (skipping `$s3`, which is the
scratch register for the magic constant / function pointer). My build:
`$s5=arg0`, `$s4=arg1`, `$s2=arg2` -- a 3-way rotation, not a simple pairwise
swap. Every other instruction in the function (the two magic-number
divisions, the table lookup, the function-pointer setup and dispatch, the
loop) is byte-identical modulo this one rotation.

**Tried and reverted (both worsened the score, per HARD RULE 6's
"if it changes WHICH REGISTER holds a value, it's the banned class"):**

1. Reordering local declarations (`i`/`t3`/`fp` before `arr`) and moving
   `arr = (void **) arg0;` to just before the loop instead of at the top:
   6/99, WORSE, and introduced an actual out-of-range drift (the function
   grew by a saved register despite matching declaration reshuffle) --
   reverted.

No `register T v asm("$N")` or operand constraint was tried or used, per
HARD RULE 6. Given `TickStyle`'s sibling register-colour-swap stall in
this same unit closed via a dead `i++; i--;` pair found by the permuter
(round 47, see that report), this function is a permuter candidate too,
untried here for lack of remaining round budget.

## Attempts

2 real builds: (1) initial body (`arr` assigned first), 87/99, 3-way
register rotation identified; (2) declaration/statement reorder experiment,
6/99 with drift, reverted to (1)'s body. Restored `INCLUDE_ASM`; (1)'s body
is preserved in `#if 0`.

### Proposed learning

**A 3-register rotation (not a pairwise swap) between three incoming
parameters is the same HARD-RULE-6 class as a 2-variable colour swap, and
the same permuter approach (a dead statement pair perturbing the
allocator) is worth trying before declaring it unfixable by hand** -- see
`TickStyle` in this unit for a confirmed instance of the technique
working. Not yet tried here for lack of round budget; flagging for the next
runner/round rather than re-deriving the structure.

## Naming

**`StyleFillEffectKind0`, tier B.**

Fills `arg1` slots of `gStyleEffectSlots` by repeatedly calling
`class_3bb8c_r.c`'s `New_Class876FC` (New_X for the `Obj876FC` class) with a
literal FIRST argument of `0`. That argument is confirmed (by reading
`New_Class876FC`'s own ctor chain, `class_3bb8c_r.c`) to become the new
object's `kind` field -- so "Kind0" in the name is the literal tag value
this function passes, not a guessed category. Selects which of two
"spawn-parameter" setup functions (`SetupStyleSpawnParamsA`/`B`) to call each
iteration via a `gStyleCounter % 7` test. STALL, 93/99, whole-function
3-register rotation; naming from mechanics, unaffected by match state.

## Track 4 (2026-09-26, round 88, charlie)

`gStyleEffectSlots` holds Class876FC objects (New_Class876FC), so the walking pointer is `Class876FC **` and the position `Vec3_d294 *`; `kind` is passed as a plain `s32` (was `(void *) N`), the params block as `(Class876FCParams *)` over the separately-declared D_8008E0A4.. symbols (one 0x24-byte Class876FCParams in the bytes; left as they are, a track 4b job), and gStyleCueSelf as the `Class6B5CC *` parent. Image byte-identical.
