# SceneNode__SetUseZ -- MATCHED (12/12 words)

> Renamed from `SceneNode__GetSetUnk10Flag7` on 2026-09-26 (tools/rename.py). Address 0x8001d450.

> Renamed from `Class6B5CC__GetSetUnk10Flag7` on 2026-09-26 (tools/rename.py). Address 0x8001d450.

> Renamed from `func_8001D450` on 2026-09-18 (tools/rename.py). Address 0x8001d450.

Round 12, runner delta. `code_d294_b`.

## Summary

Sibling of `SceneNode__SetDisplay` (the ONLY one of the five already-matched
`self->unk10` bitfield accessors that both converts its input to a boolean
(`a1 == 0`) AND inverts its own result (`== 0`) -- every other sibling does
at most one of those). This function does exactly that double-inversion, at
shift 7 width 1, so it takes the same `s32` return type as `SceneNode__SetDisplay`
rather than the plain `u32` of the other three siblings.

```c
s32 SceneNode__SetUseZ(SceneNodeObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 7, 1, a1 == 0) == 0;
}
```

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/SceneNode__SetUseZ.s`):
```
sltiu $a3, $a1, 0x1       # a3 (value) = (a1 < 1) = (a1 == 0)
addiu $a0, $a0, 0x10      # a0 = &self->unk10
ori   $a1, $zero, 0x7     # a1 (shift) = 7
jal   GetSetBitField
 ori  $a2, $zero, 0x1     # a2 (width) = 1
sltiu $v0, $v0, 0x1       # result = (raw < 1) = (raw == 0)
```

### Proposed learning

None new -- confirms the double-inversion shape is per-function, not
per-family (this and `SceneNode__SetBackClip`, matched alongside it, are now 2/9
siblings that use it; the other 7 don't).

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D450` via `tools/rename.py`. **Tier A** -- pure
`GetSetBitField` wrapper over `self->unk10`, shift 7 width 1,
double-inverted boolean shape (same as sibling `SceneNode__SetDisplay` in
`code_d294.c`, still unrenamed there). Mechanics are the whole of what
this function does (a getter/setter over a known bit range), which
qualifies as tier A "by definition" per FINISHING-PLAN.md track 3 even
though the FIELD's own game-level meaning (what bit 7 of `unk10`
represents) is not established. Safe to rename directly: the only
references outside this unit are this unit's own `include/code_d294.h`
and this unit's own `SceneNode__SetBackClip.md` report.
