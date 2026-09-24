# Class866E8__RefreshFootprint — MATCHED (34/34 words)

> Renamed from `func_8004C620` on 2026-09-24 (tools/rename.py). Address 0x8004c620.

Dispatcher: gated by `self->unk1B8`, doubles `self->unk78` into an index,
calls `func_8004CE24(self, 0)` unconditionally, then branches on
`self->unk68->unk4` between `Class866E8__ComputeFootprintFromRotation` and `func_8004CC74`, finishing
with `func_8004CE24(self, 1)`.

## New struct knowledge (`include/class_3bb8c.h`)

Two new fields carved out of the previously-opaque `pad74[0xBC-0x74]`
padding in `struct Obj866E8`:

- `unk78` (`s16`, +0x078) — read here, doubled and passed on as an index.
- `unk7A` (`s16`, +0x07A) — read here, passed straight through as an arg.

`pad74` was split into `pad74[0x78-0x74]` + `unk78` + `unk7A` +
`pad7C[0xBC-0x7C]`; total padding covered is unchanged (still 0x48 bytes),
so `Obj866E8`'s size and every later field's offset are untouched.

`Obj866E8::unk68->unk4` (already-typed field of the existing `Unk68Struct`)
is read here too, no header change needed for that one.

## Final C

```c
/* Forward declarations: all three are defined later in this file (in ROM
 * order, after Class866E8__RefreshFootprint), but Class866E8__RefreshFootprint calls them before their
 * own definitions appear. Signatures are typed from the registers loaded
 * at each call site, per this unit's established convention for calling a
 * same-unit function whose body is still INCLUDE_ASM. */
extern void func_8004CE24(Obj866E8 *self, s32 arg1);
extern void func_8004CC74(Obj866E8 *self);
extern void Class866E8__ComputeFootprintFromRotation(Obj866E8 *self, s32 arg1, s32 arg2);

void Class866E8__RefreshFootprint(Obj866E8 *self) {
    s32 idx;

    if (self->unk1B8 == 0) {
        return;
    }
    idx = self->unk78 * 2;
    func_8004CE24(self, 0);
    if (self->unk68->unk4 == 0) {
        Class866E8__ComputeFootprintFromRotation(self, idx, self->unk7A);
    } else {
        func_8004CC74(self);
    }
    func_8004CE24(self, 1);
}
```

## Attempts

1. First attempt wrote the natural reading of the branch
   (`if (self->unk68->unk4 != 0) { func_8004CC74(self); } else {
   Class866E8__ComputeFootprintFromRotation(...); }`) — compiled, size matched, but 8/34 words
   differed: retail's branch is a `bnez` jumping FORWARD to the
   `func_8004CC74` call (which sits as an out-of-line target after a `j`
   over it), with `Class866E8__ComputeFootprintFromRotation` as the in-line fallthrough. My version
   produced the mirror image: a `beqz` with `Class866E8__ComputeFootprintFromRotation` as the jump
   target and `func_8004CC74` in-line.
2. Inverting the condition and swapping the two arms
   (`if (self->unk68->unk4 == 0) { Class866E8__ComputeFootprintFromRotation(...); } else {
   func_8004CC74(self); }`, semantically identical) reproduced retail's
   exact branch polarity and instruction layout — 34/34.

### Proposed learning

**For a two-way `if`/`else` with no other differences, GCC 2.6.3 can lay
out either arm as the fallthrough** — which arm becomes fallthrough vs.
out-of-line jump target is NOT determined purely by source order (writing
the "true" arm first does not guarantee it becomes the fallthrough). When
a residue is "correct instructions, but the branch polarity and arm
layout are mirrored" (a `bnez`/`beqz` swap plus the two call blocks
swapped), try inverting the condition and swapping the arm bodies before
looking for anything more exotic. Confirmed here: `if (!c) A else B`
picked the opposite layout from `if (c) B else A` for a byte-identical
body pair.
