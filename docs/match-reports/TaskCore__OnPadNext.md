# TaskCore__OnPadNext — MATCH (27/27 words)

> Renamed from `Obj86B60__func_8003C9B0` on 2026-09-25 (tools/rename.py). Address 0x8003c9b0.

> Renamed from `func_8003C9B0` on 2026-09-24 (tools/rename.py). Address 0x8003c9b0.

**Unit:** code_2cc8c · **Size:** 27 instructions

## What it does

Structurally identical to `TaskCore__OnPadPrev` (see that report for the residue
this shape avoids), with different slots:

```c
void TaskCore__OnPadNext(Obj86B60 *self, s32 a1)
{
    void (*handler)(Obj86B60 *self);

    if (self->unk4C == NULL) {
        return;
    }
    if (self->unk3C == 1) {
        handler = self->methods->slotE8;
    } else if (self->unk3C == 2) {
        handler = self->methods->slot114;
    } else {
        return;
    }
    handler(self);
}
```

Matched first attempt, applying `TaskCore__OnPadPrev`'s lesson (shared-call-site
local variable, not inlined per-branch calls) directly.

## Struct knowledge established

- `Obj86B60Methods::slotE8` (+0x0E8, external `TaskCore__FindNextFreeSlot`) and
  `::slot114` (+0x114, external `TaskCore__AdvanceSlotCursor`) -- both `void (*)(Obj86B60*)`.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.

## Naming (round 78, delta)

**Tier C.** `func_8003C9B0` -> `TaskCore__OnPadNext`. Message-0x13
handler (slot84, corrected occupant). Body: when `self->unk4C` is set, picks
`self->methods->slotE8` (if `unk3C==1`) or `self->methods->slot114` (if
`unk3C==2`) and calls it -- the mirror pair of `TaskCore__OnPadPrev`
above. Tier C for the same reason.
