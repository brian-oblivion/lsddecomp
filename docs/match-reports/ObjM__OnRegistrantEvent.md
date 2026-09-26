# ObjM__OnRegistrantEvent

> Renamed from `func_80052E7C` on 2026-09-24 (tools/rename.py). Address 0x80052e7c.

**Unit:** class_3bb8c_l · **Size:** 16 words (0x40 bytes) ·
**Status: MATCHED 16/16**, whole-image SHA1 green.

## What it does

The callback registered by `ObjM__AttachTarget` (address-taken there). Dispatches
on `code` to one of two still-uncarved helpers (`asm/psyq_memset.s`, despite
the filename these are not Psy-Q library code).

```c
void ObjM__OnRegistrantEvent(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3) {
    if (code >= 0) {
        GetGridRecordAt(self->unk38, code);
    } else {
        GetGridRecordXY(self->unk38, arg2, arg3);
    }
}
```

## Notes

- **Branch polarity is not free to choose even when semantically
  equivalent.** The natural-reading translation `if (code < 0) { A } else
  { B }` compiles to a `bgez` test; retail's actual instruction is `bltz`
  at the SAME branch target. Writing the mathematically-equivalent `if
  (code >= 0) { B-was-A } else { A-was-B }` (i.e. swap which literal
  comparison is written, keep the target/fallthrough content mapped to
  the same branch) reproduces retail's exact `bltz` byte-for-byte. GCC
  2.6.3 consistently compiles `if (C) THEN else ELSE` as: machine test =
  NOT(written C), THEN-body placed at the FALLTHROUGH, ELSE-body placed at
  the branch TARGET. To match a specific target/fallthrough placement,
  solve for the written `C` that makes `NOT(C)` equal the actual machine
  test, don't guess from the semantics alone.
- `GetGridRecordAt(index, sub)` (now matched, code_39094.c) reads both `$a0`
  and `$a1`, and this function writes nothing to `$a1` before the jal, so
  the value the callee uses as `sub` is `code`. Until round 82 this call was
  written with one argument against a K&R declaration, reading `code` as a
  leftover register. Round 82's externcheck pass (alpha) made the forwarding
  explicit -- `GetGridRecordAt(self->unk38, code)` against a full prototype
  in `include/class_3bb8c.h` -- byte-identical, and it reads as what the
  dispatch means: a non-negative code is a linear cell index, a negative one
  hands x/y to GetGridRecordXY.

### Proposed learning

Confirms (third instance, after `ObjM__OnRegistrantEvent`'s own sibling case in
`ObjM__PollTimBlockLoad` and one more below) that GCC 2.6.3's `if`/`else` codegen
convention is completely mechanical: `NOT(written condition)` is always the
compiled test, and the `if`-body always lands at the fallthrough. When a
residue is "right content, wrong branch instruction and swapped
target/fallthrough", don't hunt for a different algorithm — just invert
the written comparison and swap the two bodies.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80052E7C` | `ObjM__OnRegistrantEvent` | B | see below |

**Evidence.** The callback address-taken by `ObjM__AttachTarget` and handed to the registrant's `slotC8`. Matches `RegistrantMethods_3bb8c_l::slotC8`'s callback shape exactly. Dispatches on the sign of `code` to one of two still-uncarved helpers (`asm/psyq_memset.s`); their own purpose is unknown, so this stays tier B.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical. It is the Class866E8's value callback (ChunkFileFn, cast at ObjM__AttachTarget); Class866E8's ComputeRateEntry keeps the callback's return as an entry's name, and this body leaves GetGridRecordAt's result in $v0, so its real return is likely that Rec1C pointer. The return type is left `void` (not changed this round). The first argument of GetGridRecordAt/XY is now `s32 index`, as their definitions take it.
