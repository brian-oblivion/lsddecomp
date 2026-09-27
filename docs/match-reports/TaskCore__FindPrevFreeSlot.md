# TaskCore__FindPrevFreeSlot — MATCHED (38/38)

> Renamed from `Obj86B60__FindPrevFreeSlot` on 2026-09-25 (tools/rename.py). Address 0x8003d444.

> Renamed from `func_8003D444` on 2026-09-24 (tools/rename.py). Address 0x8003d444.

**Unit:** Task · **Size:** 38 words · **Result:** byte-exact, first attempt

## What it does

`Obj86B60Methods::slotEC` (already recorded in `Task.h`). The
mirror image of `TaskCore__FindNextFreeSlot`: searches BACKWARD from `self->unk58 - 1`
for the next free (null) slot in `self->unk4C->unk18[]`, wrapping to
`self->unk50 - 1` when it goes negative, stopping either on an empty slot
or on wrapping all the way back to the start index. Reports the found (or
fallback) index through `self->methods->slotF0` — the same slot
`TaskCore__FindNextFreeSlot` reports through.

```c
void TaskCore__FindPrevFreeSlot(Obj86B60 *self)
{
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    i = self->unk58;
    i--;
    for (;;) {
        if (i < 0) {
            i = self->unk50 - 1;
        }
        if (i == self->unk58) {
            break;
        }
        if (self->unk4C->unk18[i--] != NULL) {
            continue;
        }
        i++;
        break;
    }
    self->methods->slotF0(self, i, 1);
}
```

## Residue

None — matched on the first attempt by directly reusing
`TaskCore__FindNextFreeSlot`'s established shape (see that report's four-attempt
derivation): the post-decrement folded into the array index expression
(`self->unk4C->unk18[i--]`), not a separate `i--;` statement, is what
avoids GCC 2.6.3's cross-jump pass merging it with the priming `i--;`
before the loop. This is the SECOND independent instance of that residue
class in this unit (see `TaskCore__FindNextFreeSlot.md` and the proposed learning
there) — applying the already-derived idiom immediately, rather than
re-discovering it, is what made this one free.

No header changes — reuses `unk4C`, `unk18`, `unk50`, `unk58`, `slotF0`,
all already modelled from `TaskCore__FindNextFreeSlot`.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__FindPrevFreeSlot`. **Tier A**: The exact mirror of FindNextFreeSlot, searching backward instead. Same tier-A reasoning.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__FindPrevFreeSlot (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
