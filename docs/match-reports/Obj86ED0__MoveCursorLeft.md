# Obj86ED0__MoveCursorLeft -- MATCH (24/24 words)

> Renamed from `func_800516C0` on 2026-09-24 (tools/rename.py). Address 0x800516c0.

Unit `class_3bb8c_i`. Obj86ED0's own "retreat frame counter, clamped at
zero" method. Mirror pair with `Obj86ED0__MoveCursorRight` (increment/clamp-at-`unk10`,
matched alongside it) -- see that report for the shared shape discussion.

```c
void Obj86ED0__MoveCursorLeft(Obj86ED0 *self)
{
    s32 old;
    s32 v;

    if (self->unk48 != NULL) {
        old = self->unk18;
        v = old - 1;
        self->unk18 = v;
        if (v >= 0) {
            self->methods->slotA4(self, v, 1);
        } else {
            self->unk18 = old;
        }
    }
}
```

Matched on the first attempt with this shape. No header changes (`slotA4`'s
existing 2-arg signature already matched: `self->methods->slotA4(self, v,
1)`, identical call shape to `Obj86ED0__MoveCursorRight`'s).
