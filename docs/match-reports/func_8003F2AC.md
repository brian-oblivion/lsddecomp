# func_8003F2AC -- STALL (nop_mflo_mfhi blocker + register saturation, not attempted)

Unit `code_2cc8c_e`, carved round 14. **Not attempted**, and it is the worst
function in this segment on two independent screens.

## Classification

Two problems, either of which alone would be enough:

```sh
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/code_2cc8c_e/func_8003F2AC.s \
  | grep -E '\b(mult|multu|div|divu)\b'          # -> hit
grep -oE 'sw +\$(s[0-7]|fp),' asm/nonmatchings/code_2cc8c_e/func_8003F2AC.s \
  | sort -u | wc -l                              # -> 6
```

1. **`nop_mflo_mfhi` blocked.** Retail reads a multiply/divide result and feeds
   it straight into another `mult`/`div` with no `nop` between; the pinned
   pipeline inserts `nop`s there. Blocked exactly like `addiu_at` and part of
   the same flag group -- see `docs/research/addiu-at-blocker.md`. Not
   something to experiment with per-function.
2. **6 distinct callee-saved registers**, against a threshold measured in
   round 13 where none of 22 matched functions needed 5 or more and both
   functions at 8+ stalled. So even with the blocker resolved this would be
   expected to land in the register-allocation stall class.

At 242 instructions it is also the largest body in the segment. Round 13
measured all three of these facts at the time `code_2cc8c_e` was left uncarved
and recorded them in the splat yaml specifically so that no round would lead a
runner with it. This report is that warning in the place `progress.py` reads.

No C was written and no score was measured.
