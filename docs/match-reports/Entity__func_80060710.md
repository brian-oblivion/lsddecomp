# Entity__func_80060710 -- MATCHED (58/58 words)

> Renamed from `func_80060710` on 2026-09-24 (tools/rename.py). Address 0x80060710.

Unit: `Entity_d` (fresh carve, round 2026-09-03). Mood-dispatch helper called
by `Entity__MoodCue43`, but itself takes only `Entity *this` -- no `out`
parameter, despite the family convention. Signature: `void
Entity__func_80060710(Entity *this)`.

## Final source

```c
void Entity__func_80060710(Entity *this) {
    s32 r;

    if (this->unkFC == 0) {
        r = rand() % 10;
        if (r >= 8) {
            this->methods->slot48(this, 1, D_80089E8C);
        } else if (r >= 5) {
            this->unk44 = 0xA;
        }
    }
    if (this->unk44 == 0xA && this->unkFC >= 0xC9) {
        this->methods->slotBC(this, D_80089DC0);
    }
}
```

## Derivation notes

- **Physical block order is load-bearing, and this is the function that
  proved it for this unit.** The natural C reading is `if (r < 8) { if (r
  >= 5) unk44=0xA; } else { slot48(...); }`, and it compiles -- but with the
  `r < 8` case as the FALLTHROUGH block and the `r >= 8`/`slot48` case as the
  jumped-to block, which is the OPPOSITE of retail's physical layout
  (fallthrough = `slot48` call, jump-target = the small `r<8`/`r>=5` block).
  Same logical condition, wrong branch polarity in the emitted code, and it
  cost one whole instruction (a missing `j` before the small block, since
  retail's `slot48` path exits early via `j 607a4` straight to the OUTER
  join point, skipping the small block entirely). Writing the `r >= 8`
  case as the `if` (first) and `r >= 5` as `else if` reproduced retail's
  layout exactly.
- **This one function's single missing instruction was responsible for the
  ENTIRE unit's other seven functions showing near-zero matches
  simultaneously.** Before fixing it, `funcdiff.py` reported "differs
  OUTSIDE this range" counts around 116,000 bytes for every other function
  in the file, and functions physically after this one in ROM order (e.g.
  `Entity__MoodCue48`, `Entity__MoodCue50`, `func_80061158`) scored 0/N despite
  being logically correct, because their comparison window was reading from
  the wrong address entirely. Fixing this function's word count collapsed
  the drift to under 200 bytes project-wide and every other function's score
  became trustworthy again. **When several functions in one unit show
  simultaneous, severe, EVEN-NUMBERED-of-total mismatches with the drift
  warning firing at a five/six-figure byte count, suspect ONE upstream
  function's word count before doubting several unrelated ones.**
- The two data-table arguments (`D_80089E8C` to `slot48`, `D_80089DC0` to
  `slotBC`) are plain `extern u8 SYM[];` externs passed directly, no offset
  arithmetic needed (contrast `Entity__MoodCue41`'s `D_80089EA2`, which does
  need one).

### Proposed learning

- **When several functions in one unit show simultaneous severe mismatches
  (near-zero words, and `funcdiff`'s "differs outside range" warning firing
  at a five/six-figure byte count for ALL of them at once), look for ONE
  function earlier in ROM order with a word-count defect before treating
  each low score as its own separate bug.** Fixing the one upstream
  size-changing function here collapsed drift from ~116KB to under 200
  bytes and turned six other functions' scores from meaningless into
  accurate, without touching them.
- **A two-way branch's PHYSICAL fallthrough/jump-target assignment is not
  free even when the logical condition is right.** Writing `if (r>=8) {
  call } else if (r>=5) { store }` versus the logically-equivalent `if (r<8)
  { if (r>=5) store } else { call }` picks which arm is the fallthrough
  block and which is reached by an explicit jump -- and only one ordering
  reproduces a given retail layout, independent of which arm reads more
  naturally as the "primary" case.
