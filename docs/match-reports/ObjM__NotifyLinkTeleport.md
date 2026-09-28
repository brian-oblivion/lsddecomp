# ObjM__NotifyLinkTeleport

> Renamed from `ObjM__NotifyParentsCodeB` on 2026-09-28 (tools/rename.py). Address 0x80053e84.

> Renamed from `func_80053E84` on 2026-09-23 (tools/rename.py). Address 0x80053e84.

**Unit:** dream_scene · **Size:** 12 instructions · **Status:** MATCHED (12/12 words)

## What this function does

One-line dispatch: `self->methods->slot30(self, 0xB)`. No frame variable
needed for `self` — the function has no other calls, so `$a0` is used
directly throughout rather than being saved to `$s0`.

## The C

```c
void ObjM__NotifyLinkTeleport(ObjM *self) {
    self->methods->slot30(self, 0xB);
}
```

This establishes `ObjMMethods::slot30(ObjM*, s32)`, later reused by
`ObjM__OnFadeNotify`, `ObjM__CloseAndNotifyNewGame` and `ObjM__CloseAndNotify` (this round, same
slot, different literal arguments each time).

## Residue

None — matched on the first attempt.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `dream_scene`.

## Naming

**ObjM__NotifyLinkTeleport** -- tier A. Pure one-line dispatch: `self->methods->notifyParents(self, 0xB)`. `notifyParents` is CONFIRMED as `BasicClass__NotifyParents` via `tools/classtable.py 0x80087034` (+0x030), so every byte of this function's behaviour is known even though the game-level meaning of event code 0xB is not -- a pure leaf whose mechanics ARE its purpose, tier A by the FINISHING-PLAN definition.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the dream_scene/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and dream_day.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Track 7 (2026-09-27, round 98, delta)

`OBJM_NOTIFY_LINK_TELEPORT` for 0xB: OnDreamSysNotify calls this slot for
DREAMSYS_LINK_TELEPORT (17 = 11 + 6). The state is not changed, and
DayTask__OnObjMNotify has no case for 11. The method name is left
(its slot, `notifyParentsCodeB`, is read in dream_scene); see the unit's
proposals. Zero bytes.
