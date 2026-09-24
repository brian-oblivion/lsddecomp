# Obj86ED0__ClearChildRefs -- MATCHED (4/4 words)

> Renamed from `func_80050CD8` on 2026-09-24 (tools/rename.py). Address 0x80050cd8.

Unit `class_3bb8c_i`, carved round 14.

Trivial reset helper on `Obj86ED0` (see `include/class_3bb8c.h`): zeroes the
two child-slot fields and the release-gated pointer. Called both from this
class's own (STALLED, gp_rel-blocked) ctor `Obj86ED0__Obj86ED0` and standalone.

```c
void Obj86ED0__ClearChildRefs(Obj86ED0 *self)
{
    self->unk34 = NULL;
    self->unk38 = NULL;
    self->unk48 = NULL;
}
```

First attempt, straight transcription of the three `sw zero` stores in
order, matched immediately.
