# GraphRoom__ReleaseGraphPoints -- MATCHED (34/34)

> Renamed from `GraphRoomObj__Destroy` on 2026-09-26 (tools/rename.py). Address 0x80058308.

> Renamed from `func_80058308` on 2026-09-24 (tools/rename.py). Address 0x80058308.

Unit: `src/world/dream_scene.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x0DC`
(resolved via `tools/classtable.py gGraphRoomMethods`) -- this class's dtor.

## Signature

```c
void GraphRoom__ReleaseGraphPoints(D_80087AACObj *self);
```

## Body

```c
extern void BMemPMgrFree(void *arg);

void GraphRoom__ReleaseGraphPoints(D_80087AACObj *self) {
    s32 i;

    BMemPMgrFree(self->unk_0x240);
    for (i = 0; i < 100; i++) {
        self->unk_0xA8[i]->methods->slot4(self->unk_0xA8[i]);
    }
    GetTaskCoreMethods()->slotDC(self);
}
```

Frees `self->unk_0x240` (`BMemPMgrFree`, the companion of the
`BMemPMgrAlloc` allocator, declared locally per convention), destructs
each of the 100 `unk_0xA8` entries through their own `+0x004` slot, then
chains to the shared base class's own dtor (`+0x0DC`).

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoom__ReleaseGraphPoints   # 34/34
```

## Naming (round 75, track 3)

**`GraphRoom__ReleaseGraphPoints`** -- tier A. Own vtable slot +0x0DC, confirmed
via `tools/classtable.py gGraphRoomMethods` as the class's dtor slot, and
its body's final act is chaining to the base class's own dtor
(`GetTaskCoreMethods()->slotDC(self)`) after tearing down every `points`
entry -- the standard dtor shape.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, charlie)

points[] are BoxFills (include/box_fill.h); the deleted `GraphRoomPoint` view's `destroy` (+0x004) is release. Zero bytes.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__Destroy` -> `GraphRoom__ReleaseGraphPoints`

The class is unified in `include/graph_room.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Occupies +0x0DC, TaskCore's `releaseTarget`. Not named for the slot (step 6): the body frees what BuildGraphPoints (the +0x0D8 override) made -- matchedDayIndices and the 100 dots -- and then calls TaskCore's releaseTarget. "Destroy" read as a destructor; the class's finalize is TaskCore__Finalize, which calls releaseTarget.

## Track 7 (2026-09-27, round 97, delta)

The loop bound is `ARRAY_COUNT(self->points)` (100). Zero bytes changed.
