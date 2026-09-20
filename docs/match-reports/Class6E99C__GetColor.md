> Renamed from `func_800403F8` on 2026-09-20 (tools/rename.py). Address 0x800403f8.

# Class6E99C__GetColor -- MATCH (13/13 words, first attempt)

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::getColor` (`+0x0E4`).

```c
void *Class6E99C__GetColor(Class6E99CObj *self) {
    if (self->unk78 == 0xF) {
        return D_8006EAA8;
    }
    return &D_8006EA90[self->unk78 * 3];
}
```

Same 3-byte-stride table lookup family as `Class6E99C__StartFadeToIndex`/`Class6E99C__StartFadeDefault`,
using `self->unk78` directly as the index (with `0xF` as a distinguished
"use the fixed table" sentinel, matching `unk78`'s established use
elsewhere in this unit as a mode/flags word tested against `0xF`).

## Naming (round 61, track 3)

**`Class6E99C__GetColor`** -- tier A. `Class6E99CMethods::getColor`
(`+0x0E4`). Pure getter: returns `D_8006EAA8` (the fixed/default table)
when `unk78 == 0xF`, else `&D_8006EA90[unk78 * 3]` (the indexed table) --
the same two tables `Class6E99C__StartFadeToIndex`/`StartFadeDefault`
write through `slotB8`. A pure leaf whose mechanics ARE its purpose (a
getter) is tier A by this project's own naming rule.
