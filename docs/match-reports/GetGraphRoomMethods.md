# GetGraphRoomMethods -- MATCHED (4/4)

> Renamed from `func_80058764` on 2026-09-24 (tools/rename.py). Address 0x80058764.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods` -- plain no-argument
getter, `return &gGraphRoomMethods;`. Not itself a vtable slot; called by this
unit's own `New_GraphRoomObj` (already matched) and `GraphRoomObj__GraphRoomObj` (still
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
`GetClass6B5CCMethods`/`Get_vtable_TaskCore` precedent. `D_80087AAC` (the
table it returns) renamed to `gGraphRoomMethods` in the same pass
(`tools/rename.py`), following the `g<Class>Methods` convention already
used for `gTaskCoreMethods`/`gClass6B5CCMethods`.
