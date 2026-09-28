# ObjM__EnterState5 -- MATCHED

> Renamed from `func_80053BE8` on 2026-09-24 (tools/rename.py). Address 0x80053be8.

Unit: `src/world/ObjMStyleActor.c`. Runner: echo, round 16.

43/43 words, byte-exact. `./build-and-verify.sh` green (whole-image SHA1 verified).

## Signature

```c
void ObjM__EnterState5(Obj87034_3bb8c_l *self);
```

## Final C

```c
void ObjM__EnterState5(Obj87034_3bb8c_l *self) {
    s32 color;

    if (self->unk3C->unk164 < 0) {
        self->methods->slot9C(self);
    } else {
        self->unk20 = 5;
        color = self->unk3C->methods->slot200(self->unk3C);
        ObjM__StartFadeUp(self, color, 0, 0xA, 1);
        self->unk3C->methods->slotFC(self->unk3C);
    }
}
```

## Notes

- `self->unk3C->unk164` is a NEW field on `DreamSysObj_3bb8c_l` (this unit's
  local, offset-only view of the class likely-DreamSys pointed to by
  `self->unk3C`/`self->unk18`). Read directly as a plain `s32` (not through
  `->methods`), sign-tested. Added to `include/class_3bb8c.h` as
  `s32 unk164` at `+0x164`, additively (padding before it, nothing after)
  since this type has no other fields past `methods` yet and nothing else
  in the tree references `DreamSysObj_3bb8c_l` by name.
- `self->methods->slot9C` is a NEW vtable slot on `Obj87034Methods_3bb8c_l`
  (the class whose table is `gObjMMethods`), a bare `void(Obj87034_3bb8c_l*)`
  dispatch. Added at `+0x09C`, splitting the existing `0x090..0x0C0`
  padding gap additively.
- `ObjM__StartFadeUp` is the sibling-unit helper (`ObjMStyleActor`, matched by
  echo round 15) already forward-declared in this file for
  `ObjM__EnterState6`'s use; that `extern` declaration was moved earlier in the
  file (still unit-local, not the shared header) since `ObjM__EnterState5`
  (ROM-earlier) now needs it too. No behavior change, pure reordering.

## Non-obvious lever: branch/block polarity

m2c's output (`if (var_a0->unk3C->unk164 < 0) { call slot9C } else { ...
unk20=5... }`) got the condition polarity right immediately, but my first
hand-written attempt inverted it (`if (field >= 0) {then-block} else
{slot9C}`) and reached only 18/43 words, with the diff visible starting at
the very first branch instruction: retail emits `bgez` (0x04410006-shape)
where the inverted C produced `bltz` with the two blocks swapped in ROM
order. GCC places the `if`-branch's body as the immediate fallthrough and
the `else`-branch's body after the skip-over jump; retail has the
`unk164 < 0` case (the single dispatch call) as the fallthrough and the
`unk164 >= 0` case (the four-statement block, including the shared-helper
call) as the branch target. Writing the condition in source exactly as
`< 0` with the short call as the `if`-arm and the long block as the
`else`-arm reproduced this exactly. Re-running through m2c first (which
already had the polarity right) would have saved the wasted 18/43 attempt.

### Proposed learning

When a byte-exact m2c seed and a from-scratch hand read of the same
disassembly disagree only in if/else polarity and block order, trust the
seed's polarity — GCC 2.6.3 consistently places the branch-target block
as the source's `else`-arm and the fallthrough as the `if`-arm, and eyeballing
`bgez`/`bltz` cold is easy to get backwards.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053BE8` | `ObjM__EnterState5` | B | see below |

**Evidence.** vtable slot +0x098. Sets `self->phase = 5`; same evidence as `ObjM__EnterState4`.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and DayTaskStageMap.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Round 95 (track 7, echo)

The fade step `0xA` is written 10. Byte-identical.
