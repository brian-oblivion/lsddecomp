# GraphRoomObj__func_80058390 -- MATCHED (29/29)

> Renamed from `func_80058390` on 2026-09-24 (tools/rename.py). Address 0x80058390.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x044`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
s32 GraphRoomObj__func_80058390(D_80087AACObj *self, void *arg1, void *arg2);
```

## Body

```c
s32 GraphRoomObj__func_80058390(D_80087AACObj *self, void *arg1, void *arg2) {
    s32 result;
    Get_vtable_TaskCore()->slot44(self, arg1, arg2);
    result = 2;
    if (self->unk_0x238 == 0) {
        result = self->unk_0x38;
    }
    return result;
}
```

Calls the shared base-class table's own `+0x044` slot (`Get_vtable_TaskCore()`,
this unit's own local view -- a plain no-argument getter established
elsewhere, e.g. `include/code_2c054.h`), then returns `self->unk_0x38` if
`self->unk_0x238 == 0`, else the literal `2`.

## Shape note

`s32 result = 2;` declared BEFORE the `Get_vtable_TaskCore()->slot44(...)`
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
tools/funcdiff.py GraphRoomObj__func_80058390   # 29/29
```
