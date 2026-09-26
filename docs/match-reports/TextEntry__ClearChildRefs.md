# TextEntry__ClearChildRefs -- MATCHED (4/4 words)

> Renamed from `Obj86ED0__ClearChildRefs` on 2026-09-26 (tools/rename.py). Address 0x80050cd8.

> Renamed from `func_80050CD8` on 2026-09-24 (tools/rename.py). Address 0x80050cd8.

Unit `class_3bb8c_i`, carved round 14.

Trivial reset helper on `Obj86ED0` (see `include/class_3bb8c.h`): zeroes the
two child-slot fields and the release-gated pointer. Called both from this
class's own (STALLED, gp_rel-blocked) ctor `TextEntry__TextEntry` and standalone.

```c
void TextEntry__ClearChildRefs(Obj86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk48 = NULL;
}
```

First attempt, straight transcription of the three `sw zero` stores in
order, matched immediately.

## Naming

- `TextEntry__ClearChildRefs` -- tier A. Pure leaf: nulls childType2/childType5/unk48. Called once from the ctor to establish the initial (empty) child-tracking state -- mechanics are the purpose, tier A by the pure-leaf rule.
