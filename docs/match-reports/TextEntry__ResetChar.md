# TextEntry__ResetChar -- MATCHED (17/17 words)

> Renamed from `Obj86ED0__ResetCountdown` on 2026-09-26 (tools/rename.py). Address 0x80051814.

> Renamed from `func_80051814` on 2026-09-24 (tools/rename.py). Address 0x80051814.

Unit: `src/class_3bb8c_j.c`. `self` is `Obj86ED0` (ROUND 75 CORRECTION: was misattributed to `Obj866E8`, actually `Obj86ED0` -- gTextEntryMethods, established by class_3bb8c_i; see TextEntry__PrevChar.md for
the class-identity evidence shared across this group).

## Body

```c
void TextEntry__ResetChar(Obj86ED0 *self)
{
    if (self->unk48) {
        self->unk1C = 0;
        self->methods->slotA8(self, self->unk18, 0, 1);
    }
}
```

An "immediate fire" sibling of `TextEntry__PrevChar`'s countdown: resets
`self->unk1C` to 0 and calls `slotA8` unconditionally with a literal 0
where `TextEntry__PrevChar` would pass the live decremented countdown. Matched
first try -- no reshaping needed.

## Naming

- `TextEntry__ResetChar` -- tier B. self->unk1C = 0; slotA8(self, unk18, 0, 1) -- the same dispatch as TextEntry__PrevChar's expiry arm, called directly/unconditionally instead of reached by counting down. classtable.py gTextEntryMethods +0x09C.
