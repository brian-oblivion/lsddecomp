# Obj86ED0__DispatchLookupValue — MATCHED (round 45, 45/45 words)

> Renamed from `func_80051998` on 2026-09-24 (tools/rename.py). Address 0x80051998.

**Unit:** class_3bb8c_j · **Size:** 45 words (0xB4 bytes)

Filed as a `gp_rel`-blocked stub in round 15. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). Matched on the first
attempt once rebuilt against the fixed toolchain, alongside its sibling
`Obj86ED0__DispatchIndexValue`.

## ROUND 75 CORRECTION

This report originally typed `self` as `Obj866E8` (D_800866E8) and typed
`self->unk44` through a unit-local `Unk44Obj866E8`/`Unk44Obj866E8Methods`
duplicate. Both were wrong, for the same reason as its sibling
`Obj86ED0__DispatchIndexValue` (see that report and
`src/class_3bb8c_j.c`'s file header comment): `self` is `Obj86ED0`
(`tools/classtable.py D_80086ED0` places this function at +0x0A8), whose
shared struct already types `self->unk44` as `ChildObj86ED0 *`. The
`+0x0C4` slot is now `ChildMethods86ED0::slotC4`, added additively next to
the sibling's `slotBC`. Zero bytes affected.

## Derivation

```c
extern u8 *D_8008AAE4;

void Obj86ED0__DispatchLookupValue(Obj86ED0 *self, s32 arg1, s32 arg2, s32 arg3)
{
    ChildObj86ED0 *obj;

    if (self->unk48) {
        self->unk28[arg1] = D_8008AAE4[arg2];
        obj = self->unk44;
        obj->methods->slotC4(obj, D_8008AAE4[arg2], arg1);
        self->unk18 = arg1;
        self->unk1C = arg2;
        if (arg3) {
            self->methods->slot60(self, 0);
        }
    }
}
```

Same `Obj86ED0` "countdown/flush" group as `Obj86ED0__DispatchIndexValue` (see that
report): gated on `self->unk48`, this one copies one byte out of a lookup
table (`D_8008AAE4`, VALUE-of `%gp_rel`, ROM image points it at
still-uncarved rodata `D_800115D0`) into `self->unk28[arg1]`, forwards the
same byte plus `arg1` to `self->unk44`'s own method table at slot `0xC4`,
records `self->unk18`/`self->unk1C`, and — if `arg3` is non-zero — notifies
through the same `self->methods->slot60(self, 0)` as its sibling.

`self->unk18 = arg1` and `self->unk1C = arg2` both land in retail's branch
delay slots (of the `beqz arg3, end` branch and its own fallthrough),
executing unconditionally whenever `self->unk48` is set — ordinary
plain-statement-before-`if` C, same shape as `Obj86ED0__DispatchIndexValue`.

`unk28`/`unk44` were already present on the shared `Obj86ED0` (established
by class_3bb8c_i); only `ChildMethods86ED0::slotC4` (offset 0x0C4) is a
new additive field in `include/class_3bb8c.h`, alongside the sibling's
`slotBC` — see `Obj86ED0__DispatchIndexValue`'s report for the full set.

### Proposed learning

None beyond `FormatNumberIntoBuffer`'s this round — this pair matched cleanly once
the stale `gp_rel` verdicts were set aside; the only real content was
identifying the shared `Obj86ED0` "countdown/flush" struct fields and the
notify-on-flag-set tail shape common to both siblings.
