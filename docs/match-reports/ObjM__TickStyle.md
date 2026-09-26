# ObjM__TickStyle

> Renamed from `func_8005393C` on 2026-09-24 (tools/rename.py). Address 0x8005393c.

**Unit:** class_3bb8c_l · **Size:** 18 words (0x48 bytes) ·
**Status: MATCHED 18/18**, whole-image SHA1 green.

## What it does

```c
void ObjM__TickStyle(Obj87034_3bb8c_l *self) {
    TickStyle(self->unk14->methods->slot10C(self->unk14, 0, 0), 0, 0);
}
```

Matched on the first attempt. `TickStyle` is still uncarved ground
(`asm/class_3bb8c_n.s`), declared locally as
`extern void TickStyle(void *arg0, void *arg1, s32 arg2);` from this
call site's own register usage.

## Notes

- `self->unk14->methods->slot10C` is the call site that RULES OUT
  `self->unk14` being `DreamSys *` despite offset `+0x10C` existing in
  `DREAMSYS_METHODS` (`DreamSys__SetSoundObj`, `void(DreamSys*, s32)`) — this call
  passes `(self->unk14, 0, 0)`, three total arguments against DreamSys's
  own two. Different arity, different class; `self->unk14`'s pointee is
  left as an unnamed local view (`Obj14_3bb8c_l`) with only this one slot
  and (from `ObjM__PollTimBlockLoad`) one plain `u16` field named.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_8005393C` | `ObjM__TickStyle` | B | see below |

**Evidence.** vtable slot +0x08C. A one-line wrapper: `TickStyle(self->unk14->methods->slot10C(self->unk14, 0, 0), 0, 0)`. Named for the global `TickStyle` it calls, whose own name is already established.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.
