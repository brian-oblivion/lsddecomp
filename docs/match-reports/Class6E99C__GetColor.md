> Renamed from `func_800403F8` on 2026-09-20 (tools/rename.py). Address 0x800403f8.

# Class6E99C__GetColor -- MATCH (13/13 words, first attempt)

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::slotE4` (`+0x0E4`).

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
