# Obj86ED0__DispatchIndexValue — MATCHED (round 45, 41/41 words)

> Renamed from `func_800518F4` on 2026-09-24 (tools/rename.py). Address 0x800518f4.

**Unit:** class_3bb8c_j · **Size:** 41 words (0xA4 bytes)

Filed as a `gp_rel`-blocked stub in round 15. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). Matched on the first
attempt once rebuilt against the fixed toolchain.

## ROUND 75 CORRECTION

This report originally typed `self` as `Obj866E8` (D_800866E8) and typed
`self->unk40` through a unit-local `Unk40Obj866E8`/`Unk40Obj866E8Methods`
duplicate. Both were wrong. `tools/classtable.py D_80086ED0` places this
function at that table's +0x0A4 (`D_800866E8`'s 80 slots hold none of this
group's six addresses) -- `self` is `Obj86ED0` (class_3bb8c_i's shared
type), whose OWN struct in `include/class_3bb8c.h` already types
`self->unk40` as `ChildObj86ED0 *`. The `+0x0BC` slot this function
dispatches through was simply missing a name on the shared
`ChildMethods86ED0` -- added additively there instead of duplicated
locally. See `src/class_3bb8c_j.c`'s file header comment and
`Obj86ED0__AdvanceCountdown.md` for the full evidence trail. Zero bytes
affected (type names are not codegen).

## Derivation

```c
typedef struct {
    s32 unk0;
    s32 unk4;
} SlotBCArg866E8_3bb8c_j;

extern s32 D_8008AADC;
extern s32 D_8008AAE0;

void Obj86ED0__DispatchIndexValue(Obj86ED0 *self, s32 arg1, s32 arg2)
{
    SlotBCArg866E8_3bb8c_j local;
    ChildObj86ED0 *obj;

    if (self->unk48) {
        local.unk4 = D_8008AAE0;
        local.unk0 = arg1 * 7 + D_8008AADC;
        obj = self->unk40;
        obj->methods->slotBC(obj, &local);
        self->unk18 = arg1;
        if (arg2) {
            self->methods->slot60(self, 0);
        }
    }
}
```

Part of the same `Obj86ED0` "countdown/flush" group established in round 15
(`Obj86ED0__AdvanceCountdown`/`Obj86ED0__ToggleFlag20`/`Obj86ED0__ResetCountdown`/`Obj86ED0__ResetAllAndFinish`, same
unit): tests `self->unk48` as a readiness gate, then calls through
`self->unk40`'s own method table at slot `0xBC` with a 2-word stack-local
argument block, then records `self->unk18 = arg1` and, if `arg2` is
non-zero, notifies through `self->methods->slot60(self, 0)`.

`self->unk18 = arg1` sits in the retail delay slot of the `beqz arg2, end`
branch, i.e. it executes unconditionally whenever `self->unk48` is set —
matching ordinary C where the assignment is a plain statement before the
`if (arg2)` block, which the scheduler is free to move into the branch's
delay slot since it doesn't depend on the branch outcome.

**Header additions** (`include/class_3bb8c.h`, additive):
- `ChildMethods86ED0::slotBC` (offset 0x0BC) — round 75, this function's own
  dispatch target on the ALREADY-shared `ChildObj86ED0` (`self->unk40`).
  `unk28`/`unk40`/`unk44`/`Obj86ED0Methods::slot60` were already present on
  the shared `Obj86ED0`/`Obj86ED0Methods`, established by class_3bb8c_i —
  no edit needed for those.

`D_8008AADC` is read here as a plain VALUE (`s32`, used arithmetically:
`arg1 * 7 + D_8008AADC`), a different reading from `class_3bb8c_i.c`'s own
`extern s32 D_8008AADC;` (there, only its ADDRESS is taken, as an opaque
`slot4C` argument). Both are legitimate independent local views of the same
global per the project's convention — they're in separate translation
units and never collide.

### Proposed learning

None beyond what `FormatNumberIntoBuffer`'s report already covers this round; this
one matched cleanly on the first attempt once the stub's stale `gp_rel`
verdict was set aside.
