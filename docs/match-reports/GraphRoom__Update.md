# GraphRoom__Update -- MATCHED (57/57)

> Renamed from `GraphRoomObj__UpdateFromLog` on 2026-09-26 (tools/rename.py). Address 0x800580e0.

> Renamed from `func_800580E0` on 2026-09-24 (tools/rename.py). Address 0x800580e0.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x05C`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
void GraphRoom__Update(D_80087AACObj *self, void *arg1, void *arg2);
```

## Body

```c
void GraphRoom__Update(D_80087AACObj *self, void *arg1, void *arg2) {
    Get_vtable_TaskCore()->slot5C(self, arg1, arg2);
    if (self->unk_0x3C == 1) {
        D_80087AACUnkA4Result *result = self->unk_0xA4->methods->slot1B0(self->unk_0xA4, 0);
        if (result->unk_0x4 != 0 || result->unk_0x8 != 0) {
            self->unk_0xA8[0]->methods->slot60(self->unk_0xA8[0], self->unk_0x1C & 1);
        }
    }
    self->methods->slot124(self);
}
```

Chains through the shared base class's `+0x05C` slot, and -- if
`self->unk_0x3C == 1` -- calls through `self->unk_0xA4`'s `+0x1B0` slot
(new return type `D_80087AACUnkA4Result`, only its two read fields
named) and, if either of those fields is nonzero, dispatches through
`self->unk_0xA8[0]` (the FIRST entry of the 100-entry array, `unk_0xA8`)
at its own `+0x060` slot. Always ends by calling `self->methods->slot124`
-- this unit's own `GraphRoom__TickHighlight` (already matched).

Matched on the first attempt with no residues.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoom__Update   # 57/57
```

## Naming (round 75, track 3)

**`GraphRoom__Update`** -- tier B. Own vtable slot +0x05C.
Fetches the day-log's current data (`dayLog->methods->getData`),
conditionally toggles the first graph point, then always calls the
class's own `tick` slot (`GraphRoom__TickHighlight`). Named for what it
does (pulls from the log, then drives the tick), not a confirmed in-game
trigger point (e.g. "on room enter" is plausible but not proven from the
body alone).

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, charlie)

points[] are BoxFills (include/BoxFill.h); the deleted `GraphRoomPoint` view's `slot60` is setDisplay: points[0] blinks with the hour's low bit. Zero bytes.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__UpdateFromLog` -> `GraphRoom__Update`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Named for its slot, track 4 step 6: +0x05C is `update` (TaskCore__Update in the parent), called with (sender, event). The +0x1B0 call is the DreamSys's DreamSys__GetSaveBlock, now through DreamSys.h's `vt`; its result is read as the unit's DreamSaveBlock record (was DayLog: fullScan/dayCount -> currentYear/currentDay). +0x03C is inputMode, +0x01C IntermediateBase's frameCounter (was elapsedHours), +0x124 the class's own slot tickHighlight (was `tick`).
