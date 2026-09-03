# func_8004FBE4 -- STALL (addiu-$at jump-table blocker, not attempted)

Unit `class_3bb8c_g`, carved round 14. **Not attempted.**

## Classification

Classified by the head at carve time (round 14, 2026-09-04):

```sh
grep -n 'addiu *$at, *$at, *%lo' asm/nonmatchings/class_3bb8c_g/func_8004FBE4.s
```

Hit and it OWNS a jump table. **A `%lo(jtbl_*)` hit counts** -- a dense `switch` is blocked
exactly like an indexed global, because the folding happens in maspsx BELOW
cc1 and maspsx cannot tell a code address from a data one. Retail has the
UNFOLDED four instructions; the pinned pipeline emits the FOLDED three. See
`docs/research/addiu-at-blocker.md`, and CLAUDE.md's screen section for why
this is the one place the grep looks like it is over-reporting when it is not.

Stub report, filed for a mechanical reason: `tools/progress.py` decides STALL
versus untouched FRESH ground purely by whether this file exists.

No C was written and no score was measured.
