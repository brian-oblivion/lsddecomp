# Obj86ED0__ResetAllAndFinish -- MATCHED (39/39 words)

> Renamed from `func_80051858` on 2026-09-24 (tools/rename.py). Address 0x80051858.

Unit: `src/class_3bb8c_j.c`. `self` is `Obj86ED0` (ROUND 75 CORRECTION: was misattributed to `Obj866E8`, actually `Obj86ED0` -- D_80086ED0, established by class_3bb8c_i; see Obj86ED0__AdvanceCountdown.md for
the class-identity evidence shared across this group). This is the "flush
all" sibling: fires `slotA8` once per remaining slot (counting down from
`self->unk10 - 1` to 0), then always fires `slotA4` once at the end.

## Body

```c
void Obj86ED0__ResetAllAndFinish(Obj86ED0 *self)
{
    s32 i;

    if (self->unk48) {
        self->unk1C = 0;
        i = self->unk10 - 1;
        if (i >= 0) {
            do {
                self->unk18 = i;
                self->methods->slotA8(self, i, self->unk1C, 0);
                i--;
            } while (i >= 0);
        }
        self->methods->slotA4(self, self->unk18, 1);
    }
}
```

## Residue and how it closed

First attempt (`self->unk1C = 0;` written AFTER `i = self->unk10 - 1;`,
inside the `if (i >= 0)` guard) scored 23/39 with two separate defects:

1. A delay-slot-fill SWAP: retail puts `sw zero,0x1C($s1)` in the `bltz`
   branch's delay slot and `move a0,s1` right after; my version put them
   in the opposite order. Same two instructions, same total count.
2. A missing REDUNDANT `move a0,s1` at the loop-skip/loop-exit merge
   point (`L800518C0`) -- `a0` already holds `self` from the branch delay
   slot, but retail re-materializes it anyway right at the merge. This is
   the project's known "redundant move" residue class
   (DECOMPILATION_LEARNINGS: "resists goto/return spelling, temp
   placement, barriers and volatile... best-posed permuter target").
   Manual attempts (a `__asm__("")` barrier, an explicit `Obj86ED0 *s =
   self;` re-mention at the merge point) did not reproduce it and in one
   case made the score worse (barrier introduced an extra unrelated
   move).

Ran the permuter (`tools/setup-permuter.sh`, `--stop-on-zero --best-only`,
found in 14 iterations / well under a minute). The zero-score candidate's
only change from my C: **`self->unk1C = 0;` moved to be the very FIRST
statement inside `if (self->unk48)`, before `i = self->unk10 - 1;` is even
computed** -- i.e. hoisted out of (before) the `if (i >= 0)` guard
entirely, unconditional on the loop running at all. That single
statement-order change fixed BOTH residues at once (the delay-slot swap
and the missing redundant move) and reached 39/39.

### Proposed learning

Add to DECOMPILATION_LEARNINGS' "redundant move" entry: at least one
instance (this one) DOES have a source-level fix, and it is not the
constructs the entry lists as tried-and-failed (goto/return spelling,
temp placement, barriers, volatile) -- it is *unconditional-store
placement relative to an unrelated guard*. Moving a later
unconditionally-executed store to occur BEFORE (rather than inside) a
guard clause that only conditions a LOOP, not the store itself, changed
delay-slot scheduling far enough downstream to also produce a merge-point
redundant move. Worth checking this lever before reaching for the
permuter on future instances of this residue class.

## Naming

- `Obj86ED0__ResetAllAndFinish` -- tier B. Loops i from unk10-1 down to 0, setting unk18=i and calling slotA8(self, i, unk1C, 0) each iteration, then calls slotA4(self, unk18, 1) once after the loop. Reads as "re-initialise every index, then finalise/notify once" -- the in-game purpose of the loop is not established. classtable.py D_80086ED0 +0x0A0.
