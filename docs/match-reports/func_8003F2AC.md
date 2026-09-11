# func_8003F2AC -- CONVERTED to a linked SDK object (round 33). NOT game code, NOT a stall.

> **ROUND 33 (2026-09-12), head. THIS FUNCTION IS NOW LINKED FROM SONY'S OWN
> OBJECT `libgs/gs_131.o` (Psy-Q 3.3) AND IS NAMED `GsSetRefView2`.**
> `src/code_2cc8c_e.c` now starts at 0x3030C / func_8003FB0C. Whole-image
> SHA1 green. Nothing here is assignable and there is no stall left to work.
>
> **Everything below is kept as the derivation it was, not as live guidance.**


> **ROUND 32 (2026-09-12), head. THIS FUNCTION IS SONY LIBRARY CODE AND
> CANNOT BE MATCHED BY WRITING C. DO NOT STAFF A RUNNER ONTO IT.**
>
> `func_8003F2AC` (0x8003f2ac, 242 words) lies FULLY inside **`libgs/gs_131.o`** (Psy-Q Psy-Q 3.3),
> an object already placed in `config/psyq-objects.txt` and verified against
> retail by relocation-masked exact match. Its bytes come from Sony's object,
> not from anything cc1 will produce from a game source file, so no source
> shape reaches a match and every attempt is spent for certain.
>
> The correct disposition is CONVERSION, not decompilation: see
> `docs/SDK-OBJECTS-GUIDE.md`. Detect the whole class with
> `python3 tools/sdkstalls.py`.
>
> **The analysis below is not wrong, it is aimed at the wrong target**, and
> it is kept because the reading of what the routine DOES is still accurate
> and still useful when the object is placed. Only the premise that it is
> game code is retracted.

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
