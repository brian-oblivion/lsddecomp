# SceneNode__GetModelHull -- MATCHED (9/9 words)

> Renamed from `SceneNode__ReadUnk20Data` on 2026-09-26 (tools/rename.py). Address 0x8001d600.

> Renamed from `Class6B5CC__ReadUnk20Data` on 2026-09-26 (tools/rename.py). Address 0x8001d600.

> Renamed from `func_8001D600` on 2026-09-18 (tools/rename.py). Address 0x8001d600.

Round 12, runner delta. `code_d294_b`.

## Summary

A plain forwarding wrapper: passes `self->unk20` as arg0 and its own 2nd
argument straight through (untouched, still in `$a1` from the caller) to
`TmdModel__GetHull` (Psy-Q, `psyq_GsLinkObject4.s`).

```c
void SceneNode__GetModelHull(SceneNodeObj *self, void *dest) {
    TmdModel__GetHull(self->unk20, dest);
}
```

## Return type: `void`, not `return TmdModel__GetHull(...)`

`SceneNode__GetModelHull` never touches `$v0` after the `jal`, so the byte match alone
doesn't distinguish `void` from a pass-through `return`. Checked
`TmdModel__GetHull`'s own disassembly instead: its last write to `$v0` before
`jr $ra` is leftover from an unrelated `lhu $v0, 0x1E($sp)` a few
instructions earlier (used only to feed the very next `sh` store), not a
value the function computed to hand back to its caller. Read as `void`.

## unk20 retyped

`SceneNodeObj::unk20` was `s32` (a placeholder the round-10/11 header
explicitly flagged for a later carve to retype). This function passes it
straight through as `TmdModel__GetHull`'s own `void *` arg0 (which
`TmdModel__GetHull` forwards unmodified to `TmdModel__ComputeBounds`, which dereferences
it at `+0x10`) -- genuinely a pointer. Retyped to `void *unk20` in
`include/code_d294.h`. The only existing write site, `self->unk20 = 0;` in
`SceneNode__SceneNode` (`src/code_d294.c`), is an integer-constant-zero assignment
and compiles unchanged under the new type (checked: `build exit=0`, full
image SHA1 still green).

## Evidence

Disassembly (`asm/nonmatchings/code_d294_b/SceneNode__GetModelHull.s`):
```
lw  $a0, 0x20($a0)      # a0 = self->unk20
jal TmdModel__GetHull
 nop                     # a1 untouched -- passes through the caller's own arg1
```

### Proposed learning

None beyond the retype itself, which is local to this unit's own header.

## Naming (round 54, bravo, track 3)

Renamed from `func_8001D600` via `tools/rename.py`. **Tier B** -- slot
`+0x08C` occupant. Mechanics fully confirmed (forwards `self->unk20` and
its own 2nd argument straight through to Psy-Q `TmdModel__GetHull`, which
fills the caller's buffer), but `unk20`'s own real content/purpose is
still opaque, so the name describes the READ operation, not what is
actually being read. Purely local to this unit + its header.

## Track 6 (round 91, echo): named `SceneNode__GetModelHull`, tier A

The body is `TmdModel__GetHull(self->model, dest)`: the linked TmdModel's eight-corner hull (TmdHull) into `dest`. Was `ReadUnk20Data` (`unk20` is now `model`). Slot +0x08C kept as `readUnk20Data` (caller in class_3bb8c_o.c); `getModelHull` proposed. The class was renamed Class6B5CC -> SceneNode in the same pass (include/SceneNode.h's banner has the evidence).
