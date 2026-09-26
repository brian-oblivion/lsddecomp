# Class86B60__SetState -- MATCH

> Renamed from `func_8004D90C` on 2026-09-24 (tools/rename.py). Address 0x8004d90c.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86B60__SetState`: 50/50 words match.

## Source

```c
void Class86B60__SetState(Class86B60 *self, s32 arg1)
{
    Get_vtable_TaskCore()->slot60(self, arg1);
    if (arg1 == 5) {
        self->methods->slot124(self, 0);
    }
    if (arg1 == 0xA) {
        self->methods->slot7C(self);
        self->methods->slotF0(self, self->unk4C->unk8, 1);
        self->methods->slot78(self);
    }
}
```

## Derivation

First attempt, byte-exact. The one thing worth writing down: the two
comparisons against `arg1` (`== 5` and `== 0xA`) both dead-end back to a
shared fall-through, and retail re-materializes the literal `0xA` TWICE --
once in the first branch's delay slot (unconditionally, since it is
needed for the second comparison whichever way the first one goes) and
again right after the intervening `slot124` call clobbers `$v0`. Reading
this as an assignment (`arg1 = 0xA;`) would have been a plausible but
wrong transcription of the lowering; it is two independent, SEQUENTIAL
`if`s on the ORIGINAL `arg1`, not one `if`/reassignment. Writing the two
guards as two plain `if (arg1 == N)` statements reproduced both
re-materializations for free, since the compiler naturally reloads the
constant after the call in either reading.

## Struct changes (additive, `include/class_3bb8c.h`)

- `Class86B60Methods`: four new slots, `slot78`/`slot7C` (both `self`
  only) inserted between `slot6C` and `slotD4`, and `slotF0`/`slot124`
  inserted between `slotD8` and `slot138`. `slotF0` here is `Class86B60`'s
  OWN vtable slot, distinct from `DreamSysViewMethods_3bb8c_c::slotF0`
  established by `Class86B60__Reset`'s report -- same offset number on two
  unrelated tables, not a conflict.
- New type `Class86B60Unk4CObj_3bb8c_d` (self->unk4C's pointee, only
  `unk8` reached, an opaque value forwarded verbatim).
- `Class86B60::unk4C` -- new field, carved from the `pad04C` gap (now
  `pad050`).

### Proposed learning

None new -- the "delay-slot instruction belongs to the taken path" and
"redundant constant re-materialization across branches" families already
cover this; filing as a confirming instance rather than a new bullet.

## Naming (round 77, naming runner delta)

Renamed `func_8004D90C` -> `Class86B60__SetState`. **Tier B**: Forwards `arg1` to the base class's own state-setter (`Get_vtable_TaskCore()->slot60(self, arg1)`) and does extra dispatch for two literal values (5, 0xA) -- the same "forward-then-special-case" shape already named `SetState` for `TaskObjF` (`class_3bb8c_g.c`). Purpose of the two particular state values not established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
