# Obj86ED0__SetName -- MATCHED (28/28 words)

> Renamed from `func_80050F28` on 2026-09-24 (tools/rename.py). Address 0x80050f28.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s vtable slot 0x040. Stores its `mode` argument and name pointer,
resets two counters, then either transliterates the name into the owned
buffer (`DecodeFullWidthSjis`, an uncarved `code_2cc8c_f` helper -- byte-translate
+ return-dest, same convention as `strcpy`) and halves `unk10` (a plain
signed `/2`, which this GCC compiles to the classic `srl`+`addu`+`sra`
round-toward-zero sequence), or falls back to a straight `strcpy` when
`mode != 1`.

```c
void Obj86ED0__SetName(Obj86ED0 *self, char *arg1, s32 mode)
{
    self->unkC = mode;
    self->unk24 = arg1;
    self->unk18 = 0;
    self->unk1C = 0;
    if (mode == 1) {
        DecodeFullWidthSjis(self->unk28, arg1);
        self->unk10 /= 2;
    } else {
        strcpy(self->unk28, arg1);
    }
}
```

## Residue and how it was closed

First attempt zeroed `unk1C` INSIDE the `mode == 1` branch (mirroring where
it reads in the raw `.s` file, which places the `sw zero,0x1C` in the
branch's delay slot). That scored 7/28: retail zeroes `unk1C`
UNCONDITIONALLY, alongside `unk18`, before the branch -- the delay slot is
just where the scheduler put an instruction that has no ordering dependency
on the branch, not where the SOURCE placed it. Moving `self->unk1C = 0;` up
next to `self->unk18 = 0;`, both unconditional and before the `if`, closed
it to 28/28.

### Proposed learning

A store sitting in a branch's delay slot in the `.s` listing is not evidence
it belongs inside that branch's C block -- MIPS delay slots hold whatever
instruction had no dependency on the branch outcome, which is frequently an
instruction that logically belongs BEFORE the branch. Check whether the
value/target is used on BOTH sides of the branch (here: `unk1C` is zeroed
regardless of which branch runs) before deciding it is conditional.
