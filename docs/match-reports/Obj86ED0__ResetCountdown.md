# Obj86ED0__ResetCountdown -- MATCHED (17/17 words)

> Renamed from `func_80051814` on 2026-09-24 (tools/rename.py). Address 0x80051814.

Unit: `src/class_3bb8c_j.c`. `self` is `Obj86ED0` (ROUND 75 CORRECTION: was misattributed to `Obj866E8`, actually `Obj86ED0` -- D_80086ED0, established by class_3bb8c_i; see Obj86ED0__AdvanceCountdown.md for
the class-identity evidence shared across this group).

## Body

```c
void Obj86ED0__ResetCountdown(Obj86ED0 *self)
{
    if (self->unk48) {
        self->unk1C = 0;
        self->methods->slotA8(self, self->unk18, 0, 1);
    }
}
```

An "immediate fire" sibling of `Obj86ED0__AdvanceCountdown`'s countdown: resets
`self->unk1C` to 0 and calls `slotA8` unconditionally with a literal 0
where `Obj86ED0__AdvanceCountdown` would pass the live decremented countdown. Matched
first try -- no reshaping needed.

## Naming

- `Obj86ED0__ResetCountdown` -- tier B. self->unk1C = 0; slotA8(self, unk18, 0, 1) -- the same dispatch as Obj86ED0__AdvanceCountdown's expiry arm, called directly/unconditionally instead of reached by counting down. classtable.py D_80086ED0 +0x09C.
