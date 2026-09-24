# ObjM__ExitSceneStyle

> Renamed from `func_800536B0` on 2026-09-24 (tools/rename.py). Address 0x800536b0.

**Unit:** class_3bb8c_l · **Size:** 45 words (0xB4 bytes) ·
**Status: MATCHED 45/45**, whole-image SHA1 green.

## What it does

```c
void ObjM__ExitSceneStyle(Obj87034_3bb8c_l *self) {
    self->methods->slotD4(self);
    self->unk3C->methods->slotFC(self->unk3C);
    self->unk3C->methods->slot50(self->unk3C);
    self->unk18->methods->slot74(self->unk18);
    self->methods->slot14(self, self->unk14);
}
```

Straight-line, five sequential vtable dispatches on four different
objects/views (`self`, `self->unk3C`, `self->unk18`, plus `self->unk14`
passed as a plain argument). Matched on the first attempt.

## Notes

- This function is the primary evidence tying `self->unk3C` AND
  `self->unk18` to the SAME class (both dispatch through offsets that
  independently cross-check against `DREAMSYS_METHODS`, `0x80087BDC` —
  `+0x0FC`/`+0x050` on `unk3C`, `+0x074` on `unk18`). See the
  `DreamSysObj_3bb8c_l` header comment in `include/class_3bb8c.h`.
- `self->unk14` is a DIFFERENT, unidentified class (`Obj14_3bb8c_l`) —
  offset `+0x10C` happens to also exist in `DREAMSYS_METHODS` but with a
  different arity at its own call site (`func_8005393C`, see that report),
  so it is NOT assumed to be DreamSys despite the coincidental offset
  match.
