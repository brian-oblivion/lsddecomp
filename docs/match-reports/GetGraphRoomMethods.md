# GetGraphRoomMethods -- MATCHED (4/4)

> Renamed from `func_80058764` on 2026-09-24 (tools/rename.py). Address 0x80058764.

Unit: `src/world/dream_scene.c`. Class: `gGraphRoomMethods` -- plain no-argument
getter, `return &gGraphRoomMethods;`. Not itself a vtable slot; called by this
unit's own `New_GraphRoom` (already matched) and `GraphRoom__GraphRoom` (still
queued).

## Body

```c
extern D_80087AACMethods gGraphRoomMethods;

D_80087AACMethods *GetGraphRoomMethods(void) {
    return &gGraphRoomMethods;
}
```

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GetGraphRoomMethods   # 4/4
```

## Naming (round 75, track 3)

**`GetGraphRoomMethods`** -- tier A. Plain no-argument getter, `return
&gGraphRoomMethods;`, same shape as the already-established
`GetSceneNodeMethods`/`GetTaskCoreMethods` precedent. `D_80087AAC` (the
table it returns) renamed to `gGraphRoomMethods` in the same pass
(`tools/rename.py`), following the `g<Class>Methods` convention already
used for `gTaskCoreMethods`/`gSceneNodeMethods`.

## Track 4 (2026-09-26, round 87, alpha): GraphRoom unified

The class is unified in `include/graph_room.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). The getter is declared once, in include/graph_room.h, with the table `gGraphRoomMethods`; the unit's local `extern GraphRoomMethods gGraphRoomMethods` is deleted.
