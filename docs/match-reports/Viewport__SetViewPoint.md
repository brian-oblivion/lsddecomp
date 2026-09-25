# Viewport__SetViewPoint — MATCHED

> Renamed from `Unk18Obj__SetViewPos` on 2026-09-25 (tools/rename.py). Address 0x8003ebc4.

> Renamed from `func_8003EBC4` on 2026-09-23 (tools/rename.py). Address 0x8003ebc4.

Unit: `code_2cc8c_d`. Round 14, runner delta. 13/13 words, full match.

## Signature

```c
void Viewport__SetViewPoint(Unk18Obj *self, Vec3_2cc8c *a1);
```

`Unk18ObjMethods`'s own `+0x078` slot occupant (`Viewport__AttachViewChild`, this
round, is the only caller found so far, dispatching through the same
slot).

## What it does

Copies `a1` wholesale into `self->unk14`, but only when `self->unk10` is
set.

```c
void Viewport__SetViewPoint(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk14 = *a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`:
- New `Vec3_2cc8c` type (`{ s32 x, y, z; }`, local view — same shape as
  `code_d294.h`'s own `Vec3_d294`, not unified per this project's
  convention).
- `Unk18Obj::unk14` **retyped** from an opaque `u8[0x030-0x014]` span
  (added earlier this round by `Viewport__AttachViewChild`, which only ever took its
  address) to `Vec3_2cc8c unk14` — this function's own three plain word
  stores at `+0x14`/`+0x18`/`+0x1C` confirm the shape. Checked
  `Viewport__AttachViewChild`'s own use (`func_8003F2AC(self->unk14)`, address-only)
  still compiles and matches under the retype — updated to
  `&self->unk14` since a struct value no longer decays to a pointer —
  and rebuilt the whole image to confirm both functions are still
  byte-exact.
- `Unk18ObjMethods::slot78` retyped from `void *` to `Vec3_2cc8c *` to
  match its own occupant's real signature.

## Naming

`Unk18Obj__SetViewPos` -- tier B. Copies `a1` wholesale into `self->unk14`, guarded by `self->unk10`. The ONLY other reader of `unk14` in this unit is `Viewport__Update`, which hands `&self->unk14` straight to Sony's `GsSetRefView2` (a GPU reference-viewpoint setter) -- a real, identified consumer, which is why this crosses from a bare `SetUnk14` into a purpose-carrying name, but still tier B (the consumer establishes 'some kind of view position', not the field's full meaning).
