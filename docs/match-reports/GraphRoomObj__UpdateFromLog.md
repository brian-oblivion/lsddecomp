# GraphRoomObj__UpdateFromLog -- MATCHED (57/57)

> Renamed from `func_800580E0` on 2026-09-24 (tools/rename.py). Address 0x800580e0.

Unit: `src/class_3bb8c_t.c`. Class: `gGraphRoomMethods`, own vtable slot `+0x05C`
(resolved via `tools/classtable.py gGraphRoomMethods`).

## Signature

```c
void GraphRoomObj__UpdateFromLog(D_80087AACObj *self, void *arg1, void *arg2);
```

## Body

```c
void GraphRoomObj__UpdateFromLog(D_80087AACObj *self, void *arg1, void *arg2) {
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
-- this unit's own `GraphRoomObj__TickHighlight` (already matched).

Matched on the first attempt with no residues.

## Verify

```
./build-and-verify.sh   # build exit=0, OK: build matches retail
tools/funcdiff.py GraphRoomObj__UpdateFromLog   # 57/57
```

## Naming (round 75, track 3)

**`GraphRoomObj__UpdateFromLog`** -- tier B. Own vtable slot +0x05C.
Fetches the day-log's current data (`dayLog->methods->getData`),
conditionally toggles the first graph point, then always calls the
class's own `tick` slot (`GraphRoomObj__TickHighlight`). Named for what it
does (pulls from the log, then drives the tick), not a confirmed in-game
trigger point (e.g. "on room enter" is plausible but not proven from the
body alone).

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, charlie)

points[] are BoxFills (include/BoxFill.h); the deleted `GraphRoomPoint` view's `slot60` is setDisplay: points[0] blinks with the hour's low bit. Zero bytes.
