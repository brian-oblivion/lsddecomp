# ObjM__AttachTarget

> Renamed from `func_80052DE8` on 2026-09-24 (tools/rename.py). Address 0x80052de8.

**Unit:** class_3bb8c_l · **Size:** 37 words (0x94 bytes) ·
**Status: MATCHED 37/37**, whole-image SHA1 green.

## What it does

`self`'s class is `Obj87034_3bb8c_l` (vtable `gObjMMethods`, resolved with
`tools/classtable.py 0x80087034` — see the header note in
`include/class_3bb8c.h` for the full derivation). This function registers a
callback with some other object (`arg1->unkC`, an unidentified
"registrant" class reached only here), stashes `arg2` into `self->unk3C`,
forwards to the shared BasicClass-family base accessor `GetClass86668Methods()`'s
slot `+0x44`, then calls its own `slot10`.

```c
void ObjM__AttachTarget(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *arg1, s32 arg2) {
    arg1->unkC->methods->slotC8(arg1->unkC, ObjM__OnRegistrantEvent, self);
    self->unk3C = (DreamSysObj_3bb8c_l *)arg2;
    GetClass86668Methods()->slot44(self, arg1, 1);
    self->methods->slot10(self, arg2);
}
```

## Notes

- `ObjM__OnRegistrantEvent` (this unit, ROM order right after this function) is
  registered here as a callback — needed a forward declaration in the
  header since it's referenced before its own definition.
- The struct field layout work for `Obj87034_3bb8c_l` done here (methods
  at +0, +0xC, +0x3C) anchors the rest of this unit's functions; see
  `include/class_3bb8c.h`'s new section for the full struct.
- **Bug caught during matching:** the very first struct draft put `unkC`
  directly after `methods` with no padding, landing it at offset 0x4
  instead of 0xC. Every subsequent field in the struct was silently
  8 bytes too low until fixed — visible immediately as an 8-byte-off field
  offset in `ObjM__AttachTarget`'s own diff (`0x3c` vs `0x34`) and in
  `ObjM__OnRegistrantEvent`'s (`0x38` vs `0x30`).

### Proposed learning

A missing `u8 padNN[...]` right after the FIRST pointer field in a
freshly-drafted struct is easy to miss because the struct still "looks"
right at a glance — the offset comment on each field is trustworthy, the
declared byte layout is not, until compiled and diffed. Compile after
drafting a struct with more than 2 fields, before writing more than one
function against it.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80052DE8` | `ObjM__AttachTarget` | B | see below |

**Evidence.** vtable slot +0x044 of `gObjMMethods` (`ObjM`, confirmed via classtable.py's ctor/dtor slots). Registers `ObjM__OnRegistrantEvent` as a callback with `arg1->unkC` (a "registrant" object), stores `arg2` into `self->target`, forwards to the shared base accessor's own slot `+0x44`, then dispatches self's own `AddChild` (slot10). Mechanics -- subscribe + link a target + add a child -- are clear; the in-game reason is not.
