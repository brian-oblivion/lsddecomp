# Viewport__SetViewRef — MATCHED

> Renamed from `Unk18Obj__SetUnk20` on 2026-09-25 (tools/rename.py). Address 0x8003ebf8.

> Renamed from `func_8003EBF8` on 2026-09-23 (tools/rename.py). Address 0x8003ebf8.

Unit: `code_2cc8c_d`. Round 14, runner delta. 13/13 words, full match.

## Signature

```c
void Viewport__SetViewRef(Unk18Obj *self, Vec3_2cc8c *a1);
```

`Unk18ObjMethods`'s own `+0x07C` slot occupant.

## What it does

Sibling of `Viewport__SetViewPoint`: copies `a1` wholesale into `self->unk20`,
guarded by the same `self->unk10` flag.

```c
void Viewport__SetViewRef(Unk18Obj *self, Vec3_2cc8c *a1) {
    if (self->unk10 != NULL) {
        self->unk20 = *a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk20` (`+0x020`, `Vec3_2cc8c`,
immediately after `unk14`, no gap); `Unk18ObjMethods::slot7C` retyped from
`void *` to `Vec3_2cc8c *`.

## Naming

`Unk18Obj__SetUnk20` -- tier A. Sibling of `Viewport__SetViewPoint`: same whole-Vec3-copy shape guarded by `self->unk10`, writes `unk20`. No consumer of `unk20` was found anywhere in this unit (unlike `unk14`), so kept as a plain mechanical name rather than guessing a role.
