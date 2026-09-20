> Renamed from `func_800402F0` on 2026-09-20 (tools/rename.py). Address 0x800402f0.

# Class6E99C__Stop -- MATCHED (66/66 words)

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::slot14` (`+0x014`,
shared with `Class6B5CCMethods`'s own inherited slot).

## Final body

```c
void Class6E99C__Stop(Class6E99CObj *self, void *a1) {
    Class6E99CMethods *methods;
    s32 mode;

    methods = self->methods;
    if (self->unk6C == 0) {
        return;
    }
    if (self->unk6C == 1) {
        mode = 5;
        if (self->unk98 == 0) {
            methods->slot60(self, 0);
            methods->slot64(self, 0);
        }
    } else {
        mode = 6;
        if (self->unk98 != 0) {
            if (self->unk78 == 0xF) {
                methods->slotB8(self, 1, D_8006EAA8);
            }
            methods->slot64(self, 0);
        }
    }
    methods->slot14(self, a1);
    if (self->unk74 < 0) {
        self->unk74 = -self->unk74;
    }
    self->unk6C = 0;
    methods->slot30(self, mode);
}
```

`methods->slot64(self, 0)` is written once per arm in the source but
compiles to a SINGLE shared physical call site either way (GCC 2.6.3's own
cross-jump pass merges the two textually-identical tail calls back
together, matching retail's own single call site).

## How the prior 64/66 stall closed (2 residues, both delay-slot-filler placement)

The prior report's body assigned `mode` INSIDE each arm's nested `if`
(`if (self->unk98 == 0) { mode = 5; ...}` / `if (self->unk98 != 0) { mode
= 6; ...}`), matching the naive reading of which branch the assignment
"belongs to". Both residues were retail filling a branch's delay slot
with the very next real assignment where every such nested-if C shape
instead left it `nop` and did the assignment as a separate instruction
afterward.

**Fix: hoist `mode`'s assignment to be the FIRST statement in EACH outer
arm, unconditional on the INNER `unk98` test** -- i.e. `mode = 5;` (or
`= 6;`) executes regardless of whether the nested `if` body runs, only
gated by the OUTER `self->unk6C` arm. This is semantically safe/free: in
the original nested-if reading, `mode` was uninitialized (read via
`methods->slot30(self, mode)` at the end) on the path where the inner
`if` was false -- `-Wall` even warned `'mode' might be used
uninitialized in this function` on that version. Retail's own path
computes `mode` regardless, which the register-identity clue was
pointing at all along: an unconditional assignment right after a branch
test naturally lands in that branch's own delay slot, which is
observably true for BOTH arms once hoisted this way, closing both
residues from ONE structural change (a single scope-level hoist applied
symmetrically to both arms, not two independent fixes):

- unk6C==1 arm: `mode = 5;` now lands in the delay slot of the
  `bnez v0,<skip>` (testing `self->unk98 != 0`) that guards the inner
  block, matching retail's `li s2,0x5` delay-slot fill exactly.
- else arm: `mode = 6;` now lands before `self->unk78` is even loaded,
  matching retail's `li s2,0x6` placement (this half was found and
  confirmed FIRST, via `permuter.py --debug`'s isolated single-residue
  diff, before the second half was derived by applying the same lever
  to the sibling arm).

Confirmed via `permuter.py --debug` (score 0, all five penalty
categories zero) and then the real oracle:
`./build-and-verify.sh` exits 0, whole-image SHA1 matches, and
`funcdiff.py Class6E99C__Stop` reports `66/66 words match`.

## Attempts (7 total across two rounds)

Round 14/15 (5, all pre-`__asm__`/nested-if shapes; see git history for
exact text) plus this round's 2:

6. Bare-randomization permuter run (`permuter-work/Class6E99C__Stop`, no
   `PERM` macros, `-j 6 --stop-on-zero --best-only`, `timeout 550`):
   score improved from base 410 (both residues open) down to 100 (one
   residue closed -- the `mode = 6` hoist, discovered blind by the
   search) over ~32,000+ iterations under heavy machine contention (5
   parallel runners). Did not reach zero; wrapper's own exit-code capture
   was lost (see "Anomaly" below), but the run's own iteration/score log
   is conclusive: it plateaued at 100 well before the bound and never
   improved further under blind mutation.
7. Manual: applied the SAME "unconditional hoist before the inner test"
   shape that the permuter's own 100-scoring candidate had found for the
   `else` arm to the OTHER (`unk6C==1`) arm by hand, reasoning from
   *why* the hoist worked (an assignment unconditional on the OUTER arm
   alone is free to schedule into that arm's own guard-branch delay
   slot) rather than by further search -- `--debug` score 0 on the first
   try. This is the reported body above.

### Anomaly: lost exit-code capture on the round-6 background permuter run

The bounded background run (`nohup bash -c '... timeout 550 permuter.py
...; echo "permuter exit=$?" >> log' &`) never wrote its trailing
`echo` line to the log -- the log ends mid-iteration-stream plus a
`multiprocessing.resource_tracker` semaphore-cleanup warning, with
no `permuter exit=` line at all. The iteration counter (~32,286 final)
and elapsed wall-clock are consistent with the `timeout 550` bound
firing (not with `--stop-on-zero`, since no zero was ever found by that
run), so this is read as **timeout, exit code not literally captured**
-- worth noting for the next runner relying on this exact wrapper
pattern: something in the harness's own background-process lifecycle
appears to reap the wrapper before its trailing statement runs, even
though the redirected FD content up to that point is intact and
trustworthy.

### Proposed learning

**"Which nesting level a value's assignment sits at" is itself a
source-level fact worth checking BEFORE accepting a "delay-slot
scheduling, no structural fix" classification.** Both of this
function's residues looked identical in kind (retail fills a delay slot
this C shape leaves `nop`), and the existing "How to read a
one-instruction residue" lever (mention/de-mention the value) does not
directly describe this case -- the value was mentioned exactly once
either way. The actual fix was an unconditional-vs-conditional
ASSIGNMENT SCOPE question: retail computes the value regardless of the
inner test (visible independently as the `-Wall` "might be used
uninitialized" warning on the naive nested reading), and once hoisted
to the assignment's true scope, the delay-slot placement followed for
free. This generalises `func_80051858`'s own already-recorded lever
("moving a later unconditionally-executed store to occur BEFORE rather
than inside a guard clause that only conditions a loop, not the store
itself") to a guard that conditions an entire computed VALUE, not just
a loop -- and confirms it transfers to a genuinely different function
shape (two symmetric arms, not one loop/merge point).
