# Obj86ED0__ToggleFlag20 -- MATCHED (10/10 words)

> Renamed from `func_800517EC` on 2026-09-24 (tools/rename.py). Address 0x800517ec.

Unit: `src/class_3bb8c_j.c`. `self` is `Obj866E8` (see Obj86ED0__AdvanceCountdown.md for
the class-identity evidence shared across this group).

## Body

```c
void Obj86ED0__ToggleFlag20(Obj866E8 *self)
{
    if (self->unk48) {
        self->unk20 ^= 1;
    }
}
```

Trivial boolean toggle, matched first try. `self->unk20` is unrelated to
the countdown fields (`unk10`/`unk14`/`unk18`/`unk1C`) the rest of this
group uses -- it is XOR-toggled here and nowhere else read in this unit.
