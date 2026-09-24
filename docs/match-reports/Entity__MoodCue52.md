# Entity__MoodCue52 -- MATCHED (78/78 words)

> Renamed from `func_80060F38` on 2026-09-24 (tools/rename.py). Address 0x80060f38.

Unit: `Entity_d` (second pass, round 2026-09-03). Mood-dispatch handler:
`void Entity__MoodCue52(Entity *this, EntityMoodHandlerArg *out)`.

## Final source

```c
void Entity__MoodCue52(Entity *this, EntityMoodHandlerArg *out) {
    out->unk10 = this->methods->slot148(this);
    if (this->unkFC < 0xBC) {
        if (this->unkFC == 0x54) {
            this->methods->slot44(this, 0, D_80089C7C);
        }
        if (out->unk4 % 20 == 0) {
            out->unk1C = 9;
        }
    } else if (this->unkFC < 0xC8) {
        this->methods->slot44(this, 0, D_80089C64);
    } else {
        this->methods->slot160(this);
        out->unk30 = 0x1E;
        this->unk44 = 1;
    }
    this->methods->slotD0(this, -0x200, 0);
}
```

## Derivation notes

Matched first attempt, no iteration needed. Straightforward three-way
`if`/`else if`/`else` dispatch on `this->unkFC`, with a nested nested check
inside the first arm. Two things worth noting for the write-up, neither of
which needed a fix:

- The `out->unk4 % 20` check runs UNCONDITIONALLY inside the `unkFC < 0xBC`
  arm (both when `unkFC == 0x54` triggers the `slot44` call and when it
  doesn't) -- both sub-paths converge on the same magic-multiply-by-20
  check before falling to the shared `slotD0` tail call. The magic
  constant `0x66666667` with a `sra ...,3` post-shift is the same family as
  the divide-by-5 (`sra ...,1`) and divide-by-10 (`sra ...,2`) idioms seen
  elsewhere in this unit, generalizing to divide-by-`5*2^(shift-1)`; here
  shift 3 gives divide-by-20, and the final compare-against-`q*20`
  reconstructs a `% 20 == 0` test. No manual instruction reconstruction
  needed -- plain `%` reproduces it.
- All three vtable slots involved (`slot148`, `slot44`, `slot160`,
  `slotD0`) were already correctly typed in `include/Entity.h` from earlier
  units/functions; no header change needed here beyond adding the
  `D_80089C7C` data-table extern (same convention as this unit's other
  `D_80089Cxx`/`D_80089Exx` externs).

### Proposed learning

None beyond the already-documented magic-multiply-divisor family --this
confirms the `0x66666667` constant's shift amount generalizes cleanly to
`sra ...,3` for divide-by-20, extending the previously-confirmed
divide-by-5/divide-by-10 instances in this same unit.
