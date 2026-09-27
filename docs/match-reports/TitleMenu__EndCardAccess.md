# TitleMenu__EndCardAccess -- MATCH

> Renamed from `TitleMenu__EndMemcardSave` on 2026-09-26 (tools/rename.py). Address 0x8004e054.

> Renamed from `Class86B60__EndMemcardSave` on 2026-09-26 (tools/rename.py). Address 0x8004e054.

> Renamed from `func_8004E054` on 2026-09-24 (tools/rename.py). Address 0x8004e054.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__EndCardAccess`: 36/36 words match.

## Source

```c
void TitleMenu__EndCardAccess(TitleMenu *self)
{
    self->methods->slot10(self, self->handlerTable->unk4);
    self->methods->slot10(self, self->unk10);
    self->methods->slot14(self, self->unkAC);
    self->unkAC->methods->slot70(self->unkAC);
}
```

First attempt, byte-exact. Four straight-line calls in exactly this order,
each re-loading `self->methods` fresh (no caching across the intervening
calls, consistent with the project's established "re-dereference; do not
cache in a named local" idiom).

## Struct changes (additive, `include/class_3bb8c.h`)

- `TitleMenuMethods::slot10` -- new slot, called TWICE in this function
  with two different opaque arguments (`self->handlerTable->unk4`, then
  `self->unk10`).
- `TitleMenuMethods::slot14` -- new slot, `(self, void *arg1)`, called
  with `self->unkAC` forwarded opaquely (not dereferenced by this caller).
- `TitleMenu::unk10` -- new field, `void *`, opaque, carved from the
  `pad010` gap.
- `TitleMenu::unkAC` **retyped** again, from the minimal
  `GenericReleaseObj_3bb8c_d *` (set by `TitleMenu__Finalize`'s report) to a new
  dedicated `TitleMenuUnkACObj_3bb8c_d *` carrying both the shared
  `release` slot at `+0x004` and this function's own `+0x070` slot. Same
  size, no layout change; same shape of retype already done once for
  `nameField` in `TitleMenu__AttachSaveTitle`'s report, for the identical reason (one
  instance needs a second slot the others never reach).

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004E054` -> `TitleMenu__EndCardAccess`. **Tier B**: Mirror-image teardown of `TitleMenu__BeginCardAccess` -- forwards `self->handlerTable->unk4` and `self->unk10` through `self->methods->slot10`/`slot14`, then `unkAC->methods->slot70(unkAC)`. Named as the paired counterpart by symmetry of the two functions' argument sets, not from independent purpose evidence.

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). slot10/slot14 are BasicClass's addChild/removeChild: this undoes BeginMemcardSave's child swap (initArgs->unk4 and unk10 back in, saveCtrl out) before saveCtrl's +0x070. Byte-identical (whole image green, 0 new warnings, nonmatching green).

## Track 7 (round 96, echo)

No change; `unk10` is proposed as `frameClock` (TitleMenu__RefreshMenu's
report).
