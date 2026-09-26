# TextEntry__RemoveAllChildren -- MATCHED (17/17 words)

> Renamed from `Obj86ED0__RemoveAllChildren` on 2026-09-26 (tools/rename.py). Address 0x80050e34.

> Renamed from `func_80050E34` on 2026-09-24 (tools/rename.py). Address 0x80050e34.

Unit `class_3bb8c_i`, carved round 14.

`Obj86ED0`'s removeAllChildren override (vtable slot 0x018): zeroes the
same three fields `TextEntry__ClearChildRefs` zeroes, then dispatches the BASE class's
own `removeAllChildren`. The three `sw zero` stores land BEFORE the `jal`
in program order even though one of them physically sits in the branch/call
delay slot -- that is pure instruction scheduling, not a source-order
question.

```c
void TextEntry__RemoveAllChildren(Obj86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk48 = NULL;
    Get_vtable_BasicClass()->removeAllChildren(self);
}
```

First attempt, straight transcription, matched immediately.

## Naming

- `TextEntry__RemoveAllChildren` -- tier A. gTextEntryMethods +0x018 (classtable.py), overrides BasicClass's removeAllChildren: zeroes the same three fields TextEntry__ClearChildRefs zeroes, then dispatches the base class's removeAllChildren.
