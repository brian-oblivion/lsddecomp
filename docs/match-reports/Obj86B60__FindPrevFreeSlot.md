# Obj86B60__FindPrevFreeSlot — MATCHED (38/38)

> Renamed from `func_8003D444` on 2026-09-24 (tools/rename.py). Address 0x8003d444.

**Unit:** code_2cc8c_b · **Size:** 38 words · **Result:** byte-exact, first attempt

## What it does

`Obj86B60Methods::slotEC` (already recorded in `code_2cc8c.h`). The
mirror image of `Obj86B60__FindNextFreeSlot`: searches BACKWARD from `self->unk58 - 1`
for the next free (null) slot in `self->unk4C->unk18[]`, wrapping to
`self->unk50 - 1` when it goes negative, stopping either on an empty slot
or on wrapping all the way back to the start index. Reports the found (or
fallback) index through `self->methods->slotF0` — the same slot
`Obj86B60__FindNextFreeSlot` reports through.

```c
void Obj86B60__FindPrevFreeSlot(Obj86B60 *self)
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
`Obj86B60__FindNextFreeSlot`'s established shape (see that report's four-attempt
derivation): the post-decrement folded into the array index expression
(`self->unk4C->unk18[i--]`), not a separate `i--;` statement, is what
avoids GCC 2.6.3's cross-jump pass merging it with the priming `i--;`
before the loop. This is the SECOND independent instance of that residue
class in this unit (see `Obj86B60__FindNextFreeSlot.md` and the proposed learning
there) — applying the already-derived idiom immediately, rather than
re-discovering it, is what made this one free.

No header changes — reuses `unk4C`, `unk18`, `unk50`, `unk58`, `slotF0`,
all already modelled from `Obj86B60__FindNextFreeSlot`.
