# StreamTaskObj__func_8003BB5C

> Renamed from `func_8003BB5C` on 2026-09-23 (tools/rename.py). Address 0x8003bb5c.

**Unit:** code_2c054 · **Size:** 46 words · **Status:** MATCHED (46/46)

## Summary

Three sequential early-return guards.

```c
void StreamTaskObj__func_8003BB5C(StreamTaskObj *self, s32 a1, s32 a2) {
    Get_vtable_TaskCore()->slot5C(self, a1, a2);
    if (self->unkA4 != 0) {
        return;
    }
    self->unkA4 = self->unkB4->methods->slot48(self->unkB4);
    if (self->unkA4 == 0) {
        return;
    }
    if (self->unkD8 != 0) {
        return;
    }
    self->methods->slot60(self, 7);
}
```

## Evidence

- `Get_vtable_TaskCore()->slot5C`: `TaskCoreMethods` slot `+0x05C`, occupied by
  `TaskCore__Update` (a different unit, not touched here). Takes `(self, a1,
  a2)` matching this function's own two forwarded parameters; result
  discarded, typed `void`.
- `self->unkA4`: established this round (`StreamTaskObj__func_8003BAB4`'s report). Here it
  is both read (first guard) and **assigned** from `slot48`'s return, unlike
  `StreamTaskObj__func_8003BAB4` where the analogous `slot40` result is tested but never
  stored — confirmed the two call sites genuinely differ, see that report's
  "pitfall" section.
- `self->unkB4->methods->slot48(self->unkB4)`: single-argument call on
  `StreamTaskUnkB4Methods`; the disassembly stores `$v0` into `self->unkA4`
  in the branch's own delay slot (`sw $v0, 0xA4($s2)` right after `beqz $v0,
  ...`), so it is `s32`-returning.
- New field `self->unkD8` (`+0x0D8`, `s32`) — the last word before the
  object's documented 0xDC size, read-only here.

## Proposed learning

Matched cleanly on the first attempt once `StreamTaskObj__func_8003BAB4`'s slot40/slot48
distinction (tested-not-stored vs. stored) was already sorted out — this is
the confirming positive case for that report's "reread, don't backfill by
analogy" note.

## Naming

**StreamTaskObj__func_8003BB5C** -- tier C. Occupies `gStreamTaskObjMethods`
slot `+0x05C`; three sequential early-return guards around a cached status
field (`unkA4`) and a completion flag (`unkD8`). Mechanics are fully
described in the report above; nothing pins down what the guarded operation
actually represents in the game, so left `Class__func_xxxxx`.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
