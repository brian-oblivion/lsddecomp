# GraphRoomObj__Destroy -- MATCHED (34/34)

> Renamed from `func_80058308` on 2026-09-24 (tools/rename.py). Address 0x80058308.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x0DC`
(resolved via `tools/classtable.py gGraphRoomMethods`) -- this class's dtor.

## Signature

```c
void GraphRoomObj__Destroy(D_80087AACObj *self);
```

## Body

```c
extern void BMemPMgrFree(void *arg);

void GraphRoomObj__Destroy(D_80087AACObj *self) {
    s32 i;

    BMemPMgrFree(self->unk_0x240);
    for (i = 0; i < 100; i++) {
        self->unk_0xA8[i]->methods->slot4(self->unk_0xA8[i]);
    }
    Get_vtable_TaskCore()->slotDC(self);
}
```

Frees `self->unk_0x240` (`BMemPMgrFree`, the companion of the
`BMemPMgrAlloc` allocator, declared locally per convention), destructs
each of the 100 `unk_0xA8` entries through their own `+0x004` slot, then
chains to the shared base class's own dtor (`+0x0DC`).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoomObj__Destroy   # 34/34
```

## Naming (round 75, track 3)

**`GraphRoomObj__Destroy`** -- tier A. Own vtable slot +0x0DC, confirmed
via `tools/classtable.py gGraphRoomMethods` as the class's dtor slot, and
its body's final act is chaining to the base class's own dtor
(`Get_vtable_TaskCore()->slotDC(self)`) after tearing down every `points`
entry -- the standard dtor shape.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, charlie)

points[] are BoxFills (include/BoxFill.h); the deleted `GraphRoomPoint` view's `destroy` (+0x004) is release. Zero bytes.
