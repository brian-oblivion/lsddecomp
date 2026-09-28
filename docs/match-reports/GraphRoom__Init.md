# GraphRoom__Init -- MATCHED (29/29)

> Renamed from `GraphRoomObj__func_80058390` on 2026-09-26 (tools/rename.py). Address 0x80058390.

> Renamed from `func_80058390` on 2026-09-24 (tools/rename.py). Address 0x80058390.

Unit: `src/world/dream_scene.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x044`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
s32 GraphRoom__Init(D_80087AACObj *self, void *arg1, void *arg2);
```

## Body

```c
s32 GraphRoom__Init(D_80087AACObj *self, void *arg1, void *arg2) {
    s32 result;
    GetTaskCoreMethods()->slot44(self, arg1, arg2);
    result = 2;
    if (self->unk_0x238 == 0) {
        result = self->unk_0x38;
    }
    return result;
}
```

Calls the shared base-class table's own `+0x044` slot (`GetTaskCoreMethods()`,
this unit's own local view -- a plain no-argument getter established
elsewhere, e.g. `include/task.h`), then returns `self->unk_0x38` if
`self->unk_0x238 == 0`, else the literal `2`.

## Shape note

`s32 result = 2;` declared BEFORE the `GetTaskCoreMethods()->slot44(...)`
call scored 26/29 -- retail reloads a fresh `v1` after the call rather
than keeping `2` live across it in a callee-saved register. Assigning
`result = 2;` AFTER the call (same value, same variable, just moved past
the call) matched exactly. Same family as the existing "assign the
default AFTER the intervening call" learning in
`docs/DECOMPILATION_LEARNINGS.md`, confirmed again here for a plain
integer literal default (that learning's own noted inverse -- "the same
lever applied to a compile-time LITERAL... makes things worse" --
refers to a DIFFERENT function's shape; it did not reproduce here).

Names `D_80087AACObj::unk_0x38` (additive split of the class struct's pad).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoom__Init   # 29/29
```

## Naming (round 75, track 3)

**`GraphRoom__Init`** -- tier C. Own vtable slot +0x044
(`tools/classtable.py gGraphRoomMethods`), so the `Class__func_xxxxx` form
applies now that the class is named. Body: chains through the base
class's own +0x044 slot, then returns `unk_0x38` if `scored == 0`, else
the literal `2`. No caller in this unit and no field/return semantics
strong enough to name past that -- kept `func_`.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__func_80058390` -> `GraphRoom__Init`

The class is unified in `include/graph_room.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Named for its slot, track 4 step 6: +0x044 is IntermediateBase's `init` (TaskCore__Init in the parent). It calls TaskCore's init with (args, mode), now typed `IntermediateBaseInitArgs *` and `s32`, and returns 2 when `scored` is set, else TaskCore's `result` (+0x038, was unk_0x38).
