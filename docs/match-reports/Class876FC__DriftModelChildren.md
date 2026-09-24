# Class876FC__DriftModelChildren -- STALL (length now EXACT: 121/121 words emitted, no address drift; raw word-match 117/121; first real diff at word 12, `asm-differ` offset 0x471d8, the same commutative register-identity swap in the very first pointer computation as before)

> Renamed from `func_800569A8` on 2026-09-23 (tools/rename.py). Address 0x800569a8.

Unit `class_3bb8c_s`, round 44 (2026-09-15), building on round 26's derivation
(see the git history of this file for the prior 120/121, 23/121-raw state).
Not a class method dispatcher itself, but calls through `self->methods` twice
and through each `arr7C` child's methods twice more. Reads `self->unk6C` as
an "active" guard and `self->unk70` as an index into a small lookup table; if
both guards and a third bound check on `self->unk24` pass, it forwards a
table handle to `self` and to each of the two `arr7C` children, folds a
per-child Vec3 offset and dispatches it through vtable slot `slotBC`, then
runs a modulus check against a `24500 / gModelChildDriftZ[idx]` quotient to decide
whether to call `Class876FC__PlaceModelChildren(self, 1)`. Always zeroes `*self->unk14` on
every exit path.

## Round 44 correction to round 26's field reading

Round 26 described `self->unk24` as "bounded `< 0x1F5`", i.e. as an entry
guard that must hold for the block to run. **That polarity is backwards.**
Rebuilding the preserved body exposed `sltiu`+`bnez`-to-exit at that guard
(retail: `sltiu v0,v0,0x1F5; bnez v0,.L80056B58`) — a `bnez`-to-skip on the
"in bounds" test means the block runs only when `self->unk24` is **NOT**
less than `0x1F5`. The correct guard is `(u32) self->unk24 >= 0x1F5`.
Flipping it (previously `< 0x1F5`, ANDed the same way) closed one whole word
of the residue outright and had no other effect — the block's meaning is
still "some overflow/extended path", just the OTHER side of the threshold
from what round 26 assumed.

## What it is (best-reached body this round, 117/121 raw words, length-exact, NOT byte-exact)

```c
#if 0
/* After 500 frames (tick >= 0x1F5), for kinds with model children and a
 * nonzero gModelChildDriftZ step: spin self and both children, move the
 * children along z, and every 24500 / step frames snap them back to their
 * layout. Always marks self's coord2 for recompute.
 *
 * round 44 (2026-09-15): best-reached body, 117/121 words, NOT byte-exact.
 * See docs/match-reports/Class876FC__DriftModelChildren.md for the residue and what was
 * tried. Kept here per the hard rule -- restore this ahead of any future
 * attempt rather than re-deriving from scratch. */
void Class876FC__DriftModelChildren(Class876FC *self)
{
    s32 idx;
    s32 *tab70;
    s32 *tab70b;
    s32 accumOffset;
    s32 divq;
    s32 modend;
    LinkNode **p;
    s32 i;

    idx = self->tableIndex;
    if (self->modelChildLayout != 0) {
        tab70 = &gModelChildDriftZ[idx];
        if (*tab70 != 0 && (u32) self->tick >= 0x1F5) {
            p = self->modelChildren;
            self->methods->updateRotation(self, 0, (s32) gSpinRotStep);

            i = 0;
            tab70b = tab70;
            accumOffset = 0;
            for (; i < 2; i++) {
                Vec3S local = gModelChildDriftInit;
                local.z += accumOffset + *tab70b;
                (*p)->methods->addTranslation(*p, &local);
                accumOffset += 3;
                (*p)->methods->updateRotation(*p, 0, (s32) gSpinRotStep);
                p++;
            }

            divq = 24500 / gModelChildDriftZ[idx];
            modend = self->tick;
            if (divq >= 0) {
                if ((u32) modend % (u32) divq == 0) {
                    Class876FC__PlaceModelChildren(self, 1);
                }
            } else {
                u32 adivq = ~divq + 1;
                if ((u32) modend % adivq == 0) {
                    Class876FC__PlaceModelChildren(self, 1);
                }
            }
        }
    }
    *self->coord2 = 0;
}
#endif
```

Declarations this needs (already committed in `src/class_3bb8c_s.c`, kept
regardless of this function's match state):

```c
/* LinkNodeMethods (round 70 names): */
void (*updateRotation)(LinkNode *self, s32 set, s32 data);  /* +0x044 */
void (*addTranslation)(LinkNode *self, void *delta);        /* +0x0BC */

/* LinkNode (round 70 names; round 44 called them unk14/unk24/unk6C/unk70/arr7C): */
s32 *coord2;                /* +0x014, `*coord2 = 0` on every exit path */
s32 tick;                   /* +0x024, entry guard is `>= 0x1F5`, NOT `< 0x1F5` */
s32 modelChildLayout;       /* +0x06C */
s32 tableIndex;             /* +0x070 */
LinkNode *modelChildren[2]; /* +0x07C */

typedef struct LinkNode Class876FC;
extern s32 gModelChildDriftZ[];
extern Vec3S gModelChildDriftInit;
extern s32 gSpinRotStep[];
```

## What round 44 fixed (from 23/121 raw / 1-word-short, to 117/121 raw / length-exact)

Three independent levers, applied in this order, each measured in isolation
before the next was tried:

1. **A second local alias for the cached pointer, assigned right before the
   call it must survive.** `tab70b = tab70;` (a plain copy, used only inside
   the loop in place of `tab70`) reproduced retail's own two-register dance
   — retail computes the address into one register, then explicitly copies
   it to a SECOND register right before the call that would otherwise clobber
   it, because the original register gets reused afterward for something
   else. Writing that second register as its own NAMED C variable (rather
   than relying on GCC to invent the copy on its own, which it never did)
   took the raw score from 23/121 to 93/121 in one change — the single
   biggest lever in this function's whole history.
2. **The `self->unk24` guard polarity was inverted** (see the correction
   above): `>= 0x1F5` instead of `< 0x1F5`. 93 -> 96/121 (some of the gain
   here is ripple from the branch reconverging correctly rather than one
   clean word).
3. **The `divq < 0` / `divq >= 0` arm order was swapped**, `if (divq >= 0)
   {...} else {...}` instead of round 26's `if (divq < 0) {...} else
   {...}` (division/modulus bodies unchanged, just which arm is textually
   first). Round 26 tried this exact swap in a WORSE context (before lever 1
   existed) and it regressed the score, which is why the report at the time
   concluded "neither ordering is reliably predictable." In the CURRENT
   context it closed the entire back half of the function (the whole
   divide/modulo/`Class876FC__PlaceModelChildren`-call tail) — 96 -> 109/121. **This directly
   confirms MATCHING-GUIDE.md's own caution that the "arm that must jump"
   polarity is not context-independent**: the same source-level change was a
   regression in one register-allocation context and the correct fix in
   another, for the identical branch.
4. **Statement placement inside the guarded block, tuned against the
   disassembly instruction-by-instruction**: `p = self->arr7C;` first (before
   the `slot44` call), then the call, then `i = 0;` immediately after the
   call (still before `tab70b = tab70;`/`accumOffset = 0;`). Moving `p`'s
   assignment any earlier (outside the innermost `if`, matching where retail
   schedules the underlying instruction) REGRESSED the whole function back to
   23/121 — confirms MATCHING-GUIDE.md's warning that block/statement
   reordering is not monotonic and a plausible-looking match to the
   disassembly's instruction order can be badly wrong. This raised the score
   109 -> 117/121 through several small, individually-tested placements.

None of these four changed the function's WORD COUNT — length has been exact
(121/121 emitted) since lever 1; only word-content differed from there on.

## Struct findings (kept in `src/class_3bb8c_s.c` independent of this stall)

- **`LinkNodeMethods` needed a NEW slot, `slotBC` at +0x0BC**, distinct from
  the already-established `slotB8` at +0x0B8 (round 26 finding, unchanged).
- **`LinkNode` needed two new fields inside previously-unnamed padding**:
  `unk14` (pointer, zeroed at the very end) and `unk24` (`s32`, the modulus
  dividend — entry-guard polarity corrected this round, see above). Both
  insertions keep the struct's total size and every other field's offset
  unchanged (whole-image SHA1 stays green either way).

## NOT CLOSED this round: one residue, 4 words, pure commutative register identity

**The `&gModelChildDriftZ[idx]` pointer computation's TWO temp registers are still
swapped** at the FIRST occurrence only (before `tab70b` exists) — `v0`/`v1`
hold the shift-result and the base address in the opposite roles from
retail:

```
retail:  sll v1,s5,2 / lui v0,%hi(gModelChildDriftZ) / addiu v0,v0,%lo(...) / addu s1,v1,v0
built:   sll v0,s5,2 / lui v1,%hi(gModelChildDriftZ) / addiu v1,v1,%lo(...) / addu s1,v0,v1
```

Tried this round, both inert (byte-identical output to the array form):
- `tab70 = gModelChildDriftZ + idx;` (pointer-arithmetic form) — confirms round
  26's own finding still holds under the new context.
- `tab70 = (s32 *)((u8 *)gModelChildDriftZ + (idx << 2));` (explicit byte-offset
  cast form) — also no effect, ruling out the array-vs-pointer-vs-manual-shift
  surface syntax entirely as a lever for THIS specific commutative pair.

This is the same "commutative operand order, not independently reachable"
class as `code_179d8_j`'s `func_80031280` (per round 26's own note), which
needed the permuter to close. Set up this round with
`tools/setup-permuter.sh Class876FC__DriftModelChildren <seed>` using the 117/121 body above
as the seed. `--debug --stack-diffs` measured a base score of 30 (a single,
tightly-scoped residue — a MUCH better-posed base than round 26's 790,
consistent with the three levers above having eliminated everything else).
A `-j 6 --stop-on-zero --best-only` search ran bounded (`timeout 300`, per
this round's own correction to round 26's unbounded-search mistake) and
reached **19712 iterations with zero errors on most candidates, but the
score never dropped below its starting value of 30** — i.e. **not closed in
~19700 iterations under this session's load**, not "permuter-exhausted."
The search process was scoped to this worktree (`permuter-work/Class876FC__DriftModelChildren`
under `lsddecomp2-wt-charlie`) and left to self-terminate on its own
`timeout` rather than killed by PID-guessing.

## Do not re-try, without a new idea

- Any resyntax of `&gModelChildDriftZ[idx]` vs `gModelChildDriftZ + idx` vs explicit
  pointer-cast-and-shift: three surface spellings tried across two rounds,
  byte-identical machine code every time. The RTL this lowers to is fixed
  regardless of source spelling; the swap is a register-allocator choice made
  from context this project has no established C-level lever for yet.
- Moving `p = self->arr7C;` outside the innermost `if` (to mirror the
  disassembly's own very-early scheduling of that address computation):
  tried twice across two rounds (once at 23/121-quality context, once at
  110/121-quality context), regressed BOTH times, confirming this is not a
  context-dependent flip like the `divq` arm-order lever turned out to be —
  it appears genuinely monotonically wrong to hoist this statement.

### Proposed learning

**The exact same source-level "swap the if/else arm order" edit was a
regression in round 26's context and the single largest lever in round 44's
context, on the literal same branch.** This is not a contradiction to
resolve by picking "the right answer" — MATCHING-GUIDE.md already documents
that arm-order polarity is sensitive to surrounding register pressure, and
this is a second, cleanly-isolated confirmation of it (the first being that
bullet's own citation). The actionable version for the next runner: **when a
residue report says an arm-order swap "made it worse," treat that as
evidence about THAT report's register-allocation context, not as a
general fact about the branch** — re-test it fresh after any other lever in
the same function has landed, exactly as round 43/44's "REOPENED" banners
already ask for gp_rel stubs. A stale negative on a context-sensitive lever
is exactly as costly as a stale blocker claim.

**Second learning, procedural**: rebuilding a round-26 "1 word short, 23/121
raw" report from scratch is not "re-deriving a known stall" — it took one
mechanical lever (a second local alias for a pointer that must survive a
call, mirroring retail's own explicit register-to-register copy) to jump
70 raw words in a single change. A stall report's own STRUCTURAL findings
(the fields, the vtable slot, the algorithm) stay valid far longer than its
residue-classification attempts; the latter are worth re-running whenever
context changes, which here was simply "one more lever landed earlier in
the same function."

## Naming

Round 70 (alpha). `func_800569A8` -> `Class876FC__DriftModelChildren`, **tier B**.

Named from its preserved body and asm (still a stall, so B): gated on
modelChildLayout != 0, gModelChildDriftZ[tableIndex] != 0 and tick >= 501;
adds gSpinRotStep via updateRotation(.., 0, ..) to self and both children,
adds a z delta via each child's slot +0x0BC (BaseObjO__AddVec14 in
D_800878D4), and every 24500 / step frames calls
Class876FC__PlaceModelChildren(self, 1) to snap them back. Always stores 0
to `*coord2` (GsCOORDINATE2.flg). Caller: Class876FC__UpdateByKind, kind 0.

Globals named in this pass (only this unit references them, tier B):
`gModelChildDriftZ` (was D_8008780C, s32[8] = {0, 0, 0, -1, -2, -4, -16,
-256}), `gModelChildDriftInit` (was D_8008782C, all-zero Vec3S) and
`gSpinRotStep` (was D_80087838, ratio triple {0/1, 1/10, 0/1}, read by
RatioToFixed12).

The preserved `#if 0` body (in the .c and above) was renamed with the unit's
fields and slots; it was pushed through cpp | cc1 once afterwards and still
compiles.

## NON_MATCHING body promoted, round 74

Moved from `#if 0 ... #endif` (dead, INCLUDE_ASM always live) into the
`#ifdef NON_MATCHING ... #else INCLUDE_ASM #endif` shape (docs/FINISHING-PLAN.md
track 1b): field/slot names already matched the round-70 renamed unit, so no
further renaming was needed. Re-measured with the body compiled live in place
of `INCLUDE_ASM`: 117/121 words match, no address drift (positional skeleton
diffs 4, all four inside the function's own window), confirming the figures
above are current. `./build-and-verify.sh` and `tools/check-nonmatching.sh`
both green with the body in its NON_MATCHING branch.
