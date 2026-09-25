# GraphRoomObj__GraphRoomObj -- MATCHED (44/44)

> Renamed from `func_80057FC8` on 2026-09-24 (tools/rename.py). Address 0x80057fc8.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x008`
-- THIS is `gGraphRoomMethods`'s own ctor (resolved via `tools/classtable.py
gGraphRoomMethods`), the callee of this unit's own `New_GraphRoomObj`'s `ctor(...)`
call.

## Signature

```c
void *GraphRoomObj__GraphRoomObj(D_80087AACObj *self, void *arg1);
```

## Body

```c
extern char D_8001176C[];

void *GraphRoomObj__GraphRoomObj(D_80087AACObj *self, void *arg1) {
    Get_vtable_TaskCore()->slot8(self, 0, D_8001176C, 0);
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
`class_3bb8c_p`'s `D800879C4__D800879C4`.

Uses `D_8001176C` (the `"ETC\ETCSE"` string, the other of this unit's two
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
tools/funcdiff.py GraphRoomObj__GraphRoomObj   # 44/44
```

## Naming (round 75, track 3)

**`GraphRoomObj__GraphRoomObj`** -- tier B. `Class__Class` ctor
convention; this IS `GraphRoomObj`'s own vtable slot +0x008
(`tools/classtable.py gGraphRoomMethods`). Class identity: see
`src/class_3bb8c_t.c`'s header comment.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
