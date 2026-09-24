# Obj86ED0__DispatchIndexValue — MATCHED (round 45, 41/41 words)

> Renamed from `func_800518F4` on 2026-09-24 (tools/rename.py). Address 0x800518f4.

**Unit:** class_3bb8c_j · **Size:** 41 words (0xA4 bytes)

Filed as a `gp_rel`-blocked stub in round 15. That blocker was RESOLVED in
round 42 (`--gp-symbols`, pinned in the Makefile). Matched on the first
attempt once rebuilt against the fixed toolchain.

## Derivation

```c
typedef struct Unk40Obj866E8 Unk40Obj866E8;
typedef struct Unk40Obj866E8Methods Unk40Obj866E8Methods;
struct Unk40Obj866E8Methods {
    u8 pad000[0x0BC];
    void (*slotBC)(Unk40Obj866E8 *self, void *arg1); /* +0x0BC */
};
struct Unk40Obj866E8 {
    Unk40Obj866E8Methods *methods; /* +0x000 */
};

typedef struct {
    s32 unk0;
    s32 unk4;
} SlotBCArg866E8_3bb8c_j;

extern s32 D_8008AADC;
extern s32 D_8008AAE0;

void Obj86ED0__DispatchIndexValue(Obj866E8 *self, s32 arg1, s32 arg2)
{
    SlotBCArg866E8_3bb8c_j local;
    Unk40Obj866E8 *obj;

    if (self->unk48) {
        local.unk4 = D_8008AAE0;
        local.unk0 = arg1 * 7 + D_8008AADC;
        obj = (Unk40Obj866E8 *)self->unk40;
        obj->methods->slotBC(obj, &local);
        self->unk18 = arg1;
        if (arg2) {
            self->methods->slot60(self, 0);
        }
    }
}
```

Part of the same `Obj866E8` "countdown/flush" group established in round 15
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

**Header additions** (`include/class_3bb8c.h`, both additive, next to
existing `Obj866E8`/`Obj866E8Methods` fields):
- `Obj866E8::unk28` (`u8 *`, offset 0x28) — established by this round's
  sibling `Obj86ED0__DispatchLookupValue`, see that report.
- `Obj866E8::unk40`, `Obj866E8::unk44` (`void *`, offsets 0x40/0x44) — kept
  opaque in the shared header since only this unit's own local method-table
  views (`Unk40Obj866E8Methods` here, `Unk44Obj866E8Methods` in
  `Obj86ED0__DispatchLookupValue`) dispatch through them.
- `Obj866E8Methods::slot60` (offset 0x60) — the notify callback both this
  function and `Obj86ED0__DispatchLookupValue` call with `(self, 0)`.

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
