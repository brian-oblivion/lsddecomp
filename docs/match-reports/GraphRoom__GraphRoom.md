# GraphRoom__GraphRoom -- MATCHED (44/44)

> Renamed from `GraphRoomObj__GraphRoomObj` on 2026-09-26 (tools/rename.py). Address 0x80057fc8.

> Renamed from `func_80057FC8` on 2026-09-24 (tools/rename.py). Address 0x80057fc8.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x008`
-- THIS is `gGraphRoomMethods`'s own ctor (resolved via `tools/classtable.py
gGraphRoomMethods`), the callee of this unit's own `New_GraphRoom`'s `ctor(...)`
call.

## Signature

```c
void *GraphRoom__GraphRoom(D_80087AACObj *self, void *arg1);
```

## Body

```c
extern char sGraphSoundBankPath[];

void *GraphRoom__GraphRoom(D_80087AACObj *self, void *arg1) {
    Get_vtable_TaskCore()->slot8(self, 0, sGraphSoundBankPath, 0);
    self->methods = GetGraphRoomMethods();
    self->unk_0x48->methods->slot9C(self->unk_0x48, -1);
    self->unk_0xA4 = arg1;
    self->methods->slotD8(self, 0);
    return self->methods->slot40(self, arg1);
}
```

Chains through the shared base class (`Get_vtable_TaskCore()`, this unit's own
local view, extended with `+0x008`), then sets its own vtable
(`self->methods = GetGraphRoomMethods()`, `&gGraphRoomMethods` -- the standard ctor
"set my own vtable" step), dispatches through `self->unk_0x48` (a new
opaque object type, `D_80087AACUnk48Obj`), stashes `arg1` into
`self->unk_0xA4`, and finally TAIL-CALLS its own class's `+0x040` slot
(`slot40`), forwarding its return value -- the exact same "ctor ends by
calling another of its own class's slots" shape already seen in
`class_3bb8c_p`'s `VariantSprite__VariantSprite`.

Uses `sGraphSoundBankPath` (the `"ETC\ETCSE"` string, the other of this unit's two
standalone strings named in its own header comment).

## Shape note: do not cache `self->unk_0x48`

```c
D_80087AACUnk48Obj *unk48 = self->unk_0x48;
...
unk48->methods->slot9C(unk48, -1);
```
scored 3/44 with the frame one word too long -- caching the field across
the intervening `GetGraphRoomMethods()` call forces a spurious callee-saved
register, same family as the existing "do not cache a `this->field`
across an intervening vtable call" learning. Re-reading `self->unk_0x48`
directly at both use sites matched exactly.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoom__GraphRoom   # 44/44
```

## Naming (round 75, track 3)

**`GraphRoom__GraphRoom`** -- tier B. `Class__Class` ctor
convention; this IS `GraphRoomObj`'s own vtable slot +0x008
(`tools/classtable.py gGraphRoomMethods`). Class identity: see
`src/class_3bb8c_t.c`'s header comment.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__GraphRoomObj` -> `GraphRoom__GraphRoom`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Prefix only. The ctor now returns void: INTERMEDIATEBASE_SLOTS fixes the ctor slot's type, and the tail call it used to return the value of is void resetCounters (+0x040, GraphRoom__Reset); the bytes are the same either way. That call passes `dreamSys` as a second argument (retail loads `$a1` from `$s1`) which the slot does not have, so it casts to `GraphRoomResetCallFn` (no code). The old `unk48->slot9C(-1)` is TaskCore's `sound` (a VabStreamObj) at +0x09C, VabStreamObj__SetPitchOffset (`classtable.py gVabStreamObjMethods`); `slotD8(self, 0)` is `setTarget(self, NULL)`, which this class overrides with BuildGraphPoints. The argument is the DreamSys (see New_GraphRoom), kept as `dreamSys` (+0x0A4, was `dayLog`).

## Track 7 (2026-09-27, round 97, delta)

- **Naming: `D_8001176C` -> `sGraphSoundBankPath`** (tier A): the rodata string `"ETC\ETCSE"`, passed as TaskCore's ctor's `soundBankPath` (include/TaskCore.h). This ctor is its only user; `s` for data only this unit reads, like `sTitleTimPath`. TitleMenu's ctor passes its own copy of the same string (`D_800114DC`, class_3bb8c.h).
- The pointer zeros (TaskCore ctor's `target` and `sound`, `setTarget`'s target) are written `NULL`. Zero bytes changed.
