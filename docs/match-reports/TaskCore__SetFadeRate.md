# TaskCore__SetFadeRate — MATCH (2/2 words)

> Renamed from `Obj86B60__SetFadeRate` on 2026-09-25 (tools/rename.py). Address 0x8003cbb8.

> Renamed from `func_8003CBB8` on 2026-09-24 (tools/rename.py). Address 0x8003cbb8.

**Unit:** code_2cc8c · **Size:** 2 instructions

## What it does

```c
void TaskCore__SetFadeRate(Obj86B60 *self, s32 a1)
{
    self->unk84 = a1;
}
```

The unit's smallest real function (a single `sw`, plus its `jr $ra` delay
slot). Confirms `Obj86B60::unk84` (already established as a multiplier by
`TaskCore__TickColorFade`) is a plain setter target, not computed internally.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c. 1 attempt.

## Naming (round 78, delta)

**Tier A** (pure setter). `func_8003CBB8` -> `TaskCore__SetFadeRate`. Body:
`self->unk84 = a1;`, nothing else. Corroborated by `TaskCore__TickColorFade`,
the sole reader of `unk84`, which multiplies it against `frameCounter` to
build the per-tick colour delta -- exactly what a "rate" describes
mechanically, without asserting what the fade itself means in the game.
