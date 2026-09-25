# TaskCore__func_8003C7F4 — MATCH (25/25 words)

> Renamed from `Obj86B60__func_8003C7F4` on 2026-09-25 (tools/rename.py). Address 0x8003c7f4.

> Renamed from `func_8003C7F4` on 2026-09-24 (tools/rename.py). Address 0x8003c7f4.

**Unit:** code_2cc8c · **Size:** 25 instructions

## What it does

```c
void TaskCore__func_8003C7F4(Obj86B60 *self, s32 a1)
{
    if (self->unk4C != NULL) {
        self->methods->slot70(self, 0x10);
        self->methods->slot60(self, 0xA);
    }
}
```

`self->unk4C` is used ONLY as a null/non-null gate here (never dereferenced)
-- the first of five functions in this unit
(`TaskCore__func_8003C7F4`/`TaskCore__OnPadConfirm`/`TaskCore__OnPadCancel`/`TaskCore__OnPadPrev`/
`TaskCore__OnPadNext`) that share this exact gate-then-forward shape, all reached
from the same message dispatcher (`TaskCore__OnPadEvent`, STALL -- see its report)
via consecutive vtable slots `+0x074`..`+0x084`. `slot70` IS this unit's own
`TaskCore__PlaySound`; `slot60` is external (`Class86B60__SetState`).

The parameter `a1` is passed through the caller's own `a1` register into
these two `slotNN` calls unmodified but is otherwise unused by THIS
function's own body -- confirmed a genuine argument (not a stray unused
value) because the sibling `TaskCore__OnPadEvent` dispatcher and `TaskCore__Update`
both forward a live `a1` into this slot, and every other function in the
family (C858/C8D0/C944/C9B0) shares the same `(self, a1)` vtable-slot
signature even where their own bodies also never read `a1`.

## Struct knowledge established

- `Obj86B60::unk4C` (`Unk4CObj *`, +0x04C) -- gate-only use here; fully
  dereferenced by `TaskCore__Tick` (see that report) and `TaskCore__SetState`
  (STALL, read off the disassembly only).
- `Obj86B60Methods::slot70` (+0x070) -- IS `TaskCore__PlaySound`.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c.

## Naming (round 78, delta)

**Tier C** (`Class__func_xxxxx` -- class established via slot occupancy,
purpose not). `func_8003C7F4` -> `TaskCore__func_8003C7F4`. One of the five
message handlers `TaskCore__OnPadEvent` dispatches to (message code 0x21,
slot74 -- see `include/code_2cc8c.h`'s round-78 correction of this slot's
occupant, which was previously listed reversed). Body: when `self->unk4C` is
set, calls `slot70(self, 0x10)` then `slot60(self, 0xA)` -- a conditional
child-forward followed by a state transition. Same shape as the other four
siblings in this dispatch group with no independent evidence distinguishing
what message 0x21 specifically represents, so kept at the tier-C
class-scoped form rather than guessing.
