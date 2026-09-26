# TextEntry__AttachTarget -- MATCHED (28/28 words)

> Renamed from `Obj86ED0__AttachTarget` on 2026-09-26 (tools/rename.py). Address 0x80051200.

> Renamed from `func_80051200` on 2026-09-24 (tools/rename.py). Address 0x80051200.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s vtable slot 0x04C: adds two children via `self->methods->
addChild` (the class's OWN overridden addChild, `TextEntry__AddChild`, reached
through `self->methods` rather than the explicit base-table call this
time), stashes a third pointer verbatim into `unk3C` (typed `TargetObj86ED0
*`, the same opaque type `TextEntry__NotifyTarget` dispatches through), and resets
two counters.

```c
void TextEntry__AttachTarget(Obj86ED0 *self, void *arg1, void *arg2, TargetObj86ED0 *arg3)
{
    self->methods->addChild(self, arg1);
    self->methods->addChild(self, arg2);
    self->unk3C = arg3;
    self->unk2C = 0;
    self->unk20 = 0;
}
```

First attempt, transcribed directly (order matters: both `addChild` calls
before any of the three stores), matched immediately once the earlier
in-unit drift (from `TextEntry__OnNotify`/`TextEntry__SetText`, fixed first) was
resolved -- this function's own C never changed.

## Naming

- `TextEntry__AttachTarget` -- tier B. gTextEntryMethods +0x04C (classtable.py). Adds two children via self->methods->addChild, stores a third pointer as `target`, resets closeState/unk20. Mechanics clear (bind two tagged children plus a dispatch target); the game-level reason this bundle is attached together is not established.
