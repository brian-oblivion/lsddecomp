# TaskObjF__OnInputEvent -- MATCH

> Renamed from `TaskObjF__OnNotify` on 2026-09-26 (tools/rename.py). Address 0x8004ff90.

> Renamed from `Class86E00_3bb8c_g__OnNotify` on 2026-09-23 (tools/rename.py). Address 0x8004ff90.

> Renamed from `func_8004FF90` on 2026-09-23 (tools/rename.py). Address 0x8004ff90.

Unit `TitleMenuTaskObjF`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__OnInputEvent`: 25/25 words match.

## Source

```c
void TaskObjF__OnInputEvent(Class86E00_3bb8c_g *self, s32 arg1, s32 arg2)
{
    if (self->unk28 != 0) {
        if (arg2 == 0x19) {
            self->methods->slot90(self);
        } else if (arg2 == 0x17) {
            self->methods->slot94(self);
        }
    }
}
```

First attempt, byte-exact. `arg1` (`$a1`) is never read anywhere in the
body -- confirmed genuinely unused, kept as a declared-but-unread
parameter per the established "an unused parameter just needs declaring,
not using" convention.

## Struct changes (additive, `include/class_3bb8c.h`)

- `Class86E00Methods_3bb8c_g::slot90`/`slot94` -- both already declared
  ahead of this function while surveying the whole unit
  (`TaskObjF__ReleaseCardIcon`'s report); this is the function that exercises them.

### Proposed learning

None new.

## Naming

`TaskObjF__OnInputEvent` (was `func_8004FF90`), tier B: signature
shape `(self, arg1, arg2)` with `arg1` unread and `arg2` a small dispatch
code (`0x19`/`0x17`) gated on `self->unk28 != 0` matches this project's
established `(self, sender, event)` notify-receiver shape
(`BasicClass__OnNotify`/`slot38`, `include/code_8220.h`) closely enough to
use the same verb, though this class's own table is not BasicClass's and
`arg1` is never read as a sender here. What the two event codes mean is
not established.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__OnNotify` so that name could go to the +0x038 override. Slot +0x088, which TaskObjF__OnNotify (+0x038) calls for a sender whose class id has low nibble 2 (the child TaskObjF__AddChild keeps in `inputSource`, +0x060). Events 0x19 and 0x17 from it run advanceState and forceIdleFromState.

## Track 7 (2026-09-27, round 95)

Parameter `arg2` -> `event`. 0x19 and 0x17 are `PAD_EVENT_PRESSED +
PAD_BUTTON_RRIGHT` and `PAD_EVENT_PRESSED + PAD_BUTTON_RDOWN` (include/Pad.h:
0x12 + 7, 0x12 + 5): the sender is the class-id-2 child, a Pad, so circle
pressed runs advanceState and cross pressed forceIdleFromState. `state != 0`
is `TASKOBJF_STATE_IDLE`. Image byte-identical.
