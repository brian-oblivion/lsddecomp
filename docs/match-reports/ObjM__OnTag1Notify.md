# ObjM__OnTag1Notify

> Renamed from `ObjM__OnSelectTransfer` on 2026-09-26 (tools/rename.py). Address 0x800531a0.

> Renamed from `func_800531A0` on 2026-09-24 (tools/rename.py). Address 0x800531a0.

**Unit:** class_3bb8c_k · **Size:** 11 words (0x2C bytes) ·
**Status: MATCHED 11/11**, whole-image SHA1 green.

## What it does

```c
void ObjM__OnTag1Notify(Obj87034_3bb8c_l *self, void *arg1, s32 sel) {
    if (sel == 2) {
        ObjM__PollTimBlockLoad(self, self->unk58);
    }
}
```

`arg1` (the function's own second parameter) is never read in this body —
only `sel` (compared against the literal 2) and `self->unk58` (forwarded as
`ObjM__PollTimBlockLoad`'s second argument) are used. Kept as a real parameter
anyway since the caller passes three arguments at this call site.

## Notes

- `self->unk58` typed as `Obj87034_3bb8c_l *` (not `void *`) precisely
  because it's forwarded into `ObjM__PollTimBlockLoad`'s `other` parameter, which
  the callee (below) dereferences with the same offsets as `self` itself
  (`0x0`, `0x3C`, `0x50`, `0x60`... etc) — first evidence tying `unk58`'s
  pointee to the same class as `self`.
- `ObjM__PollTimBlockLoad` needed a forward declaration in the header (ROM order:
  this function is defined before it).

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_800531A0` | `ObjM__OnTag1Notify` | B | see below |

**Evidence.** vtable slot +0x054. A thin selector: forwards to `ObjM__PollTimBlockLoad` only when `sel == 2`, otherwise a no-op. The meaning of the other `sel` values is not established from this body alone.


## Track 4 (2026-09-26, round 89, echo)

Renamed from `ObjM__OnSelectTransfer` (rename.py): it occupies IntermediateBase's `onTag1Notify` (+0x054) and does only what that slot's event 2 asks, running `ObjM__PollTimBlockLoad(self, self->timBlockSrc)`. Tier A for the mechanics. The argument is the TimBlockSrc (below), not another ObjM, so "transfer" is gone.
