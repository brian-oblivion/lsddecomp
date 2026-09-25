# TaskCore__OnPadCancel — MATCH (29/29 words)

> Renamed from `Obj86B60__func_8003C8D0` on 2026-09-25 (tools/rename.py). Address 0x8003c8d0.

> Renamed from `func_8003C8D0` on 2026-09-24 (tools/rename.py). Address 0x8003c8d0.

**Unit:** code_2cc8c · **Size:** 29 instructions

## What it does

```c
void TaskCore__OnPadCancel(Obj86B60 *self, s32 a1)
{
    if (self->unk4C != NULL && self->unk3C != 1) {
        self->methods->slot70(self, 0x10);
        self->methods->slot60(self, 0x11);
    }
}
```

Same gate-then-forward family; the gate is compound (`unk4C != NULL &&
unk3C != 1` -- both a null check on `unk4C` and an exclusion on `unk3C`
route to the SAME skip target in retail, so a plain `&&` reproduces it
directly). Reason code `0x11` is a fixed literal, no branch needed.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.

## Naming (round 78, delta)

**Tier C.** `func_8003C8D0` -> `TaskCore__OnPadCancel`. Message-0x17
handler (slot7C -- the one slot in this group of five whose occupant the old,
reversed header comment happened to get right, since it is its own mirror
image). Body: when `self->unk4C` is set AND `self->unk3C != 1`, calls
`slot70(self, 0x10)` then `slot60(self, 0x11)`. Same shape as siblings, no
independent purpose evidence. Tier C.
